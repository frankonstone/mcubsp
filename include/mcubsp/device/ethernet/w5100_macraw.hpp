#pragma once

// @req-file REQ-0002 REQ-0009 REQ-0015 REQ-0017 REQ-0019 REQ-0035 REQ-0044 REQ-0046
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcubsp/device/ethernet/w5100_types.hpp>

namespace mcubsp::device {

// Fixed-slot raw Ethernet device over W5100 socket 0 MACRAW mode. Bus provides
// expected<uint8_t, w5100_error> read(uint16_t) and
// expected<void, w5100_error> write(uint16_t, uint8_t).
template<typename Bus, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
class w5100_macraw_device {
public:
    static_assert(RxDescriptors != 0U && TxDescriptors != 0U);

    static constexpr std::size_t rx_descriptor_count = RxDescriptors;
    static constexpr std::size_t tx_descriptor_count = TxDescriptors;
    static constexpr std::size_t buffer_size = 1536U;
    static constexpr std::size_t ethernet_header_size = 14U;

    explicit w5100_macraw_device(Bus& bus) noexcept : bus_(bus) {}

    [[nodiscard]] std::expected<void, w5100_error>
    initialize(w5100_configuration configuration) noexcept {
        if (!valid(configuration)) {
            return fail(w5100_error::invalid_argument);
        }
        cancel_all();
        configuration_ = configuration;
        initialized_ = false;

        auto result = write(w5100_register::mode, reset_bit);
        if (!result) return fail(result.error());
        bool reset_complete{};
        for (std::uint16_t poll = 0U;
             poll < configuration_.command_poll_limit; ++poll) {
            const auto value = read(w5100_register::mode);
            if (!value) return fail(value.error());
            if ((*value & reset_bit) == 0U) {
                reset_complete = true;
                break;
            }
        }
        if (!reset_complete) return fail(w5100_error::reset_failed);

        // The W5100 has no version register. Reset values of both memory-size
        // registers, followed by a read-back of SHAR, form the non-destructive
        // identity check for this original-W5100 protocol.
        const auto rx_default = read(w5100_register::rx_memory_size);
        const auto tx_default = read(w5100_register::tx_memory_size);
        if (!rx_default || !tx_default) {
            return fail(!rx_default ? rx_default.error() : tx_default.error());
        }
        if (*rx_default != default_memory_size
            || *tx_default != default_memory_size) {
            return fail(w5100_error::identity_mismatch);
        }

        for (std::size_t index = 0U;
             index < configuration_.mac.octets.size(); ++index) {
            result = write(static_cast<std::uint16_t>(
                w5100_register::source_hardware_address + index),
                configuration_.mac.octets[index]);
            if (!result) return fail(result.error());
        }
        for (std::size_t index = 0U;
             index < configuration_.mac.octets.size(); ++index) {
            const auto value = read(static_cast<std::uint16_t>(
                w5100_register::source_hardware_address + index));
            if (!value) return fail(value.error());
            if (*value != configuration_.mac.octets[index]) {
                return fail(w5100_error::identity_mismatch);
            }
        }

        // 0x03 gives socket 0 the complete 8 KiB RX and TX memories. Per the
        // datasheet, later sockets receive no memory after the 8 KiB limit.
        if (!(result = write(w5100_register::rx_memory_size,
                             socket0_entire_memory))
            || !(result = write(w5100_register::tx_memory_size,
                                socket0_entire_memory))) {
            return fail(result.error());
        }
        const auto opened = open_macraw();
        if (!opened) return fail(opened.error());
        initialized_ = true;
        needs_recovery_ = false;
        last_error_.reset();
        return {};
    }

    [[nodiscard]] bool arm_receive(std::size_t index,
                                   std::span<std::byte> buffer) noexcept {
        if (!initialized_ || index >= RxDescriptors
            || buffer.size() < buffer_size || rx_active_[index]) {
            return false;
        }
        rx_buffers_[index] = buffer.first(buffer_size);
        rx_completions_[index].reset();
        rx_active_[index] = true;
        return true;
    }

    [[nodiscard]] bool start_transmit(
        std::size_t index, std::span<const std::byte> frame) noexcept {
        if (!initialized_ || needs_recovery_ || index >= TxDescriptors || tx_active_[index]
            || frame.size() < ethernet_header_size
            || frame.size() > buffer_size) {
            return false;
        }
        tx_frames_[index] = frame;
        tx_completions_[index].reset();
        tx_active_[index] = true;
        return true;
    }

    void poll() noexcept {
        if (initialized_) static_cast<void>(poll_once());
    }

