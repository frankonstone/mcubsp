#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/i2c.hpp>
#include <mcuhw/mcu/esp32p4/description.hpp>

namespace mcuhw::backend::esp32p4 {

struct i2c_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t rise_time_ns;
    std::uint32_t fall_time_ns;

    std::uint8_t low_count;
    std::uint8_t high_count;
    std::uint32_t source_divider;
    [[nodiscard]] constexpr operator driver::i2c_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm, rise_time_ns, fall_time_ns};
    }
    friend constexpr bool operator==(i2c_timing, i2c_timing) = default;
};

enum class i2c_recovery_policy : std::uint8_t {
    none,
    exclusive_peripheral_reset,
};

namespace detail {

template<std::uint8_t Instance>
struct i2c_owner { static inline bool claimed{}; };

[[nodiscard]] constexpr std::uintptr_t i2c_base(
    std::uint8_t instance) noexcept {
    return mcu::esp32p4::i2c0_base
        + static_cast<std::uintptr_t>(instance) * 0x1000U;
}

} // namespace detail

// The HP I2C controller takes the 40 MHz XTAL through an integer pre-divider.
// low_count is the complete low interval and high_count the complete high interval; the
// backend splits high_count into the controller's active-high and wait-high fields.
// Edge times are included in the wire-period and Standard/Fast-mode minima.
// @req REQ-0011 REQ-0024 REQ-0034
[[nodiscard]] constexpr std::expected<i2c_timing, driver::i2c_error>
solve_i2c_timing(driver::i2c_timing_request request) noexcept {
    if (request.clock_hz != 40'000'000U) {
        return std::unexpected(driver::i2c_error::invalid_clock);
    }
    if (request.target_hz == 0U || request.target_hz > 400'000U) {
        return std::unexpected(driver::i2c_error::invalid_rate);
    }
    const bool standard = request.target_hz <= 100'000U;
    if (request.rise_time_ns > (standard ? 1'000U : 300U)
        || request.fall_time_ns > 300U) {
        return std::unexpected(driver::i2c_error::invalid_rise_fall);
    }

    const std::uint32_t minimum_low_ns = standard ? 4'700U : 1'300U;
    const std::uint32_t minimum_high_ns = standard ? 4'000U : 600U;
    const auto target_period_ns = driver::i2c_detail::divide_rounded(
        1'000'000'000ULL, request.target_hz);
    const auto edge_ns = static_cast<std::uint64_t>(request.rise_time_ns)
        + request.fall_time_ns;
    if (target_period_ns <= edge_ns) {
        return std::unexpected(driver::i2c_error::unattainable_tolerance);
    }

    bool found{};
    i2c_timing best{};
    for (std::uint32_t divider = 1U; divider <= 256U; ++divider) {
        const auto source_hz = request.clock_hz / divider;
        if (source_hz == 0U) break;
        const auto desired_counts = driver::i2c_detail::divide_rounded(
            (target_period_ns - edge_ns) * source_hz, 1'000'000'000ULL);
        if (desired_counts < 2U || desired_counts > 510U) continue;

        for (std::uint32_t low = 1U; low <= 255U; ++low) {
            const auto low_ns = driver::i2c_detail::divide_rounded(
                static_cast<std::uint64_t>(low) * 1'000'000'000ULL,
                source_hz) + request.fall_time_ns;
            if (low_ns < minimum_low_ns) continue;

            const auto nominal_high = desired_counts > low
                ? desired_counts - low : 0U;
            const auto first_high = nominal_high > 2U
                ? nominal_high - 2U : 1U;
            const auto last_high = nominal_high < 253U
                ? nominal_high + 2U : 255U;
            for (auto high = first_high; high <= last_high; ++high) {
                if (high < 6U || high > 255U) continue;
                const auto wait_high = high / 2U - 2U;
                if (wait_high > 127U || high <= wait_high + 1U) continue;
                const auto high_ns = driver::i2c_detail::divide_rounded(
                    static_cast<std::uint64_t>(high) * 1'000'000'000ULL,
                    source_hz) + request.rise_time_ns;
                if (high_ns < minimum_high_ns) continue;
                const auto period_ns = low_ns + high_ns;
                const auto achieved = static_cast<std::uint32_t>(
                    driver::i2c_detail::divide_rounded(
                        1'000'000'000ULL, period_ns));
                const auto delta = achieved > request.target_hz
                    ? achieved - request.target_hz
                    : request.target_hz - achieved;
                const auto error = static_cast<std::uint32_t>(
                    driver::i2c_detail::divide_rounded(
                        static_cast<std::uint64_t>(delta) * 1'000'000ULL,
                        request.target_hz));
                const i2c_timing candidate{
                    .clock_hz = request.clock_hz,
                    .target_hz = request.target_hz,
                    .achieved_hz = achieved,
                    .error_ppm = error,
                    .rise_time_ns = request.rise_time_ns,
                    .fall_time_ns = request.fall_time_ns,
                    .low_count = static_cast<std::uint8_t>(low),
                    .high_count = static_cast<std::uint8_t>(high),
                    .source_divider = divider,
                };
                if (!found || error < best.error_ppm
                    || (error == best.error_ppm
                        && divider < best.source_divider)) {
                    best = candidate;
                    found = true;
                }
            }
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::i2c_error::unattainable_tolerance);
    }
    return best;
}

template<std::uint8_t Instance, core::pin_id SclPin, core::pin_id SdaPin,
         std::uint32_t ApbGateMask =
             mcu::esp32p4::i2c_clock_controls[Instance].apb_gate_mask,
         std::uint32_t ResetMask =
             mcu::esp32p4::i2c_clock_controls[Instance].reset_mask>
struct i2c_binding {
    static_assert(Instance == 0U,
                  "source-backed initial ESP32-P4 route covers HP I2C0");
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        SclPin, mcu::esp32p4::signal_id(mcu::esp32p4::signal::i2c0_scl),
        core::electrical_mode::open_drain>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        SdaPin, mcu::esp32p4::signal_id(mcu::esp32p4::signal::i2c0_sda),
        core::electrical_mode::open_drain>);
    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id scl_pin = SclPin;
    static constexpr core::pin_id sda_pin = SdaPin;
    static constexpr std::uintptr_t base = detail::i2c_base(Instance);
    static constexpr auto clocks = [] {
        auto result = mcu::esp32p4::i2c_clock_controls[Instance];
        result.apb_gate_mask = ApbGateMask;
        result.reset_mask = ResetMask;
        return result;
    }();
    static constexpr std::uint16_t scl_matrix_signal = 68U;
    static constexpr std::uint16_t sda_matrix_signal = 69U;
};

// One fixed-storage operation maps to one hardware command-list execution.
// The explicit address commands make NACK attribution deterministic, and the
// 32-byte FIFO limit is rejected before any bus activity begins.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0009 REQ-0014 REQ-0017 REQ-0019 REQ-0024 REQ-0025
class i2c_backend {
public:
    using pool_type = core::operation_pool<driver::i2c_transfer_result,
                                           driver::i2c_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    i2c_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::i2c_error> initialize(
        i2c_timing timing) noexcept {
        if (initialized_ || detail::i2c_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::i2c_error::already_owned);
        }
        if (timing.clock_hz != 40'000'000U
            || timing.source_divider == 0U
            || timing.source_divider > 256U
            || timing.low_count < 4U || timing.high_count < 6U) {
            return std::unexpected(driver::i2c_error::invalid_rate);
        }

        detail::i2c_owner<Binding::instance>::claimed = true;
        owned_ = true;
        const auto divider = timing.source_divider - 1U;
        const auto wait_high = static_cast<std::uint32_t>(timing.high_count) / 2U - 2U;
        const auto active_high = static_cast<std::uint32_t>(timing.high_count)
            - wait_high;
        const auto setup_hold = timing.low_count < timing.high_count
            ? timing.low_count : timing.high_count;
        const auto timeout = timeout_exponent(
            static_cast<std::uint32_t>(timing.low_count) + timing.high_count);

        if (!modify(Binding::clocks.apb_gate_register,
                    Binding::clocks.apb_gate_mask, 0U)) {
            return initialization_failed();
        }
        if (!pulse_reset()) return initialization_failed();
        if (!modify(Binding::clocks.source_register,
                    1U << Binding::clocks.controller_enable_shift,
                    1U << Binding::clocks.source_select_shift)) {
            return initialization_failed();
        }
        if (!modify(Binding::clocks.divider_register,
                    divider << Binding::clocks.divider_shift,
                    0xFFU << Binding::clocks.divider_shift)) {
            return initialization_failed();
        }
        if (!configure_pin(Binding::scl_pin, Binding::scl_matrix_signal)) {
            return initialization_failed();
        }
        if (!configure_pin(Binding::sda_pin, Binding::sda_matrix_signal)) {
            return initialization_failed();
        }
        if (!write(reg_interrupt_enable, 0U)) return initialization_failed();
        if (!write(reg_interrupt_clear, all_interrupts)) {
            return initialization_failed();
        }
        if (!write(reg_scl_low, timing.low_count - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_scl_high, active_high | (wait_high << 9U))) {
            return initialization_failed();
        }
        if (!write(reg_sda_hold, timing.low_count / 4U - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_sda_sample, timing.high_count / 2U - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_start_hold, setup_hold - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_restart_setup, setup_hold - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_stop_hold, setup_hold - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_stop_setup, setup_hold - 1U)) {
            return initialization_failed();
        }
        if (!write(reg_timeout, timeout | (1U << 5U))) {
            return initialization_failed();
        }
        if (!modify(Binding::base + reg_fifo_configuration,
                    fifo_pointer_enable, nonfifo_enable | fifo_reset_mask)) {
            return initialization_failed();
        }
        if (!reset_fifos()) return initialization_failed();
        if (!configure_controller()) return initialization_failed();
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result write(
        driver::i2c_address address,
        std::span<const std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure(driver::i2c_error::empty_transfer);
        return begin(address, bytes, {}, operation_kind::write);
    }

    [[nodiscard]] start_result read(
        driver::i2c_address address,
        std::span<std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure(driver::i2c_error::empty_transfer);
        return begin(address, {}, bytes, operation_kind::read);
    }

    [[nodiscard]] start_result write_read(
        driver::i2c_address address,
        std::span<const std::uint8_t> output,
        std::span<std::uint8_t> input) noexcept {
        if (output.empty() || input.empty()) {
            return failure(driver::i2c_error::empty_transfer);
        }
        return begin(address, output, input, operation_kind::write_read);
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        if (!id_) return false;
        const auto interrupts = read(reg_interrupt_status);
        if (!interrupts) return finish_error(driver::i2c_error::mmio);
        if ((*interrupts & int_arbitration_lost) != 0U) {
            (void)write(reg_interrupt_clear, int_arbitration_lost);
            return finish_error(driver::i2c_error::arbitration_lost);
        }
        if ((*interrupts & (int_timeout | int_scl_stretch_timeout)) != 0U) {
            (void)write(reg_interrupt_clear,
                        *interrupts & (int_timeout | int_scl_stretch_timeout));
            return finish_error(driver::i2c_error::timeout);
        }
        if ((*interrupts & int_nack) != 0U) {
            const auto error = classify_nack();
            (void)write(reg_interrupt_clear, int_nack);
            return finish_error(error);
        }
        if ((*interrupts & int_complete) == 0U) return false;

        (void)write(reg_interrupt_clear, int_complete);
        if (!set_interrupts(0U, transfer_interrupts)) {
            return finish_error(driver::i2c_error::mmio);
        }
        if (!input_.empty()) {
            const auto status = read(reg_status);
            if (!status) return finish_error(driver::i2c_error::mmio);
            const auto available = static_cast<std::size_t>(
                (*status >> 8U) & 0x3FU);
            if (available < input_.size()) {
                return finish_error(driver::i2c_error::transport);
            }
            for (std::size_t index = 0U; index < input_.size(); ++index) {
                const auto value = read(reg_data);
                if (!value) return finish_error(driver::i2c_error::mmio);
                input_[index] = static_cast<std::uint8_t>(*value);
            }
        }

        const auto current = *id_;
        const driver::i2c_transfer_result result{output_.size(), input_.size()};
        reset_state();
        return operations_.complete(current, result);
    }

    [[nodiscard]] bool handle_event_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_error_interrupt() noexcept {
        return handle_interrupt();
    }

    [[nodiscard]] std::expected<void, driver::i2c_error> recover_bus(
        i2c_recovery_policy policy) noexcept {
        if (!initialized_) {
            return std::unexpected(driver::i2c_error::not_initialized);
        }
        if (id_ || policy == i2c_recovery_policy::none) {
            return std::unexpected(driver::i2c_error::busy);
        }
        if (!modify(Binding::base + reg_control, control_fsm_reset, 0U)
            || !write(reg_scl_special, (9U << 1U) | 1U)
            || !modify(Binding::base + reg_control, control_update, 0U)) {
            return mmio_failure();
        }
        for (std::size_t attempt = 0U; attempt < recovery_wait_limit;
             ++attempt) {
            const auto clearing = read(reg_scl_special);
            if (!clearing) return mmio_failure();
            if ((*clearing & 1U) == 0U) {
                const auto status = read(reg_status);
                if (!status) return mmio_failure();
                return (*status & status_bus_busy) == 0U
                    ? std::expected<void, driver::i2c_error>{}
                    : std::unexpected(driver::i2c_error::bus_stuck);
            }
        }
        return std::unexpected(driver::i2c_error::bus_stuck);
    }

    [[nodiscard]] std::expected<void, driver::i2c_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::i2c_error::not_initialized);
        }
        if (id_) return std::unexpected(driver::i2c_error::busy);
        if (!set_interrupts(0U, all_interrupts)
            || !modify(Binding::base + reg_control, 0U,
                       control_master | control_clock_enable)
            || !restore_pin(Binding::sda_pin, Binding::sda_matrix_signal)
            || !restore_pin(Binding::scl_pin, Binding::scl_matrix_signal)
            || !modify(Binding::clocks.source_register, 0U,
                       1U << Binding::clocks.controller_enable_shift)
            || !modify(Binding::clocks.apb_gate_register, 0U,
                       Binding::clocks.apb_gate_mask)) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    enum class operation_kind : std::uint8_t { write, read, write_read };

    static constexpr std::uintptr_t reg_scl_low = 0x00U;
    static constexpr std::uintptr_t reg_control = 0x04U;
    static constexpr std::uintptr_t reg_status = 0x08U;
    static constexpr std::uintptr_t reg_timeout = 0x0CU;
    static constexpr std::uintptr_t reg_fifo_configuration = 0x18U;
    static constexpr std::uintptr_t reg_data = 0x1CU;
    static constexpr std::uintptr_t reg_interrupt_clear = 0x24U;
    static constexpr std::uintptr_t reg_interrupt_enable = 0x28U;
    static constexpr std::uintptr_t reg_interrupt_status = 0x2CU;
    static constexpr std::uintptr_t reg_sda_hold = 0x30U;
    static constexpr std::uintptr_t reg_sda_sample = 0x34U;
    static constexpr std::uintptr_t reg_scl_high = 0x38U;
    static constexpr std::uintptr_t reg_start_hold = 0x40U;
    static constexpr std::uintptr_t reg_restart_setup = 0x44U;
    static constexpr std::uintptr_t reg_stop_hold = 0x48U;
    static constexpr std::uintptr_t reg_stop_setup = 0x4CU;
    static constexpr std::uintptr_t reg_command0 = 0x58U;
    static constexpr std::uintptr_t reg_scl_special = 0x80U;

    static constexpr std::uint32_t control_master = 1U << 4U;
    static constexpr std::uint32_t control_start = 1U << 5U;
    static constexpr std::uint32_t control_clock_enable = 1U << 8U;
    static constexpr std::uint32_t control_arbitration = 1U << 9U;
    static constexpr std::uint32_t control_fsm_reset = 1U << 10U;
    static constexpr std::uint32_t control_update = 1U << 11U;
    static constexpr std::uint32_t status_bus_busy = 1U << 4U;
    static constexpr std::uint32_t fifo_reset_mask = (1U << 12U)
        | (1U << 13U);
    static constexpr std::uint32_t fifo_pointer_enable = 1U << 14U;
    static constexpr std::uint32_t nonfifo_enable = 1U << 10U;
    static constexpr std::uint32_t int_arbitration_lost = 1U << 5U;
    static constexpr std::uint32_t int_complete = 1U << 7U;
    static constexpr std::uint32_t int_timeout = 1U << 8U;
    static constexpr std::uint32_t int_nack = 1U << 10U;
    static constexpr std::uint32_t int_scl_stretch_timeout = 1U << 13U;
    static constexpr std::uint32_t transfer_interrupts = int_arbitration_lost
        | int_complete | int_timeout | int_nack | int_scl_stretch_timeout;
    static constexpr std::uint32_t all_interrupts = 0x0007'FFFFU;
    static constexpr std::uint32_t command_done = 1U << 31U;
    static constexpr std::size_t fifo_capacity = 32U;
    static constexpr std::size_t command_count = 8U;
    static constexpr std::size_t recovery_wait_limit = 4096U;
    static constexpr std::uint8_t no_command = 0xFFU;

    [[nodiscard]] start_result begin(
        driver::i2c_address address,
        std::span<const std::uint8_t> output,
        std::span<std::uint8_t> input,
        operation_kind operation) noexcept {
        if (!initialized_) return failure(driver::i2c_error::not_initialized);
        if (id_) return failure(driver::i2c_error::busy);
        if (address.value < 0x08U || address.value > 0x77U) {
            return failure(driver::i2c_error::invalid_address);
        }
        const auto tx_bytes = output.size()
            + (operation == operation_kind::write_read ? 2U : 1U);
        if (tx_bytes > fifo_capacity || input.size() > fifo_capacity) {
            return failure(driver::i2c_error::excessive_length);
        }
        const auto status = read(reg_status);
        if (!status) return failure(driver::i2c_error::mmio);
        if ((*status & status_bus_busy) != 0U) {
            return failure(driver::i2c_error::bus_stuck);
        }

        auto started = operations_.try_start({.quiesce = &cancel,
                                               .context = this});
        if (!started) return std::unexpected(started.error());
        id_ = started->id();
        output_ = output;
        input_ = input;
        operation_ = operation;
        first_address_command_ = no_command;
        second_address_command_ = no_command;

        if (!prepare_transaction(address) || !set_interrupts(
                transfer_interrupts, all_interrupts & ~transfer_interrupts)
            || !modify(Binding::base + reg_control,
                       control_start | control_update, 0U)) {
            (void)finish_error(driver::i2c_error::mmio);
        }
        return started;
    }

    [[nodiscard]] bool prepare_transaction(driver::i2c_address address) noexcept {
        if (!reset_fifos() || !write(reg_interrupt_clear, all_interrupts)) {
            return false;
        }
        const auto write_address = static_cast<std::uint8_t>(address.value << 1U);
        const auto read_address = static_cast<std::uint8_t>(write_address | 1U);
        std::array<std::uint32_t, command_count> commands{};
        commands.fill(command(command_end));
        std::size_t next{};
        commands[next++] = command(command_restart);
        first_address_command_ = static_cast<std::uint8_t>(next);
        commands[next++] = command(command_write, 1U, true);

        if (!write(reg_data, operation_ == operation_kind::read
                                ? read_address : write_address)) {
            return false;
        }
        if (operation_ != operation_kind::read) {
            commands[next++] = command(command_write, output_.size(), true);
            for (const auto byte : output_) {
                if (!write(reg_data, byte)) return false;
            }
        }
        if (operation_ == operation_kind::write_read) {
            commands[next++] = command(command_restart);
            second_address_command_ = static_cast<std::uint8_t>(next);
            commands[next++] = command(command_write, 1U, true);
            if (!write(reg_data, read_address)) return false;
        }
        if (operation_ != operation_kind::write) {
            if (input_.size() > 1U) {
                commands[next++] = command(command_read, input_.size() - 1U,
                                           false, false);
            }
            commands[next++] = command(command_read, 1U, false, true);
        }
        commands[next++] = command(command_stop);
        if (next > command_count) return false;
        for (std::size_t index = 0U; index < command_count; ++index) {
            if (!write(reg_command0 + index * 4U, commands[index])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] driver::i2c_error classify_nack() noexcept {
        if (operation_ == operation_kind::read) {
            return driver::i2c_error::address_nack;
        }
        const auto status = read(reg_status);
        if (status) {
            const auto last_state = (*status >> 24U) & 0x7U;
            if (last_state == 1U || last_state == 2U) {
                return driver::i2c_error::address_nack;
            }
        }
        if (first_address_command_ != no_command) {
            const auto first = read(reg_command0
                + static_cast<std::uintptr_t>(first_address_command_) * 4U);
            if (!first || (*first & command_done) == 0U) {
                return driver::i2c_error::address_nack;
            }
        }
        if (second_address_command_ != no_command) {
            const auto second = read(reg_command0
                + static_cast<std::uintptr_t>(second_address_command_) * 4U);
            if (!second || (*second & command_done) == 0U) {
                return driver::i2c_error::address_nack;
            }
        }
        return driver::i2c_error::data_nack;
    }

    [[nodiscard]] bool configure_controller() noexcept {
        const auto current = read(reg_control);
        if (!current) return false;
        constexpr std::uint32_t controlled = 0x7FFFU;
        const auto next = static_cast<std::uint32_t>(
            (*current & ~controlled) | control_master
            | control_clock_enable | control_arbitration | control_update);
        return write(reg_control, next);
    }

    [[nodiscard]] bool configure_pin(core::pin_id pin,
                                     std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto pin_control = pin_control_address(pin);
        const auto output_select = output_select_address(pin);
        const auto input_select = input_select_address(signal);
        const auto current_pad = read_absolute(pad);
        const auto current_pin = read_absolute(pin_control);
        const auto current_output = read_absolute(output_select);
        const auto current_input = read_absolute(input_select);
        if (!current_pad || !current_pin || !current_output || !current_input) {
            return false;
        }
        constexpr std::uint32_t pad_controlled = (1U << 7U) | (1U << 8U)
            | (1U << 9U) | (0x3U << 10U) | (0x7U << 12U);
        const auto next_pad = static_cast<std::uint32_t>(
            (*current_pad & ~pad_controlled) | (1U << 8U) | (1U << 9U)
            | (2U << 10U) | (1U << 12U));
        const auto next_pin = static_cast<std::uint32_t>(*current_pin
                                                         | (1U << 2U));
        const auto next_output = static_cast<std::uint32_t>(
            (*current_output & ~0x7FFU) | signal);
        const auto next_input = static_cast<std::uint32_t>(
            (*current_input & ~0xFFU) | pin.index | (1U << 7U));
        return write_absolute(output_set_address(pin), pin_mask(pin))
            && write_absolute(pin_control, next_pin)
            && write_absolute(output_select, next_output)
            && write_absolute(input_select, next_input)
            && write_absolute(pad, next_pad);
    }

    [[nodiscard]] bool restore_pin(core::pin_id pin,
                                   std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto pin_control = pin_control_address(pin);
        const auto output_select = output_select_address(pin);
        const auto input_select = input_select_address(signal);
        const auto current_pad = read_absolute(pad);
        const auto current_pin = read_absolute(pin_control);
        const auto current_output = read_absolute(output_select);
        const auto current_input = read_absolute(input_select);
        if (!current_pad || !current_pin || !current_output || !current_input) {
            return false;
        }
        constexpr std::uint32_t pad_controlled = (1U << 7U) | (1U << 8U)
            | (1U << 9U) | (0x7U << 12U);
        const auto next_pad = static_cast<std::uint32_t>(
            (*current_pad & ~pad_controlled) | (1U << 9U) | (1U << 12U));
        return write_absolute(pin_control, *current_pin & ~(1U << 2U))
            && write_absolute(output_select,
                              (*current_output & ~0x7FFU) | 256U | (1U << 10U))
            && write_absolute(input_select,
                              (*current_input & ~0xFFU) | 0x3EU | (1U << 7U))
            && write_absolute(pad, next_pad);
    }

    [[nodiscard]] bool reset_fifos() noexcept {
        const auto current = read(reg_fifo_configuration);
        return current
            && write(reg_fifo_configuration, *current | fifo_reset_mask)
            && write(reg_fifo_configuration, *current & ~fifo_reset_mask);
    }

    [[nodiscard]] bool pulse_reset() noexcept {
        return modify(Binding::clocks.reset_register,
                      Binding::clocks.reset_mask, 0U)
            && modify(Binding::clocks.reset_register, 0U,
                      Binding::clocks.reset_mask);
    }

    [[nodiscard]] std::expected<void, driver::i2c_error>
    initialization_failed() noexcept {
        (void)restore_pin(Binding::sda_pin, Binding::sda_matrix_signal);
        (void)restore_pin(Binding::scl_pin, Binding::scl_matrix_signal);
        release_owner();
        return mmio_failure();
    }

    [[nodiscard]] bool set_interrupts(std::uint32_t set,
                                      std::uint32_t clear) noexcept {
        const auto current = read(reg_interrupt_enable);
        return current && write(reg_interrupt_enable,
                                (*current | set) & ~clear);
    }

    [[nodiscard]] static core::quiesce_result cancel(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<i2c_backend*>(context);
        if (!self.id_ || *self.id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.quiesce_hardware();
        self.reset_state();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool quiesce_hardware() noexcept {
        return set_interrupts(0U, transfer_interrupts)
            && modify(Binding::base + reg_control, control_fsm_reset, 0U)
            && reset_fifos()
            && write(reg_interrupt_clear, all_interrupts);
    }

    [[nodiscard]] bool finish_error(driver::i2c_error error) noexcept {
        if (!id_) return false;
        const bool quiesced = quiesce_hardware();
        const auto current = *id_;
        reset_state();
        return operations_.fail(current, quiesced ? error
                                                  : driver::i2c_error::mmio);
    }

    void reset_state() noexcept {
        id_.reset();
        output_ = {};
        input_ = {};
        first_address_command_ = no_command;
        second_address_command_ = no_command;
    }

    [[nodiscard]] static constexpr std::uint32_t command(
        std::uint32_t opcode, std::size_t count = 0U,
        bool check_ack = false, bool nack = false) noexcept {
        return static_cast<std::uint32_t>(count)
            | (check_ack ? 1U << 8U : 0U)
            | (nack ? 1U << 10U : 0U)
            | (opcode << 11U);
    }

    [[nodiscard]] static constexpr std::uint32_t timeout_exponent(
        std::uint32_t period) noexcept {
        auto value = period * 5U;
        std::uint32_t exponent{};
        while (value > 1U) {
            value >>= 1U;
            ++exponent;
        }
        exponent += 2U;
        return exponent < 32U ? exponent : 31U;
    }

    [[nodiscard]] static constexpr std::uintptr_t pad_address(
        core::pin_id pin) noexcept {
        return mcu::esp32p4::io_mux_base + 0x04U
            + static_cast<std::uintptr_t>(pin.index) * 4U;
    }
    [[nodiscard]] static constexpr std::uintptr_t pin_control_address(
        core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x74U
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
    [[nodiscard]] static constexpr std::uintptr_t output_set_address(
        core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base
            + (pin.index < 32U ? 0x08U : 0x14U);
    }
    [[nodiscard]] static constexpr std::uint32_t pin_mask(
        core::pin_id pin) noexcept {
        return 1U << (pin.index % 32U);
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

    void release_owner() noexcept {
        if (owned_) {
            detail::i2c_owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    [[nodiscard]] static start_result failure(
        driver::i2c_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation,
            .cause = error});
    }
    [[nodiscard]] static std::expected<void, driver::i2c_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::i2c_error::mmio);
    }

    static constexpr std::uint32_t command_write = 1U;
    static constexpr std::uint32_t command_stop = 2U;
    static constexpr std::uint32_t command_read = 3U;
    static constexpr std::uint32_t command_end = 4U;
    static constexpr std::uint32_t command_restart = 6U;

    MmioPolicy& mmio_;
    pool_type operations_;
    std::optional<core::operation_id> id_{};
    std::span<const std::uint8_t> output_{};
    std::span<std::uint8_t> input_{};
    operation_kind operation_{operation_kind::write};
    std::uint8_t first_address_command_{no_command};
    std::uint8_t second_address_command_{no_command};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::esp32p4
