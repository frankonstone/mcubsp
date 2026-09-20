#pragma once

#include <array>
#include <cstdint>

#include <mcuhw/mcu/esp32p4/pins.hpp>

namespace mcuhw::mcu::esp32p4 {

inline constexpr core::source_ref interrupt_source{
    "ESP32-P4 TRM", "pre-release v0.7",
    "Chapter 13, Tables 13.4-1 and register summary, PDF pages 1224-1267"};

// Values are peripheral source numbers from Table 13.4-1, not CPU interrupt
// inputs. The matrix independently routes each source to either HP core.
enum class event : std::uint16_t {
    lp_gpio = 10,
    lp_i2c = 11,
    lp_uart = 16,
    spi2 = 25,
    spi3 = 26,
    uart0 = 31,
    uart1 = 32,
    uart2 = 33,
    uart3 = 34,
    uart4 = 35,
    adc = 37,
    mcpwm0 = 38,
    mcpwm1 = 39,
    i2c0 = 44,
    i2c1 = 45,
    timer_group0_timer0 = 46,
    timer_group0_timer1 = 47,
    timer_group1_timer0 = 49,
    timer_group1_timer1 = 50,
    ledc = 52,
    systimer_target0 = 53,
    systimer_target1 = 54,
    systimer_target2 = 55,
    ahb_gdma_in0 = 56,
    ahb_gdma_out0 = 59,
    axi_gdma_in0 = 62,
    axi_gdma_out0 = 65,
    gpio0 = 74,
    ethernet = 92,
};

[[nodiscard]] constexpr std::uint16_t event_number(event value) noexcept {
    return static_cast<std::uint16_t>(value);
}

[[nodiscard]] constexpr bool documented_event(std::uint16_t value) noexcept {
    return value <= 130U && value != 83U && value != 99U
        && value != 112U && value != 101U && value != 102U;
}

constexpr core::interrupt_route matrix_irq(event source,
                                           std::uint16_t cpu_line) noexcept {
    return {event_number(source), cpu_line,
            core::interrupt_route_kind::interrupt_matrix, cpu_line, false,
            interrupt_source};
}

// Representative, non-exclusive assignments exercise the common contract;
// valid_cpu_route exposes the hardware's full any-source-to-any-input matrix.
inline constexpr std::array interrupt_routes{
    matrix_irq(event::uart0, 1),
    matrix_irq(event::i2c0, 2),
    matrix_irq(event::spi2, 3),
    matrix_irq(event::adc, 4),
    matrix_irq(event::mcpwm0, 5),
    matrix_irq(event::ledc, 6),
    matrix_irq(event::systimer_target0, 7),
    matrix_irq(event::timer_group0_timer0, 8),
    matrix_irq(event::gpio0, 9),
    matrix_irq(event::ahb_gdma_in0, 10),
    matrix_irq(event::axi_gdma_in0, 11),
    matrix_irq(event::ethernet, 12),
};

enum class hp_core : std::uint8_t { core0, core1 };

struct interrupt_target {
    hp_core core;
    std::uintptr_t register_base;
    std::uint8_t input_count;
    core::source_ref provenance;
};

inline constexpr std::uintptr_t interrupt_matrix_base = 0x500D'6000U;
inline constexpr std::array interrupt_targets{
    interrupt_target{hp_core::core0, interrupt_matrix_base + 0x000U, 32, interrupt_source},
    interrupt_target{hp_core::core1, interrupt_matrix_base + 0x400U, 32, interrupt_source},
};

struct interrupt_database {
    [[nodiscard]] static constexpr bool valid_cpu_route(
        event source, std::uint16_t line) noexcept {
        return line < 32U && documented_event(event_number(source));
    }
};

using interrupt_source_map = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x0000'001FU, false,
    core::reserved_bit_policy::write_zero>;
using interrupt_status0 = core::register_descriptor<std::uint32_t, 0x200,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using interrupt_cpu_line = core::field_descriptor<interrupt_source_map, 0, 5>;

inline constexpr std::array interrupt_registers{
    register_record{"CORE0_LP_RTC_INT_MAP_REG", interrupt_matrix_base + 0x000U, 4, core::access_semantics::read_write, 0x1FU, 0U, 0U, core::reserved_bit_policy::write_zero, interrupt_source},
    register_record{"CORE0_UART0_INT_MAP_REG", interrupt_matrix_base + 0x07CU, 4, core::access_semantics::read_write, 0x1FU, 0U, 0U, core::reserved_bit_policy::write_zero, interrupt_source},
    register_record{"CORE0_INTR_STATUS_REG_0_REG", interrupt_matrix_base + 0x200U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, interrupt_source},
    register_record{"CORE1_UART0_INT_MAP_REG", interrupt_matrix_base + 0x400U + 0x07CU, 4, core::access_semantics::read_write, 0x1FU, 0U, 0U, core::reserved_bit_policy::write_zero, interrupt_source},
};

inline constexpr core::source_ref dma_source{
    "ESP32-P4 TRM", "pre-release v0.7",
    "Chapters 6-9; Tables 6.4-1, 6.4-2 and Section 9.3.4"};

inline constexpr auto all_dma_directions = static_cast<std::uint8_t>(
    static_cast<std::uint8_t>(core::dma_direction::memory_to_memory)
    | static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral)
    | static_cast<std::uint8_t>(core::dma_direction::peripheral_to_memory));

