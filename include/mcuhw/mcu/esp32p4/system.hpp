#pragma once

#include <array>
#include <cstdint>

#include <mcuhw/mcu/esp32p4/model.hpp>

namespace mcuhw::mcu::esp32p4 {

struct sdmmc_instance {
    static constexpr std::uintptr_t base = 0x5008'3000U;
    static constexpr std::uintptr_t hp_clock_base = 0x500E'6000U;
    static constexpr std::uintptr_t lp_clock_base = 0x5011'1000U;
    static constexpr std::uintptr_t pmu_base = 0x5011'5000U;
};


inline constexpr core::source_ref identity_source{
    "ESP32-P4 Series Datasheet / Chip Revision v3.x User Guide",
    "pre-release v0.7 / v3.x", "Datasheet Tables 1-1 and 2-1; QFN104; revision guide"};

inline constexpr core::device_identity identity{
    "Espressif Systems", "ESP32-P4", "ESP32-P4NRW16X", "QFN104",
    "production v3.0/v3.1/v3.2", identity_source};

inline constexpr std::array supported_parts{
    part_record{part_variant::nrw16x, "ESP32-P4NRW16X", 16U * 1024U * 1024U, series_datasheet},
    part_record{part_variant::nrw32x, "ESP32-P4NRW32X", 32U * 1024U * 1024U, series_datasheet},
};

inline constexpr std::array silicon_revisions{
    silicon_record{silicon_revision::v3_0, 400'000'000U, true, revision_guide},
    silicon_record{silicon_revision::v3_1, 400'000'000U, true, revision_guide},
    silicon_record{silicon_revision::v3_2, 400'000'000U, true, revision_guide},
};

inline constexpr core::source_ref cpu_source{
    "ESP32-P4 TRM / Series Datasheet", "pre-release v0.7",
    "TRM Chapters 2-3 and 11.1.2; Datasheet Section 1"};
inline constexpr std::array cpu_cores{
    riscv_core_properties{cpu_domain::hp_cpu0, 32, 400'000'000U, 0x3010'0000U, false, cpu_source},
    riscv_core_properties{cpu_domain::hp_cpu1, 32, 400'000'000U, 0x3010'0000U, true, cpu_source},
    riscv_core_properties{cpu_domain::lp_cpu, 32, 40'000'000U, 0x5010'0000U, true, cpu_source},
};

enum class memory_id : std::uint16_t {
    hp_spm,
    external_flash_cached,
    external_ram_cached,
    hp_rom_cached,
    hp_l2mem_cached,
    lp_rom,
    lp_sram,
    external_flash_direct,
    external_ram_direct,
    hp_rom_direct,
    hp_l2mem_direct,
    mipi_csi,
    mipi_dsi,
};

inline constexpr core::source_ref memory_source{
    "ESP32-P4 TRM", "pre-release v0.7",
    "Chapter 9, Tables 9.3-1 and 9.3-2, PDF pages 901-915"};

// The selected NRW16X description exposes 16 MiB of the 64 MiB external-RAM
// virtual window. Direct 0x8... aliases are separately identified as uncached.
inline constexpr std::array memory_regions{
    core::memory_region{static_cast<std::uint16_t>(memory_id::hp_spm), 0x3010'0000U, 8U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::external_flash_cached), 0x4000'0000U, 64U * 1024U * 1024U, true, false, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::external_ram_cached), 0x4800'0000U, 16U * 1024U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::hp_rom_cached), 0x4FC0'0000U, 128U * 1024U, true, false, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::hp_l2mem_cached), 0x4FF0'0000U, 768U * 1024U, true, true, core::cache_policy::software_maintained, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::lp_rom), 0x5010'0000U, 16U * 1024U, true, false, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::lp_sram), 0x5010'8000U, 32U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::external_flash_direct), 0x8000'0000U, 64U * 1024U * 1024U, true, false, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::external_ram_direct), 0x8800'0000U, 16U * 1024U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::hp_rom_direct), 0x8FC0'0000U, 128U * 1024U, true, false, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::hp_l2mem_direct), 0x8FF0'0000U, 768U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::mipi_csi), 0x5010'4000U, 4U * 1024U, true, true, core::cache_policy::uncached, memory_source},
    core::memory_region{static_cast<std::uint16_t>(memory_id::mipi_dsi), 0x5010'5000U, 4U * 1024U, true, true, core::cache_policy::uncached, memory_source},
};