    [[nodiscard]] std::expected<void, w5100_error> poll_once() noexcept {
        if (!initialized_) return std::unexpected(w5100_error::socket_state);
        if (needs_recovery_) return std::unexpected(*last_error_);
        auto budget = static_cast<std::size_t>(configuration_.payload_bytes_per_poll);
        if (!poll_transmit_completion()) return std::unexpected(*last_error_);
        // Alternate direction priority so continuous reception cannot starve TX.
        transmit_first_ = !transmit_first_;
        if (transmit_first_) {
            if (!start_queued_transmit(budget) || !poll_receive(budget)) {
                return std::unexpected(*last_error_);
            }
        } else if (!poll_receive(budget) || !start_queued_transmit(budget)) {
            return std::unexpected(*last_error_);
        }
        return {};
    }

    [[nodiscard]] std::optional<w5100_receive_completion>
    take_receive(std::size_t index) noexcept {
        if (index >= RxDescriptors) return std::nullopt;
        auto result = rx_completions_[index];
        rx_completions_[index].reset();
        return result;
    }

    [[nodiscard]] std::optional<w5100_transmit_completion>
    take_transmit_result(std::size_t index) noexcept {
        if (index >= TxDescriptors) return std::nullopt;
        auto result = tx_completions_[index];
        tx_completions_[index].reset();
        return result;
    }

    [[nodiscard]] bool take_transmit(std::size_t index) noexcept {
        const auto result = take_transmit_result(index);
        return result.has_value() && !result->error;
    }

    void cancel_all() noexcept {
        // SEND continues inside the chip after host buffers are released.
        // Reopen the socket before another SEND so a late SEND_OK cannot be
        // attributed to a new descriptor. Partial, uncommitted copies are safe
        // to discard without closing the socket.
        if (tx_in_flight_) {
            needs_recovery_ = true;
            last_error_ = w5100_error::cancelled;
        }
        bool cancelled{};
        for (std::size_t index = 0U; index < RxDescriptors; ++index) {
            cancelled = cancelled || rx_active_[index];
            rx_active_[index] = false;
            rx_buffers_[index] = {};
            rx_completions_[index].reset();
        }
        for (std::size_t index = 0U; index < TxDescriptors; ++index) {
            cancelled = cancelled || tx_active_[index];
            tx_active_[index] = false;
            tx_frames_[index] = {};
            tx_completions_[index].reset();
        }
        tx_in_flight_.reset();
        rx_copy_.reset();
        tx_copy_.reset();
        if (cancelled) ++metrics_.cancellations;
    }

    [[nodiscard]] std::expected<void, w5100_error> recover() noexcept {
        if (!initialized_) return std::unexpected(w5100_error::socket_state);
        needs_recovery_ = true;
        complete_active_transmit_with_error();
        if (rx_copy_) {
            const auto slot = rx_copy_->slot;
            rx_completions_[slot] = w5100_receive_completion{0U, true};
            rx_active_[slot] = false;
            rx_buffers_[slot] = {};
            rx_copy_.reset();
        }
        if (tx_copy_) {
            const auto slot = tx_copy_->slot;
            tx_completions_[slot] = w5100_transmit_completion{true};
            tx_active_[slot] = false;
            tx_frames_[slot] = {};
            tx_copy_.reset();
        }
        const auto opened = open_macraw();
        if (!opened) return fail(opened.error());
        ++metrics_.recoveries;
        needs_recovery_ = false;
        last_error_.reset();
        return {};
    }

    [[nodiscard]] bool initialized() const noexcept { return initialized_; }
    [[nodiscard]] const w5100_metrics& metrics() const noexcept {
        return metrics_;
    }
    [[nodiscard]] std::optional<w5100_error> last_error() const noexcept {
        return last_error_;
    }

private:
    [[nodiscard]] static constexpr bool
    valid(const w5100_configuration& configuration) noexcept {
        bool any{};
        bool all_ff{true};
        for (const auto octet : configuration.mac.octets) {
            any = any || octet != 0U;
            all_ff = all_ff && octet == 0xFFU;
        }
        return configuration.command_poll_limit != 0U
            && configuration.stable_read_limit >= 2U
            && configuration.payload_bytes_per_poll != 0U
            && configuration.payload_bytes_per_poll <= buffer_size
            && any && !all_ff
            && (configuration.mac.octets[0] & 1U) == 0U;
    }

