#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <limits>

#include <mcubsp/core/transfer.hpp>

namespace mcubsp::driver {

enum class pwm_error : std::uint8_t {
    invalid_clock,
    invalid_frequency,
    invalid_counter_width,
    invalid_duty,
    unattainable_tolerance,
    already_owned,
    already_initialized,
    not_initialized,
    already_running,
    not_running,
    mmio,
};

struct pwm_timing_request {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t maximum_error_ppm;
    std::uint8_t counter_width;
};

// Vendor-independent timing report. Native configuration is owned by the backend.
struct pwm_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t prescaler;
    std::uint8_t counter_width;
    std::uint64_t period_counts;

    friend constexpr bool operator==(pwm_timing, pwm_timing) = default;
};

struct pwm_duty {
    std::uint32_t numerator;
    std::uint32_t denominator;

    [[nodiscard]] static constexpr pwm_duty percent(
        std::uint32_t value) noexcept {
        return {value, 100U};
    }
};

enum class pwm_duty_mode : std::uint8_t {
    force_low,
    compare,
    force_high,
};

struct pwm_compare {
    pwm_duty_mode mode;
    std::uint32_t compare_counts;

    friend constexpr bool operator==(pwm_compare, pwm_compare) = default;
};

namespace detail {

[[nodiscard]] constexpr std::uint64_t rounded_quotient(
    std::uint64_t numerator, std::uint64_t denominator) noexcept {
    const auto quotient = numerator / denominator;
    const auto remainder = numerator % denominator;
    return quotient + (remainder >= denominator - remainder ? 1U : 0U);
}

[[nodiscard]] constexpr std::uint32_t rounded_ppm(
    std::uint64_t difference, std::uint64_t target_product) noexcept {
    constexpr std::uint64_t scale = 1'000'000U;
    const auto scaled = difference * scale;
    return static_cast<std::uint32_t>(
        rounded_quotient(scaled, target_product));
}

} // namespace detail

// Rational duty values keep rounding independent of floating-point support.
// Interior duty produces a compare strictly inside the period; the backend
// implements the force-low and force-high endpoint modes.
// @req REQ-0022
[[nodiscard]] constexpr std::expected<pwm_compare, pwm_error>
solve_pwm_duty(std::uint64_t period_counts, pwm_duty duty) noexcept {
    if (period_counts < 2U || duty.denominator == 0U
        || duty.numerator > duty.denominator) {
        return std::unexpected(pwm_error::invalid_duty);
    }
    if (duty.numerator == 0U) {
        return pwm_compare{pwm_duty_mode::force_low, 0U};
    }
    if (duty.numerator == duty.denominator) {
        return pwm_compare{pwm_duty_mode::force_high,
                           static_cast<std::uint32_t>(period_counts)};
    }

    const auto whole = period_counts / duty.denominator;
    const auto remainder = period_counts % duty.denominator;
    const auto low_product = whole * duty.numerator;
    const auto remainder_product = remainder * duty.numerator;
    auto compare = low_product
        + detail::rounded_quotient(remainder_product, duty.denominator);
    if (compare == 0U) {
        compare = 1U;
    } else if (compare >= period_counts) {
        compare = period_counts - 1U;
    }
    if (compare > std::numeric_limits<std::uint32_t>::max()) {
        return std::unexpected(pwm_error::invalid_duty);
    }
    return pwm_compare{pwm_duty_mode::compare,
                       static_cast<std::uint32_t>(compare)};
}

// Functional PWM ownership and lifecycle. Register addresses and sequencing
// remain entirely in Backend.
template<typename Backend>
class pwm_channel {
public:
    using period_transfer = typename Backend::period_transfer;
    using period_start_result = typename Backend::period_start_result;

    explicit constexpr pwm_channel(Backend& backend) noexcept
        : backend_(backend) {}

    pwm_channel(const pwm_channel&) = delete;
    pwm_channel& operator=(const pwm_channel&) = delete;
    pwm_channel(pwm_channel&&) = delete;
    pwm_channel& operator=(pwm_channel&&) = delete;

    // @req REQ-0003 REQ-0014 REQ-0022
    template<typename Timing>
        requires requires(Backend& backend, Timing timing) { backend.configure(timing, pwm_compare{}); }
    [[nodiscard]] std::expected<void, pwm_error>
    initialize(Timing timing, pwm_duty duty) noexcept {
        if (initialized_) {
            return std::unexpected(pwm_error::already_initialized);
        }
        const auto compare = solve_pwm_duty(timing.period_counts, duty);
        if (!compare) {
            return std::unexpected(compare.error());
        }
        const auto claimed = backend_.claim();
        if (!claimed) {
            return claimed;
        }
        const auto configured = backend_.configure(timing, *compare);
        if (!configured) {
            backend_.release();
            return configured;
        }
        timing_ = timing;
        initialized_ = true;
        return {};
    }

    [[nodiscard]] std::expected<void, pwm_error> start() noexcept {
        if (!initialized_) {
            return std::unexpected(pwm_error::not_initialized);
        }
        if (running_) {
            return std::unexpected(pwm_error::already_running);
        }
        const auto result = backend_.start();
        if (result) {
            running_ = true;
        }
        return result;
    }

    [[nodiscard]] std::expected<void, pwm_error> stop() noexcept {
        if (!initialized_) {
            return std::unexpected(pwm_error::not_initialized);
        }
        if (!running_) {
            return std::unexpected(pwm_error::not_running);
        }
        const auto result = backend_.stop();
        if (result) {
            running_ = false;
        }
        return result;
    }

    [[nodiscard]] std::expected<void, pwm_error>
    update_duty(pwm_duty duty) noexcept {
        if (!initialized_) {
            return std::unexpected(pwm_error::not_initialized);
        }
        const auto compare = solve_pwm_duty(timing_.period_counts, duty);
        if (!compare) {
            return std::unexpected(compare.error());
        }
        return backend_.update_duty(*compare, running_);
    }

    [[nodiscard]] period_start_result next_period() noexcept {
        if (!initialized_ || !running_) {
            return std::unexpected(typename Backend::transfer_error{
                .kind = core::transfer_error_kind::invalid_operation,
                .cause = pwm_error::not_running,
            });
        }
        return backend_.next_period();
    }

    [[nodiscard]] std::expected<void, pwm_error> release() noexcept {
        if (!initialized_) {
            return std::unexpected(pwm_error::not_initialized);
        }
        const auto result = backend_.shutdown();
        if (!result) {
            return result;
        }
        backend_.release();
        initialized_ = false;
        running_ = false;
        return {};
    }

    [[nodiscard]] constexpr bool initialized() const noexcept {
        return initialized_;
    }
    [[nodiscard]] constexpr bool running() const noexcept { return running_; }
    [[nodiscard]] constexpr pwm_timing timing() const noexcept {
        return timing_;
    }

private:
    Backend& backend_;
    pwm_timing timing_{};
    bool initialized_{};
    bool running_{};
};

} // namespace mcubsp::driver
