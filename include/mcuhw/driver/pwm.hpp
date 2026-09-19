#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <limits>

#include <mcuhw/core/transfer.hpp>

namespace mcuhw::driver {

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

struct pwm_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t prescaler;
    std::uint8_t prescaler_encoding;
    std::uint8_t counter_width;
    std::uint64_t period_counts;
    std::uint32_t period_register;

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
    std::uint64_t difference, std::uint32_t clock_hz) noexcept {
    constexpr std::uint64_t scale = 1'000'000U;
    const auto scaled = difference * scale;
    return static_cast<std::uint32_t>(
        rounded_quotient(scaled, clock_hz));
}

} // namespace detail

// The RA4M1 GPT prescaler set is also the common initial timer contract. The
// result retains both the divider and its hardware encoding, so a backend does
// not have to silently reinterpret a solved frequency.
// @req REQ-0004 REQ-0011 REQ-0022
[[nodiscard]] constexpr std::expected<pwm_timing, pwm_error>
solve_pwm_timing(pwm_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(pwm_error::invalid_clock);
    }
    if (request.target_hz == 0U) {
        return std::unexpected(pwm_error::invalid_frequency);
    }
    if (request.counter_width != 16U && request.counter_width != 32U) {
        return std::unexpected(pwm_error::invalid_counter_width);
    }

    constexpr std::array<std::uint32_t, 6> prescalers{
        1U, 4U, 16U, 64U, 256U, 1024U};
    const auto maximum_counts = request.counter_width == 32U
        ? (std::uint64_t{1U} << 32U)
        : (std::uint64_t{1U} << 16U);

    bool found{};
    pwm_timing best{};
    for (std::uint8_t encoding = 0U; encoding < prescalers.size();
         ++encoding) {
        const auto divider = prescalers[encoding];
        const auto requested_denominator =
            static_cast<std::uint64_t>(request.target_hz) * divider;
        auto counts = detail::rounded_quotient(request.clock_hz,
                                               requested_denominator);
        if (counts == 0U || counts > maximum_counts) {
            continue;
        }

        const auto actual_denominator =
            static_cast<std::uint64_t>(divider) * counts;
        const auto target_product =
            static_cast<std::uint64_t>(request.target_hz)
            * actual_denominator;
        const auto difference = target_product > request.clock_hz
            ? target_product - request.clock_hz
            : request.clock_hz - target_product;
        const auto error_ppm = detail::rounded_ppm(
            difference, request.clock_hz);
        const auto achieved_hz = static_cast<std::uint32_t>(
            detail::rounded_quotient(request.clock_hz,
                                     actual_denominator));
        const pwm_timing candidate{
            .clock_hz = request.clock_hz,
            .target_hz = request.target_hz,
            .achieved_hz = achieved_hz,
            .error_ppm = error_ppm,
            .prescaler = divider,
            .prescaler_encoding = encoding,
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
    }

    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(pwm_error::unattainable_tolerance);
    }
    return best;
}

// Duty is a rational value so endpoints and rounding do not depend on
// floating-point support. Interior updates always produce a legal compare
// strictly between zero and the period; endpoints use the GPT duty controls.
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
    [[nodiscard]] std::expected<void, pwm_error>
    initialize(pwm_timing timing, pwm_duty duty) noexcept {
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

} // namespace mcuhw::driver
