#pragma once
// @req-file REQ-0055
// PS 4413_417 v1.11 7.1, tables 61-63; generated product facts.
#include <array>
#include <cstdint>
#include <optional>
#include <mcubsp/core/metadata.hpp>
namespace mcubsp::mcu::nrf52840::product {
enum class package : unsigned { aqfn73, qfn48, wlcsp94 };
inline constexpr std::array<std::array<std::uint32_t, 2>, 3> package_gpio_masks{{
    {0xFFFFFFFFU, 0x0000FFFFU},
    {0xF1FE7FBFU, 0x00008301U},
    {0xFFFFFFFFU, 0x0000FFFFU},
}};
inline constexpr std::array<std::array<std::uint32_t, 2>, 3> low_frequency_masks{{
    {0xF000060CU, 0x0000FCFEU},
    {0xF000060CU, 0x00008000U},
    {0xF000060CU, 0x0000FCFEU},
}};
[[nodiscard]] constexpr bool bonded(package kind, core::pin_id pin) noexcept {
    auto p = static_cast<unsigned>(kind);
    return p < 3 && pin.port < 2 && pin.index < 32 && (package_gpio_masks[p][pin.port] & (1U << pin.index));
}
[[nodiscard]] constexpr bool low_frequency_only(package kind, core::pin_id pin) noexcept {
    return bonded(kind, pin) && (low_frequency_masks[static_cast<unsigned>(kind)][pin.port] & (1U << pin.index));
}
inline constexpr std::uint32_t low_frequency_limit_hz = 10000;
// Electrical restrictions are the same for a die pin in every package.
[[nodiscard]] constexpr bool silicon_low_frequency_only(core::pin_id pin) noexcept {
    return pin.port < 2 && pin.index < 32 && ((low_frequency_masks[0][pin.port] | low_frequency_masks[1][pin.port] | low_frequency_masks[2][pin.port]) & (1U << pin.index));
}
[[nodiscard]] constexpr bool digital_route_supported(package kind, core::pin_id pin, std::uint32_t hz, bool high_drive = false) noexcept {
    return bonded(kind, pin) && (!low_frequency_only(kind, pin) || (!high_drive && hz <= low_frequency_limit_hz));
}
template <package Kind, core::pin_id Pin> requires(bonded(Kind, Pin))
struct package_pin { static constexpr core::pin_id id = Pin; };
// FICR INFO.PACKAGE, PS 4.4.1.16.3. Unknown is not a largest-package fallback.
[[nodiscard]] constexpr std::optional<package> package_from_ficr(std::uint32_t code) noexcept {
    switch (code) { case 0x2004: return package::aqfn73; case 0x2000: return package::qfn48;
        case 0x2005: return package::wlcsp94; default: return std::nullopt; }
}
}
