#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcubsp/backend/stm32h753/detail.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/i2c.hpp>

namespace mcubsp::backend::stm32h753 {

struct i2c_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t rise_time_ns;
    std::uint32_t fall_time_ns;

    std::uint32_t timingr;
    [[nodiscard]] constexpr operator driver::i2c_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm, rise_time_ns, fall_time_ns};
    }
    friend constexpr bool operator==(i2c_timing, i2c_timing) = default;
};

enum class i2c_recovery_policy : std::uint8_t { none, exclusive_peripheral_reset };

namespace detail {
template<std::uint8_t Instance>
struct i2c_owner { static inline bool claimed{}; };
} // namespace detail

// Integer search for the STM32 TIMINGR fields. The search enforces the I2C
// Standard/Fast-mode high/low minima and includes configured edge times in the
// achieved wire period. Analog/digital filter delay remains disabled here.
// @req REQ-0011 REQ-0024 REQ-0033
[[nodiscard]] constexpr std::expected<i2c_timing, driver::i2c_error>
solve_stm32_i2c_timing(driver::i2c_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
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
    const auto min_low = standard ? 4'700U : 1'300U;
    const auto min_high = standard ? 4'000U : 600U;
    const auto setup = standard ? 250U : 100U;
    bool found{};
    i2c_timing best{};
    for (std::uint32_t presc = 0U; presc < 16U; ++presc) {
        const auto tick_ps = driver::i2c_detail::divide_rounded(
            static_cast<std::uint64_t>(presc + 1U) * 1'000'000'000'000ULL,
            request.clock_hz);
        if (tick_ps == 0U) continue;
        const auto scldel_counts = driver::i2c_detail::ceil_divide(
            static_cast<std::uint64_t>(request.rise_time_ns + setup) * 1000U,
            tick_ps);
        const auto sdadel_counts = driver::i2c_detail::ceil_divide(
            static_cast<std::uint64_t>(request.fall_time_ns) * 1000U,
            tick_ps);
        if (scldel_counts == 0U || scldel_counts > 16U
            || sdadel_counts > 15U) continue;
        for (std::uint32_t scll = 0U; scll < 256U; ++scll) {
            const auto low_ns = ((scll + 1U) * tick_ps) / 1000U
                + request.fall_time_ns;
            if (low_ns < min_low) continue;
            const auto target_period_ns = driver::i2c_detail::divide_rounded(
                1'000'000'000ULL, request.target_hz);
            if (low_ns + request.rise_time_ns >= target_period_ns) continue;
            const auto desired_high_clock_ns = target_period_ns - low_ns
                - request.rise_time_ns;
            const auto desired_counts = driver::i2c_detail::divide_rounded(
                desired_high_clock_ns * 1000U, tick_ps);
            const auto first_count = desired_counts > 1U
                ? desired_counts - 1U : 1U;
            const auto last_count = desired_counts < 255U
                ? desired_counts + 1U : 256U;
            for (auto high_counts = first_count;
                 high_counts <= last_count; ++high_counts) {
                if (high_counts == 0U || high_counts > 256U) continue;
                const auto sclh = high_counts - 1U;
                const auto high_ns = ((sclh + 1U) * tick_ps) / 1000U
                    + request.rise_time_ns;
                if (high_ns < min_high) continue;
                const auto period_ns = low_ns + high_ns;
                const auto achieved = static_cast<std::uint32_t>(
                    driver::i2c_detail::divide_rounded(1'000'000'000ULL,
                                                       period_ns));
                const auto delta = achieved > request.target_hz
                    ? achieved - request.target_hz
                    : request.target_hz - achieved;
                const auto error = static_cast<std::uint32_t>(
                    driver::i2c_detail::divide_rounded(
                        static_cast<std::uint64_t>(delta) * 1'000'000ULL,
                        request.target_hz));
                const auto timingr = (presc << 28U)
                    | ((scldel_counts - 1U) << 20U)
                    | (sdadel_counts << 16U) | (sclh << 8U) | scll;
                const i2c_timing candidate{
                    .clock_hz = request.clock_hz,
                    .target_hz = request.target_hz,
                    .achieved_hz = achieved,
                    .error_ppm = error,
                    .rise_time_ns = request.rise_time_ns,
                    .fall_time_ns = request.fall_time_ns,
                    .timingr = static_cast<std::uint32_t>(timingr),
                };
                if (!found || error < best.error_ppm
                    || (error == best.error_ppm && timingr < best.timingr)) {
                    best = candidate; found = true;
                }
            }
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::i2c_error::unattainable_tolerance);
    }
    return best;
}

template<std::uint8_t Instance, core::pin_id SclPin, core::pin_id SdaPin>
struct i2c_binding {
    static_assert(Instance == 1U,
                  "source-backed initial Nucleo route covers I2C1");
    static_assert(core::route_exists(
        mcu::stm32h753::pin_routes, SclPin,
        mcu::stm32h753::signal_id(mcu::stm32h753::signal::i2c1_scl)));
    static_assert(core::route_exists(
        mcu::stm32h753::pin_routes, SdaPin,
        mcu::stm32h753::signal_id(mcu::stm32h753::signal::i2c1_sda)));
    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id scl_pin = SclPin;
    static constexpr core::pin_id sda_pin = SdaPin;
    static constexpr auto metadata = detail::instance(
        mcu::stm32h753::peripheral_kind::i2c, Instance);
    static constexpr auto scl_af = detail::alternate_function(
        SclPin, mcu::stm32h753::signal::i2c1_scl);
    static constexpr auto sda_af = detail::alternate_function(
        SdaPin, mcu::stm32h753::signal::i2c1_sda);
};

// IRQ-driven controller for transfers up to the hardware's 255-byte NBYTES
// window. Combined transfers issue a repeated START without publishing an
// intermediate result.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0009 REQ-0014 REQ-0017 REQ-0019 REQ-0024 REQ-0025
class i2c_controller_backend {
public:
    using pool_type = core::operation_pool<driver::i2c_transfer_result,
                                           driver::i2c_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    i2c_controller_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::i2c_error> initialize(
        i2c_timing timing) noexcept {
        if (initialized_ || detail::i2c_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::i2c_error::already_owned);
        }
        if (timing.timingr == 0U) {
            return std::unexpected(driver::i2c_error::invalid_rate);
        }
        detail::i2c_owner<Binding::instance>::claimed = true; owned_ = true;
        detail::pinmux<MmioPolicy> pins{mmio_};
        if (!detail::enable_peripheral_clock(mmio_, Binding::metadata)
            || !pins.alternate(Binding::scl_pin, Binding::scl_af, true, 1U)
            || !pins.alternate(Binding::sda_pin, Binding::sda_af, true, 1U)
            || !write(0x00U, 0U) || !write(0x10U, timing.timingr)
            || !write(0x1CU, 0x0000'3F38U) || !write(0x00U, 1U)) {
            release_owner(); return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result write(driver::i2c_address address,
                                     std::span<const std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure(driver::i2c_error::empty_transfer);
        return begin(address, bytes, {}, kind::write);
    }
    [[nodiscard]] start_result read(driver::i2c_address address,
                                    std::span<std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure(driver::i2c_error::empty_transfer);
        return begin(address, {}, bytes, kind::read);
    }
    [[nodiscard]] start_result write_read(
        driver::i2c_address address, std::span<const std::uint8_t> output,
        std::span<std::uint8_t> input) noexcept {
        if (output.empty() || input.empty()) {
            return failure(driver::i2c_error::empty_transfer);
        }
        return begin(address, output, input, kind::write_read);
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        if (!id_) return false;
        const auto status = read_reg(0x18U);
        if (!status) return finish_error(driver::i2c_error::mmio);
        if ((*status & (1U << 9U)) != 0U) {
            (void)write(0x1CU, 1U << 9U);
            return finish_error(driver::i2c_error::arbitration_lost);
        }
        if ((*status & (1U << 8U)) != 0U) {
            (void)write(0x1CU, 1U << 8U);
            return finish_error(driver::i2c_error::transport);
        }
        if ((*status & (1U << 4U)) != 0U) {
            pending_error_ = write_index_ == 0U && read_index_ == 0U
                ? driver::i2c_error::address_nack
                : driver::i2c_error::data_nack;
            (void)write(0x1CU, 1U << 4U);
            return request_stop();
        }
        if ((*status & (1U << 1U)) != 0U && write_index_ < output_.size()) {
            return write(0x28U, output_[write_index_++])
                ? true : finish_error(driver::i2c_error::mmio);
        }
        if ((*status & (1U << 2U)) != 0U && read_index_ < input_.size()) {
            const auto value = read_reg(0x24U);
            if (!value) return finish_error(driver::i2c_error::mmio);
            input_[read_index_++] = static_cast<std::uint8_t>(*value);
            return true;
        }
        if ((*status & (1U << 6U)) != 0U && operation_kind_ == kind::write_read
            && phase_write_) {
            phase_write_ = false;
            return program_phase(true, input_.size(), true)
                ? true : finish_error(driver::i2c_error::mmio);
        }
        if ((*status & (1U << 5U)) != 0U) {
            (void)write(0x1CU, 1U << 5U);
            if (pending_error_) return finish_error(*pending_error_);
            const auto current = *id_;
            const driver::i2c_transfer_result result{write_index_, read_index_};
            reset();
            return operations_.complete(current, result);
        }
        return false;
    }
    [[nodiscard]] bool handle_transmit_empty_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_transmit_end_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_error_interrupt() noexcept {
        return handle_interrupt();
    }

    [[nodiscard]] std::expected<void, driver::i2c_error> recover_bus(
        i2c_recovery_policy policy) noexcept {
        if (!initialized_) return std::unexpected(driver::i2c_error::not_initialized);
        if (id_ || policy == i2c_recovery_policy::none) {
            return std::unexpected(driver::i2c_error::busy);
        }
        if (!write(0x00U, 0U) || !write(0x1CU, 0x0000'3F38U)
            || !write(0x00U, 1U)) return mmio_failure();
        const auto status = read_reg(0x18U);
        if (!status) return mmio_failure();
        return (*status & (1U << 15U)) == 0U
            ? std::expected<void, driver::i2c_error>{}
            : std::unexpected(driver::i2c_error::bus_stuck);
    }

    [[nodiscard]] std::expected<void, driver::i2c_error> shutdown() noexcept {
        if (!initialized_) return std::unexpected(driver::i2c_error::not_initialized);
        if (id_) return std::unexpected(driver::i2c_error::busy);
        detail::pinmux<MmioPolicy> pins{mmio_};
        if (!write(0x00U, 0U) || !pins.restore(Binding::scl_pin)
            || !pins.restore(Binding::sda_pin)) return mmio_failure();
        initialized_ = false; release_owner(); return {};
    }

private:
    enum class kind : std::uint8_t { write, read, write_read };
    static constexpr std::uint32_t interrupt_mask = 0xF6U;

    [[nodiscard]] start_result begin(driver::i2c_address address,
        std::span<const std::uint8_t> output, std::span<std::uint8_t> input,
        kind operation) noexcept {
        if (!initialized_) return failure(driver::i2c_error::not_initialized);
        if (id_) return failure(driver::i2c_error::busy);
        if (output.size() > 255U || input.size() > 255U) {
            return failure(driver::i2c_error::excessive_length);
        }
        const auto status = read_reg(0x18U);
        if (!status) return failure(driver::i2c_error::mmio);
        if ((*status & (1U << 15U)) != 0U) return failure(driver::i2c_error::bus_stuck);
        auto started = operations_.try_start({.quiesce = &cancel, .context = this});
        if (!started) return std::unexpected(started.error());
        id_ = started->id(); address_ = address; output_ = output; input_ = input;
        write_index_ = 0U; read_index_ = 0U; operation_kind_ = operation;
        phase_write_ = operation != kind::read; pending_error_.reset();
        const auto count = phase_write_ ? output.size() : input.size();
        if (!write(0x1CU, 0x0000'3F38U)
            || !update_cr1(interrupt_mask, 0U)
            || !program_phase(!phase_write_, count, operation != kind::write_read)) {
            (void)finish_error(driver::i2c_error::mmio);
        }
        return started;
    }
    [[nodiscard]] bool program_phase(bool read_phase, std::size_t count,
                                     bool autoend) noexcept {
        std::uint32_t cr2 = static_cast<std::uint32_t>(address_.value) << 1U;
        cr2 |= static_cast<std::uint32_t>(count) << 16U;
        if (read_phase) cr2 |= 1U << 10U;
        if (autoend) cr2 |= 1U << 25U;
        cr2 |= 1U << 13U;
        return write(0x04U, cr2);
    }
    [[nodiscard]] bool request_stop() noexcept {
        const auto control = read_reg(0x04U);
        return control && write(0x04U, *control | (1U << 14U));
    }
    [[nodiscard]] static core::quiesce_result cancel(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<i2c_controller_backend*>(context);
        if (!self.id_ || *self.id_ != id) return core::quiesce_result::reusable;
        const bool ok = self.update_cr1(0U, interrupt_mask)
            && self.write(0x04U, 1U << 14U);
        self.reset();
        return ok ? core::quiesce_result::reusable : core::quiesce_result::faulted;
    }
    [[nodiscard]] bool finish_error(driver::i2c_error error) noexcept {
        if (!id_) return false;
        (void)update_cr1(0U, interrupt_mask);
        const auto current = *id_; reset();
        return operations_.fail(current, error);
    }
    void reset() noexcept {
        (void)update_cr1(0U, interrupt_mask);
        id_.reset(); output_ = {}; input_ = {}; pending_error_.reset();
    }
    [[nodiscard]] bool update_cr1(std::uint32_t set, std::uint32_t clear) noexcept {
        const auto current = read_reg(0x00U);
        return current && write(0x00U, (*current | set) & ~clear);
    }
    [[nodiscard]] auto read_reg(std::uintptr_t offset) noexcept {
        return mmio_.template read<std::uint32_t>(Binding::metadata.base + offset);
    }
    [[nodiscard]] bool write(std::uintptr_t offset, std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(Binding::metadata.base + offset,
                                                   value).has_value();
    }
    void release_owner() noexcept {
        if (owned_) { detail::i2c_owner<Binding::instance>::claimed = false; owned_ = false; }
    }
    [[nodiscard]] static start_result failure(driver::i2c_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation, .cause = error});
    }
    [[nodiscard]] static std::expected<void, driver::i2c_error>
    mmio_failure() noexcept { return std::unexpected(driver::i2c_error::mmio); }

    MmioPolicy& mmio_;
    pool_type operations_;
    std::optional<core::operation_id> id_{};
    std::optional<driver::i2c_error> pending_error_{};
    driver::i2c_address address_{};
    std::span<const std::uint8_t> output_{};
    std::span<std::uint8_t> input_{};
    std::size_t write_index_{};
    std::size_t read_index_{};
    kind operation_kind_{kind::write};
    bool phase_write_{};
    bool initialized_{};
    bool owned_{};
};

} // namespace mcubsp::backend::stm32h753
