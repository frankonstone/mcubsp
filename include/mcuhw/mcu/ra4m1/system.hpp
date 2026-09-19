#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <mcuhw/mcu/ra4m1/model.hpp>

namespace mcuhw::mcu::ra4m1 {

inline constexpr core::source_ref identity_source{
    "R01UH0887EJ0110 / R01DS0355EJ0110", "Rev.1.10",
    "R01UH 1.3-1.7, PDF pages 57-74; R01DS Tables 1.13-1.14"};

inline constexpr core::device_identity identity{
    "Renesas", "RA4M1", "R7FA4M1AB3CFM#AA0",
    "PLQP0064KB-C, 64-pin LQFP", "MCU revisions covered by Rev.1.10",
    identity_source};

enum class memory_id : std::uint16_t { code_flash, sram0, data_flash };
inline constexpr core::source_ref memory_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "1.4 and 4.1, PDF pages 59 and 92"};

inline constexpr std::array memory_regions{
    core::memory_region{static_cast<std::uint16_t>(memory_id::code_flash),
        0x0000'0000U, 256U * 1024U, true, false,
        core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::sram0),
        0x2000'0000U, 32U * 1024U, true, true,
        core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::data_flash),
        0x4010'0000U, 8U * 1024U, true, false,
        core::cache_policy::uncached, memory_source},
};

inline constexpr core::source_ref clock_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "8.1 Tables 8.1-8.2 and 8.2.1-8.2.10, PDF pages 129-142"};

