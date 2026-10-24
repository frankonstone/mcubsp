#pragma once

#include <mcubsp/driver/i2c.hpp>

namespace mcubsp::backend::ra4m1 {

struct i2c_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t rise_time_ns;
    std::uint32_t fall_time_ns;
    std::uint8_t clock_select;
    std::uint8_t brl;
    std::uint8_t brh;

    [[nodiscard]] constexpr operator driver::i2c_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm, rise_time_ns, fall_time_ns};
    }

    friend constexpr bool operator==(i2c_timing, i2c_timing) = default;
};

// Initial RA IIC timing uses the manual's normal SCLE=1, NFE=0 mode.
// Rise/fall time and Standard/Fast-mode minimum high and low periods are
// enforced before selecting BRH/BRL.
// @req REQ-0011 REQ-0024
[[nodiscard]] constexpr std::expected<i2c_timing, driver::i2c_error>
solve_i2c_timing(driver::i2c_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::i2c_error::invalid_clock);
    }
    if (request.target_hz == 0U || request.target_hz > 400'000U) {
        return std::unexpected(driver::i2c_error::invalid_rate);
    }
    const bool standard = request.target_hz <= 100'000U;
    const auto maximum_rise = standard ? 1'000U : 300U;
    if (request.rise_time_ns > maximum_rise
        || request.fall_time_ns > 300U) {
        return std::unexpected(driver::i2c_error::invalid_rise_fall);
    }
    const std::uint32_t minimum_low_ns = standard ? 4'700U : 1'300U;
    const std::uint32_t minimum_high_ns = standard ? 4'000U : 600U;

    bool found{};
    i2c_timing best{};
    std::uint32_t divider = 1U;
    for (std::uint8_t cks = 0U; cks < 8U; ++cks) {
        const auto reference = request.clock_hz / divider;
        if (reference == 0U) {
            break;
        }
        const auto offset = cks == 0U ? 3U : 2U;
        const auto minimum_low_counts = driver::i2c_detail::ceil_divide(
            static_cast<std::uint64_t>(minimum_low_ns) * reference,
            1'000'000'000ULL);
        const auto minimum_high_counts = driver::i2c_detail::ceil_divide(
            static_cast<std::uint64_t>(minimum_high_ns) * reference,
            1'000'000'000ULL);
        const auto brl_min = minimum_low_counts > offset
            ? minimum_low_counts - offset : 1U;
        const auto brh_min = minimum_high_counts > offset
            ? minimum_high_counts - offset : 1U;
        if (brl_min > 32U || brh_min > 32U) {
            divider *= 2U;
            continue;
        }

        for (std::uint64_t low = brl_min; low <= 32U; ++low) {
            for (std::uint64_t high = brh_min; high <= 32U; ++high) {
                const auto wire_ns = static_cast<std::uint64_t>(
                    request.rise_time_ns) + request.fall_time_ns;
                const auto clock_ns = driver::i2c_detail::divide_rounded(
                    (low + high + 2U * offset) * 1'000'000'000ULL,
                    reference);
                const auto period_ns = wire_ns + clock_ns;
                if (period_ns == 0U) {
                    continue;
                }
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
                const i2c_timing candidate{
                    .clock_hz = request.clock_hz,
                    .target_hz = request.target_hz,
                    .achieved_hz = achieved,
                    .error_ppm = error,
                    .rise_time_ns = request.rise_time_ns,
                    .fall_time_ns = request.fall_time_ns,
                    .clock_select = cks,
                    .brl = static_cast<std::uint8_t>(low - 1U),
                    .brh = static_cast<std::uint8_t>(high - 1U),
                };
                if (!found || candidate.error_ppm < best.error_ppm
                    || (candidate.error_ppm == best.error_ppm
                        && cks < best.clock_select)) {
                    best = candidate;
                    found = true;
                }
            }
        }
        divider *= 2U;
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::i2c_error::unattainable_tolerance);
    }
    return best;
}

} // namespace mcubsp::backend::ra4m1