inline constexpr std::array memory_details{
    memory_properties{0, memory_domain::hp_spm, 32, true, true, false, memory_source},
    memory_properties{1, memory_domain::external_flash, 128, true, true, false, memory_source},
    memory_properties{2, memory_domain::external_ram, 128, true, true, false, memory_source},
    memory_properties{3, memory_domain::hp_rom, 128, true, true, false, memory_source},
    memory_properties{4, memory_domain::hp_l2mem, 128, true, true, false, memory_source},
    memory_properties{5, memory_domain::lp_rom, 32, true, true, false, memory_source},
    memory_properties{6, memory_domain::lp_sram, 32, true, true, false, memory_source},
    memory_properties{7, memory_domain::external_flash, 128, true, true, true, memory_source},
    memory_properties{8, memory_domain::external_ram, 128, true, true, true, memory_source},
    memory_properties{9, memory_domain::hp_rom, 128, true, true, true, memory_source},
    memory_properties{10, memory_domain::hp_l2mem, 128, true, true, true, memory_source},
    memory_properties{11, memory_domain::mipi_csi, 32, true, false, false, memory_source},
    memory_properties{12, memory_domain::mipi_dsi, 32, true, false, false, memory_source},
};

inline constexpr core::source_ref clock_source{
    "ESP32-P4 TRM", "pre-release v0.7",
    "Chapter 11, Figure 11.2-1 and Tables 11.2-1 through 11.2-7"};