inline constexpr std::array clock_nodes{
    core::clock_node{static_cast<std::uint16_t>(clock_id::loco), {32'768U}, {32'768U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::moco), {8'000'000U}, {8'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::hoco), {24'000'000U}, {64'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::mosc), {1'000'000U}, {20'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::sosc), {32'768U}, {32'768U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pll), {24'000'000U}, {64'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::iclk), {1U}, {48'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclka), {1U}, {48'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclkb), {1U}, {32'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclkc), {1U}, {64'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclkd), {1U}, {64'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::fclk), {1'000'000U}, {32'000'000U}, clock_source},
};

[[nodiscard]] constexpr core::clock_edge source_to_domain(
    clock_id source, clock_id domain, std::uint32_t encoding) {
    return core::clock_edge{static_cast<std::uint16_t>(source),
        static_cast<std::uint16_t>(domain), 1, 64, false,
        core::clock_route_kind::selectable, encoding, clock_source};
}

// All six selectable system sources feed every divided system domain. This
// table records representative HOCO routes; source selection encodings are
// exposed separately and do not duplicate 36 identical topology edges.
inline constexpr std::array clock_edges{
    source_to_domain(clock_id::hoco, clock_id::iclk, 0U),
    source_to_domain(clock_id::hoco, clock_id::pclka, 0U),
    source_to_domain(clock_id::hoco, clock_id::pclkb, 0U),
    source_to_domain(clock_id::hoco, clock_id::pclkc, 0U),
    source_to_domain(clock_id::hoco, clock_id::pclkd, 0U),
    source_to_domain(clock_id::hoco, clock_id::fclk, 0U),
};

using sckdivcr = core::register_descriptor<std::uint32_t, 0x20,
    core::access_semantics::read_write, 0x7777'0777U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x2100'2122U, 0xFFFF'FFFFU>;
using sckscr = core::register_descriptor<std::uint8_t, 0x26,
    core::access_semantics::read_write, 0x07U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x01U, 0xFFU>;
using pllcr = core::register_descriptor<std::uint8_t, 0x2A,
    core::access_semantics::read_write, 0x01U, true,
    core::reserved_bit_policy::write_zero, 0U, 0x01U, 0xFFU>;
using pllccr2 = core::register_descriptor<std::uint8_t, 0x2B,
    core::access_semantics::read_write, 0xDFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x07U, 0xFFU>;
using hococr2 = core::register_descriptor<std::uint8_t, 0x37,
    core::access_semantics::read_write, 0x38U, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0U>;
using prcr = core::register_descriptor<std::uint16_t, 0x3FE,
    core::access_semantics::write_only, 0x000BU, false,
    core::reserved_bit_policy::write_zero, 0xA500U, 0U, 0xFFFFU>;
using mstpcra = core::register_descriptor<std::uint32_t, 0x01C,
    core::access_semantics::read_write, 0x0040'0041U, true,
    core::reserved_bit_policy::preserve, 0U, 0xFFBF'FFBEU, 0xFFFF'FFFFU>;
using mstpcrb = core::register_descriptor<std::uint32_t, 0x000,
    core::access_semantics::read_write, 0xE04C'0B04U, true,
    core::reserved_bit_policy::preserve, 0U, 0xFFFF'FFFFU, 0xFFFF'FFFFU>;
using mstpcrd = core::register_descriptor<std::uint32_t, 0x008,
    core::access_semantics::read_write, 0xA019'406CU, true,
    core::reserved_bit_policy::preserve, 0U, 0xFFFF'FFFFU, 0xFFFF'FFFFU>;

using pckd = core::field_descriptor<sckdivcr, 0, 3>;
using pckc = core::field_descriptor<sckdivcr, 4, 3>;
using pckb = core::field_descriptor<sckdivcr, 8, 3>;
using pcka = core::field_descriptor<sckdivcr, 12, 3>;
using ick = core::field_descriptor<sckdivcr, 24, 3>;
using fck = core::field_descriptor<sckdivcr, 28, 3>;
using cksel = core::field_descriptor<sckscr, 0, 3>;
using pll_stopped = core::field_descriptor<pllcr, 0, 1>;
using hoco_frequency = core::field_descriptor<hococr2, 3, 3>;
using prc0 = core::field_descriptor<prcr, 0, 1>;
using prc1 = core::field_descriptor<prcr, 1, 1>;

inline constexpr std::uintptr_t system_base = 0x4001'E000U;
inline constexpr std::uintptr_t mstp_base = 0x4004'7000U;
inline constexpr core::source_ref module_stop_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "10.2.2-10.2.5, PDF pages 175-178"};

inline constexpr std::array system_registers{
    register_record{"SCKDIVCR", 0x4001'E020U, 4, core::access_semantics::read_write, 0x7777'0777U, 0x2100'2122U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, clock_source},
    register_record{"SCKSCR", 0x4001'E026U, 1, core::access_semantics::read_write, 0x07U, 0x01U, 0xFFU, core::reserved_bit_policy::write_zero, clock_source},
    register_record{"PLLCR", 0x4001'E02AU, 1, core::access_semantics::read_write, 0x01U, 0x01U, 0xFFU, core::reserved_bit_policy::write_zero, clock_source},
    register_record{"PLLCCR2", 0x4001'E02BU, 1, core::access_semantics::read_write, 0xDFU, 0x07U, 0xFFU, core::reserved_bit_policy::write_zero, clock_source},
    register_record{"HOCOCR2", 0x4001'E037U, 1, core::access_semantics::read_write, 0x38U, 0U, 0U, core::reserved_bit_policy::write_zero, clock_source},
    register_record{"PRCR", 0x4001'E3FEU, 2, core::access_semantics::write_only, 0x000BU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, {"R01UH0887EJ0110", "Rev.1.10", "12.2.1, PDF pages 224-225"}},
    register_record{"MSTPCRA", 0x4001'E01CU, 4, core::access_semantics::read_write, 0x0040'0041U, 0xFFBF'FFBEU, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, module_stop_source},
    register_record{"MSTPCRB", 0x4004'7000U, 4, core::access_semantics::read_write, 0xE04C'0B04U, 0xFFFF'FFFFU, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, module_stop_source},
    register_record{"MSTPCRD", 0x4004'7008U, 4, core::access_semantics::read_write, 0xA019'406CU, 0xFFFF'FFFFU, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, module_stop_source},
};

// Module-stop controls are gates, not independent resets. RA4M1 peripherals
// share the MCU reset sources; no fabricated per-instance reset is declared.
inline constexpr std::array reset_controls{
    core::reset_control{0, static_cast<std::uint16_t>(clock_id::iclk),
        0xE000'ED0CU, 1U << 2U, true,
        static_cast<std::uint16_t>(protection_group::none),
        {"R01UH0887EJ0110 / ARM DDI 0439D", "Rev.1.10", "5.3.6, AIRCR.SYSRESETREQ"}},
};

inline constexpr std::array protections{
    core::protection_requirement{static_cast<std::uint16_t>(protection_group::prc0_clock), 0x3FEU, true, {"R01UH0887EJ0110", "Rev.1.10", "12.1-12.2.1, PDF pages 223-225"}},
    core::protection_requirement{static_cast<std::uint16_t>(protection_group::prc1_low_power), 0x3FEU, true, {"R01UH0887EJ0110", "Rev.1.10", "12.1-12.2.1, PDF pages 223-225"}},
    core::protection_requirement{static_cast<std::uint16_t>(protection_group::pfs), 0xD03U, true, {"R01UH0887EJ0110", "Rev.1.10", "19.2.6 and 19.3.4, PDF pages 363-366"}},
    core::protection_requirement{static_cast<std::uint16_t>(protection_group::gpt), 0x000U, true, {"R01UH0887EJ0110", "Rev.1.10", "22.2.1 and 22.7, PDF pages 397 and 508"}},
};

} // namespace mcuhw::mcu::ra4m1
