#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/core/metadata.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/backend/ra4m1/timing/pwm.hpp>
#include <mcubsp/mcu/ra4m1/peripherals.hpp>

namespace mcubsp::backend::ra4m1 {

namespace detail {

template<std::uint8_t Instance>
struct gpt_owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::signal gpt_signal(
    std::uint8_t instance, mcu::ra4m1::gpt_output output) noexcept {
    const auto first = static_cast<std::uint16_t>(
        mcu::ra4m1::signal::gpt0_a);
    return static_cast<mcu::ra4m1::signal>(
        first + instance * 2U
        + (output == mcu::ra4m1::gpt_output::b ? 1U : 0U));
}

} // namespace detail

// A binding identifies one real GPT output and one package pin at compile
// time. It cannot represent a third output and rejects pins absent from the
// source-backed RA4M1 route table.
template<std::uint8_t Instance, mcu::ra4m1::gpt_output Output,
         core::pin_id Pin>
struct gpt_binding {
    static_assert(Instance < mcu::ra4m1::gpt_instances.size());
    static_assert(Output == mcu::ra4m1::gpt_output::a
                  || Output == mcu::ra4m1::gpt_output::b, "GPT output must be A or B");
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, Pin,
        mcu::ra4m1::signal_id(detail::gpt_signal(Instance, Output))));

    static constexpr std::uint8_t instance = Instance;
    static constexpr mcu::ra4m1::gpt_output output = Output;
    static constexpr core::pin_id pin = Pin;
    static constexpr auto metadata = mcu::ra4m1::gpt_instances[Instance];
};

// RA4M1 GPT saw-wave PWM backend. Compare writes go to GTCCRC/GTCCRE and
// transfer to GTCCRA/GTCCRB only at overflow. Endpoint duty changes use the
// dedicated GTUDDTYC mechanism, which is likewise synchronized at overflow
// while running. Count stop always drives the selected output low.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0004 REQ-0009 REQ-0011 REQ-0014 REQ-0022
class gpt_pwm_backend {
public:
    using transfer_pool = core::operation_pool<void, driver::pwm_error, 1,
                                               ExecutionPolicy>;
    using period_transfer = typename transfer_pool::transfer_type;
    using transfer_error = typename transfer_pool::error_type;
    using period_start_result =
        std::expected<period_transfer, transfer_error>;

    explicit gpt_pwm_backend(MmioPolicy& mmio,
                             ExecutionPolicy& execution) noexcept
        : mmio_(mmio), period_events_(execution) {}

