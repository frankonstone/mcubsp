#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/spi.hpp>
#include <mcuhw/mcu/esp32p4/description.hpp>

namespace mcuhw::backend::esp32p4 {

struct spi_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;

    std::uint32_t clock_register;
    [[nodiscard]] constexpr operator driver::spi_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm};
    }
    friend constexpr bool operator==(spi_timing, spi_timing) = default;
};

namespace detail {

template<std::uint8_t Instance>
struct spi_owner { static inline bool claimed{}; };

template<bool Enabled>
struct spi_select_handler {};

template<>
struct spi_select_handler<true> {
    bool (*set)(void*, bool) noexcept{};
    void* context{};
};

} // namespace detail

// GPSPI either passes the source clock directly or divides it by (pre * n),
// with pre in 1..16 and n in 2..64. Select the fastest rate which does not
// exceed the device limit; clock_register is the native SPI_CLOCK_REG value.
// @req REQ-0011 REQ-0028 REQ-0034
[[nodiscard]] constexpr std::expected<spi_timing, driver::spi_error>
solve_spi_timing(driver::spi_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::spi_error::invalid_clock);
    }
    if (request.target_hz == 0U || request.target_hz > request.clock_hz) {
        return std::unexpected(driver::spi_error::invalid_rate);
    }
    if (request.target_hz == request.clock_hz) {
        return spi_timing{
            .clock_hz = request.clock_hz,
            .target_hz = request.target_hz,
            .achieved_hz = request.clock_hz,
            .error_ppm = 0U,
            .clock_register = 1U << 31U,
        };
    }

    bool found{};
    spi_timing best{};
    for (std::uint32_t pre = 1U; pre <= 16U; ++pre) {
        for (std::uint32_t n = 2U; n <= 64U; ++n) {
            const auto achieved = request.clock_hz / (pre * n);
            if (achieved == 0U || achieved > request.target_hz) continue;
            const auto error = static_cast<std::uint32_t>(
                (static_cast<std::uint64_t>(request.target_hz - achieved)
                     * 1'000'000ULL + request.target_hz / 2U)
                / request.target_hz);
            const auto high = n / 2U;
            const auto encoding = (n - 1U) | ((high - 1U) << 6U)
                | ((n - 1U) << 12U) | ((pre - 1U) << 18U);
            if (!found || achieved > best.achieved_hz
                || (achieved == best.achieved_hz
                    && encoding < best.clock_register)) {
                best = {
                    .clock_hz = request.clock_hz,
                    .target_hz = request.target_hz,
                    .achieved_hz = achieved,
                    .error_ppm = error,
                    .clock_register = encoding,
                };
                found = true;
            }
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::spi_error::unattainable_tolerance);
    }
    return best;
}

template<std::uint8_t Instance, core::pin_id MisoPin, core::pin_id MosiPin,
         core::pin_id ClockPin, core::pin_id SelectPin>
struct spi_binding {
    static_assert(Instance == 2U,
                  "source-backed initial ESP32-P4 backend covers GPSPI2");
    static_assert(MisoPin != MosiPin && MisoPin != ClockPin
                  && MisoPin != SelectPin && MosiPin != ClockPin
                  && MosiPin != SelectPin && ClockPin != SelectPin,
                  "SPI pins are exclusive within a binding");
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        MisoPin, mcu::esp32p4::signal_id(mcu::esp32p4::signal::spi2_miso),
        core::electrical_mode::input>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        MosiPin, mcu::esp32p4::signal_id(mcu::esp32p4::signal::spi2_mosi),
        core::electrical_mode::push_pull>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        ClockPin, mcu::esp32p4::signal_id(mcu::esp32p4::signal::spi2_sck),
        core::electrical_mode::push_pull>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        SelectPin, mcu::esp32p4::signal_id(mcu::esp32p4::signal::spi2_cs),
        core::electrical_mode::push_pull>);

    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id miso_pin = MisoPin;
    static constexpr core::pin_id mosi_pin = MosiPin;
    static constexpr core::pin_id clock_pin = ClockPin;
    static constexpr core::pin_id select_pin = SelectPin;
    static constexpr std::uintptr_t base = mcu::esp32p4::spi2_base;
    static constexpr auto clocks = mcu::esp32p4::spi_clock_controls[0];
    static constexpr std::uint16_t clock_matrix_signal = 53U;
    static constexpr std::uint16_t miso_matrix_signal = 54U;
    static constexpr std::uint16_t mosi_matrix_signal = 55U;
    static constexpr std::uint16_t select_matrix_signal = 62U;
};

