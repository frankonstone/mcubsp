#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/backend/stm32h753/detail.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/pwm.hpp>

namespace mcuhw::backend::stm32h753 {

struct pwm_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t prescaler;
    std::uint8_t counter_width;
    std::uint64_t period_counts;

    std::uint32_t period_register;
    [[nodiscard]] constexpr operator driver::pwm_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm, prescaler, counter_width, period_counts};
    }
    friend constexpr bool operator==(pwm_timing, pwm_timing) = default;
};

namespace detail {

[[nodiscard]] constexpr mcu::stm32h753::timer_instance timer(
    std::uint8_t number) noexcept {
    for (const auto& candidate : mcu::stm32h753::timer_instances) {
        if (candidate.instance == number) {
            return candidate;
        }
    }
    return {};
}

[[nodiscard]] constexpr mcu::stm32h753::signal timer_signal(
    std::uint8_t instance, mcu::stm32h753::timer_output output) noexcept {
    if (instance == 1U) {
        return static_cast<mcu::stm32h753::signal>(
            static_cast<std::uint16_t>(mcu::stm32h753::signal::tim1_ch1)
            + static_cast<std::uint8_t>(output));
    }
    return mcu::stm32h753::signal::tim2_ch1;
}

template<std::uint8_t Instance>
struct timer_owner {
    static inline bool claimed{};
};

} // namespace detail

// STM32 timers accept every integer prescaler from 1 through 65536.
// The native configuration retains the full prescaler and period register.
// @req REQ-0004 REQ-0011 REQ-0022
[[nodiscard]] constexpr std::expected<pwm_timing, driver::pwm_error>
solve_timer_pwm(driver::pwm_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::pwm_error::invalid_clock);
    }
    if (request.target_hz == 0U) {
        return std::unexpected(driver::pwm_error::invalid_frequency);
    }
    if (request.counter_width != 16U && request.counter_width != 32U) {
        return std::unexpected(driver::pwm_error::invalid_counter_width);
    }
    const auto maximum_counts = request.counter_width == 32U
        ? (std::uint64_t{1U} << 32U)
        : (std::uint64_t{1U} << 16U);
    bool found{};
    pwm_timing best{};
    for (std::uint32_t prescaler = 1U; prescaler <= 65'536U; ++prescaler) {
        const auto denominator =
            static_cast<std::uint64_t>(request.target_hz) * prescaler;
        const auto counts = driver::detail::rounded_quotient(
            request.clock_hz, denominator);
        if (counts < 2U || counts > maximum_counts) {
            continue;
        }
        const auto actual_denominator =
            static_cast<std::uint64_t>(prescaler) * counts;
        const auto target_product =
            static_cast<std::uint64_t>(request.target_hz)
            * actual_denominator;
        const auto difference = target_product > request.clock_hz
            ? target_product - request.clock_hz
            : request.clock_hz - target_product;
        const pwm_timing candidate{
            .clock_hz = request.clock_hz,
            .target_hz = request.target_hz,
            .achieved_hz = static_cast<std::uint32_t>(
                driver::detail::rounded_quotient(request.clock_hz,
                                                 actual_denominator)),
            .error_ppm = driver::detail::rounded_ppm(difference,
                                                     target_product),
            .prescaler = prescaler,
            .counter_width = request.counter_width,
            .period_counts = counts,
            .period_register = static_cast<std::uint32_t>(counts - 1U),
        };
        if (!found || candidate.error_ppm < best.error_ppm
            || (candidate.error_ppm == best.error_ppm
                && candidate.prescaler < best.prescaler)) {
            best = candidate;
            found = true;
        }
        if (best.error_ppm == 0U) {
            break;
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::pwm_error::unattainable_tolerance);
    }
    return best;
}

template<std::uint8_t Instance, mcu::stm32h753::timer_output Output,
         core::pin_id Pin>
