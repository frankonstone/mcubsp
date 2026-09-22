#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/dma.hpp>
#include <mcuhw/driver/spi.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

namespace mcuhw::backend::ra4m1 {

namespace spi_detail {

template<std::uint8_t Instance>
struct owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::signal miso_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_miso
                          : mcu::ra4m1::signal::spi1_miso;
}

[[nodiscard]] constexpr mcu::ra4m1::signal mosi_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_mosi
                          : mcu::ra4m1::signal::spi1_mosi;
}

[[nodiscard]] constexpr mcu::ra4m1::signal clock_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_rspck
                          : mcu::ra4m1::signal::spi1_rspck;
}

[[nodiscard]] constexpr mcu::ra4m1::signal select_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_ssl0
                          : mcu::ra4m1::signal::spi1_ssl0;
}

[[nodiscard]] constexpr mcu::ra4m1::instance_record metadata(
    std::uint8_t instance) noexcept {
    for (const auto& candidate : mcu::ra4m1::instances) {
        if (candidate.id.kind
                == static_cast<std::uint16_t>(
                    mcu::ra4m1::peripheral_kind::spi)
            && candidate.id.instance == instance) {
            return candidate;
        }
    }
    return {};
}

} // namespace spi_detail

template<std::uint8_t Instance, core::pin_id MisoPin, core::pin_id MosiPin,
         core::pin_id ClockPin, core::pin_id SelectPin>