// A fixed-storage transaction is flattened into one GPSPI CPU-buffer command.
// Native CS therefore remains asserted across every caller segment. Success is
// published only after TRANS_DONE and the self-clearing USR bit both show idle.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy,
         bool ExternalSelect = false>
// @req REQ-0003 REQ-0009 REQ-0014 REQ-0017 REQ-0019 REQ-0028 REQ-0034
class spi_master_backend {
public:
    using pool_type = core::operation_pool<driver::spi_transfer_result,
                                           driver::spi_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    spi_master_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::spi_error> initialize(
        spi_timing timing,
        driver::spi_configuration configuration = {}) noexcept {
        if (initialized_ || detail::spi_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::spi_error::already_owned);
        }
        if (!timing_is_valid(timing)) {
            return std::unexpected(driver::spi_error::invalid_rate);
        }
        if (!configuration_is_valid(configuration)) {
            return std::unexpected(driver::spi_error::invalid_frame);
        }
        detail::spi_owner<Binding::instance>::claimed = true;
        owned_ = true;
        configuration_ = configuration;

        if (!modify(Binding::clocks.system_gate_register,
                    Binding::clocks.system_gate_mask, 0U))
            return initialization_failed();
        if (!modify(Binding::clocks.apb_gate_register,
                    Binding::clocks.apb_gate_mask, 0U))
            return initialization_failed();
        if (!pulse_reset() || !configure_source())
            return initialization_failed();
        if (!configure_output_pin(Binding::clock_pin,
                                  Binding::clock_matrix_signal))
            return initialization_failed();
        if (!configure_output_pin(Binding::mosi_pin,
                                  Binding::mosi_matrix_signal))
            return initialization_failed();
        if (!configure_input_pin(Binding::miso_pin,
                                 Binding::miso_matrix_signal))
            return initialization_failed();
        if (!configure_select_pin())
            return initialization_failed();
        if (!write(reg_interrupt_enable, 0U)
            || !write(reg_interrupt_clear, all_interrupts))
            return initialization_failed();
        if (!write(reg_slave, 0U) || !write(reg_dma_configuration, 0U)
            || !write(reg_clock_gate, 0x3U))
            return initialization_failed();
        if (!program(timing, configuration))
            return initialization_failed();
        initialized_ = true;
        configuration_valid_ = true;
        return {};
    }

    [[nodiscard]] bool busy() const noexcept { return id_.has_value(); }

    [[nodiscard]] std::expected<void, driver::spi_error>
    set_select_handler(bool (*set)(void*, bool) noexcept,
                       void* context) noexcept requires(ExternalSelect) {
        if (busy()) return std::unexpected(driver::spi_error::busy);
        select_handler_ = {set, context};
        return {};
    }

    [[nodiscard]] std::expected<void, driver::spi_error> reconfigure(
        spi_timing timing,
        driver::spi_configuration configuration = {}) noexcept {
        if (!initialized_) return std::unexpected(driver::spi_error::not_initialized);
        if (busy()) return std::unexpected(driver::spi_error::busy);
        if (!timing_is_valid(timing)) return std::unexpected(driver::spi_error::invalid_rate);
        if (!configuration_is_valid(configuration)) return std::unexpected(driver::spi_error::invalid_frame);
        configuration_ = configuration;
        configuration_valid_ = program(timing, configuration);
        if (!configuration_valid_) return std::unexpected(driver::spi_error::mmio);
        return {};
    }

    [[nodiscard]] start_result transaction(
        std::span<const driver::spi_segment> segments) noexcept {
        if (!initialized_) return failure(driver::spi_error::not_initialized);
        if (!configuration_valid_) return failure(driver::spi_error::mmio);
        if (id_) return failure(driver::spi_error::busy);
        const auto count = validate(segments);
        if (!count) return failure(count.error());

        auto started = operations_.try_start({.quiesce = &cancel,
                                              .context = this});
        if (!started) return std::unexpected(started.error());
        id_ = started->id();
        segments_ = segments;
        total_frames_ = *count;

        const bool prepared = write(reg_interrupt_enable, 0U)
            && write(reg_interrupt_clear, all_interrupts)
            && reset_fifos()
            && fill_buffer()
            && write(reg_data_length,
                     static_cast<std::uint32_t>(total_frames_)
                         * frame_bits() - 1U)
            && synchronize()
            && set_select(true)
            && write(reg_interrupt_enable, transfer_interrupts)
            && write(reg_command, command_user);
        if (!prepared) static_cast<void>(finish(driver::spi_error::mmio));
        return started;
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        if (!id_) return false;
        const auto status = read(reg_interrupt_raw);
        if (!status) return finish(driver::spi_error::mmio);
        if ((*status & interrupt_rx_full) != 0U) {
            return finish(driver::spi_error::overrun);
        }
        if ((*status & interrupt_tx_empty) != 0U) {
            return finish(driver::spi_error::transport);
        }
        if ((*status & interrupt_done) == 0U) return false;
        const auto command = read(reg_command);
        if (!command) return finish(driver::spi_error::mmio);
        if ((*command & command_user) != 0U) return false;
        if (!store_buffer()) return finish(driver::spi_error::mmio);

        const auto current = *id_;
        const auto frames = total_frames_;
        const bool interrupts_stopped = write(reg_interrupt_enable, 0U);
        const bool cleared = write(reg_interrupt_clear, all_interrupts);
        const bool deselected = set_select(false);
        const bool stopped = interrupts_stopped && cleared && deselected;
        reset_state();
        return stopped
            ? operations_.complete(current,
                                   driver::spi_transfer_result{frames})
            : operations_.fail(current, driver::spi_error::mmio);
    }

    [[nodiscard]] bool handle_transmit_empty_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_idle_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_error_interrupt() noexcept {
        return handle_interrupt();
    }

    [[nodiscard]] std::expected<void, driver::spi_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::spi_error::not_initialized);
        }
        if (id_) return std::unexpected(driver::spi_error::busy);
        if (!write(reg_interrupt_enable, 0U)
            || !restore_select_pin()
            || !restore_input_pin(Binding::miso_pin,
                                  Binding::miso_matrix_signal)
            || !restore_output_pin(Binding::mosi_pin)
            || !restore_output_pin(Binding::clock_pin)) {
            return std::unexpected(driver::spi_error::mmio);
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    [[nodiscard]] static constexpr bool timing_is_valid(
        spi_timing timing) noexcept {
        if (timing.clock_hz != 40'000'000U || timing.achieved_hz == 0U
            || timing.achieved_hz > timing.target_hz
            || timing.target_hz > timing.clock_hz
            || (timing.clock_register & ~clock_register_mask) != 0U) {
            return false;
        }
        if ((timing.clock_register & (1U << 31U)) != 0U) {
            return timing.clock_register == (1U << 31U)
                && timing.achieved_hz == timing.clock_hz;
        }
        const auto pre = ((timing.clock_register >> 18U) & 0xFU) + 1U;
        const auto n = ((timing.clock_register >> 12U) & 0x3FU) + 1U;
        const auto high = ((timing.clock_register >> 6U) & 0x3FU) + 1U;
        const auto low = (timing.clock_register & 0x3FU) + 1U;
        return n >= 2U && low == n && high < n
            && timing.achieved_hz == timing.clock_hz / (pre * n);
    }

    [[nodiscard]] static constexpr bool configuration_is_valid(
        driver::spi_configuration configuration) noexcept {
        return static_cast<std::uint8_t>(configuration.mode) <= 3U
            && static_cast<std::uint8_t>(configuration.bit_order) <= 1U
            && (configuration.frame_width == driver::spi_frame_width::bits8
                || configuration.frame_width == driver::spi_frame_width::bits16)
            && static_cast<std::uint8_t>(configuration.chip_select) <= 1U;
    }

    [[nodiscard]] bool program(spi_timing timing,
                               driver::spi_configuration configuration) noexcept {
        return write(reg_clock, timing.clock_register)
            && write(reg_control, control_value(configuration))
            && write(reg_user, user_value(configuration))
            && write(reg_user1, 0U) && write(reg_user2, 0U)
            && write(reg_misc, misc_value(configuration)) && synchronize();
    }

    [[nodiscard]] bool configure_select_pin() noexcept {
        if constexpr (ExternalSelect) return true;
        else return configure_output_pin(Binding::select_pin,
                                         Binding::select_matrix_signal);
    }

    [[nodiscard]] bool restore_select_pin() noexcept {
        if constexpr (ExternalSelect) return true;
        else return restore_output_pin(Binding::select_pin);
    }

    [[nodiscard]] bool set_select(bool active) noexcept {
        if constexpr (ExternalSelect) {
            return select_handler_.set
                && select_handler_.set(select_handler_.context, active);
        } else {
            return true;
        }
    }

    static constexpr std::uintptr_t reg_command = 0x00U;
    static constexpr std::uintptr_t reg_control = 0x08U;
    static constexpr std::uintptr_t reg_clock = 0x0CU;
    static constexpr std::uintptr_t reg_user = 0x10U;
    static constexpr std::uintptr_t reg_user1 = 0x14U;
    static constexpr std::uintptr_t reg_user2 = 0x18U;
    static constexpr std::uintptr_t reg_data_length = 0x1CU;
    static constexpr std::uintptr_t reg_misc = 0x20U;
    static constexpr std::uintptr_t reg_dma_configuration = 0x30U;
    static constexpr std::uintptr_t reg_interrupt_enable = 0x34U;
    static constexpr std::uintptr_t reg_interrupt_clear = 0x38U;
    static constexpr std::uintptr_t reg_interrupt_raw = 0x3CU;
    static constexpr std::uintptr_t reg_buffer = 0x98U;
    static constexpr std::uintptr_t reg_slave = 0xE0U;
    static constexpr std::uintptr_t reg_clock_gate = 0xE8U;
    static constexpr std::uint32_t command_update = 1U << 23U;
    static constexpr std::uint32_t command_user = 1U << 24U;
    static constexpr std::uint32_t interrupt_done = 1U << 12U;
    static constexpr std::uint32_t interrupt_rx_full = 1U << 17U;
    static constexpr std::uint32_t interrupt_tx_empty = 1U << 18U;
    static constexpr std::uint32_t transfer_interrupts = interrupt_done
        | interrupt_rx_full | interrupt_tx_empty;
    static constexpr std::uint32_t all_interrupts = 0x001F'FFFFU;
    static constexpr std::uint32_t clock_register_mask = 0x803F'FFFFU;
    static constexpr std::size_t fifo_bytes = 64U;
    static constexpr std::size_t synchronization_limit = 1'024U;

    [[nodiscard]] constexpr std::uint32_t frame_bits() const noexcept {
        return static_cast<std::uint32_t>(configuration_.frame_width);
    }

    [[nodiscard]] static constexpr std::uint32_t control_value(
        driver::spi_configuration configuration) noexcept {
        return configuration.bit_order
                == driver::spi_bit_order::least_significant_first
            ? (1U << 23U) | (1U << 25U) : 0U;
    }

    [[nodiscard]] static constexpr std::uint32_t user_value(
        driver::spi_configuration configuration) noexcept {
        const auto mode = static_cast<std::uint32_t>(configuration.mode);
        return (1U << 0U) | (1U << 6U) | (1U << 7U)
            | ((mode == 1U || mode == 2U) ? 1U << 9U : 0U)
            | (1U << 27U) | (1U << 28U);
    }

    [[nodiscard]] static constexpr std::uint32_t misc_value(
        driver::spi_configuration configuration) noexcept {
        const auto mode = static_cast<std::uint32_t>(configuration.mode);
        return (ExternalSelect ? 0x3FU : 0x3EU)
            | (configuration.chip_select
                    == driver::spi_chip_select_polarity::active_high
                   ? 1U << 7U : 0U)
            | ((mode == 2U || mode == 3U) ? 1U << 29U : 0U);
    }

    [[nodiscard]] std::expected<std::size_t, driver::spi_error> validate(
        std::span<const driver::spi_segment> segments) const noexcept {
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
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (count > maximum_frames() - total) {
                return std::unexpected(driver::spi_error::excessive_length);
            }
            if (configuration_.frame_width == driver::spi_frame_width::bits8) {
                if (segment.transmit.empty() && segment.dummy > 0xFFU) {
                    return std::unexpected(driver::spi_error::invalid_frame);
                }
                for (const auto value : segment.transmit) {
                    if (value > 0xFFU) {
                        return std::unexpected(driver::spi_error::invalid_frame);
                    }
                }
            }
            total += count;
        }
        return total;
    }

    [[nodiscard]] constexpr std::size_t maximum_frames() const noexcept {
        return fifo_bytes / (frame_bits() / 8U);
    }

    [[nodiscard]] std::uint16_t frame(std::size_t flat) const noexcept {
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

    void store(std::size_t flat, std::uint16_t value) noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) {
                if (!segment.receive.empty()) segment.receive[flat] = value;
                return;
            }
            flat -= count;
        }
    }

    [[nodiscard]] bool fill_buffer() noexcept {
        const auto bytes_per_frame = frame_bits() / 8U;
        for (std::size_t word_index = 0U;
             word_index < (total_frames_ * bytes_per_frame + 3U) / 4U;
             ++word_index) {
            std::uint32_t word{};
            for (std::size_t byte = 0U; byte < 4U; ++byte) {
                const auto flat_byte = word_index * 4U + byte;
                const auto flat_frame = flat_byte / bytes_per_frame;
                if (flat_frame >= total_frames_) break;
                const auto value = frame(flat_frame);
                word |= static_cast<std::uint32_t>(
                    (value >> ((flat_byte % bytes_per_frame) * 8U)) & 0xFFU)
                    << (byte * 8U);
            }
            if (!write(reg_buffer + word_index * 4U, word)) return false;
        }
        return true;
    }

    [[nodiscard]] bool store_buffer() noexcept {
        const auto bytes_per_frame = frame_bits() / 8U;
        std::uint32_t word{};
        for (std::size_t flat = 0U; flat < total_frames_; ++flat) {
            const auto byte_offset = flat * bytes_per_frame;
            if ((byte_offset % 4U) == 0U) {
                const auto value = read(reg_buffer + (byte_offset / 4U) * 4U);
                if (!value) return false;
                word = *value;
            }
            const auto shift = (byte_offset % 4U) * 8U;
            auto value = static_cast<std::uint16_t>((word >> shift) & 0xFFU);
            if (bytes_per_frame == 2U) {
                if ((byte_offset % 4U) == 3U) {
                    const auto next = read(reg_buffer
                        + ((byte_offset + 1U) / 4U) * 4U);
                    if (!next) return false;
                    value |= static_cast<std::uint16_t>(*next & 0xFFU) << 8U;
                    word = *next;
                } else {
                    value |= static_cast<std::uint16_t>(
                        (word >> (shift + 8U)) & 0xFFU) << 8U;
                }
            }
            store(flat, value);
        }
        return true;
    }

    [[nodiscard]] bool configure_source() noexcept {
        constexpr std::uint32_t source_mask = 0x7U;
        constexpr std::uint32_t divider_mask = (0xFFU << 4U)
            | (0xFFU << 12U);
        const auto enabled = (1U << Binding::clocks.high_speed_enable_shift)
            | (1U << Binding::clocks.master_enable_shift);
        return modify(Binding::clocks.source_register, enabled,
                      source_mask | divider_mask);
    }

    [[nodiscard]] bool synchronize() noexcept {
        if (!write(reg_command, command_update)) return false;
        for (std::size_t attempt = 0U; attempt < synchronization_limit;
             ++attempt) {
            const auto command = read(reg_command);
            if (!command) return false;
            if ((*command & command_update) == 0U) return true;
        }
        return false;
    }

    [[nodiscard]] bool reset_fifos() noexcept {
        return write(reg_dma_configuration, (1U << 29U) | (1U << 30U))
            && write(reg_dma_configuration, 0U);
    }

    [[nodiscard]] bool pulse_reset() noexcept {
        return modify(Binding::clocks.reset_register,
                      Binding::clocks.reset_mask, 0U)
            && modify(Binding::clocks.reset_register, 0U,
                      Binding::clocks.reset_mask);
    }

    [[nodiscard]] bool configure_output_pin(core::pin_id pin,
                                            std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto output = output_select_address(pin);
        const auto current_pad = read_absolute(pad);
        const auto current_output = read_absolute(output);
        if (!current_pad || !current_output) return false;
        constexpr std::uint32_t pad_controlled = (1U << 9U)
            | (0x3U << 10U) | (0x7U << 12U);
        const auto next_pad = static_cast<std::uint32_t>(
            (*current_pad & ~pad_controlled) | (2U << 10U) | (1U << 12U));
        const auto next_output = static_cast<std::uint32_t>(
            (*current_output & ~0x7FFU) | signal);
        return write_absolute(output, next_output)
            && write_absolute(pad, next_pad);
    }

    [[nodiscard]] bool configure_input_pin(core::pin_id pin,
                                           std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto input = input_select_address(signal);
        const auto current_pad = read_absolute(pad);
        const auto current_input = read_absolute(input);
        if (!current_pad || !current_input) return false;
        constexpr std::uint32_t pad_controlled = (1U << 9U)
            | (0x3U << 10U) | (0x7U << 12U);
        const auto next_pad = static_cast<std::uint32_t>(
            (*current_pad & ~pad_controlled) | (1U << 9U)
            | (2U << 10U) | (1U << 12U));
        const auto next_input = static_cast<std::uint32_t>(
            (*current_input & ~0xFFU) | pin.index | (1U << 7U));
        return write_absolute(input, next_input)
            && write_absolute(pad, next_pad);
    }

    [[nodiscard]] bool restore_output_pin(core::pin_id pin) noexcept {
        const auto pad = pad_address(pin);
        const auto output = output_select_address(pin);
        const auto current_pad = read_absolute(pad);
        const auto current_output = read_absolute(output);
        if (!current_pad || !current_output) return false;
        return write_absolute(output,
                              (*current_output & ~0x7FFU) | 256U | (1U << 10U))
            && write_absolute(pad, (*current_pad & ~(0x7U << 12U))
                                      | (1U << 9U) | (1U << 12U));
    }

    [[nodiscard]] bool restore_input_pin(core::pin_id pin,
                                         std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto input = input_select_address(signal);
        const auto current_pad = read_absolute(pad);
        const auto current_input = read_absolute(input);
        if (!current_pad || !current_input) return false;
        return write_absolute(input,
                              (*current_input & ~0xFFU) | 0x3EU | (1U << 7U))
            && write_absolute(pad, (*current_pad & ~(0x7U << 12U))
                                      | (1U << 9U) | (1U << 12U));
    }

    [[nodiscard]] static core::quiesce_result cancel(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<spi_master_backend*>(context);
        if (!self.id_ || *self.id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.quiesce();
        self.reset_state();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool quiesce() noexcept {
        const bool interrupts_stopped = write(reg_interrupt_enable, 0U);
        const bool reset_asserted = write(reg_slave, 1U << 27U);
        const bool reset_released = write(reg_slave, 0U);
        const bool cleared = write(reg_interrupt_clear, all_interrupts);
        const bool fifos_reset = reset_fifos();
        const bool deselected = set_select(false);
        return interrupts_stopped && reset_asserted && reset_released
            && cleared && fifos_reset && deselected;
    }

    [[nodiscard]] bool finish(driver::spi_error error) noexcept {
        if (!id_) return false;
        const bool stopped = quiesce();
        const auto current = *id_;
        reset_state();
        return operations_.fail(current, stopped ? error
                                                 : driver::spi_error::mmio);
    }

    [[nodiscard]] std::expected<void, driver::spi_error>
    initialization_failed() noexcept {
        static_cast<void>(restore_select_pin());
        static_cast<void>(restore_input_pin(Binding::miso_pin,
                                            Binding::miso_matrix_signal));
        static_cast<void>(restore_output_pin(Binding::mosi_pin));
        static_cast<void>(restore_output_pin(Binding::clock_pin));
        release_owner();
        return std::unexpected(driver::spi_error::mmio);
    }

    [[nodiscard]] start_result failure(driver::spi_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation,
            .cause = error});
    }

    void reset_state() noexcept {
        id_.reset();
        segments_ = {};
        total_frames_ = 0U;
    }

    void release_owner() noexcept {
        if (owned_) {
            detail::spi_owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    [[nodiscard]] static constexpr std::uintptr_t pad_address(
        core::pin_id pin) noexcept {
        return mcu::esp32p4::io_mux_base + 0x04U
            + static_cast<std::uintptr_t>(pin.index) * 4U;
    }
    [[nodiscard]] static constexpr std::uintptr_t output_select_address(
        core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x558U
            + static_cast<std::uintptr_t>(pin.index) * 4U;
    }
    [[nodiscard]] static constexpr std::uintptr_t input_select_address(
        std::uint16_t signal) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x158U
            + static_cast<std::uintptr_t>(signal) * 4U;
    }

    [[nodiscard]] auto read(std::uintptr_t offset) noexcept {
        return read_absolute(Binding::base + offset);
    }
    [[nodiscard]] bool write(std::uintptr_t offset,
                             std::uint32_t value) noexcept {
        return write_absolute(Binding::base + offset, value);
    }
    [[nodiscard]] auto read_absolute(std::uintptr_t address) noexcept {
        return mmio_.template read<std::uint32_t>(address);
    }
    [[nodiscard]] bool write_absolute(std::uintptr_t address,
                                      std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(address, value).has_value();
    }
    [[nodiscard]] bool modify(std::uintptr_t address, std::uint32_t set,
                              std::uint32_t clear) noexcept {
        const auto current = read_absolute(address);
        return current && write_absolute(address, (*current & ~clear) | set);
    }

    MmioPolicy& mmio_;
    [[no_unique_address]] detail::spi_select_handler<ExternalSelect> select_handler_{};
    pool_type operations_;
    driver::spi_configuration configuration_{};
    std::optional<core::operation_id> id_{};
    std::span<const driver::spi_segment> segments_{};
    std::size_t total_frames_{};
    bool initialized_{};
    bool configuration_valid_{};
    bool owned_{};
};

} // namespace mcuhw::backend::esp32p4
