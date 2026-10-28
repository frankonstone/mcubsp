#pragma once
// @req-file REQ-0055 REQ-0056 REQ-0057 REQ-0058 REQ-0073
#include <array>
#include <cstdint>
#include <mcubsp/mcu/nrf52840/model.hpp>

namespace mcubsp::mcu::nrf52840 {
// Product Specification v1.11: TIMER/RTC configuration and memory chapters.
inline constexpr std::array<std::uintptr_t, 5> timer_bases{0x40008000, 0x40009000, 0x4000A000,
                                                           0x4001A000, 0x4001B000};
inline constexpr std::array<unsigned, 5> timer_channels{4, 4, 4, 6, 6};
inline constexpr std::array<unsigned, 5> timer_irqs{8, 9, 10, 26, 27};
inline constexpr std::array<std::uintptr_t, 3> rtc_bases{0x4000B000, 0x40011000, 0x40024000};
inline constexpr std::array<unsigned, 3> rtc_irqs{11, 17, 36};
inline constexpr std::array<std::uintptr_t, 6> egu_bases{0x40014000, 0x40015000, 0x40016000,
                                                         0x40017000, 0x40018000, 0x40019000};
inline constexpr std::uintptr_t clock_base = 0x40000000, power_base = clock_base;
inline constexpr std::uintptr_t gpiote_base = 0x40006000, ppi_base = 0x4001F000,
                                wdt_base = 0x40010000;
inline constexpr std::uint32_t rtc_mask = 0xFFFFFF, lf_frequency = 32768;
// Positions 44/46 and holes 30/31 have no external IRQ in this device.
[[nodiscard]] constexpr bool valid_irq(unsigned n) noexcept {
    return n < 48 && n != 30 && n != 31 && n != 43 && n != 44 && n != 46;
}
// Product Specification v1.11 p.493, table 29. The primary endpoints are
// immutable in silicon; only enable/group/fork controls are programmable.
enum class ppi_personality : std::uint8_t { ordinary, radio, aar, ccm };
struct fixed_ppi_binding {
    std::uintptr_t event, task;
    ppi_personality event_owner, task_owner;
};
inline constexpr std::array fixed_ppi_bindings{
    fixed_ppi_binding{0x40008140, 0x40001000, ppi_personality::ordinary, ppi_personality::radio},
    fixed_ppi_binding{0x40008140, 0x40001004, ppi_personality::ordinary, ppi_personality::radio},
    fixed_ppi_binding{0x40008144, 0x40001010, ppi_personality::ordinary, ppi_personality::radio},
    fixed_ppi_binding{0x40001128, 0x4000F000, ppi_personality::radio, ppi_personality::aar},
    fixed_ppi_binding{0x40001100, 0x4000F000, ppi_personality::radio, ppi_personality::ccm},
    fixed_ppi_binding{0x40001104, 0x4000F004, ppi_personality::radio, ppi_personality::ccm},
    fixed_ppi_binding{0x40001104, 0x40008044, ppi_personality::radio, ppi_personality::ordinary},
    fixed_ppi_binding{0x4000110C, 0x40008048, ppi_personality::radio, ppi_personality::ordinary},
    fixed_ppi_binding{0x4000B140, 0x40001000, ppi_personality::ordinary, ppi_personality::radio},
    fixed_ppi_binding{0x4000B140, 0x40001004, ppi_personality::ordinary, ppi_personality::radio},
    fixed_ppi_binding{0x4000B140, 0x4000800C, ppi_personality::ordinary, ppi_personality::ordinary},
    fixed_ppi_binding{0x4000B140, 0x40008000, ppi_personality::ordinary, ppi_personality::ordinary},
};
[[nodiscard]] constexpr unsigned ppi_resource(unsigned channel) noexcept {
    return channel < 20 ? 24 + channel : 64 + channel - 20;
}
[[nodiscard]] constexpr std::uintptr_t ppi_resource_address(unsigned channel) noexcept {
    return channel < 20 ? ppi_base + 0x510 + channel * 8 : ppi_base + 0x910 + channel * 4;
}
struct silicon_identity {
    std::uint32_t part{}, variant{}, package{}, ram_kib{}, flash_kib{};
    // Pinned Nordic nrf52_erratas.h uses these ROM identification fields.
    // They supplement FICR; reading them does not itself establish applicability.
    std::uint32_t rom_part{}, rom_revision{};
    [[nodiscard]] constexpr bool is_nrf52840() const noexcept { return part == 0x52840; }
    // Variant's four ASCII bytes identify the factory build. No V1940 inference.
};
[[nodiscard]] constexpr unsigned ram_section_count(unsigned block) noexcept {
    return block < 8 ? 2U : block == 8 ? 6U : 0U;
}
inline constexpr bool complete_product_description =
    false; // field/mode/core/pin/errata gate remains open
} // namespace mcubsp::mcu::nrf52840