inline constexpr std::array clock_nodes{
    core::clock_node{static_cast<std::uint16_t>(clock_id::xtal), {40'000'000U}, {40'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::rc_fast), {1U}, {20'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::rc_slow), {150'000U}, {150'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::xtal32k), {32'000U}, {32'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::cpll), {400'000'000U}, {400'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::mpll), {500'000'000U}, {500'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::spll), {480'000'000U}, {480'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::root), {1U}, {400'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::cpu), {1U}, {400'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::mem), {1U}, {200'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::sys), {1U}, {200'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::apb), {1U}, {100'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::lp_fast), {1U}, {40'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::lp_slow), {1U}, {150'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::lp_peripheral), {1U}, {40'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::uart), {1U}, {80'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::i2c), {1U}, {40'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::gpspi), {1U}, {240'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::i2s), {1U}, {160'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::adc), {1U}, {80'000'000U}, clock_source},
    core::clock_node{static_cast<std::uint16_t>(clock_id::pll_f80m), {80'000'000U}, {80'000'000U}, clock_source},
};

constexpr core::clock_edge clock_edge(clock_id source, clock_id destination,
                                      std::uint16_t maximum_divider,
                                      bool fractional = false,
                                      std::uint32_t encoding = 0U) noexcept {
    return {static_cast<std::uint16_t>(source), static_cast<std::uint16_t>(destination),
            1U, maximum_divider, fractional, core::clock_route_kind::selectable,
            encoding, clock_source};
}

inline constexpr std::array clock_edges{
    clock_edge(clock_id::xtal, clock_id::root, 1, false, 0),
    clock_edge(clock_id::cpll, clock_id::root, 1, false, 1),
    clock_edge(clock_id::rc_fast, clock_id::root, 1, false, 2),
    clock_edge(clock_id::root, clock_id::cpu, 8, true),
    clock_edge(clock_id::cpu, clock_id::mem, 8, true),
    clock_edge(clock_id::mem, clock_id::sys, 8, true),
    clock_edge(clock_id::sys, clock_id::apb, 8, true),
    clock_edge(clock_id::rc_fast, clock_id::lp_fast, 1, false, 0),
    clock_edge(clock_id::xtal, clock_id::lp_fast, 1, false, 1),
    clock_edge(clock_id::rc_slow, clock_id::lp_slow, 1, false, 0),
    clock_edge(clock_id::xtal32k, clock_id::lp_slow, 1, false, 1),
    clock_edge(clock_id::lp_fast, clock_id::lp_peripheral, 256, true),
    clock_edge(clock_id::rc_fast, clock_id::uart, 256, true, 1),
    clock_edge(clock_id::xtal, clock_id::i2c, 256, true),
    clock_edge(clock_id::spll, clock_id::gpspi, 256, true),
    clock_edge(clock_id::xtal, clock_id::i2s, 256, true),
    clock_edge(clock_id::spll, clock_id::adc, 256, true),
    clock_edge(clock_id::xtal, clock_id::uart, 256, true, 0),
    clock_edge(clock_id::pll_f80m, clock_id::uart, 256, true, 2),
    core::clock_edge{static_cast<std::uint16_t>(clock_id::cpll),
                     static_cast<std::uint16_t>(clock_id::pll_f80m),
                     5U, 5U, false, core::clock_route_kind::fixed, 0U,
                     clock_source},
};

inline constexpr std::uintptr_t hp_clkrst_base = 0x500E'6000U;
inline constexpr std::uintptr_t hp_soc_clock_ctrl1 = hp_clkrst_base + 0x18U;
inline constexpr std::uintptr_t hp_soc_clock_ctrl2 = hp_clkrst_base + 0x1CU;
inline constexpr std::uintptr_t hp_soc_clock_ctrl3 = hp_clkrst_base + 0x20U;
inline constexpr std::uintptr_t hp_rst_en0 = hp_clkrst_base + 0xC0U;
inline constexpr std::uintptr_t hp_rst_en1 = hp_clkrst_base + 0xC4U;
inline constexpr std::uintptr_t hp_rst_en2 = hp_clkrst_base + 0xC8U;

struct uart_clock_control {
    std::uint8_t instance;
    std::uintptr_t system_gate_register;
    std::uint32_t system_gate_mask;
    std::uintptr_t apb_gate_register;
    std::uint32_t apb_gate_mask;
    std::uintptr_t source_register;
    std::uintptr_t divider_register;
    std::uintptr_t reset_register;
    std::uint32_t core_reset_mask;
    std::uint32_t apb_reset_mask;
    core::source_ref provenance;
};

consteval auto make_uart_clock_controls() {
    std::array<uart_clock_control, 5> result{};
    for (std::uint8_t instance = 0U; instance < result.size(); ++instance) {
        result[instance] = {
            .instance = instance,
            .system_gate_register = hp_soc_clock_ctrl1,
            .system_gate_mask = 1U << (18U + instance),
            .apb_gate_register = hp_soc_clock_ctrl2,
            .apb_gate_mask = 1U << (7U + instance),
            .source_register = hp_clkrst_base + 0x68U + instance * 4U,
            .divider_register = hp_clkrst_base + 0x6CU + instance * 4U,
            .reset_register = hp_rst_en1,
            .core_reset_mask = 1U << (8U + instance),
            .apb_reset_mask = 1U << (13U + instance),
            .provenance = clock_source,
        };
    }
    return result;
}

inline constexpr auto uart_clock_controls = make_uart_clock_controls();

struct spi_clock_control {
    std::uint8_t instance;
    std::uintptr_t system_gate_register;
    std::uint32_t system_gate_mask;
    std::uintptr_t apb_gate_register;
    std::uint32_t apb_gate_mask;
    std::uintptr_t source_register;
    std::uint8_t source_select_shift;
    std::uint8_t high_speed_enable_shift;
    std::uint8_t high_speed_divider_shift;
    std::uint8_t master_divider_shift;
    std::uint8_t master_enable_shift;
    std::uintptr_t reset_register;
    std::uint32_t reset_mask;
    core::source_ref provenance;
};

// GPSPI2/3 use their own system/APB gates and reset bits. GPSPI2's source,
// high-speed pre-divider and master pre-divider share PERI_CLK_CTRL116.
inline constexpr std::array spi_clock_controls{
    spi_clock_control{2U, hp_soc_clock_ctrl1, 1U << 0U,
                      hp_soc_clock_ctrl2, 1U << 19U,
                      hp_clkrst_base + 0x80U, 0U, 3U, 4U, 12U, 20U,
                      hp_rst_en2, 1U << 7U, clock_source},
};

struct i2c_clock_control {
    std::uint8_t instance;
    std::uintptr_t apb_gate_register;
    std::uint32_t apb_gate_mask;
    std::uintptr_t source_register;
    std::uintptr_t divider_register;
    std::uint8_t source_select_shift;
    std::uint8_t controller_enable_shift;
    std::uint8_t divider_shift;
    std::uintptr_t reset_register;
    std::uint32_t reset_mask;
    core::source_ref provenance;
};

// I2C0 keeps its source, controller gate and integer divider in PERI_CLK_CTRL10.
// I2C1 shares the source/gate register and places its divider in CTRL11.
inline constexpr std::array i2c_clock_controls{
    i2c_clock_control{0U, hp_soc_clock_ctrl2, 1U << 19U,
                      hp_clkrst_base + 0x40U, hp_clkrst_base + 0x40U,
                      0U, 1U, 2U,
                      hp_rst_en1, 1U << 9U, clock_source},
    i2c_clock_control{1U, hp_soc_clock_ctrl2, 1U << 18U,
                      hp_clkrst_base + 0x40U, hp_clkrst_base + 0x44U,
                      26U, 27U, 0U,
                      hp_rst_en1, 1U << 10U, clock_source},
};

using hp_root_clock_ctrl0 = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using hp_reset_enable1 = core::register_descriptor<std::uint32_t, 0xC4,
    core::access_semantics::read_write, 0xF7FF'FFFFU, false,
    core::reserved_bit_policy::preserve>;

enum class reset_id : std::uint16_t {
    hp_cpu1, io_mux, systimer, timer_group0, timer_group1,
    uart0, i2c0, mcpwm0, ledc, gdma_ahb, gdma_axi, dma_2d,
};

inline constexpr std::array reset_controls{
    core::reset_control{static_cast<std::uint16_t>(reset_id::hp_cpu1), static_cast<std::uint16_t>(clock_id::cpu), hp_rst_en0, 1U << 28U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::io_mux), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 28U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::systimer), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 26U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::timer_group0), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 25U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::timer_group1), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 24U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::uart0), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, (1U << 8U) | (1U << 13U), true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::i2c0), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 9U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::mcpwm0), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 7U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::ledc), static_cast<std::uint16_t>(clock_id::apb), hp_rst_en1, 1U << 2U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::gdma_ahb), static_cast<std::uint16_t>(clock_id::sys), hp_rst_en1, 1U << 1U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::gdma_axi), static_cast<std::uint16_t>(clock_id::sys), hp_rst_en1, 1U << 29U, true, 0U, clock_source},
    core::reset_control{static_cast<std::uint16_t>(reset_id::dma_2d), static_cast<std::uint16_t>(clock_id::sys), hp_rst_en0, 1U << 0U, true, 0U, clock_source},
};

