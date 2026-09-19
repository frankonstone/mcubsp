#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace mcuhw::driver {

enum class spi_error : std::uint8_t {
    invalid_clock,
    invalid_rate,
    unattainable_tolerance,
    invalid_frame,
    empty_transfer,
    mismatched_segment,
    excessive_length,
    busy,
    overrun,
    mode_fault,
    parity,
    transport,
    not_initialized,
    already_owned,
    mmio,
};

enum class spi_mode : std::uint8_t { mode0, mode1, mode2, mode3 };
enum class spi_bit_order : std::uint8_t { most_significant_first,
                                           least_significant_first };
enum class spi_frame_width : std::uint8_t { bits8 = 8, bits16 = 16 };
enum class spi_chip_select_polarity : std::uint8_t { active_low, active_high };

struct spi_timing_request {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t maximum_error_ppm;
};

struct spi_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint8_t spbr;
    std::uint8_t brdv;

    friend constexpr bool operator==(spi_timing, spi_timing) = default;
};

// Select the fastest clock that does not exceed the requested device limit.
// @req REQ-0028
[[nodiscard]] constexpr std::expected<spi_timing, spi_error>
solve_spi_timing(spi_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(spi_error::invalid_clock);
    }
    if (request.target_hz == 0U || request.target_hz > request.clock_hz / 2U) {
        return std::unexpected(spi_error::invalid_rate);
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
        return std::unexpected(spi_error::unattainable_tolerance);
    }
    return best;
}

struct spi_configuration {
    spi_mode mode{spi_mode::mode0};
    spi_bit_order bit_order{spi_bit_order::most_significant_first};
    spi_frame_width frame_width{spi_frame_width::bits8};
    spi_chip_select_polarity chip_select{spi_chip_select_polarity::active_low};
};

// Each segment owns zero or more transmit and receive frames. When both are
// present their lengths must match. An empty transmit span generates dummy
// frames; an empty receive span discards received frames. Chip select remains
// asserted between every segment in one transaction.
struct spi_segment {
    std::span<const std::uint16_t> transmit{};
    std::span<std::uint16_t> receive{};
    std::uint16_t dummy{0xFFFFU};
};

struct spi_transfer_result {
    std::size_t frames;

    friend constexpr bool operator==(spi_transfer_result,
                                     spi_transfer_result) = default;
};

template<typename Backend>
class spi_controller {
public:
    explicit spi_controller(Backend& backend) noexcept : backend_(backend) {}

    [[nodiscard]] std::expected<void, spi_error>
    initialize(spi_timing timing, spi_configuration configuration = {}) noexcept {
        return backend_.initialize(timing, configuration);
    }

    [[nodiscard]] auto transaction(std::span<const spi_segment> segments) noexcept {
        return backend_.transaction(segments);
    }

    [[nodiscard]] std::expected<void, spi_error> shutdown() noexcept {
        return backend_.shutdown();
    }

private:
    Backend& backend_;
};

} // namespace mcuhw::driver