    [[nodiscard]] std::expected<void, driver::pwm_error> claim() noexcept {
        if (detail::gpt_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::pwm_error::already_owned);
        }
        detail::gpt_owner<Binding::instance>::claimed = true;
        owned_ = true;
        return {};
    }

    void release() noexcept {
        if (owned_) {
            detail::gpt_owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    [[nodiscard]] std::expected<void, driver::pwm_error>
    configure(pwm_timing timing,
              driver::pwm_compare compare) noexcept {
        if (!owned_) {
            return std::unexpected(driver::pwm_error::not_initialized);
        }
        if (timing.counter_width != Binding::metadata.counter_width
            || timing.prescaler_encoding > 5U
            || timing.period_counts < 2U
            || timing.period_register > counter_mask()) {
            return std::unexpected(driver::pwm_error::invalid_frequency);
        }
        if (!release_module_stop() || !configure_pin()) {
            return mmio_failure();
        }
        if (!unlock_gpt()) {
            return mmio_failure();
        }

        gtcr_stopped_ = static_cast<std::uint32_t>(
            timing.prescaler_encoding) << 24U;
        const auto active_compare = compare.mode == driver::pwm_duty_mode::compare
            ? compare.compare_counts
            : 1U;
        const bool configured =
            write<std::uint32_t>(base() + 0x2CU, gtcr_stopped_)
            && write<std::uint32_t>(base() + 0x48U, 0U)
            && write<std::uint32_t>(base() + 0x64U,
                                    timing.period_register)
            && write<std::uint32_t>(compare_register(), active_compare)
            && write<std::uint32_t>(buffer_register(), active_compare)
            && write<std::uint32_t>(base() + 0x40U, buffer_enable())
            && write<std::uint32_t>(base() + 0x34U, output_control())
            && apply_duty_control(compare, true);
        const bool locked = lock_gpt();
        if (!configured || !locked) {
            return mmio_failure();
        }
        configured_ = true;
        return {};
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> start() noexcept {
        if (!configured_ || !unlock_gpt()) {
            return mmio_failure();
        }
        const bool started = write<std::uint32_t>(base() + 0x2CU,
                                                  gtcr_stopped_ | 1U);
        const bool locked = lock_gpt();
        if (!started || !locked) {
            return mmio_failure();
        }
        running_ = true;
        arch::data_synchronization_barrier();
        return {};
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> stop() noexcept {
        if (!configured_ || !unlock_gpt()) {
            return mmio_failure();
        }
        const bool stopped = write<std::uint32_t>(base() + 0x2CU,
                                                  gtcr_stopped_);
        const bool locked = lock_gpt();
        if (!stopped || !locked) {
            return mmio_failure();
        }
        running_ = false;
        arch::data_synchronization_barrier();
        return {};
    }

    [[nodiscard]] std::expected<void, driver::pwm_error>
    update_duty(driver::pwm_compare compare, bool running) noexcept {
        if (!configured_ || !unlock_gpt()) {
            return mmio_failure();
        }
        bool updated = true;
        if (compare.mode == driver::pwm_duty_mode::compare) {
            // Never write the active compare register during operation.
            updated = write<std::uint32_t>(buffer_register(),
                                           compare.compare_counts);
            if (!running) {
                updated = updated
                    && write<std::uint32_t>(compare_register(),
                                            compare.compare_counts);
            }
        }
        updated = updated && apply_duty_control(compare, !running);
        const bool locked = lock_gpt();
        if (!updated || !locked) {
            return mmio_failure();
        }
        return {};
    }

    [[nodiscard]] period_start_result next_period() noexcept {
        auto started = period_events_.try_start(
            {.quiesce = &cancel_period, .context = this});
        if (started) {
            pending_period_ = started->id();
        }
        return started;
    }

    // Called by the target-owned GPT overflow ISR after ICU dispatch. The
    // method acknowledges TCFPO before publishing completion.
    [[nodiscard]] bool handle_overflow_interrupt() noexcept {
        const auto status = mmio_.template read<std::uint32_t>(base() + 0x3CU);
        if (!status || (*status & (1U << 6U)) == 0U) {
            return false;
        }
        if (!write<std::uint32_t>(base() + 0x3CU,
                                  *status & ~(1U << 6U))) {
            return false;
        }
        if (!pending_period_) {
            return true;
        }
        const auto id = *pending_period_;
        pending_period_.reset();
        return period_events_.complete(id);
    }

    [[nodiscard]] std::expected<void, driver::pwm_error> shutdown() noexcept {
        if (running_) {
            const auto stopped = stop();
            if (!stopped) {
                return stopped;
            }
        }
        if (!unlock_gpt()) {
            return mmio_failure();
        }
        const bool disabled = write<std::uint32_t>(base() + 0x34U, 0U);
        const bool locked = lock_gpt();
        if (!disabled || !locked || !restore_pin()) {
            return mmio_failure();
        }
        configured_ = false;
        return {};
    }

private:
    [[nodiscard]] static core::quiesce_result cancel_period(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<gpt_pwm_backend*>(context);
        if (self.pending_period_ && *self.pending_period_ == id) {
            self.pending_period_.reset();
        }
        return core::quiesce_result::reusable;
    }

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return Binding::metadata.base;
    }
    [[nodiscard]] static constexpr std::uint32_t counter_mask() noexcept {
        return Binding::metadata.counter_width == 32U ? 0xFFFF'FFFFU
                                                      : 0x0000'FFFFU;
    }
    [[nodiscard]] static constexpr std::uintptr_t
    compare_register() noexcept {
        return base() + (Binding::output == mcu::ra4m1::gpt_output::a
                             ? 0x4CU
                             : 0x50U);
    }
    [[nodiscard]] static constexpr std::uintptr_t
    buffer_register() noexcept {
        return base() + (Binding::output == mcu::ra4m1::gpt_output::a
                             ? 0x54U
                             : 0x58U);
    }
    [[nodiscard]] static constexpr std::uint32_t buffer_enable() noexcept {
        return Binding::output == mcu::ra4m1::gpt_output::a
            ? (1U << 16U)
            : (1U << 18U);
    }
    [[nodiscard]] static constexpr std::uint32_t output_control() noexcept {
        // Initial high, low at compare, high at cycle end; stopped low.
        constexpr std::uint32_t function = 0b1'10'01U;
        return Binding::output == mcu::ra4m1::gpt_output::a
            ? function | (1U << 8U)
            : (function << 16U) | (1U << 24U);
    }

    [[nodiscard]] bool apply_duty_control(driver::pwm_compare compare,
                                          bool force_now) noexcept {
        constexpr std::uint32_t count_up = 1U;
        const auto shift = Binding::output == mcu::ra4m1::gpt_output::a
            ? 16U
            : 24U;
        const auto force_bit = Binding::output == mcu::ra4m1::gpt_output::a
            ? 18U
            : 26U;
        std::uint32_t mode{};
        if (compare.mode == driver::pwm_duty_mode::force_low) {
            mode = 0b10U;
        } else if (compare.mode == driver::pwm_duty_mode::force_high) {
            mode = 0b11U;
        }
        auto control = count_up | (mode << shift);
        if (force_now) {
            if (!write<std::uint32_t>(base() + 0x30U,
                                      control | (1U << 1U)
                                          | (mode != 0U
                                                 ? (1U << force_bit)
                                                 : 0U))) {
                return false;
            }
        }
        return write<std::uint32_t>(base() + 0x30U, control);
    }

    [[nodiscard]] bool release_module_stop() noexcept {
        constexpr std::uintptr_t prcr = 0x4001'E3FEU;
        constexpr std::uintptr_t mstpcrd = 0x4004'7008U;
        const auto current = mmio_.template read<std::uint32_t>(mstpcrd);
        if (!current || !write<std::uint16_t>(prcr, 0xA502U)) {
            return false;
        }
        const auto enabled = write<std::uint32_t>(
            mstpcrd,
            *current & ~(Binding::instance < 2U ? (1U << 5U)
                                                : (1U << 6U)));
        const auto relocked = write<std::uint16_t>(prcr, 0xA500U);
        arch::data_memory_barrier();
        return enabled && relocked;
    }

    [[nodiscard]] bool configure_pin() noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        const auto address = mcu::ra4m1::pfs_address(Binding::pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 14U);
        const auto next = (*current & ~controlled) | (0x03U << 24U)
            | (1U << 16U);
        const bool configured = write<std::uint32_t>(address, next);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return configured && locked;
    }

    [[nodiscard]] bool restore_pin() noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        const auto address = mcu::ra4m1::pfs_address(Binding::pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 2U) | 1U;
        const bool restored = write<std::uint32_t>(
            address, *current & ~controlled);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return restored && locked;
    }

    [[nodiscard]] bool unlock_gpt() noexcept {
        return write<std::uint32_t>(base(), 0x0000'A500U);
    }
    [[nodiscard]] bool lock_gpt() noexcept {
        const bool result = write<std::uint32_t>(base(), 0x0000'A501U);
        arch::data_memory_barrier();
        return result;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return mmio_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] static std::expected<void, driver::pwm_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::pwm_error::mmio);
    }

    MmioPolicy& mmio_;
    transfer_pool period_events_;
    std::optional<core::operation_id> pending_period_{};
    std::uint32_t gtcr_stopped_{};
    bool owned_{};
    bool configured_{};
    bool running_{};
};

} // namespace mcubsp::backend::ra4m1