inline constexpr core::dma_constraints gdma_ahb_constraints{
    core::dma_engine_kind::esp_gdma_ahb, all_dma_directions, 1U | 2U | 4U,
    1U, 4'095U};
inline constexpr core::dma_constraints gdma_axi_constraints{
    core::dma_engine_kind::esp_gdma_axi, all_dma_directions, 1U | 2U | 4U,
    1U, 4'095U};
struct dma_engine_properties {
    core::dma_engine_kind kind;
    std::uintptr_t base;
    std::uint8_t channel_count;
    std::uint8_t descriptor_alignment;
    bool descriptors_may_be_external;
    std::string_view topology;
    core::source_ref provenance;
};

inline constexpr std::array dma_engines{
    dma_engine_properties{core::dma_engine_kind::esp_gdma_ahb, 0x5008'5000U, 6, 4, false, "3 RX + 3 TX", dma_source},
    dma_engine_properties{core::dma_engine_kind::esp_gdma_axi, 0x5008'A000U, 6, 8, true, "3 RX + 3 TX", dma_source},
    dma_engine_properties{core::dma_engine_kind::esp_vdma, 0x5008'1000U, 4, 64, true, "4 unidirectional source-to-destination", dma_source},
    dma_engine_properties{core::dma_engine_kind::esp_2d_dma, 0x5008'8000U, 7, 8, true, "3 RX + 4 TX", dma_source},
};

// Request numbers are engine-local peripheral-selector encodings.
inline constexpr std::array dma_requests{
    core::dma_request{2, gdma_ahb_constraints, dma_source},  // UHCI
    core::dma_request{3, gdma_ahb_constraints, dma_source},  // I2S0
    core::dma_request{4, gdma_ahb_constraints, dma_source},  // I2S1
    core::dma_request{5, gdma_ahb_constraints, dma_source},  // I2S2
    core::dma_request{8, gdma_ahb_constraints, dma_source},  // ADC
    core::dma_request{10, gdma_ahb_constraints, dma_source}, // RMT
    core::dma_request{0, gdma_axi_constraints, dma_source},  // LCD/CAM
    core::dma_request{1, gdma_axi_constraints, dma_source},  // SPI2
    core::dma_request{2, gdma_axi_constraints, dma_source},  // SPI3
    core::dma_request{3, gdma_axi_constraints, dma_source},  // PARLIO
    core::dma_request{4, gdma_axi_constraints, dma_source},  // AES
    core::dma_request{5, gdma_axi_constraints, dma_source},  // SHA
};

constexpr core::dma_reachability reachable(core::dma_engine_kind engine,
                                           memory_id region) noexcept {
    return {engine, static_cast<std::uint16_t>(region), true, true};
}

inline constexpr std::array dma_reachability{
    reachable(core::dma_engine_kind::esp_gdma_ahb, memory_id::external_flash_cached),
    reachable(core::dma_engine_kind::esp_gdma_ahb, memory_id::external_ram_cached),
    reachable(core::dma_engine_kind::esp_gdma_ahb, memory_id::hp_l2mem_cached),
    reachable(core::dma_engine_kind::esp_gdma_axi, memory_id::external_flash_cached),
    reachable(core::dma_engine_kind::esp_gdma_axi, memory_id::external_ram_cached),
    reachable(core::dma_engine_kind::esp_gdma_axi, memory_id::hp_l2mem_cached),
    reachable(core::dma_engine_kind::esp_vdma, memory_id::external_flash_cached),
    reachable(core::dma_engine_kind::esp_vdma, memory_id::external_ram_cached),
    reachable(core::dma_engine_kind::esp_vdma, memory_id::hp_l2mem_cached),
    reachable(core::dma_engine_kind::esp_vdma, memory_id::mipi_csi),
    reachable(core::dma_engine_kind::esp_vdma, memory_id::mipi_dsi),
    reachable(core::dma_engine_kind::esp_2d_dma, memory_id::external_flash_cached),
    reachable(core::dma_engine_kind::esp_2d_dma, memory_id::external_ram_cached),
    reachable(core::dma_engine_kind::esp_2d_dma, memory_id::hp_l2mem_cached),
};

inline constexpr std::uintptr_t gdma_ahb_base = 0x5008'5000U;
inline constexpr std::uintptr_t gdma_axi_base = 0x5008'A000U;

using ahb_dma_in_peri_sel_ch0 = core::register_descriptor<std::uint32_t, 0xA0,
    core::access_semantics::read_write, 0x3FU, false,
    core::reserved_bit_policy::write_zero>;
using axi_dma_in_peri_sel_ch0 = core::register_descriptor<std::uint32_t, 0x44,
    core::access_semantics::read_write, 0x3FU, false,
    core::reserved_bit_policy::write_zero>;
using gdma_peripheral_select = core::field_descriptor<ahb_dma_in_peri_sel_ch0, 0, 6>;

inline constexpr std::array dma_registers{
    register_record{"AHB_DMA_IN_PERI_SEL_CH0_REG", gdma_ahb_base + 0xA0U, 4, core::access_semantics::read_write, 0x3FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"AHB_DMA_OUT_PERI_SEL_CH0_REG", gdma_ahb_base + 0x100U, 4, core::access_semantics::read_write, 0x3FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"AHB_DMA_IN_LINK_ADDR_CH0_REG", gdma_ahb_base + 0x3ACU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, dma_source},
    register_record{"AXI_DMA_IN_PERI_SEL_CH0_REG", gdma_axi_base + 0x44U, 4, core::access_semantics::read_write, 0x3FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"AXI_DMA_OUT_PERI_SEL_CH0_REG", gdma_axi_base + 0x17CU, 4, core::access_semantics::read_write, 0x3FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
};

} // namespace mcuhw::mcu::esp32p4
