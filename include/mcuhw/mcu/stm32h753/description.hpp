#pragma once

#include <array>
#include <span>
#include <type_traits>

#include <mcuhw/mcu/stm32h753/peripherals.hpp>
#include <mcuhw/mcu/stm32h753/coverage.hpp>

namespace mcuhw::mcu::stm32h753 {

enum class peripheral_capability : std::uint8_t {
    gpio_atomic_set_reset,
    timer_16_bit,
    timer_32_bit,
    center_aligned,
    complementary_outputs,
    dead_time,
    i2c_multi_master,
    usart_fifo,
    synchronous_usart,
    spi_master,
    spi_slave,
    adc_16_bit,
    dma,
    domain_limited_dma,
    tcm_access,
};

struct advanced_timer_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::timer_16_bit,
    peripheral_capability::center_aligned,
    peripheral_capability::complementary_outputs,
    peripheral_capability::dead_time> {
    static constexpr auto provenance = timer_source;
};
struct general_timer32_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::timer_32_bit,
    peripheral_capability::center_aligned> {
    static constexpr auto provenance = timer_source;
};
struct usart_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::usart_fifo,
    peripheral_capability::synchronous_usart> {
    static constexpr auto provenance = usart_source;
};
struct dma12_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::dma> {
    static constexpr auto provenance = dma_source;
};
struct bdma_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::dma,
    peripheral_capability::domain_limited_dma> {
    static constexpr auto provenance = dma_source;
};
struct mdma_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::dma,
    peripheral_capability::tcm_access> {
    static constexpr auto provenance = dma_source;
};

[[nodiscard]] constexpr std::uint32_t capability_bit(
    peripheral_capability value) noexcept {
    return 1U << static_cast<unsigned>(value);
}

inline constexpr auto no_kernel_clock = std::uint16_t{0xFFFFU};

constexpr instance_record native_instance(
    peripheral_kind kind, std::uint16_t index, std::uintptr_t base,
    clock_id bus, std::uint16_t kernel, std::uintptr_t enable_register,
    std::uint32_t enable_mask, std::uintptr_t reset_register,
    std::uint32_t capabilities, core::source_ref source) noexcept {
    return {peripheral(kind, index), base, static_cast<std::uint16_t>(bus),
            kernel, enable_register, enable_mask, reset_register, enable_mask,
            capabilities, source};
}

inline constexpr auto gpio_cap = capability_bit(
    peripheral_capability::gpio_atomic_set_reset);
inline constexpr auto tim16_cap = capability_bit(peripheral_capability::timer_16_bit);
inline constexpr auto tim32_cap = capability_bit(peripheral_capability::timer_32_bit)
    | capability_bit(peripheral_capability::center_aligned);
inline constexpr auto advanced_tim_cap = tim16_cap
    | capability_bit(peripheral_capability::center_aligned)
    | capability_bit(peripheral_capability::complementary_outputs)
    | capability_bit(peripheral_capability::dead_time);
inline constexpr auto transfer_cap = capability_bit(peripheral_capability::dma);

inline constexpr std::uintptr_t ahb3enr = rcc_base + 0xD4U;
inline constexpr std::uintptr_t ahb1enr = rcc_base + 0xD8U;
inline constexpr std::uintptr_t ahb4enr = rcc_base + 0xE0U;
inline constexpr std::uintptr_t apb1lenr = rcc_base + 0xE8U;
inline constexpr std::uintptr_t apb2enr = rcc_base + 0xF0U;
inline constexpr std::uintptr_t apb4enr = rcc_base + 0xF4U;

// All instances in the initial categories are retained even where only a
// representative register catalogue or pin route slice is reviewed.
inline constexpr std::array instances{
    native_instance(peripheral_kind::gpio, 0, gpio_base(0), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 0, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 1, gpio_base(1), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 1, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 2, gpio_base(2), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 2, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 3, gpio_base(3), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 3, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 4, gpio_base(4), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 4, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 5, gpio_base(5), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 5, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 6, gpio_base(6), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 6, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 7, gpio_base(7), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 7, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 8, gpio_base(8), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 8, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 9, gpio_base(9), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 9, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::gpio, 10, gpio_base(10), clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 10, rcc_base + 0x88U, gpio_cap, alternate_function_source),
    native_instance(peripheral_kind::timer, 1, 0x4001'0000U, clock_id::pclk2, static_cast<std::uint16_t>(clock_id::timy_ker), apb2enr, 1U << 0, rcc_base + 0x98U, advanced_tim_cap, timer_source),
    native_instance(peripheral_kind::timer, 2, 0x4000'0000U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 0, rcc_base + 0x90U, tim32_cap, timer_source),
    native_instance(peripheral_kind::timer, 3, 0x4000'0400U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 1, rcc_base + 0x90U, tim16_cap | capability_bit(peripheral_capability::center_aligned), timer_source),
    native_instance(peripheral_kind::timer, 4, 0x4000'0800U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 2, rcc_base + 0x90U, tim16_cap | capability_bit(peripheral_capability::center_aligned), timer_source),
    native_instance(peripheral_kind::timer, 5, 0x4000'0C00U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 3, rcc_base + 0x90U, tim32_cap, timer_source),
    native_instance(peripheral_kind::timer, 8, 0x4001'0400U, clock_id::pclk2, static_cast<std::uint16_t>(clock_id::timy_ker), apb2enr, 1U << 1, rcc_base + 0x98U, advanced_tim_cap, timer_source),
    native_instance(peripheral_kind::timer, 12, 0x4000'1800U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 6, rcc_base + 0x90U, tim16_cap, timer_source),
    native_instance(peripheral_kind::timer, 13, 0x4000'1C00U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 7, rcc_base + 0x90U, tim16_cap, timer_source),
    native_instance(peripheral_kind::timer, 14, 0x4000'2000U, clock_id::pclk1, static_cast<std::uint16_t>(clock_id::timx_ker), apb1lenr, 1U << 8, rcc_base + 0x90U, tim16_cap, timer_source),
    native_instance(peripheral_kind::timer, 15, 0x4001'4000U, clock_id::pclk2, static_cast<std::uint16_t>(clock_id::timy_ker), apb2enr, 1U << 16, rcc_base + 0x98U, tim16_cap | capability_bit(peripheral_capability::complementary_outputs) | capability_bit(peripheral_capability::dead_time), timer_source),
    native_instance(peripheral_kind::timer, 16, 0x4001'4400U, clock_id::pclk2, static_cast<std::uint16_t>(clock_id::timy_ker), apb2enr, 1U << 17, rcc_base + 0x98U, tim16_cap | capability_bit(peripheral_capability::complementary_outputs) | capability_bit(peripheral_capability::dead_time), timer_source),
    native_instance(peripheral_kind::timer, 17, 0x4001'4800U, clock_id::pclk2, static_cast<std::uint16_t>(clock_id::timy_ker), apb2enr, 1U << 18, rcc_base + 0x98U, tim16_cap | capability_bit(peripheral_capability::complementary_outputs) | capability_bit(peripheral_capability::dead_time), timer_source),
    native_instance(peripheral_kind::i2c, 1, 0x4000'5400U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 21, rcc_base + 0x90U, capability_bit(peripheral_capability::i2c_multi_master), i2c_source),
    native_instance(peripheral_kind::i2c, 2, 0x4000'5800U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 22, rcc_base + 0x90U, capability_bit(peripheral_capability::i2c_multi_master), i2c_source),
    native_instance(peripheral_kind::i2c, 3, 0x4000'5C00U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 23, rcc_base + 0x90U, capability_bit(peripheral_capability::i2c_multi_master), i2c_source),
    native_instance(peripheral_kind::i2c, 4, 0x5800'1C00U, clock_id::pclk4, no_kernel_clock, apb4enr, 1U << 7, rcc_base + 0xA0U, capability_bit(peripheral_capability::i2c_multi_master), i2c_source),
    native_instance(peripheral_kind::usart, 1, 0x4001'1000U, clock_id::pclk2, no_kernel_clock, apb2enr, 1U << 4, rcc_base + 0x98U, capability_bit(peripheral_capability::usart_fifo) | capability_bit(peripheral_capability::synchronous_usart), usart_source),
    native_instance(peripheral_kind::usart, 2, 0x4000'4400U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 17, rcc_base + 0x90U, capability_bit(peripheral_capability::usart_fifo) | capability_bit(peripheral_capability::synchronous_usart), usart_source),
    native_instance(peripheral_kind::usart, 3, 0x4000'4800U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 18, rcc_base + 0x90U, capability_bit(peripheral_capability::usart_fifo) | capability_bit(peripheral_capability::synchronous_usart), usart_source),
    native_instance(peripheral_kind::usart, 4, 0x4000'4C00U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 19, rcc_base + 0x90U, capability_bit(peripheral_capability::usart_fifo), usart_source),
    native_instance(peripheral_kind::usart, 5, 0x4000'5000U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 20, rcc_base + 0x90U, capability_bit(peripheral_capability::usart_fifo), usart_source),
    native_instance(peripheral_kind::usart, 6, 0x4001'1400U, clock_id::pclk2, no_kernel_clock, apb2enr, 1U << 5, rcc_base + 0x98U, capability_bit(peripheral_capability::usart_fifo) | capability_bit(peripheral_capability::synchronous_usart), usart_source),
    native_instance(peripheral_kind::usart, 7, 0x4000'7800U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 30, rcc_base + 0x90U, capability_bit(peripheral_capability::usart_fifo), usart_source),
    native_instance(peripheral_kind::usart, 8, 0x4000'7C00U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 31, rcc_base + 0x90U, capability_bit(peripheral_capability::usart_fifo), usart_source),
    native_instance(peripheral_kind::usart, 9, 0x5800'0C00U, clock_id::pclk4, no_kernel_clock, apb4enr, 1U << 3, rcc_base + 0xA0U, capability_bit(peripheral_capability::usart_fifo), usart_source),
    native_instance(peripheral_kind::spi, 1, 0x4001'3000U, clock_id::pclk2, no_kernel_clock, apb2enr, 1U << 12, rcc_base + 0x98U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::spi, 2, 0x4000'3800U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 14, rcc_base + 0x90U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::spi, 3, 0x4000'3C00U, clock_id::pclk1, no_kernel_clock, apb1lenr, 1U << 15, rcc_base + 0x90U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::spi, 4, 0x4001'3400U, clock_id::pclk2, no_kernel_clock, apb2enr, 1U << 13, rcc_base + 0x98U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::spi, 5, 0x4001'5000U, clock_id::pclk2, no_kernel_clock, apb2enr, 1U << 20, rcc_base + 0x98U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::spi, 6, 0x5800'1400U, clock_id::pclk4, no_kernel_clock, apb4enr, 1U << 5, rcc_base + 0xA0U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source),
    native_instance(peripheral_kind::adc, 1, 0x4002'2000U, clock_id::hclk, static_cast<std::uint16_t>(clock_id::adc_ker), ahb1enr, 1U << 5, rcc_base + 0x80U, capability_bit(peripheral_capability::adc_16_bit), adc_source),
    native_instance(peripheral_kind::adc, 2, 0x4002'2100U, clock_id::hclk, static_cast<std::uint16_t>(clock_id::adc_ker), ahb1enr, 1U << 5, rcc_base + 0x80U, capability_bit(peripheral_capability::adc_16_bit), adc_source),
    native_instance(peripheral_kind::adc, 3, 0x5802'6000U, clock_id::hclk, static_cast<std::uint16_t>(clock_id::adc_ker), ahb4enr, 1U << 24, rcc_base + 0x88U, capability_bit(peripheral_capability::adc_16_bit), adc_source),
    native_instance(peripheral_kind::dma, 1, dma1_base, clock_id::hclk, no_kernel_clock, ahb1enr, 1U << 0, rcc_base + 0x80U, transfer_cap, dma_source),
    native_instance(peripheral_kind::dma, 2, dma2_base, clock_id::hclk, no_kernel_clock, ahb1enr, 1U << 1, rcc_base + 0x80U, transfer_cap, dma_source),
    native_instance(peripheral_kind::bdma, 1, 0x5802'5400U, clock_id::hclk, no_kernel_clock, ahb4enr, 1U << 21, rcc_base + 0x88U, transfer_cap | capability_bit(peripheral_capability::domain_limited_dma), dma_source),
    native_instance(peripheral_kind::mdma, 1, 0x5200'0000U, clock_id::hclk, no_kernel_clock, ahb3enr, 1U << 0, rcc_base + 0x7CU, transfer_cap | capability_bit(peripheral_capability::tcm_access), dma_source),
};

inline constexpr std::array hclk_input{static_cast<std::uint16_t>(clock_id::hclk)};
inline constexpr std::array tim1_clocks{static_cast<std::uint16_t>(clock_id::pclk2), static_cast<std::uint16_t>(clock_id::timy_ker)};
inline constexpr std::array tim2_clocks{static_cast<std::uint16_t>(clock_id::pclk1), static_cast<std::uint16_t>(clock_id::timx_ker)};
inline constexpr std::array i2c1_clocks{static_cast<std::uint16_t>(clock_id::pclk1)};
inline constexpr std::array adc_clocks{static_cast<std::uint16_t>(clock_id::hclk), static_cast<std::uint16_t>(clock_id::adc_ker)};
inline constexpr std::array<std::uint16_t, 0> no_relations{};
inline constexpr std::array tim1_events{event_number(event::tim1_break), event_number(event::tim1_update), event_number(event::tim1_trigger_commutation), event_number(event::tim1_capture_compare)};
inline constexpr std::array tim2_events{event_number(event::tim2)};
inline constexpr std::array i2c1_events{event_number(event::i2c1_event), event_number(event::i2c1_error)};
inline constexpr std::array usart1_events{event_number(event::usart1)};
inline constexpr std::array spi1_events{event_number(event::spi1)};
inline constexpr std::array adc_events{event_number(event::adc12)};
inline constexpr std::array tim1_dma{std::uint16_t{11}, std::uint16_t{12}, std::uint16_t{13}, std::uint16_t{14}, std::uint16_t{15}, std::uint16_t{16}, std::uint16_t{17}};
inline constexpr std::array tim2_dma{std::uint16_t{18}, std::uint16_t{19}, std::uint16_t{20}, std::uint16_t{21}, std::uint16_t{22}};
inline constexpr std::array i2c1_dma{std::uint16_t{33}, std::uint16_t{34}};
inline constexpr std::array usart1_dma{std::uint16_t{41}, std::uint16_t{42}};
inline constexpr std::array spi1_dma{std::uint16_t{37}, std::uint16_t{38}};
inline constexpr std::array adc_dma{std::uint16_t{9}, std::uint16_t{10}};

inline constexpr std::array peripheral_instances{
    core::peripheral_metadata{peripheral(peripheral_kind::timer, 1), tim1_clocks, no_relations, tim1_events, tim1_dma, timer_source},
    core::peripheral_metadata{peripheral(peripheral_kind::timer, 2), tim2_clocks, no_relations, tim2_events, tim2_dma, timer_source},
    core::peripheral_metadata{peripheral(peripheral_kind::i2c, 1), i2c1_clocks, no_relations, i2c1_events, i2c1_dma, i2c_source},
    core::peripheral_metadata{peripheral(peripheral_kind::usart, 1), i2c1_clocks, no_relations, usart1_events, usart1_dma, usart_source},
    core::peripheral_metadata{peripheral(peripheral_kind::spi, 1), tim1_clocks, no_relations, spi1_events, spi1_dma, spi_source},
    core::peripheral_metadata{peripheral(peripheral_kind::adc, 1), adc_clocks, no_relations, adc_events, adc_dma, adc_source},
    core::peripheral_metadata{peripheral(peripheral_kind::dma, 1), hclk_input, no_relations, no_relations, no_relations, dma_source},
};

inline constexpr std::array errata{
    erratum_record{"2.1.1-2.1.5", "Cortex-M7/cache", 0x07U, "Apply the revision-independent core/cache workarounds before runtime use.", errata_sheet},
    erratum_record{"2.1.6", "FPU", 0x07U, "FPU interrupt is not present; NVIC line 81 is not exposed.", errata_sheet},
    erratum_record{"2.2.10", "AXI SRAM", 0x01U, "Revision Y/W read-corruption workaround is required.", errata_sheet},
    erratum_record{"2.4.1", "BDMA", 0x07U, "BDMA limitation applies to every supported revision.", errata_sheet},
    erratum_record{"2.5.1-2.5.5", "DMAMUX", 0x07U, "Apply DMAMUX limitations; RGCFR is modeled write-only.", errata_sheet},
    erratum_record{"2.9.1-2.9.13", "ADC", 0x07U, "ADC limitations are retained per revision; Y/W-only cases are not generalized.", errata_sheet},
    erratum_record{"2.16.1-2.16.4", "TIM", 0x07U, "Timer limitations apply to every supported revision.", errata_sheet},
};

inline constexpr std::array source_gaps{
    source_gap{"GAP-STM32H753-001", "correction-overlay-applied",
        "Corrected ADC maxima and GPIO_AFRH offset are applied. Corrected bus-matrix accounting uses the overlay and per-engine narrative; removed Table 3 routes are not exposed."},
};

inline constexpr std::array review_ledger{
    block_review{"identity/package", coverage_state::verified, 1, 1, "Full source-unit scope reconciled in complete_source_inventory", identity_source},
    block_review{"Cortex-M7 and memory domains", coverage_state::verified, 10, 10, "Full source-unit scope reconciled in complete_source_inventory", memory_source},
    block_review{"clock roots/domains/reset", coverage_state::verified, 15, 15, "Full source-unit scope reconciled in complete_source_inventory", clock_source},
    block_review{"GPIO and initial AF/package routes", coverage_state::verified, 10, 10, "Full source-unit scope reconciled in complete_source_inventory", alternate_function_source},
    block_review{"fixed NVIC initial routes", coverage_state::verified, interrupt_routes.size(), interrupt_routes.size(), "Full source-unit scope reconciled in complete_source_inventory", nvic_source},
    block_review{"DMA1/2 and reachability", coverage_state::verified, 10, 10, "Full source-unit scope reconciled in complete_source_inventory", dma_source},
    block_review{"BDMA reachability", coverage_state::verified, 2, 2, "Full source-unit scope reconciled in complete_source_inventory", dma_source},
    block_review{"MDMA reachability", coverage_state::verified, 10, 10, "Full source-unit scope reconciled in complete_source_inventory", dma_source},
    block_review{"DMAMUX requests and errata", coverage_state::verified, 6, 6, "Full source-unit scope reconciled in complete_source_inventory", dma_source},
    block_review{"TIM initial instances/registers", coverage_state::verified, timer_registers.size(), timer_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", timer_source},
    block_review{"I2C initial registers", coverage_state::verified, i2c_registers.size(), i2c_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", i2c_source},
    block_review{"USART/UART initial registers", coverage_state::verified, usart_registers.size(), usart_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", usart_source},
    block_review{"SPI initial registers", coverage_state::verified, spi_registers.size(), spi_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", spi_source},
    block_review{"ADC initial registers", coverage_state::verified, adc_registers.size(), adc_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", adc_source},
};

// This is declarative metadata only: no MMIO execution, startup, interrupt
// dispatch, board aliases, peripheral sequencing, or RTOS integration.
// @req REQ-0003 REQ-0004 REQ-0005 REQ-0006 REQ-0007 REQ-0010 REQ-0011 REQ-0012 REQ-0013 REQ-0033 REQ-0040
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
    using pin_database_type = lqfp100_pin_database;
    using representative_register = gpio_moder;
    using representative_field = gpio_mode0;
};

static_assert(std::is_trivial_v<description>);

} // namespace mcuhw::mcu::stm32h753
