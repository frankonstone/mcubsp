#pragma once

// @req-file REQ-0053
#include <mcubsp/board/others/promicro_nrf52840/pins.hpp>
#include <mcubsp/mcu/nrf52840/model.hpp>

namespace mcubsp::board::others {

struct promicro_nrf52840_bindings : promicro_nrf52840_pins {
    // Zephyr nrf52840_partition_uf2_sdv6.dtsi; UF2/S140-v6 profile only.
    static constexpr std::uintptr_t application_begin = 0x0002'6000U;
    static constexpr std::size_t application_bytes = 0x000C'6000U;
    static constexpr std::uintptr_t storage_begin = 0x000E'C000U;
    static constexpr std::uintptr_t bootloader_begin = 0x000F'4000U;
};

} // namespace mcubsp::board::others
