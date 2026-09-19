#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <mcuhw/mcu/stm32h753/model.hpp>

namespace mcuhw::mcu::stm32h753 {

inline constexpr core::source_ref identity_source{
    "DS12117 / ES0392", "Rev.10 / Rev.15",
    "DS12117 Tables 1-2 and Sections 5, 8-9; ES0392 Tables 1-3"};

// STM32H753VI in LQFP100 is the exact reference identity used by the common
// description contract. supported_packages retains the complete xI baseline.
inline constexpr core::device_identity identity{
    "STMicroelectronics", "STM32H7", "STM32H753VI",
    "LQFP100", "Y/W (0x1003), X (0x2001), V (0x2003)", identity_source};

inline constexpr std::array supported_packages{
    package_record{package_variant::lqfp100, "VI", "LQFP100", 100, datasheet},
    package_record{package_variant::tfbga100, "VI", "TFBGA100", 100, datasheet},
    package_record{package_variant::lqfp144, "ZI", "LQFP144", 144, datasheet},
    package_record{package_variant::ufbga169, "II", "UFBGA169", 169, datasheet},
    package_record{package_variant::lqfp176, "II", "LQFP176", 176, datasheet},
    package_record{package_variant::ufbga176_plus_25, "BI", "UFBGA176+25", 201, datasheet},
    package_record{package_variant::lqfp208, "XI", "LQFP208", 208, datasheet},
    package_record{package_variant::tfbga240_plus_25, "AI", "TFBGA240+25", 265, datasheet},
};

inline constexpr std::array silicon_revisions{
    silicon_record{silicon_revision::yw, 0x1003U, 400'000'000U, errata_sheet},
    silicon_record{silicon_revision::x, 0x2001U, 400'000'000U, errata_sheet},
    silicon_record{silicon_revision::v, 0x2003U, 480'000'000U, errata_sheet},
};

inline constexpr core::source_ref core_source{
    "DS12117 / PM0253", "Rev.10 / Rev.6",
    "DS12117 Sections 1 and 3.2; PM0253 Table 2 and Sections 4.5-4.6"};
inline constexpr cortex_m7_properties core_properties{
    480'000'000U, 16U * 1024U, 16U * 1024U, 16U, true, core_source};

enum class memory_id : std::uint16_t {
    flash_bank1,
    flash_bank2,
    itcm,
    dtcm,
    axi_sram,
    sram1,
    sram2,
    sram3,
    sram4,
    backup_sram,
};

inline constexpr core::source_ref memory_source{
    "RM0433 / DS12117", "Rev.8 / Rev.10",
    "RM0433 Table 7 and Section 2.4, PDF pages 130-137; DS12117 Section 3.3.3"};

inline constexpr std::array memory_regions{
    core::memory_region{static_cast<std::uint16_t>(memory_id::flash_bank1), 0x0800'0000U, 1024U * 1024U, true, false, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::flash_bank2), 0x0810'0000U, 1024U * 1024U, true, false, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::itcm), 0x0000'0000U, 64U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::dtcm), 0x2000'0000U, 128U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::axi_sram), 0x2400'0000U, 512U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::sram1), 0x3000'0000U, 128U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::sram2), 0x3002'0000U, 128U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::sram3), 0x3004'0000U, 32U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::sram4), 0x3800'0000U, 64U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::backup_sram), 0x3880'0000U, 4U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
};

inline constexpr std::array memory_details{
    memory_properties{0, memory_domain::flash, 64, true, true, false, memory_source},
    memory_properties{1, memory_domain::flash, 64, true, true, false, memory_source},
    memory_properties{2, memory_domain::tcm, 64, true, false, false, memory_source},
    memory_properties{3, memory_domain::tcm, 64, true, false, false, memory_source},
    memory_properties{4, memory_domain::d1, 64, true, true, true, memory_source},
    memory_properties{5, memory_domain::d2, 32, true, true, true, memory_source},
    memory_properties{6, memory_domain::d2, 32, true, true, true, memory_source},
    memory_properties{7, memory_domain::d2, 32, true, true, true, memory_source},
    memory_properties{8, memory_domain::d3, 32, true, true, true, memory_source},
    memory_properties{9, memory_domain::backup, 32, false, true, true, memory_source},
};

inline constexpr core::source_ref clock_source{
    "RM0433 + ST correction overlay", "Rev.8 + 2026-09-13 review",
    "RM0433 Chapter 8, Tables 60 and 84; corrected ADC maxima 80/80/40/40 MHz"};