struct timer_binding {
    static constexpr auto timer_metadata = detail::timer(Instance);
    static constexpr auto metadata = detail::instance(
        mcu::stm32h753::peripheral_kind::timer, Instance);
    static constexpr auto signal = detail::timer_signal(Instance, Output);
    static_assert(timer_metadata.instance == Instance);
    static_assert(Output == mcu::stm32h753::timer_output::ch1,
                  "initial STM32 PWM backend supports channel 1 outputs");
    static_assert(core::route_exists(mcu::stm32h753::pin_routes, Pin,
                                     mcu::stm32h753::signal_id(signal)));
    static_assert(mcu::stm32h753::package_contains(
        mcu::stm32h753::package_variant::lqfp144, Pin,
        mcu::stm32h753::package_memberships));
    static constexpr std::uint8_t instance = Instance;
    static constexpr auto output = Output;
    static constexpr core::pin_id pin = Pin;
    static constexpr std::uint32_t af =
        detail::alternate_function(Pin, signal);
};

// Edge-aligned, active-high PWM with preload. ARR and CCR1 are transferred by
// the generated update event; stopped channels are forced low through GPIO.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0004 REQ-0009 REQ-0013 REQ-0014 REQ-0022 REQ-0033
class timer_pwm_backend {
public:
    using transfer_pool = core::operation_pool<void, driver::pwm_error, 1,
                                               ExecutionPolicy>;
    using period_transfer = typename transfer_pool::transfer_type;
    using transfer_error = typename transfer_pool::error_type;
    using period_start_result = std::expected<period_transfer, transfer_error>;

