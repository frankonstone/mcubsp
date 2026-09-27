#pragma once

#include <array>

#include <mcuhw/driver/pwm.hpp>

namespace mcuhw::backend::ra4m1 {

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

    [[nodiscard]] constexpr operator driver::pwm_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm, prescaler, counter_width, period_counts};
    }

    friend constexpr bool operator==(pwm_timing, pwm_timing) = default;
};

// Solve the RA GPT prescaler set and retain its native register encoding.
// @req REQ-0004 REQ-0011 REQ-0022
[[nodiscard]] constexpr std::expected<pwm_timing, driver::pwm_error>
solve_pwm_timing(driver::pwm_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::pwm_error::invalid_clock);
    }
    if (request.target_hz == 0U) {
        return std::unexpected(driver::pwm_error::invalid_frequency);
    }
    if (request.counter_width != 16U && request.counter_width != 32U) {
        return std::unexpected(driver::pwm_error::invalid_counter_width);
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
        auto counts = driver::detail::rounded_quotient(request.clock_hz,
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
        const auto error_ppm = driver::detail::rounded_ppm(
            difference, target_product);
        const auto achieved_hz = static_cast<std::uint32_t>(
            driver::detail::rounded_quotient(request.clock_hz,
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
        return std::unexpected(driver::pwm_error::unattainable_tolerance);
    }
    return best;
}

} // namespace mcuhw::backend::ra4m1
