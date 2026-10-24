#pragma once

#include <mcubsp/driver/spi.hpp>

namespace mcubsp::backend::ra4m1 {

struct spi_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint8_t spbr;
    std::uint8_t brdv;

    [[nodiscard]] constexpr operator driver::spi_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm};
    }

    friend constexpr bool operator==(spi_timing, spi_timing) = default;
};

// Select the fastest clock that does not exceed the requested device limit.
// @req REQ-0028
[[nodiscard]] constexpr std::expected<spi_timing, driver::spi_error>
solve_spi_timing(driver::spi_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::spi_error::invalid_clock);
    }
    if (request.target_hz == 0U || request.target_hz > request.clock_hz / 2U) {
        return std::unexpected(driver::spi_error::invalid_rate);
    }
    bool found{};
    spi_timing best{};
    for (std::uint8_t brdv = 0U; brdv < 4U; ++brdv) {
        const auto scale = static_cast<std::uint32_t>(1U << brdv);
        for (std::uint16_t spbr = 0U; spbr <= 255U; ++spbr) {
            const auto divider = 2U * (static_cast<std::uint32_t>(spbr) + 1U)
                * scale;
            const auto achieved = request.clock_hz / divider;
            if (achieved == 0U || achieved > request.target_hz) {
                continue;
            }
            const auto delta = request.target_hz - achieved;
            const auto error = static_cast<std::uint32_t>(
                (static_cast<std::uint64_t>(delta) * 1'000'000ULL
                 + request.target_hz / 2U)
                / request.target_hz);
            const spi_timing candidate{
                .clock_hz = request.clock_hz,
                .target_hz = request.target_hz,
                .achieved_hz = achieved,
                .error_ppm = error,
                .spbr = static_cast<std::uint8_t>(spbr),
                .brdv = brdv,
            };
            if (!found || candidate.achieved_hz > best.achieved_hz
                || (candidate.achieved_hz == best.achieved_hz
                    && candidate.brdv < best.brdv)) {
                best = candidate;
                found = true;
            }
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::spi_error::unattainable_tolerance);
    }
    return best;
}

} // namespace mcubsp::backend::ra4m1