inline constexpr std::array clock_nodes{
    core::clock_node{static_cast<std::uint16_t>(clock_id::hsi), {64'000'000U}, {64'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::csi), {4'000'000U}, {4'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::hse), {4'000'000U}, {48'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pll1_p), {1U}, {480'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pll2_p), {1U}, {480'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pll3_r), {1U}, {480'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::per_ck), {1U}, {64'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::sys_ck), {1U}, {480'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::hclk), {1U}, {240'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclk1), {1U}, {120'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclk2), {1U}, {120'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclk3), {1U}, {120'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pclk4), {1U}, {120'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::timx_ker), {1U}, {240'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::timy_ker), {1U}, {240'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::adc_ker), {1U}, {80'000'000U}, clock_source},
};

constexpr core::clock_edge clock_edge(clock_id source, clock_id destination,
                                      std::uint16_t max_divider,
                                      std::uint32_t encoding = 0U) noexcept {
    return {static_cast<std::uint16_t>(source), static_cast<std::uint16_t>(destination),
            1, max_divider, false, core::clock_route_kind::selectable,
            encoding, clock_source};
}

inline constexpr std::array clock_edges{
    clock_edge(clock_id::hsi, clock_id::sys_ck, 1, 0U),
    clock_edge(clock_id::csi, clock_id::sys_ck, 1, 1U),
    clock_edge(clock_id::hse, clock_id::sys_ck, 1, 2U),
    clock_edge(clock_id::pll1_p, clock_id::sys_ck, 1, 3U),
    clock_edge(clock_id::sys_ck, clock_id::hclk, 512),
    clock_edge(clock_id::hclk, clock_id::pclk1, 16),
    clock_edge(clock_id::hclk, clock_id::pclk2, 16),
    clock_edge(clock_id::hclk, clock_id::pclk3, 16),
    clock_edge(clock_id::hclk, clock_id::pclk4, 16),
    clock_edge(clock_id::pclk1, clock_id::timx_ker, 1),
    clock_edge(clock_id::pclk2, clock_id::timy_ker, 1),
    clock_edge(clock_id::pll2_p, clock_id::adc_ker, 256, 0U),
    clock_edge(clock_id::pll3_r, clock_id::adc_ker, 256, 1U),
    clock_edge(clock_id::per_ck, clock_id::adc_ker, 256, 2U),
};

inline constexpr std::uintptr_t rcc_base = 0x5802'4400U;
using rcc_cr = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x3FFF'FFFFU, false,
    core::reserved_bit_policy::preserve, 0U, 0x0000'0083U, 0xFFFF'FFFFU>;
using rcc_cfgr = core::register_descriptor<std::uint32_t, 0x10,
    core::access_semantics::read_write, 0x0007'FFFFU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;
using rcc_d1cfgr = core::register_descriptor<std::uint32_t, 0x18,
    core::access_semantics::read_write, 0x0000'0F7FU, false,
    core::reserved_bit_policy::preserve>;
using rcc_ahb4enr = core::register_descriptor<std::uint32_t, 0xE0,
    core::access_semantics::read_write, 0x3FF0'07FFU, true,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;
using rcc_apb1lenr = core::register_descriptor<std::uint32_t, 0xE8,
    core::access_semantics::read_write, 0xFFFF'FFFFU, true,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;
using rcc_apb2enr = core::register_descriptor<std::uint32_t, 0xF0,
    core::access_semantics::read_write, 0xFFFF'FFFFU, true,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;

using sys_clock_switch = core::field_descriptor<rcc_cfgr, 0, 3>;
using hpre = core::field_descriptor<rcc_d1cfgr, 0, 4>;
using d1ppre = core::field_descriptor<rcc_d1cfgr, 4, 3>;

inline constexpr std::array system_registers{
    register_record{"RCC_CR", rcc_base + 0x00U, 4, core::access_semantics::read_write, 0x3FFF'FFFFU, 0x83U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_CFGR", rcc_base + 0x10U, 4, core::access_semantics::read_write, 0x0007'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_D1CFGR", rcc_base + 0x18U, 4, core::access_semantics::read_write, 0x0000'0F7FU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_D2CFGR", rcc_base + 0x1CU, 4, core::access_semantics::read_write, 0x0000'0F77U, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_D3CFGR", rcc_base + 0x20U, 4, core::access_semantics::read_write, 0x0000'0070U, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_D2CCIP2R", rcc_base + 0x54U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_D3CCIPR", rcc_base + 0x58U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_AHB4ENR", rcc_base + 0xE0U, 4, core::access_semantics::read_write, 0x3FF0'07FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_APB1LENR", rcc_base + 0xE8U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, clock_source},
    register_record{"RCC_APB2ENR", rcc_base + 0xF0U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, clock_source},
};

enum class reset_id : std::uint16_t {
    gpioa,
    dma1,
    dma2,
    tim1,
    tim2,
    i2c1,
    usart1,
    spi1,
    adc12,
};

inline constexpr std::array reset_controls{
    core::reset_control{static_cast<std::uint16_t>(reset_id::gpioa), static_cast<std::uint16_t>(clock_id::hclk), rcc_base + 0x88U, 1U << 0U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::dma1), static_cast<std::uint16_t>(clock_id::hclk), rcc_base + 0x80U, 1U << 0U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::dma2), static_cast<std::uint16_t>(clock_id::hclk), rcc_base + 0x80U, 1U << 1U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::tim1), static_cast<std::uint16_t>(clock_id::pclk2), rcc_base + 0x98U, 1U << 0U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::tim2), static_cast<std::uint16_t>(clock_id::pclk1), rcc_base + 0x90U, 1U << 0U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::i2c1), static_cast<std::uint16_t>(clock_id::pclk1), rcc_base + 0x90U, 1U << 21U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::usart1), static_cast<std::uint16_t>(clock_id::pclk2), rcc_base + 0x98U, 1U << 4U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::spi1), static_cast<std::uint16_t>(clock_id::pclk2), rcc_base + 0x98U, 1U << 12U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::adc12), static_cast<std::uint16_t>(clock_id::hclk), rcc_base + 0x80U, 1U << 5U, true, 0U, clock_source},
};

inline constexpr std::array<core::protection_requirement, 0> protections{};

} // namespace mcuhw::mcu::stm32h753