    [[nodiscard]] std::expected<void, w5100_error> open_macraw() noexcept {
        auto closed = command(close_command);
        if (!closed) return closed;
        auto result = write(w5100_register::socket0_interrupt,
                            all_socket_interrupts);
        if (!result) return result;
        result = write(w5100_register::socket0_mode, macraw_mode);
        if (!result) return result;
        result = command(open_command);
        if (!result) return result;
        const auto status = read(w5100_register::socket0_status);
        if (!status) return std::unexpected(status.error());
        if (*status != macraw_status) {
            return std::unexpected(w5100_error::socket_state);
        }
        return {};
    }

    [[nodiscard]] std::expected<void, w5100_error>
    command(std::uint8_t value) noexcept {
        auto result = write(w5100_register::socket0_command, value);
        if (!result) return result;
        for (std::uint16_t poll = 0U;
             poll < configuration_.command_poll_limit; ++poll) {
            const auto current = read(w5100_register::socket0_command);
            if (!current) return std::unexpected(current.error());
            if (*current == 0U) return {};
        }
        return std::unexpected(w5100_error::command_timeout);
    }

    [[nodiscard]] std::expected<std::uint16_t, w5100_error>
    read_stable_u16(std::uint16_t address) noexcept {
        std::optional<std::uint16_t> previous;
        for (std::uint8_t attempt = 0U;
             attempt < configuration_.stable_read_limit; ++attempt) {
            const auto high = read(address);
            const auto low = read(static_cast<std::uint16_t>(address + 1U));
            if (!high || !low) {
                return std::unexpected(!high ? high.error() : low.error());
            }
            const auto value = static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(*high) << 8U) | *low);
            if (previous && *previous == value) return value;
            previous = value;
        }
        return std::unexpected(w5100_error::unstable_register);
    }

    [[nodiscard]] std::expected<void, w5100_error>
    write_u16(std::uint16_t address, std::uint16_t value) noexcept {
        auto result = write(address, static_cast<std::uint8_t>(value >> 8U));
        if (!result) return result;
        return write(static_cast<std::uint16_t>(address + 1U),
                     static_cast<std::uint8_t>(value));
    }

    [[nodiscard]] bool poll_transmit_completion() noexcept {
        if (!tx_in_flight_) return true;
        const auto interrupts = read(w5100_register::socket0_interrupt);
        if (!interrupts) return record_poll_error(interrupts.error());
        if ((*interrupts & timeout_interrupt) != 0U) {
            const auto cleared = write(w5100_register::socket0_interrupt,
                                       timeout_interrupt);
            if (!cleared) return record_commit_error(cleared.error());
            const auto slot = *tx_in_flight_;
            tx_completions_[slot] = w5100_transmit_completion{.error = true};
            tx_active_[slot] = false;
            tx_frames_[slot] = {};
            tx_in_flight_.reset();
            ++metrics_.timeouts;
            last_error_ = w5100_error::timeout;
            return true;
        }
        if ((*interrupts & send_ok_interrupt) != 0U) {
            const auto cleared = write(w5100_register::socket0_interrupt,
                                       send_ok_interrupt);
            if (!cleared) return record_commit_error(cleared.error());
            const auto slot = *tx_in_flight_;
            tx_completions_[slot] = w5100_transmit_completion{};
            tx_active_[slot] = false;
            tx_frames_[slot] = {};
            tx_in_flight_.reset();
            ++metrics_.transmitted_frames;
        }
        return true;
    }

    [[nodiscard]] bool poll_receive(std::size_t& budget) noexcept {
        if (budget == 0U) return true;
        if (!rx_copy_) {
            const auto available = read_stable_u16(
                w5100_register::socket0_rx_received_size);
            if (!available) return record_poll_error(available.error());
            if (*available == 0U) return true;

            const auto slot = first_armed_receive();
            if (!slot) {
                ++metrics_.receive_backpressure;
                return true;
            }
            const auto pointer = read_stable_u16(
                w5100_register::socket0_rx_read_pointer);
            if (!pointer) return record_poll_error(pointer.error());
            const auto header = read_ring_header(*pointer);
            if (!header) return record_poll_error(header.error());
            const auto stored_size = *header;
            if (stored_size < 2U || stored_size > *available
                || stored_size > socket_memory_size) {
                complete_receive_error(*slot, w5100_error::malformed_frame);
                return recover_after_malformed();
            }
            const auto frame_size = static_cast<std::size_t>(stored_size - 2U);
            if (frame_size < ethernet_header_size
                || frame_size > buffer_size
                || frame_size > rx_buffers_[*slot].size()) {
                const auto error = frame_size > buffer_size
                    ? w5100_error::frame_too_large
                    : w5100_error::malformed_frame;
                complete_receive_error(*slot, error);
                const auto consumed = consume_receive(*pointer, stored_size);
                if (!consumed) return record_commit_error(consumed.error());
                return true;
            }
            rx_copy_ = packet_copy{*slot, *pointer, frame_size, 0U};
        }
        auto& copy = *rx_copy_;
        while (budget != 0U && copy.offset != copy.size) {
            const auto copied = read_ring(
                static_cast<std::uint16_t>(copy.pointer + 2U + copy.offset),
                rx_buffers_[copy.slot].subspan(copy.offset, 1U));
            if (!copied) return record_poll_error(copied.error());
            ++copy.offset;
            --budget;
        }
        if (copy.offset != copy.size) return true;

        const auto consumed = consume_receive(copy.pointer,
            static_cast<std::uint16_t>(copy.size + 2U));
        if (!consumed) return record_commit_error(consumed.error());
        rx_completions_[copy.slot] = w5100_receive_completion{copy.size, false};
        rx_active_[copy.slot] = false;
        rx_buffers_[copy.slot] = {};
        rx_copy_.reset();
        ++metrics_.received_frames;
        return true;
    }

    [[nodiscard]] bool start_queued_transmit(std::size_t& budget) noexcept {
        if (tx_in_flight_ || budget == 0U) return true;
        if (!tx_copy_) {
            const auto slot = first_queued_transmit();
            if (!slot) return true;
            const auto free = read_stable_u16(
                w5100_register::socket0_tx_free_size);
            if (!free) return record_poll_error(free.error());
            if (*free < tx_frames_[*slot].size()) return true;
            const auto pointer = read_stable_u16(
                w5100_register::socket0_tx_write_pointer);
            if (!pointer) return record_poll_error(pointer.error());
            const auto cleared = write(w5100_register::socket0_interrupt,
                                       send_ok_interrupt | timeout_interrupt);
            if (!cleared) return record_poll_error(cleared.error());
            tx_copy_ = packet_copy{*slot, *pointer, tx_frames_[*slot].size(), 0U};
        }
        auto& copy = *tx_copy_;
        while (budget != 0U && copy.offset != copy.size) {
            const auto copied = write_ring(
                static_cast<std::uint16_t>(copy.pointer + copy.offset),
                tx_frames_[copy.slot].subspan(copy.offset, 1U));
            if (!copied) return record_poll_error(copied.error());
            ++copy.offset;
            --budget;
        }
        if (copy.offset != copy.size) return true;

        const auto advanced = write_u16(
            w5100_register::socket0_tx_write_pointer,
            static_cast<std::uint16_t>(copy.pointer + copy.size));
        if (!advanced) return record_commit_error(advanced.error());
        const auto sent = command(send_command);
        if (!sent) return record_commit_error(sent.error());
        tx_in_flight_ = copy.slot;
        tx_copy_.reset();
        return true;
    }

    [[nodiscard]] std::expected<std::uint16_t, w5100_error>
    read_ring_header(std::uint16_t pointer) noexcept {
        std::array<std::byte, 2U> header{};
        const auto result = read_ring(pointer, header);
        if (!result) return std::unexpected(result.error());
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(header[0]))
             << 8U)
            | std::to_integer<std::uint8_t>(header[1]));
    }

    [[nodiscard]] std::expected<void, w5100_error>
    read_ring(std::uint16_t pointer, std::span<std::byte> destination) noexcept {
        for (std::size_t index = 0U; index < destination.size(); ++index) {
            const auto address = static_cast<std::uint16_t>(
                w5100_register::socket0_rx_base
                + ((static_cast<std::size_t>(pointer) + index)
                   & socket_memory_mask));
            const auto value = read(address);
            if (!value) return std::unexpected(value.error());
            destination[index] = static_cast<std::byte>(*value);
        }
        return {};
    }

    [[nodiscard]] std::expected<void, w5100_error>
    write_ring(std::uint16_t pointer,
               std::span<const std::byte> source) noexcept {
        for (std::size_t index = 0U; index < source.size(); ++index) {
            const auto address = static_cast<std::uint16_t>(
                w5100_register::socket0_tx_base
                + ((static_cast<std::size_t>(pointer) + index)
                   & socket_memory_mask));
            const auto result = write(
                address, std::to_integer<std::uint8_t>(source[index]));
            if (!result) return result;
        }
        return {};
    }

    [[nodiscard]] std::expected<void, w5100_error>
    consume_receive(std::uint16_t pointer, std::uint16_t size) noexcept {
        auto result = write_u16(w5100_register::socket0_rx_read_pointer,
                                static_cast<std::uint16_t>(pointer + size));
        if (!result) return result;
        return command(receive_command);
    }

    [[nodiscard]] std::optional<std::size_t>
    first_armed_receive() const noexcept {
        for (std::size_t index = 0U; index < RxDescriptors; ++index) {
            if (rx_active_[index]) return index;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::size_t>
    first_queued_transmit() const noexcept {
        for (std::size_t index = 0U; index < TxDescriptors; ++index) {
            if (tx_active_[index] && (!tx_in_flight_ || *tx_in_flight_ != index)) {
                return index;
            }
        }
        return std::nullopt;
    }

    void complete_receive_error(std::size_t slot, w5100_error error) noexcept {
        rx_completions_[slot] = w5100_receive_completion{0U, true};
        rx_active_[slot] = false;
        rx_buffers_[slot] = {};
        if (error == w5100_error::frame_too_large) {
            ++metrics_.oversize_frames;
        } else {
            ++metrics_.malformed_frames;
        }
        last_error_ = error;
    }

    [[nodiscard]] bool recover_after_malformed() noexcept {
        const auto recovered = recover();
        return recovered.has_value();
    }

    void complete_active_transmit_with_error() noexcept {
        if (!tx_in_flight_) return;
        const auto slot = *tx_in_flight_;
        tx_completions_[slot] = w5100_transmit_completion{.error = true};
        tx_active_[slot] = false;
        tx_frames_[slot] = {};
        tx_in_flight_.reset();
    }

    // A failure while committing a hardware pointer or command may have
    // reached the chip. Do not repeat that side effect on the next poll.
    // recover() closes and reopens the socket before traffic can resume.
    [[nodiscard]] bool record_commit_error(w5100_error error) noexcept {
        needs_recovery_ = true;
        return record_poll_error(error);
    }

    [[nodiscard]] bool record_poll_error(w5100_error error) noexcept {
        static_cast<void>(fail(error));
        return false;
    }

    template<typename T = void>
    [[nodiscard]] std::expected<T, w5100_error>
    fail(w5100_error error) noexcept {
        last_error_ = error;
        if (error == w5100_error::transport) ++metrics_.transport_errors;
        if (error == w5100_error::timeout
            || error == w5100_error::command_timeout) {
            ++metrics_.timeouts;
        }
        return std::unexpected(error);
    }

    [[nodiscard]] std::expected<std::uint8_t, w5100_error>
    read(std::uint16_t address) noexcept {
        return bus_.read(address);
    }

    [[nodiscard]] std::expected<void, w5100_error>
    write(std::uint16_t address, std::uint8_t value) noexcept {
        return bus_.write(address, value);
    }

    static constexpr std::uint8_t reset_bit = 0x80U;
    static constexpr std::uint8_t default_memory_size = 0x55U;
    static constexpr std::uint8_t socket0_entire_memory = 0x03U;
    static constexpr std::uint8_t macraw_mode = 0x04U;
    static constexpr std::uint8_t macraw_status = 0x42U;
    static constexpr std::uint8_t open_command = 0x01U;
    static constexpr std::uint8_t close_command = 0x10U;
    static constexpr std::uint8_t send_command = 0x20U;
    static constexpr std::uint8_t receive_command = 0x40U;
    static constexpr std::uint8_t send_ok_interrupt = 0x10U;
    static constexpr std::uint8_t timeout_interrupt = 0x08U;
    static constexpr std::uint8_t all_socket_interrupts = 0x1FU;
    static constexpr std::size_t socket_memory_size = 8192U;
    static constexpr std::size_t socket_memory_mask = socket_memory_size - 1U;

    struct packet_copy {
        std::size_t slot;
        std::uint16_t pointer;
        std::size_t size;
        std::size_t offset;
    };

    Bus& bus_;
    w5100_configuration configuration_{};
    std::array<std::span<std::byte>, RxDescriptors> rx_buffers_{};
    std::array<std::optional<w5100_receive_completion>, RxDescriptors>
        rx_completions_{};
    std::array<bool, RxDescriptors> rx_active_{};
    std::array<std::span<const std::byte>, TxDescriptors> tx_frames_{};
    std::array<std::optional<w5100_transmit_completion>, TxDescriptors>
        tx_completions_{};
    std::array<bool, TxDescriptors> tx_active_{};
    std::optional<std::size_t> tx_in_flight_{};
    std::optional<packet_copy> rx_copy_{};
    std::optional<packet_copy> tx_copy_{};
    std::optional<w5100_error> last_error_{};
    w5100_metrics metrics_{};
    bool initialized_{};
    bool transmit_first_{};
    bool needs_recovery_{};
};

} // namespace mcubsp::device
