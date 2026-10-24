#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>

namespace mcubsp::driver {

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

// Vendor-independent timing report. Native configuration is owned by the backend.
struct uart_baud_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_baud;
    std::uint32_t achieved_baud;
    std::uint32_t error_ppm;

    friend constexpr bool operator==(uart_baud_timing, uart_baud_timing) = default;
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
        rounded_divide(difference * 1'000'000ULL, target));
}


} // namespace uart_detail

template<typename Backend>
class uart {
public:
    explicit uart(Backend& backend) noexcept : backend_(backend) {}

    template<typename Timing>
        requires requires(Backend& backend, Timing timing) { backend.initialize(timing, uart_format{}); }
    [[nodiscard]] std::expected<void, uart_error>
    initialize(Timing timing, uart_format format = {}) noexcept {
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

} // namespace mcubsp::driver
