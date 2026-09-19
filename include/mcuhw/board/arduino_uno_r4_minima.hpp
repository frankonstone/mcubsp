#pragma once

#include <cstdint>
#include <string_view>

#include <mcuhw/core/metadata.hpp>

namespace mcuhw::board::arduino_uno_r4_minima {

inline constexpr std::string_view name = "Arduino UNO R4 Minima";
inline constexpr std::string_view device = "R7FA4M1AB3CFM#AA0";
inline constexpr std::uint32_t application_flash_origin = 0x0000'4000U;
inline constexpr std::uint32_t application_flash_length = 0x0003'C000U;
inline constexpr std::uint32_t sram_origin = 0x2000'0000U;
inline constexpr std::uint32_t sram_length = 0x0000'8000U;
inline constexpr std::uint32_t iclk_hz = 48'000'000U;

// Arduino digital-header composition, independently source checked against
// ArduinoCore-renesas 1.6.0 variants/MINIMA/variant.cpp.
inline constexpr core::pin_id d0_rx{3, 1};
inline constexpr core::pin_id d1_tx{3, 2};
inline constexpr core::pin_id d2{1, 5};
inline constexpr core::pin_id d3{1, 4};
inline constexpr core::pin_id d10_ssl{1, 8};
inline constexpr core::pin_id d11_mosi{1, 9};
inline constexpr core::pin_id d12_miso{1, 10};
inline constexpr core::pin_id led_builtin_d13{1, 11};
inline constexpr core::pin_id d13_sck = led_builtin_d13;
inline constexpr core::pin_id a1_an000{0, 0};
inline constexpr core::pin_id a2_an001{0, 1};
inline constexpr core::pin_id a4_sda{1, 1};
inline constexpr core::pin_id a5_scl{1, 0};

} // namespace mcuhw::board::arduino_uno_r4_minima
