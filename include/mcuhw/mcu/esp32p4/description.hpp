#pragma once

#include <array>
#include <span>
#include <type_traits>

#include <mcuhw/mcu/esp32p4/peripherals.hpp>
#include <mcuhw/mcu/esp32p4/coverage.hpp>

namespace mcuhw::mcu::esp32p4 {

enum class peripheral_capability : std::uint8_t {
    gpio_atomic_set_clear,
    programmable_gpio_matrix,
    timer_52_bit,
    timer_54_bit,
    pwm_20_bit,
    pwm_eight_outputs,
    motor_pwm,
    complementary_outputs,
    dead_time,
    i2c_multi_master,
    uart_fifo,
    spi_master,
    spi_slave,
    adc_dual_unit,
    dma,
    external_descriptors,
};

struct ledc_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::pwm_20_bit,
    peripheral_capability::pwm_eight_outputs> {
    static constexpr auto provenance = ledc_source;
};
struct mcpwm_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::motor_pwm,
    peripheral_capability::complementary_outputs,
    peripheral_capability::dead_time> {
    static constexpr auto provenance = mcpwm_source;
};
struct timer_group_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::timer_54_bit> {
    static constexpr auto provenance = timer_group_source;
};
struct gdma_ahb_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::dma> {
    static constexpr auto provenance = dma_source;
};
struct gdma_axi_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::dma,
    peripheral_capability::external_descriptors> {
    static constexpr auto provenance = dma_source;
};
struct i3c_capabilities : core::capability_set<peripheral_capability> {
    static constexpr auto provenance = technical_reference_manual;
};

[[nodiscard]] constexpr std::uint32_t capability_bit(
    peripheral_capability value) noexcept {
    return 1U << static_cast<unsigned>(value);
}

inline constexpr std::uintptr_t hp_soc_clock_ctrl1 = hp_clkrst_base + 0x18U;
inline constexpr std::uintptr_t hp_soc_clock_ctrl2 = hp_clkrst_base + 0x1CU;
inline constexpr std::uintptr_t hp_soc_clock_ctrl3 = hp_clkrst_base + 0x20U;
inline constexpr std::uintptr_t no_control = 0U;
inline constexpr std::uint16_t no_kernel_clock = 0xFFFFU;

constexpr instance_record native_instance(
    peripheral_kind kind, std::uint16_t index, std::uintptr_t base,
    clock_id bus, std::uint16_t kernel, std::uintptr_t clock_register,
    std::uint32_t clock_mask, std::uintptr_t reset_register,
    std::uint32_t reset_mask, std::uint32_t capabilities,
    core::source_ref source) noexcept {
    return {peripheral(kind, index), base, static_cast<std::uint16_t>(bus),
            kernel, clock_register, clock_mask, reset_register, reset_mask,
            capabilities, source};
}

inline constexpr auto gpio_cap = capability_bit(peripheral_capability::gpio_atomic_set_clear)
    | capability_bit(peripheral_capability::programmable_gpio_matrix);
inline constexpr auto systimer_cap = capability_bit(peripheral_capability::timer_52_bit);
inline constexpr auto timg_cap = capability_bit(peripheral_capability::timer_54_bit);
inline constexpr auto ledc_cap = capability_bit(peripheral_capability::pwm_20_bit)
    | capability_bit(peripheral_capability::pwm_eight_outputs);
inline constexpr auto mcpwm_cap = capability_bit(peripheral_capability::motor_pwm)
    | capability_bit(peripheral_capability::complementary_outputs)
    | capability_bit(peripheral_capability::dead_time);
inline constexpr auto transfer_cap = capability_bit(peripheral_capability::dma);

// Gate/reset masks below come from the named Chapter 11 registers. A zero
// control address is explicit: that dependency has not been claimed in this
// reviewed slice and remains in the coverage ledger.
inline constexpr std::array instances{
    native_instance(peripheral_kind::gpio_matrix, 0, gpio_matrix_base, clock_id::apb, no_kernel_clock, no_control, 0, no_control, 0, gpio_cap, gpio_matrix_source),
    native_instance(peripheral_kind::io_mux, 0, io_mux_base, clock_id::apb, no_kernel_clock, no_control, 0, hp_rst_en1, 1U << 28U, gpio_cap, gpio_matrix_source),
    native_instance(peripheral_kind::systimer, 0, systimer_base, clock_id::apb, no_kernel_clock, hp_soc_clock_ctrl2, 1U << 8U, hp_rst_en1, 1U << 26U, systimer_cap, systimer_source),
    native_instance(peripheral_kind::timer_group, 0, timer_group0_base, clock_id::apb, no_kernel_clock, hp_soc_clock_ctrl2, 1U << 10U, hp_rst_en1, 1U << 25U, timg_cap, timer_group_source),
    native_instance(peripheral_kind::timer_group, 1, timer_group1_base, clock_id::apb, no_kernel_clock, hp_soc_clock_ctrl2, 1U << 9U, hp_rst_en1, 1U << 24U, timg_cap, timer_group_source),
    native_instance(peripheral_kind::ledc, 0, ledc_base, clock_id::apb, no_kernel_clock, hp_soc_clock_ctrl3, 1U << 31U, hp_rst_en1, 1U << 2U, ledc_cap, ledc_source),
    native_instance(peripheral_kind::mcpwm, 0, mcpwm0_base, clock_id::apb, no_kernel_clock, hp_soc_clock_ctrl2, 1U << 4U, hp_rst_en1, 1U << 7U, mcpwm_cap, mcpwm_source),
    native_instance(peripheral_kind::mcpwm, 1, mcpwm1_base, clock_id::apb, no_kernel_clock, hp_soc_clock_ctrl2, 1U << 3U, hp_rst_en1, 1U << 6U, mcpwm_cap, mcpwm_source),
    native_instance(peripheral_kind::i2c, 0, i2c0_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::i2c), hp_soc_clock_ctrl2, 1U << 19U, hp_rst_en1, 1U << 9U, capability_bit(peripheral_capability::i2c_multi_master), i2c_source),
    native_instance(peripheral_kind::i2c, 1, i2c1_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::i2c), hp_soc_clock_ctrl2, 1U << 18U, hp_rst_en1, 1U << 10U, capability_bit(peripheral_capability::i2c_multi_master), i2c_source),
    native_instance(peripheral_kind::uart, 0, uart0_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::uart), hp_soc_clock_ctrl2, 1U << 24U, hp_rst_en1, (1U << 23U) | (1U << 18U), capability_bit(peripheral_capability::uart_fifo), uart_source),
    native_instance(peripheral_kind::uart, 1, uart1_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::uart), hp_soc_clock_ctrl2, 1U << 23U, hp_rst_en1, (1U << 22U) | (1U << 17U), capability_bit(peripheral_capability::uart_fifo), uart_source),
    native_instance(peripheral_kind::uart, 2, uart2_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::uart), hp_soc_clock_ctrl2, 1U << 22U, hp_rst_en1, (1U << 21U) | (1U << 16U), capability_bit(peripheral_capability::uart_fifo), uart_source),
    native_instance(peripheral_kind::uart, 3, uart3_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::uart), hp_soc_clock_ctrl2, 1U << 21U, hp_rst_en1, (1U << 20U) | (1U << 15U), capability_bit(peripheral_capability::uart_fifo), uart_source),
    native_instance(peripheral_kind::uart, 4, uart4_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::uart), hp_soc_clock_ctrl2, 1U << 20U, hp_rst_en1, (1U << 19U) | (1U << 14U), capability_bit(peripheral_capability::uart_fifo), uart_source),
    native_instance(peripheral_kind::spi, 2, spi2_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::gpspi), hp_soc_clock_ctrl2, 1U << 12U, no_control, 0, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::spi, 3, spi3_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::gpspi), hp_soc_clock_ctrl2, 1U << 11U, no_control, 0, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::adc, 1, adc_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::adc), hp_soc_clock_ctrl2, 1U << 26U, no_control, 0, capability_bit(peripheral_capability::adc_dual_unit), adc_source),
    native_instance(peripheral_kind::adc, 2, adc_base, clock_id::apb, static_cast<std::uint16_t>(clock_id::adc), hp_soc_clock_ctrl2, 1U << 26U, no_control, 0, capability_bit(peripheral_capability::adc_dual_unit), adc_source),
    native_instance(peripheral_kind::gdma_ahb, 0, gdma_ahb_base, clock_id::sys, no_kernel_clock, hp_soc_clock_ctrl1, 1U << 28U, hp_rst_en1, 1U << 30U, transfer_cap, dma_source),
    native_instance(peripheral_kind::gdma_axi, 0, gdma_axi_base, clock_id::sys, no_kernel_clock, hp_soc_clock_ctrl1, 1U << 27U, hp_rst_en1, 1U << 29U, transfer_cap | capability_bit(peripheral_capability::external_descriptors), dma_source),
    native_instance(peripheral_kind::vdma, 0, 0x5008'1000U, clock_id::sys, no_kernel_clock, no_control, 0, no_control, 0, transfer_cap | capability_bit(peripheral_capability::external_descriptors), dma_source),
    native_instance(peripheral_kind::dma_2d, 0, 0x5008'8000U, clock_id::sys, no_kernel_clock, hp_soc_clock_ctrl1, 1U << 25U, hp_rst_en0, 1U << 0U, transfer_cap | capability_bit(peripheral_capability::external_descriptors), dma_source),
};

inline constexpr std::array apb_clock{static_cast<std::uint16_t>(clock_id::apb)};
inline constexpr std::array sys_clock{static_cast<std::uint16_t>(clock_id::sys)};
inline constexpr std::array uart_clocks{static_cast<std::uint16_t>(clock_id::apb), static_cast<std::uint16_t>(clock_id::uart)};
inline constexpr std::array i2c_clocks{static_cast<std::uint16_t>(clock_id::apb), static_cast<std::uint16_t>(clock_id::i2c)};
inline constexpr std::array spi_clocks{static_cast<std::uint16_t>(clock_id::apb), static_cast<std::uint16_t>(clock_id::gpspi)};
inline constexpr std::array adc_clocks{static_cast<std::uint16_t>(clock_id::apb), static_cast<std::uint16_t>(clock_id::adc)};
inline constexpr std::array<std::uint16_t, 0> no_relations{};
inline constexpr std::array uart0_events{event_number(event::uart0)};
inline constexpr std::array i2c0_events{event_number(event::i2c0)};
inline constexpr std::array spi2_events{event_number(event::spi2)};
inline constexpr std::array adc_events{event_number(event::adc)};
inline constexpr std::array ledc_events{event_number(event::ledc)};
inline constexpr std::array mcpwm0_events{event_number(event::mcpwm0)};
inline constexpr std::array adc_dma{std::uint16_t{8}};
inline constexpr std::array spi2_dma{std::uint16_t{1}};
inline constexpr std::array uart_dma{std::uint16_t{2}};

inline constexpr std::array peripheral_instances{
    core::peripheral_metadata{peripheral(peripheral_kind::uart, 0), uart_clocks, no_relations, uart0_events, uart_dma, uart_source},
    core::peripheral_metadata{peripheral(peripheral_kind::i2c, 0), i2c_clocks, no_relations, i2c0_events, no_relations, i2c_source},
    core::peripheral_metadata{peripheral(peripheral_kind::spi, 2), spi_clocks, no_relations, spi2_events, spi2_dma, spi_source},
    core::peripheral_metadata{peripheral(peripheral_kind::adc, 1), adc_clocks, no_relations, adc_events, adc_dma, adc_source},
    core::peripheral_metadata{peripheral(peripheral_kind::ledc, 0), apb_clock, no_relations, ledc_events, no_relations, ledc_source},
    core::peripheral_metadata{peripheral(peripheral_kind::mcpwm, 0), apb_clock, no_relations, mcpwm0_events, no_relations, mcpwm_source},
    core::peripheral_metadata{peripheral(peripheral_kind::gdma_ahb, 0), sys_clock, no_relations, no_relations, no_relations, dma_source},
    core::peripheral_metadata{peripheral(peripheral_kind::gdma_axi, 0), sys_clock, no_relations, no_relations, no_relations, dma_source},
};

inline constexpr std::array source_gaps{
    source_gap{"GAP-ESP32P4-001", "unavailable-in-source-exclusion-accepted",
        "TRM v0.7 announces I3C but contains no I3C chapter; D09 excludes all I3C instances and placeholders from the accepted gate."},
    source_gap{"GAP-ESP32P4-002", "preliminary-source",
        "All TRM/datasheet definitions require re-review when Espressif publishes a new pinned revision."},
    source_gap{"GAP-ESP32P4-003", "source-conflict-recorded",
        "TRM v0.7 Figure 11.2-1 and the datasheet specify 400 MHz, while nearby TRM prose says 360 MHz; production-v3.x metadata retains the published 400 MHz device limit."},
};

inline constexpr std::array review_ledger{
    block_review{"identity, QFN104 and production v3.x", coverage_state::verified, 1, 1, "Other package families are outside the declared baseline", identity_source},
    block_review{"HP/LP RISC-V ownership", coverage_state::verified, 3, 3, "Boot, startup and concurrent execution are PLN-0019", cpu_source},
    block_review{"memory map, cached/direct aliases", coverage_state::verified, memory_regions.size(), memory_regions.size(), "Cache initialization and permission programming are PLN-0019", memory_source},
    block_review{"clock roots and HP/LP domains", coverage_state::verified, clock_nodes.size(), clock_nodes.size(), "Frequency source conflict retained as GAP-ESP32P4-003", clock_source},
    block_review{"reset controls and dependencies", coverage_state::verified, reset_controls.size(), reset_controls.size(), "Full source-unit scope reconciled in complete_source_inventory", clock_source},
    block_review{"QFN104 GPIO ownership and IO MUX", coverage_state::verified, package_pins.size(), package_pins.size(), "Full source-unit scope reconciled in complete_source_inventory", gpio_matrix_source},
    block_review{"HP interrupt matrix initial sources", coverage_state::verified, interrupt_routes.size(), interrupt_routes.size(), "Full source-unit scope reconciled in complete_source_inventory", interrupt_source},
    block_review{"GDMA-AHB/AXI topology and requests", coverage_state::verified, dma_requests.size(), dma_requests.size(), "Full source-unit scope reconciled in complete_source_inventory", dma_source},
    block_review{"VDMA and 2D-DMA topology/reachability", coverage_state::verified, 2, 2, "Full source-unit scope reconciled in complete_source_inventory", dma_source},
    block_review{"system timer and timer groups", coverage_state::verified, 6, 6, "Full source-unit scope reconciled in complete_source_inventory", timer_group_source},
    block_review{"LEDC real PWM", coverage_state::verified, 4, 4, "Full source-unit scope reconciled in complete_source_inventory", ledc_source},
    block_review{"MCPWM motor-control PWM", coverage_state::verified, 4, 4, "Full source-unit scope reconciled in complete_source_inventory", mcpwm_source},
    block_review{"UART initial registers", coverage_state::verified, 3, 3, "Full source-unit scope reconciled in complete_source_inventory", uart_source},
    block_review{"SPI initial registers", coverage_state::verified, 2, 2, "Full source-unit scope reconciled in complete_source_inventory", spi_source},
    block_review{"I2C initial registers", coverage_state::verified, 3, 3, "Full source-unit scope reconciled in complete_source_inventory", i2c_source},
    block_review{"ADC units, pins and DMA", coverage_state::verified, adc_registers.size(), adc_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", adc_source},
    block_review{"I3C announced chapter", coverage_state::unavailable_in_source, 1, 0, "GAP-ESP32P4-001; compile-time capability remains absent", technical_reference_manual},
};

// Declarative metadata only: no MMIO execution, startup, interrupt dispatch,
// cache/permission setup, DMA sequencing, multicore boot, or RTOS integration.
// @req REQ-0003 REQ-0004 REQ-0005 REQ-0006 REQ-0007 REQ-0010 REQ-0011 REQ-0012 REQ-0013 REQ-0034 REQ-0040
struct description {
    static constexpr auto device = identity;
    static constexpr auto source_units = std::span{complete_source_inventory};
    static constexpr auto memories = std::span{memory_regions};
    static constexpr auto clocks = std::span{clock_nodes};
    static constexpr auto clock_relations = std::span{clock_edges};
    static constexpr auto resets = std::span{reset_controls};
    static constexpr auto pins = std::span{package_pins};
    static constexpr auto routes = std::span{pin_routes};
    static constexpr auto irqs = std::span{interrupt_routes};
    static constexpr auto transfers = std::span{dma_requests};
    static constexpr auto reachability = std::span{dma_reachability};
    static constexpr auto peripherals = std::span{peripheral_instances};
    static constexpr auto native_instances = std::span{instances};
    using pin_database_type = qfn104_pin_database;
    using representative_register = gpio_out;
    using representative_field = gpio0_output;
};

static_assert(std::is_trivial_v<description>);

} // namespace mcuhw::mcu::esp32p4
