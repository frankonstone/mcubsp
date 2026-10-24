#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace mcubsp::device::es8311 {

inline constexpr std::uint8_t default_i2c_address = 0x18U;

struct register_write {
    std::uint8_t address;
    std::uint8_t value;

    friend constexpr bool operator==(register_write,
                                     register_write) = default;
};

// ES8311 slave-mode startup for a 12.288 MHz MCLK, 48 kHz, 16-bit Philips
// stream, analog microphone input and DAC output. This is the bounded register
// sequence used by Espressif's codec driver and the board vendor's example.
// Keep the power amplifier disabled until the caller has applied the complete
// sequence and deliberately unmutes the external speaker path.
inline constexpr std::array startup_48khz_16bit{
    register_write{0x0DU, 0xFAU}, register_write{0x44U, 0x08U},
    register_write{0x44U, 0x08U}, register_write{0x01U, 0x30U},
    register_write{0x02U, 0x00U}, register_write{0x03U, 0x10U},
    register_write{0x16U, 0x24U}, register_write{0x04U, 0x10U},
    register_write{0x05U, 0x00U}, register_write{0x0BU, 0x00U},
    register_write{0x0CU, 0x00U}, register_write{0x10U, 0x1FU},
    register_write{0x11U, 0x7FU}, register_write{0x00U, 0x80U},
    register_write{0x01U, 0x3FU}, register_write{0x06U, 0x00U},
    register_write{0x13U, 0x10U}, register_write{0x1BU, 0x0AU},
    register_write{0x1CU, 0x6AU}, register_write{0x44U, 0x08U},
    register_write{0x02U, 0x00U}, register_write{0x03U, 0x10U},
    register_write{0x04U, 0x10U}, register_write{0x05U, 0x00U},
    register_write{0x06U, 0x03U}, register_write{0x07U, 0x00U},
    register_write{0x08U, 0xFFU}, register_write{0x09U, 0x0CU},
    register_write{0x0AU, 0x0CU}, register_write{0x16U, 0x04U},
    register_write{0x17U, 0xBFU}, register_write{0x0EU, 0x02U},
    register_write{0x12U, 0x00U}, register_write{0x14U, 0x1AU},
    register_write{0x0DU, 0x01U}, register_write{0x15U, 0x40U},
    register_write{0x37U, 0x08U}, register_write{0x45U, 0x00U},
};

[[nodiscard]] constexpr register_write output_volume(
    std::uint8_t native_volume) noexcept {
    return {0x32U, native_volume};
}

inline constexpr register_write conservative_output_volume =
    output_volume(0x60U);
// ES8311 register 0x32 uses 0xBF for 0 dB digital DAC gain. Keep this
// separate from the conservative default so audible board tests opt in.
inline constexpr register_write unity_output_volume = output_volume(0xBFU);
inline constexpr register_write unmute_dac{0x31U, 0x00U};
inline constexpr register_write mute_dac{0x31U, 0x60U};

// @req REQ-0011 REQ-0024 REQ-0034
[[nodiscard]] constexpr std::span<const register_write>
startup_sequence_48khz_16bit() noexcept {
    return startup_48khz_16bit;
}

} // namespace mcubsp::device::es8311
