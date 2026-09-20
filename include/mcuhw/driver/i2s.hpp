#pragma once

#include <cstddef>
#include <cstdint>

namespace mcuhw::driver {

enum class i2s_error : std::uint8_t {
    invalid_clock,
    invalid_rate,
    unsupported_format,
    empty_transfer,
    excessive_length,
    invalid_buffer,
    busy,
    transport,
    not_initialized,
    already_owned,
    mmio,
};

struct i2s_timing_request {
    std::uint32_t clock_hz;
    std::uint32_t sample_rate_hz;
    std::uint16_t mclk_multiple{256U};
    std::uint8_t slot_bits{16U};
    std::uint8_t slot_count{2U};
    std::uint32_t maximum_error_ppm;
};

// The fractional divider is N + numerator / denominator. The backend-specific
// X/Y/Z encoding is deliberately retained beside the portable clock result.
struct i2s_timing {
    std::uint32_t clock_hz;
    std::uint32_t sample_rate_hz;
    std::uint32_t achieved_sample_rate_hz;
    std::uint32_t mclk_hz;
    std::uint32_t bclk_hz;
    std::uint32_t error_ppm;
    std::uint16_t divider_numerator;
    std::uint16_t divider_denominator;
    std::uint8_t divider_integer;
    std::uint8_t bclk_divider;
    std::uint8_t slot_bits;
    std::uint8_t slot_count;

    friend constexpr bool operator==(i2s_timing, i2s_timing) = default;
};

enum class i2s_direction : std::uint8_t { receive, transmit };

struct i2s_transfer_result {
    i2s_direction direction;
    std::size_t bytes;

    friend constexpr bool operator==(i2s_transfer_result,
                                     i2s_transfer_result) = default;
};

} // namespace mcuhw::driver