    timer_pwm_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), period_events_(execution) {}

    [[nodiscard]] std::expected<void, driver::pwm_error> claim() noexcept {
        if (detail::timer_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::pwm_error::already_owned);
        }
        detail::timer_owner<Binding::instance>::claimed = true;
        owned_ = true;
        return {};
    }

    void release() noexcept {
        if (owned_) {
            detail::timer_owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> configure(
        pwm_timing timing, driver::pwm_compare compare) noexcept {
        if (!owned_) {
            return std::unexpected(driver::pwm_error::not_initialized);
        }
        if (timing.counter_width != Binding::timer_metadata.counter_width
            || timing.prescaler == 0U || timing.prescaler > 65'536U
            || timing.period_counts < 2U || timing.period_register > mask()) {
            return std::unexpected(driver::pwm_error::invalid_frequency);
        }
        detail::pinmux<MmioPolicy> pins{mmio_};
        if (!detail::enable_peripheral_clock(mmio_, Binding::metadata)
            || !pins.alternate(Binding::pin, Binding::af)) {
            return mmio_failure();
        }
        timing_ = timing;
        compare_ = compare;
        const auto ccmr = mode(compare) | (1U << 3U); // OC1PE
        const bool configured =
            write(0x00U, 1U << 7U) // ARPE
            && write(0x0CU, 0U)
            && write(0x18U, ccmr)
            && write(0x20U, 1U) // CC1E
            && write(0x28U, timing.prescaler - 1U)
            && write(0x2CU, timing.period_register)
            && write(0x34U, compare_value(compare))
            && (!advanced() || write(0x44U, 1U << 15U)) // MOE
            && write(0x14U, 1U); // UG transfers PSC/ARR/CCR
        if (!configured) {
            return mmio_failure();
        }
        configured_ = true;
        return {};
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> start() noexcept {
        if (!configured_ || !write(0x18U, mode(compare_) | (1U << 3U))
            || !write(0x14U, 1U) || !write(0x20U, 1U)
            || !set_cr1(true)) {
            return mmio_failure();
        }
        running_ = true;
        arch::data_synchronization_barrier();
        return {};
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> stop() noexcept {
        if (!configured_ || !write(0x18U, (4U << 4U) | (1U << 3U))
            || !write(0x14U, 1U) || !set_cr1(false)) {
            return mmio_failure();
        }
        running_ = false;
        return {};
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> update_duty(
        driver::pwm_compare compare, bool running) noexcept {
        if (!configured_) {
            return std::unexpected(driver::pwm_error::not_initialized);
        }
        compare_ = compare;
        const bool updated = write(0x34U, compare_value(compare))
            && write(0x18U, mode(compare) | (1U << 3U))
            && (running || write(0x14U, 1U));
        return updated ? success() : mmio_failure();
    }

    [[nodiscard]] period_start_result next_period() noexcept {
        auto result = period_events_.try_start(
            {.quiesce = &cancel_period, .context = this});
        if (result) {
            pending_period_ = result->id();
            if (!update_bits(0x0CU, 1U, true)) {
                const auto id = *pending_period_;
                pending_period_.reset();
                (void)period_events_.fail(id, driver::pwm_error::mmio);
            }
        }
        return result;
    }

    [[nodiscard]] bool handle_update_interrupt() noexcept {
        const auto status = read(0x10U);
        if (!status || (*status & 1U) == 0U || !write(0x10U, 0x1FFFEU)) {
            return false;
        }
        if (!pending_period_) {
            return true;
        }
        const auto id = *pending_period_;
        pending_period_.reset();
        (void)update_bits(0x0CU, 1U, false);
        return period_events_.complete(id);
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> shutdown() noexcept {
        detail::pinmux<MmioPolicy> pins{mmio_};
        if ((running_ && !set_cr1(false)) || !write(0x20U, 0U)
            || !pins.output(Binding::pin, false)) {
            return mmio_failure();
        }
        configured_ = false;
        running_ = false;
        return {};
    }

private:
    [[nodiscard]] static core::quiesce_result cancel_period(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<timer_pwm_backend*>(context);
        if (self.pending_period_ && *self.pending_period_ == id) {
            self.pending_period_.reset();
            (void)self.update_bits(0x0CU, 1U, false);
        }
        return core::quiesce_result::reusable;
    }

    [[nodiscard]] static constexpr std::uint32_t mask() noexcept {
        return Binding::timer_metadata.counter_width == 32U
            ? 0xFFFF'FFFFU : 0xFFFFU;
    }
    [[nodiscard]] static constexpr bool advanced() noexcept {
        return Binding::timer_metadata.dead_time;
    }
    [[nodiscard]] static constexpr std::uint32_t mode(
        driver::pwm_compare compare) noexcept {
        if (compare.mode == driver::pwm_duty_mode::force_low) return 4U << 4U;
        if (compare.mode == driver::pwm_duty_mode::force_high) return 5U << 4U;
        return 6U << 4U;
    }
    [[nodiscard]] std::uint32_t compare_value(
        driver::pwm_compare compare) const noexcept {
        if (compare.mode == driver::pwm_duty_mode::force_low) {
            return 0U;
        }
        if (compare.mode == driver::pwm_duty_mode::force_high) {
            return timing_.period_register == mask()
                ? timing_.period_register : timing_.period_register + 1U;
        }
        return compare.compare_counts;
    }
    [[nodiscard]] bool set_cr1(bool enable) noexcept {
        return update_bits(0x00U, 1U, enable);
    }
    [[nodiscard]] bool update_bits(std::uintptr_t offset, std::uint32_t bits,
                                   bool set) noexcept {
        const auto current = read(offset);
        return current && write(offset, set ? (*current | bits)
                                            : (*current & ~bits));
    }
    [[nodiscard]] auto read(std::uintptr_t offset) noexcept {
        return mmio_.template read<std::uint32_t>(Binding::metadata.base
                                                  + offset);
    }
    [[nodiscard]] bool write(std::uintptr_t offset,
                             std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(Binding::metadata.base
                                                   + offset, value).has_value();
    }
    [[nodiscard]] static std::expected<void, driver::pwm_error>
    success() noexcept { return {}; }
    [[nodiscard]] static std::expected<void, driver::pwm_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::pwm_error::mmio);
    }

    MmioPolicy& mmio_;
    transfer_pool period_events_;
    std::optional<core::operation_id> pending_period_{};
    pwm_timing timing_{};
    driver::pwm_compare compare_{};
    bool owned_{};
    bool configured_{};
    bool running_{};
};

} // namespace mcuhw::backend::stm32h753