struct spi_binding {
    static_assert(Instance < 2U);
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, MisoPin,
        mcu::ra4m1::signal_id(spi_detail::miso_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, MosiPin,
        mcu::ra4m1::signal_id(spi_detail::mosi_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, ClockPin,
        mcu::ra4m1::signal_id(spi_detail::clock_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, SelectPin,
        mcu::ra4m1::signal_id(spi_detail::select_signal(Instance))));

    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id miso_pin = MisoPin;
    static constexpr core::pin_id mosi_pin = MosiPin;
    static constexpr core::pin_id clock_pin = ClockPin;
    static constexpr core::pin_id select_pin = SelectPin;
    static constexpr auto metadata = spi_detail::metadata(Instance);
};

// The hardware SSL pin is deliberately driven as GPIO. That makes one
// transaction the ownership boundary and keeps CS asserted across arbitrary
// caller-declared segments. SPII is the only completion point, so the final
// frame has left the wire before CS is released.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0014 REQ-0017 REQ-0019 REQ-0028 REQ-0030
class spi_master_backend {
public:
    using pool_type = core::operation_pool<driver::spi_transfer_result,
                                           driver::spi_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit spi_master_backend(MmioPolicy& mmio,
                                ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::spi_error>
    initialize(driver::spi_timing timing,
               driver::spi_configuration configuration = {}) noexcept {
        if (initialized_ || spi_detail::owner<Binding::instance>::claimed) {
            return std::unexpected(driver::spi_error::already_owned);
        }
        if (!timing_is_valid(timing)) {
            return std::unexpected(driver::spi_error::invalid_rate);
        }
        spi_detail::owner<Binding::instance>::claimed = true;
        owned_ = true;
        timing_ = timing;
        configuration_ = configuration;
        if (!release_module_stop() || !configure_pins()
            || !program_controller()) {
            static_cast<void>(restore_pins());
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result
    transaction(std::span<const driver::spi_segment> segments) noexcept {
        if (!initialized_) {
            return start_failure(driver::spi_error::not_initialized);
        }
        if (operation_id_ || immediate_active_) {
            return start_failure(driver::spi_error::busy);
        }
        const auto validation = validate_segments(segments);
        if (!validation) {
            return start_failure(validation.error());
        }
        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        segments_ = segments;
        total_frames_ = *validation;
        transmitted_frames_ = 0U;
        received_frames_ = 0U;

        if (!clear_errors() || !set_chip_select(true)
            || !write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            || !write<std::uint8_t>(base() + 0x00U, spcr_active)) {
            static_cast<void>(finish_error(driver::spi_error::mmio));
        }
        return started;
    }

    // A bounded polling path for devices such as the original W5100 that use
    // thousands of independently chip-selected four-byte commands. It keeps
    // the same controller ownership and CS-idle guarantees as transaction(),
    // but avoids allocating and publishing an asynchronous operation per
    // command. The caller supplies an explicit poll bound for each hardware
    // status transition.
    // @req REQ-0019 REQ-0028 REQ-0044
    [[nodiscard]] std::expected<std::size_t, driver::spi_error>
    transaction_immediate(std::span<const std::uint8_t> transmit,
                          std::span<std::uint8_t> receive,
                          std::uint32_t poll_limit) noexcept {
        if (!initialized_) {
            return std::unexpected(driver::spi_error::not_initialized);
        }
        if (operation_id_ || immediate_active_) {
            return std::unexpected(driver::spi_error::busy);
        }
        if (configuration_.frame_width != driver::spi_frame_width::bits8) {
            return std::unexpected(driver::spi_error::invalid_frame);
        }
        if (transmit.empty()) {
            return std::unexpected(driver::spi_error::empty_transfer);
        }
        if (!receive.empty() && receive.size() != transmit.size()) {
            return std::unexpected(driver::spi_error::mismatched_segment);
        }
        if (transmit.size() > 65'535U) {
            return std::unexpected(driver::spi_error::excessive_length);
        }
        if (poll_limit == 0U) {
            return std::unexpected(driver::spi_error::timeout);
        }

        immediate_active_ = true;
        if (!clear_errors() || !set_chip_select(true)
            || !write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            || !write<std::uint8_t>(base() + 0x00U, spcr_polling)) {
            return finish_immediate_error(driver::spi_error::mmio);
        }
        for (std::size_t index = 0U; index < transmit.size(); ++index) {
            const auto transmit_ready = wait_for_status(
                status_transmit_empty, true, poll_limit);
            if (!transmit_ready) {
                return finish_immediate_error(transmit_ready.error());
            }
            if (!write<std::uint8_t>(base() + 0x04U, transmit[index])) {
                return finish_immediate_error(driver::spi_error::mmio);
            }
            const auto receive_ready = wait_for_status(
                status_receive_full, true, poll_limit);
            if (!receive_ready) {
                return finish_immediate_error(receive_ready.error());
            }
            const auto value = mmio_.template read<std::uint8_t>(
                base() + 0x04U);
            if (!value) {
                return finish_immediate_error(driver::spi_error::mmio);
            }
            if (!receive.empty()) receive[index] = *value;
        }
        const auto idle = wait_for_status(status_not_idle, false, poll_limit);
        if (!idle) return finish_immediate_error(idle.error());
        const bool stopped = stop_immediate();
        return stopped
            ? std::expected<std::size_t, driver::spi_error>{transmit.size()}
            : std::unexpected(driver::spi_error::mmio);
    }

    template<typename ReceiveDma, typename TransmitDma>
    [[nodiscard]] start_result
    transaction_dma(driver::spi_dma_buffers buffers,
                    ReceiveDma& receive_dma,
                    TransmitDma& transmit_dma) noexcept {
        if (!initialized_) {
            return start_failure(driver::spi_error::not_initialized);
        }
        if (operation_id_ || immediate_active_) {
            return start_failure(driver::spi_error::busy);
        }
        if (configuration_.frame_width != driver::spi_frame_width::bits8) {
            return start_failure(driver::spi_error::invalid_frame);
        }
        if (buffers.transmit.empty() || buffers.receive.empty()) {
            return start_failure(driver::spi_error::empty_transfer);
        }
        if (buffers.transmit.size() != buffers.receive.size()) {
            return start_failure(driver::spi_error::mismatched_segment);
        }
        if (buffers.transmit.size() > 65'535U) {
            return start_failure(driver::spi_error::excessive_length);
        }

        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        total_frames_ = buffers.transmit.size();
        dma_mode_ = true;
        if (!clear_errors() || !set_chip_select(true)) {
            static_cast<void>(finish_error(driver::spi_error::mmio));
            return started;
        }

        auto receive_started = receive_dma.peripheral({
            .memory = reinterpret_cast<std::uintptr_t>(buffers.receive.data()),
            .peripheral = base() + 0x04U,
            .count = static_cast<std::uint32_t>(buffers.receive.size()),
            .width = driver::dma_width::bits8,
            .event = receive_dma_event(),
            .direction = driver::dma_peripheral_direction::peripheral_to_memory,
        });
        if (!receive_started) {
            static_cast<void>(finish_error(driver::spi_error::dma));
            return started;
        }
        receive_dma_transfer_.emplace(std::move(*receive_started));

        auto transmit_started = transmit_dma.peripheral({
            .memory = reinterpret_cast<std::uintptr_t>(buffers.transmit.data()),
            .peripheral = base() + 0x04U,
            .count = static_cast<std::uint32_t>(buffers.transmit.size()),
            .width = driver::dma_width::bits8,
            .event = transmit_dma_event(),
            .direction = driver::dma_peripheral_direction::memory_to_peripheral,
        });
        if (!transmit_started) {
            static_cast<void>(finish_error(driver::spi_error::dma));
            return started;
        }
        transmit_dma_transfer_.emplace(std::move(*transmit_started));

        if (!write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            || !write<std::uint8_t>(base() + 0x00U, spcr_active)) {
            static_cast<void>(finish_error(driver::spi_error::mmio));
        }
        return started;
    }

    [[nodiscard]] bool handle_dma_receive_completion() noexcept {
        return handle_dma_completion(receive_dma_transfer_,
                                     receive_dma_complete_,
                                     received_frames_);
    }

    [[nodiscard]] bool handle_dma_transmit_completion() noexcept {
        return handle_dma_completion(transmit_dma_transfer_,
                                     transmit_dma_complete_,
                                     transmitted_frames_);
    }

    [[nodiscard]] bool handle_transmit_empty_interrupt() noexcept {
        if (!operation_id_ || dma_mode_
            || transmitted_frames_ >= total_frames_) {
            return false;
        }
        const auto value = transmit_frame(transmitted_frames_);
        const bool written = configuration_.frame_width
                == driver::spi_frame_width::bits8
            ? write<std::uint8_t>(base() + 0x04U,
                                  static_cast<std::uint8_t>(value))
            : write<std::uint16_t>(base() + 0x04U, value);
        if (!written) {
            return finish_error(driver::spi_error::mmio);
        }
        ++transmitted_frames_;
        if (transmitted_frames_ == total_frames_) {
            return write<std::uint8_t>(base() + 0x00U, spcr_receive_only);
        }
        return true;
    }

    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        if (!operation_id_ || dma_mode_
            || received_frames_ >= transmitted_frames_) {
            return false;
        }
        std::uint16_t value{};
        if (configuration_.frame_width == driver::spi_frame_width::bits8) {
            const auto received = mmio_.template read<std::uint8_t>(
                base() + 0x04U);
            if (!received) {
                return finish_error(driver::spi_error::mmio);
            }
            value = *received;
        } else {
            const auto received = mmio_.template read<std::uint16_t>(
                base() + 0x04U);
            if (!received) {
                return finish_error(driver::spi_error::mmio);
            }
            value = *received;
        }
        store_receive_frame(received_frames_, value);
        ++received_frames_;
        if (received_frames_ == total_frames_) {
            return write<std::uint8_t>(base() + 0x0FU,
                                       spcr2_auto_stop | spcr2_idle_irq);
        }
        return true;
    }

    [[nodiscard]] bool handle_idle_interrupt() noexcept {
        if (!operation_id_ || received_frames_ != total_frames_
            || transmitted_frames_ != total_frames_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x03U);
        if (!status) {
            return finish_error(driver::spi_error::mmio);
        }
        if ((*status & status_not_idle) != 0U) {
            return false;
        }
        const auto id = *operation_id_;
        const auto frames = total_frames_;
        const bool stopped = write<std::uint8_t>(base() + 0x00U, spcr_stopped)
            && write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            && set_chip_select(false);
        reset_transaction();
        if (!stopped) {
            return operations_.fail(id, driver::spi_error::mmio);
        }
        return operations_.complete(id, driver::spi_transfer_result{frames});
    }

    [[nodiscard]] bool handle_error_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x03U);
        if (!status) {
            return finish_error(driver::spi_error::mmio);
        }
        const auto error = (*status & status_overrun) != 0U
            ? driver::spi_error::overrun
            : ((*status & status_mode_fault) != 0U
                   ? driver::spi_error::mode_fault
                   : ((*status & status_parity) != 0U
                          ? driver::spi_error::parity
                          : driver::spi_error::transport));
        static_cast<void>(clear_errors());
        return finish_error(error);
    }

