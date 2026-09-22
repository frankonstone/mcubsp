#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>

namespace mcuhw::driver {

enum class uart_error : std::uint8_t {
    invalid_clock,
    invalid_baud,
    invalid_format,
    unattainable_tolerance,
    empty_transfer,
    busy,
    not_initialized,
    already_owned,
    overrun,
    framing,
    parity,
    transport,
    dma,
    mmio,
};

enum class uart_data_bits : std::uint8_t { seven = 7, eight = 8 };
enum class uart_parity : std::uint8_t { none, even, odd };
enum class uart_stop_bits : std::uint8_t { one = 1, two = 2 };

struct uart_format {
    uart_data_bits data_bits{uart_data_bits::eight};
    uart_parity parity{uart_parity::none};
    uart_stop_bits stop_bits{uart_stop_bits::one};
};

struct uart_baud_request {
    std::uint32_t clock_hz;
    std::uint32_t target_baud;
    std::uint32_t maximum_error_ppm;
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
    // Vendor backends with a wider baud register retain their exact encoding
    // here. RA4M1 continues to use the fields above.
    std::uint32_t backend_encoding{};

    friend constexpr bool operator==(uart_baud_timing,
                                     uart_baud_timing) = default;
};

namespace uart_detail {

[[nodiscard]] constexpr std::uint64_t rounded_divide(
    std::uint64_t numerator, std::uint64_t denominator) noexcept {
    return (numerator + denominator / 2U) / denominator;
}

[[nodiscard]] constexpr std::uint32_t baud_error_ppm(
    std::uint32_t clock, std::uint32_t baud,
    std::uint64_t denominator) noexcept {
    const auto target = static_cast<std::uint64_t>(baud) * denominator;
    const auto difference = target > clock ? target - clock : clock - target;
    return static_cast<std::uint32_t>(
        rounded_divide(difference * 1'000'000ULL, clock));
}

struct baud_mode {
    std::uint8_t base_divisor;
    bool bgdm;
    bool abcs;
    bool abcse;
};

} // namespace uart_detail

// RA SCI asynchronous timing is B = PCLKA / (base * 4^CKS * (BRR+1)).
// All four documented asynchronous sampling modes are searched and the
// concrete hardware encoding is retained in the result.
// @req REQ-0011 REQ-0027
[[nodiscard]] constexpr std::expected<uart_baud_timing, uart_error>
solve_uart_baud(uart_baud_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(uart_error::invalid_clock);
    }
    if (request.target_baud == 0U) {
        return std::unexpected(uart_error::invalid_baud);
    }

    constexpr std::array modes{
        uart_detail::baud_mode{32U, false, false, false},
        uart_detail::baud_mode{16U, true, false, false},
        uart_detail::baud_mode{8U, true, true, false},
        uart_detail::baud_mode{6U, false, false, true},
    };
    bool found{};
    uart_baud_timing best{};
    for (const auto mode : modes) {
        std::uint32_t clock_divider = 1U;
        for (std::uint8_t cks = 0U; cks < 4U; ++cks) {
            const auto unit = static_cast<std::uint64_t>(mode.base_divisor)
                * clock_divider;
            const auto brr_plus_one = uart_detail::rounded_divide(
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
                    uart_detail::rounded_divide(request.clock_hz,
                                                denominator)),
                .error_ppm = uart_detail::baud_error_ppm(
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
        return std::unexpected(uart_error::unattainable_tolerance);
    }
    return best;
}

template<typename Backend>
class uart {
public:
    explicit uart(Backend& backend) noexcept : backend_(backend) {}

    [[nodiscard]] std::expected<void, uart_error>
    initialize(uart_baud_timing timing, uart_format format = {}) noexcept {
        return backend_.initialize(timing, format);
    }

    [[nodiscard]] auto transmit(std::span<const std::uint8_t> bytes) noexcept {
        return backend_.transmit(bytes);
    }

    [[nodiscard]] auto receive(std::span<std::uint8_t> bytes) noexcept {
        return backend_.receive(bytes);
    }

    template<typename Dma>
    [[nodiscard]] auto transmit_dma(std::span<const std::uint8_t> bytes,
                                    Dma& dma) noexcept {
        return backend_.transmit_dma(bytes, dma);
    }

    template<typename Dma>
    [[nodiscard]] auto receive_dma(std::span<std::uint8_t> bytes,
                                   Dma& dma) noexcept {
        return backend_.receive_dma(bytes, dma);
    }

    [[nodiscard]] std::expected<void, uart_error> shutdown() noexcept {
        return backend_.shutdown();
    }

private:
    Backend& backend_;
};

} // namespace mcuhw::driver
