#pragma once

#include <array>

#include <mcuhw/driver/uart.hpp>

namespace mcuhw::backend::ra4m1 {

struct baud_mode {
    std::uint8_t base_divisor;
    bool bgdm;
    bool abcs;
    bool abcse;
};

struct uart_baud_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_baud;
    std::uint32_t achieved_baud;
    std::uint32_t error_ppm;
    std::uint16_t divisor;
    std::uint8_t clock_select;
    std::uint8_t brr;
    bool double_speed;
    bool asynchronous_base_clock_select;
    bool extended_base_clock_select;

    [[nodiscard]] constexpr operator driver::uart_baud_timing() const noexcept {
        return {clock_hz, target_baud, achieved_baud, error_ppm};
    }

    friend constexpr bool operator==(uart_baud_timing,
                                     uart_baud_timing) = default;
};

// RA SCI asynchronous timing is B = PCLKA / (base * 4^CKS * (BRR+1)).
// All four documented asynchronous sampling modes are searched and the
// concrete hardware encoding is retained in the result.
// @req REQ-0011 REQ-0027
[[nodiscard]] constexpr std::expected<uart_baud_timing, driver::uart_error>
solve_uart_baud(driver::uart_baud_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::uart_error::invalid_clock);
    }
    if (request.target_baud == 0U) {
        return std::unexpected(driver::uart_error::invalid_baud);
    }

    constexpr std::array modes{
        baud_mode{32U, false, false, false},
        baud_mode{16U, true, false, false},
        baud_mode{8U, true, true, false},
        baud_mode{6U, false, false, true},
    };
    bool found{};
    uart_baud_timing best{};
    for (const auto mode : modes) {
        std::uint32_t clock_divider = 1U;
        for (std::uint8_t cks = 0U; cks < 4U; ++cks) {
            const auto unit = static_cast<std::uint64_t>(mode.base_divisor)
                * clock_divider;
            const auto brr_plus_one = driver::uart_detail::rounded_divide(
                request.clock_hz,
                static_cast<std::uint64_t>(request.target_baud) * unit);
            if (brr_plus_one == 0U || brr_plus_one > 256U) {
                clock_divider *= 4U;
                continue;
            }
            const auto denominator = unit * brr_plus_one;
            const auto candidate = uart_baud_timing{
                .clock_hz = request.clock_hz,
                .target_baud = request.target_baud,
                .achieved_baud = static_cast<std::uint32_t>(
                    driver::uart_detail::rounded_divide(request.clock_hz,
                                                denominator)),
                .error_ppm = driver::uart_detail::baud_error_ppm(
                    request.clock_hz, request.target_baud, denominator),
                .divisor = static_cast<std::uint16_t>(denominator),
                .clock_select = cks,
                .brr = static_cast<std::uint8_t>(brr_plus_one - 1U),
                .double_speed = mode.bgdm,
                .asynchronous_base_clock_select = mode.abcs,
                .extended_base_clock_select = mode.abcse,
            };
            if (!found || candidate.error_ppm < best.error_ppm
                || (candidate.error_ppm == best.error_ppm
                    && candidate.divisor < best.divisor)) {
                best = candidate;
                found = true;
            }
            clock_divider *= 4U;
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::uart_error::unattainable_tolerance);
    }
    return best;
}

} // namespace mcuhw::backend::ra4m1