    [[nodiscard]] std::expected<void, driver::spi_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::spi_error::not_initialized);
        }
        if (operation_id_ || immediate_active_) {
            return std::unexpected(driver::spi_error::busy);
        }
        if (!write<std::uint8_t>(base() + 0x00U, 0U)
            || !restore_pins()) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr std::uint8_t status_overrun = 1U << 0U;
    static constexpr std::uint8_t status_not_idle = 1U << 1U;
    static constexpr std::uint8_t status_mode_fault = 1U << 2U;
    static constexpr std::uint8_t status_parity = 1U << 3U;
    static constexpr std::uint8_t status_transmit_empty = 1U << 5U;
    static constexpr std::uint8_t status_receive_full = 1U << 7U;
    static constexpr std::uint8_t spcr_stopped = 0x09U;
    static constexpr std::uint8_t spcr_active = 0xF9U;
    static constexpr std::uint8_t spcr_receive_only = 0xD9U;
    static constexpr std::uint8_t spcr_wait_idle = 0x59U;
    static constexpr std::uint8_t spcr_polling = 0x49U;
    static constexpr std::uint8_t spcr2_idle_irq = 1U << 2U;
    static constexpr std::uint8_t spcr2_auto_stop = 1U << 4U;

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return Binding::metadata.base;
    }

    [[nodiscard]] static constexpr std::uint16_t
    receive_dma_event() noexcept {
        return mcu::ra4m1::event_number(
            Binding::instance == 0U ? mcu::ra4m1::event::spi0_rxi
                                    : mcu::ra4m1::event::spi1_rxi);
    }

    [[nodiscard]] static constexpr std::uint16_t
    transmit_dma_event() noexcept {
        return mcu::ra4m1::event_number(
            Binding::instance == 0U ? mcu::ra4m1::event::spi0_txi
                                    : mcu::ra4m1::event::spi1_txi);
    }

    template<typename DmaTransfer>
    [[nodiscard]] bool handle_dma_completion(
        std::optional<DmaTransfer>& transfer, bool& completed,
        std::size_t& frames) noexcept {
        if (!operation_id_ || !dma_mode_ || !transfer || completed) {
            return false;
        }
        const auto inspected = transfer->inspect();
        if (!inspected) {
            if (inspected.error().kind
                == core::transfer_error_kind::pending) {
                return false;
            }
            static_cast<void>(transfer->take());
            transfer.reset();
            return finish_error(driver::spi_error::dma);
        }
        const auto result = transfer->take();
        transfer.reset();
        if (!result || result->units != total_frames_
            || result->width != driver::dma_width::bits8) {
            return finish_error(driver::spi_error::dma);
        }
        completed = true;
        frames = total_frames_;
        if (!receive_dma_complete_ || !transmit_dma_complete_) {
            return true;
        }
        if (!write<std::uint8_t>(base() + 0x00U, spcr_wait_idle)
            || !write<std::uint8_t>(base() + 0x0FU,
                                    spcr2_auto_stop | spcr2_idle_irq)) {
            return finish_error(driver::spi_error::mmio);
        }
        return true;
    }

    template<typename DmaTransfer>
    [[nodiscard]] static bool quiesce_dma_transfer(
        std::optional<DmaTransfer>& transfer) noexcept {
        if (!transfer) {
            return true;
        }
        const auto inspected = transfer->inspect();
        if (!inspected
            && inspected.error().kind == core::transfer_error_kind::pending) {
            const auto cancelled = transfer->cancel();
            if (!cancelled) {
                static_cast<void>(transfer->take());
                transfer.reset();
                return false;
            }
        }
        const auto retired = transfer->take();
        transfer.reset();
        return retired.has_value()
            || retired.error().kind == core::transfer_error_kind::cancelled;
    }

    [[nodiscard]] bool quiesce_dma_transfers() noexcept {
        const bool receive = quiesce_dma_transfer(receive_dma_transfer_);
        const bool transmit = quiesce_dma_transfer(transmit_dma_transfer_);
        return receive && transmit;
    }

    [[nodiscard]] static constexpr bool
    timing_is_valid(driver::spi_timing timing) noexcept {
        if (timing.clock_hz == 0U || timing.target_hz == 0U
            || timing.brdv > 3U) {
            return false;
        }
        const auto divider = 2U * (static_cast<std::uint32_t>(timing.spbr) + 1U)
            * static_cast<std::uint32_t>(1U << timing.brdv);
        return timing.achieved_hz == timing.clock_hz / divider
            && timing.achieved_hz <= timing.target_hz;
    }

    [[nodiscard]] std::expected<std::size_t, driver::spi_error>
    validate_segments(std::span<const driver::spi_segment> segments) const noexcept {
        if (segments.empty()) {
            return std::unexpected(driver::spi_error::empty_transfer);
        }
        std::size_t total{};
        for (const auto& segment : segments) {
            if (segment.transmit.empty() && segment.receive.empty()) {
                return std::unexpected(driver::spi_error::empty_transfer);
            }
            if (!segment.transmit.empty() && !segment.receive.empty()
                && segment.transmit.size() != segment.receive.size()) {
                return std::unexpected(driver::spi_error::mismatched_segment);
            }
            const auto frames = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (frames > 65'535U - total) {
                return std::unexpected(driver::spi_error::excessive_length);
            }
            if (configuration_.frame_width == driver::spi_frame_width::bits8) {
                if (segment.transmit.empty() && segment.dummy > 0xFFU) {
                    return std::unexpected(driver::spi_error::invalid_frame);
                }
                for (const auto frame : segment.transmit) {
                    if (frame > 0xFFU) {
                        return std::unexpected(driver::spi_error::invalid_frame);
                    }
                }
            }
            total += frames;
        }
        return total;
    }

    [[nodiscard]] std::uint16_t transmit_frame(std::size_t flat) const noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) {
                return segment.transmit.empty() ? segment.dummy
                                                : segment.transmit[flat];
            }
            flat -= count;
        }
        return 0U;
    }

    void store_receive_frame(std::size_t flat, std::uint16_t value) noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) {
                if (!segment.receive.empty()) {
                    segment.receive[flat] = value;
                }
                return;
            }
            flat -= count;
        }
    }

    [[nodiscard]] static core::quiesce_result cancel_operation(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<spi_master_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool dma_stopped = self.quiesce_dma_transfers();
        const bool peripheral_stopped = self.template write<std::uint8_t>(
                                            base() + 0x00U, spcr_stopped)
            && self.template write<std::uint8_t>(base() + 0x0FU,
                                                 spcr2_auto_stop)
            && self.set_chip_select(false);
        const bool stopped = dma_stopped && peripheral_stopped;
        self.reset_transaction();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool finish_error(driver::spi_error error) noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto id = *operation_id_;
        const bool dma_stopped = quiesce_dma_transfers();
        static_cast<void>(write<std::uint8_t>(base() + 0x00U, spcr_stopped));
        static_cast<void>(write<std::uint8_t>(base() + 0x0FU,
                                              spcr2_auto_stop));
        static_cast<void>(set_chip_select(false));
        reset_transaction();
        return operations_.fail(id, dma_stopped ? error
                                                : driver::spi_error::dma);
    }

    void reset_transaction() noexcept {
        operation_id_.reset();
        segments_ = {};
        total_frames_ = 0U;
        transmitted_frames_ = 0U;
        received_frames_ = 0U;
        receive_dma_transfer_.reset();
        transmit_dma_transfer_.reset();
        receive_dma_complete_ = false;
        transmit_dma_complete_ = false;
        dma_mode_ = false;
    }

    [[nodiscard]] bool clear_errors() noexcept {
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x03U);
        return status && write<std::uint8_t>(
            base() + 0x03U,
            static_cast<std::uint8_t>((*status & ~0x1DU) | 0xA0U));
    }

    [[nodiscard]] std::expected<void, driver::spi_error>
    wait_for_status(std::uint8_t mask, bool set,
                    std::uint32_t poll_limit) noexcept {
        for (std::uint32_t poll = 0U; poll < poll_limit; ++poll) {
            const auto status = mmio_.template read<std::uint8_t>(
                base() + 0x03U);
            if (!status) return std::unexpected(driver::spi_error::mmio);
            if ((*status & status_overrun) != 0U) {
                return std::unexpected(driver::spi_error::overrun);
            }
            if ((*status & status_mode_fault) != 0U) {
                return std::unexpected(driver::spi_error::mode_fault);
            }
            if ((*status & status_parity) != 0U) {
                return std::unexpected(driver::spi_error::parity);
            }
            if (((*status & mask) != 0U) == set) return {};
        }
        return std::unexpected(driver::spi_error::timeout);
    }

    [[nodiscard]] bool stop_immediate() noexcept {
        const bool controller_stopped =
            write<std::uint8_t>(base() + 0x00U, spcr_stopped);
        const bool interrupts_stopped =
            write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop);
        const bool deselected = set_chip_select(false);
        immediate_active_ = false;
        return controller_stopped && interrupts_stopped && deselected;
    }

    [[nodiscard]] std::expected<std::size_t, driver::spi_error>
    finish_immediate_error(driver::spi_error error) noexcept {
        return stop_immediate()
            ? std::unexpected(error)
            : std::unexpected(driver::spi_error::mmio);
    }

    [[nodiscard]] bool program_controller() noexcept {
        const auto phase = configuration_.mode == driver::spi_mode::mode1
                || configuration_.mode == driver::spi_mode::mode3
            ? 1U : 0U;
        const auto polarity = configuration_.mode == driver::spi_mode::mode2
                || configuration_.mode == driver::spi_mode::mode3
            ? 1U : 0U;
        const auto bit_order = configuration_.bit_order
                == driver::spi_bit_order::least_significant_first
            ? 1U : 0U;
        const auto frame = configuration_.frame_width
                == driver::spi_frame_width::bits8
            ? 0x04U : 0x0FU;
        const auto command = static_cast<std::uint16_t>(
            phase | (polarity << 1U) | (timing_.brdv << 2U)
            | (frame << 8U) | (bit_order << 12U));
        const auto data_control = configuration_.frame_width
                == driver::spi_frame_width::bits8
            ? 0x40U : 0x00U;
        return write<std::uint8_t>(base() + 0x00U, 0U)
            && write<std::uint8_t>(base() + 0x01U, 0U)
            && write<std::uint8_t>(base() + 0x02U, 0U)
            && clear_errors()
            && write<std::uint8_t>(base() + 0x0AU, timing_.spbr)
            && write<std::uint8_t>(base() + 0x0BU, data_control)
            && write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            && write<std::uint16_t>(base() + 0x10U, command)
            && write<std::uint8_t>(base() + 0x00U, spcr_stopped);
    }

    [[nodiscard]] bool release_module_stop() noexcept {
        constexpr std::uintptr_t prcr = 0x4001'E3FEU;
        const auto current = mmio_.template read<std::uint32_t>(
            Binding::metadata.module_stop_register);
        if (!current || !write<std::uint16_t>(prcr, 0xA502U)) {
            return false;
        }
        const bool enabled = write<std::uint32_t>(
            Binding::metadata.module_stop_register,
            *current & ~Binding::metadata.module_stop_mask);
        const bool locked = write<std::uint16_t>(prcr, 0xA500U);
        arch::data_memory_barrier();
        return enabled && locked;
    }

    [[nodiscard]] bool configure_pins() noexcept {
        return configure_peripheral_pin(Binding::miso_pin,
                                        spi_detail::miso_signal(Binding::instance))
            && configure_peripheral_pin(Binding::mosi_pin,
                                        spi_detail::mosi_signal(Binding::instance))
            && configure_peripheral_pin(Binding::clock_pin,
                                        spi_detail::clock_signal(Binding::instance))
            && configure_select_pin();
    }

    [[nodiscard]] bool restore_pins() noexcept {
        return restore_pin(Binding::miso_pin) && restore_pin(Binding::mosi_pin)
            && restore_pin(Binding::clock_pin) && restore_pin(Binding::select_pin);
    }

    [[nodiscard]] std::uint32_t psel_for(core::pin_id pin,
                                         mcu::ra4m1::signal signal) const noexcept {
        for (const auto& route : mcu::ra4m1::pin_routes) {
            if (route.pin == pin
                && route.signal == mcu::ra4m1::signal_id(signal)) {
                return route.vendor_encoding;
            }
        }
        return 0U;
    }

    [[nodiscard]] bool configure_peripheral_pin(
        core::pin_id pin, mcu::ra4m1::signal signal) noexcept {
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 2U);
        return update_pfs(pin, controlled,
                          (psel_for(pin, signal) << 24U) | (1U << 16U));
    }

    [[nodiscard]] bool configure_select_pin() noexcept {
        if (!set_chip_select(false)) {
            return false;
        }
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 2U)
            | 1U;
        const auto inactive_high = configuration_.chip_select
                == driver::spi_chip_select_polarity::active_low
            ? 1U : 0U;
        return update_pfs(Binding::select_pin, controlled,
                          (1U << 2U) | inactive_high);
    }

    [[nodiscard]] bool restore_pin(core::pin_id pin) noexcept {
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 2U)
            | 1U;
        return update_pfs(pin, controlled, 0U);
    }

    [[nodiscard]] bool update_pfs(core::pin_id pin, std::uint32_t controlled,
                                  std::uint32_t selected) noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        const auto address = mcu::ra4m1::pfs_address(pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        const bool configured = write<std::uint32_t>(
            address, (*current & ~controlled) | selected);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return configured && locked;
    }

    [[nodiscard]] bool set_chip_select(bool active) noexcept {
        const auto mask = static_cast<std::uint32_t>(
            1U << Binding::select_pin.index);
        const bool high = active
            ? configuration_.chip_select
                  == driver::spi_chip_select_polarity::active_high
            : configuration_.chip_select
                  == driver::spi_chip_select_polarity::active_low;
        const auto command = high ? mask : static_cast<std::uint32_t>(mask << 16U);
        const bool result = write<std::uint32_t>(
            0x4004'0008U + Binding::select_pin.port * 0x20U, command);
        arch::data_memory_barrier();
        return result;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return mmio_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] static start_result
    start_failure(driver::spi_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::spi_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::spi_error::mmio);
    }

    void release_owner() noexcept {
        if (owned_) {
            spi_detail::owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    MmioPolicy& mmio_;
    pool_type operations_;
    driver::spi_timing timing_{};
    driver::spi_configuration configuration_{};
    std::span<const driver::spi_segment> segments_{};
    std::optional<core::operation_id> operation_id_{};
    std::size_t total_frames_{};
    std::size_t transmitted_frames_{};
    std::size_t received_frames_{};
    using dma_transfer_type = core::transfer<driver::dma_transfer_result,
                                             driver::dma_error, 1,
                                             ExecutionPolicy>;
    std::optional<dma_transfer_type> receive_dma_transfer_{};
    std::optional<dma_transfer_type> transmit_dma_transfer_{};
    bool receive_dma_complete_{};
    bool transmit_dma_complete_{};
    bool dma_mode_{};
    bool immediate_active_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::ra4m1
