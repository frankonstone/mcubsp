#pragma once

// @req-file REQ-0053
#include <cstddef>
#include <cstdint>
#include <mcubsp/core/metadata.hpp>

namespace mcubsp::mcu::nrf52840 {

// Initial GPIO/core slice; TWI metadata is in twi.hpp. No complete MCU or silicon review.
// Nordic nrfx v3.2.0 mdk/nrf52840.h and nrf52840_bitfields.h, GPIO and FICR.
inline constexpr core::source_ref register_source{
    "Nordic nrfx MDK", "v3.2.0", "nrf52840.h; nrf52840_bitfields.h: GPIO/FICR"};
inline constexpr std::uint32_t cpu_frequency = 64'000'000U;
inline constexpr std::uintptr_t flash_begin = 0U;
inline constexpr std::size_t flash_bytes = 0x10'0000U;
inline constexpr std::uintptr_t ram_begin = 0x2000'0000U;
inline constexpr std::size_t ram_bytes = 0x4'0000U;
inline constexpr std::uintptr_t ficr_part = 0x1000'0100U;

[[nodiscard]] constexpr bool valid_pin(core::pin_id pin) noexcept {
    return (pin.port == 0U && pin.index < 32U)
        || (pin.port == 1U && pin.index < 16U);
}

// Conservatively leave LF crystal, NFC and reset pads to their owners.
// No UICR writes or implicit conversion of those pads to GPIO are performed.
[[nodiscard]] constexpr bool gpio_supported(core::pin_id pin) noexcept {
    return valid_pin(pin) && (pin.port != 0U
        || (pin.index != 0U && pin.index != 1U && pin.index != 9U
            && pin.index != 10U && pin.index != 18U));
}

} // namespace mcubsp::mcu::nrf52840
