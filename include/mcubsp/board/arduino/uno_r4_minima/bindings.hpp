#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <mcubsp/board/arduino/uno_r4_minima/pins.hpp>

namespace mcubsp::board::arduino {

// Board identity, clock/memory facts and connections; no I/O facade or backend
// implementation is required to use these bindings.
struct uno_r4_minima_bindings : uno_r4_minima_pins {
    static constexpr std::string_view name = "Arduino UNO R4 Minima";
    static constexpr std::string_view device = "R7FA4M1AB3CFM#AA0";
    static constexpr std::uint32_t application_flash_origin = 0x0000'4000U;
    static constexpr std::uint32_t application_flash_length = 0x0003'C000U;
    static constexpr std::uint32_t sram_origin = 0x2000'0000U;
    static constexpr std::uint32_t sram_length = 0x0000'8000U;
    static constexpr std::uint32_t iclk_hz = 48'000'000U;
};

} // namespace mcubsp::board::arduino