inline constexpr std::array system_registers{
    register_record{"HP_SYS_CLKRST_ROOT_CLK_CTRL0_REG", hp_clkrst_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_ROOT_CLK_CTRL1_REG", hp_clkrst_base + 0x08U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_ROOT_CLK_CTRL2_REG", hp_clkrst_base + 0x0CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_SOC_CLK_CTRL2_REG", hp_clkrst_base + 0x1CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_PERI_CLK_CTRL10_REG", hp_clkrst_base + 0x40U, 4, core::access_semantics::read_write, 0x0FFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_PERI_CLK_CTRL11_REG", hp_clkrst_base + 0x44U, 4, core::access_semantics::read_write, 0x00FF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_PERI_CLK_CTRL110_REG", hp_clkrst_base + 0x68U, 4, core::access_semantics::read_write, 0x0700'0000U, 0U, 0x0400'0000U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_PERI_CLK_CTRL111_REG", hp_clkrst_base + 0x6CU, 4, core::access_semantics::read_write, 0x07FF'FFFFU, 0U, 0x0400'0000U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_HP_RST_EN0_REG", hp_rst_en0, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
    register_record{"HP_SYS_CLKRST_HP_RST_EN1_REG", hp_rst_en1, 4, core::access_semantics::read_write, 0xF7FF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, clock_source},
};

inline constexpr std::array<core::protection_requirement, 0> protections{};

} // namespace mcuhw::mcu::esp32p4
