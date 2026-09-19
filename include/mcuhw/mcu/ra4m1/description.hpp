#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

#include <mcuhw/mcu/ra4m1/peripherals.hpp>
#include <mcuhw/mcu/ra4m1/coverage.hpp>

namespace mcuhw::mcu::ra4m1 {

enum class peripheral_capability : std::uint8_t {
    gpio_atomic_set_clear,
    timer_32_bit,
    timer_16_bit,
    pwm_output_a,
    pwm_output_b,
    fifo,
    asynchronous_uart,
    simple_iic,
    simple_spi,
    iic_multi_master,
    spi_master,
    spi_slave,
    adc_14_bit,
    dma,
    dtc,
};

struct gpt32_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::timer_32_bit,
    peripheral_capability::pwm_output_a,
    peripheral_capability::pwm_output_b> {
    static constexpr auto provenance = gpt_source;
};
struct gpt16_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::timer_16_bit,
    peripheral_capability::pwm_output_a,
    peripheral_capability::pwm_output_b> {
    static constexpr auto provenance = gpt_source;
};
struct sci_fifo_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::fifo,
    peripheral_capability::asynchronous_uart,
    peripheral_capability::simple_iic,
    peripheral_capability::simple_spi> {
    static constexpr auto provenance = sci_source;
};
struct sci_basic_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::asynchronous_uart,
    peripheral_capability::simple_iic,
    peripheral_capability::simple_spi> {
    static constexpr auto provenance = sci_source;
};
struct transfer_capabilities : core::capability_set<peripheral_capability,
    peripheral_capability::dma, peripheral_capability::dtc> {
    static constexpr core::source_ref provenance{
        "R01UH0887EJ0110", "Rev.1.10", "Chapters 16-17"};
};

[[nodiscard]] constexpr std::uint32_t capability_bit(
    peripheral_capability value) noexcept {
    return 1U << static_cast<unsigned>(value);
}

inline constexpr auto no_secondary_clock = std::uint16_t{0xFFFFU};

inline constexpr std::array instances{
    instance_record{peripheral(peripheral_kind::gpio, 0), 0x4004'0000U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpio, 1), 0x4004'0020U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpio, 2), 0x4004'0040U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpio, 3), 0x4004'0060U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpio, 4), 0x4004'0080U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpio, 5), 0x4004'00A0U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpio, 9), 0x4004'0120U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0U, 0U, capability_bit(peripheral_capability::gpio_atomic_set_clear), gpio_source},
    instance_record{peripheral(peripheral_kind::gpt, 0), 0x4007'8000U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 5U, capability_bit(peripheral_capability::timer_32_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 1), 0x4007'8100U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 5U, capability_bit(peripheral_capability::timer_32_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 2), 0x4007'8200U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 6U, capability_bit(peripheral_capability::timer_16_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 3), 0x4007'8300U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 6U, capability_bit(peripheral_capability::timer_16_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 4), 0x4007'8400U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 6U, capability_bit(peripheral_capability::timer_16_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 5), 0x4007'8500U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 6U, capability_bit(peripheral_capability::timer_16_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 6), 0x4007'8600U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 6U, capability_bit(peripheral_capability::timer_16_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::gpt, 7), 0x4007'8700U, static_cast<std::uint16_t>(clock_id::pclkd), no_secondary_clock, 0x4004'7008U, 1U << 6U, capability_bit(peripheral_capability::timer_16_bit) | capability_bit(peripheral_capability::pwm_output_a) | capability_bit(peripheral_capability::pwm_output_b), gpt_source},
    instance_record{peripheral(peripheral_kind::iic, 0), 0x4005'3000U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0x4004'7000U, 1U << 9U, capability_bit(peripheral_capability::iic_multi_master), iic_source},
    instance_record{peripheral(peripheral_kind::iic, 1), 0x4005'3100U, static_cast<std::uint16_t>(clock_id::pclkb), no_secondary_clock, 0x4004'7000U, 1U << 8U, capability_bit(peripheral_capability::iic_multi_master), iic_source},
    instance_record{peripheral(peripheral_kind::sci, 0), 0x4007'0000U, static_cast<std::uint16_t>(clock_id::pclka), no_secondary_clock, 0x4004'7000U, 1U << 31U, capability_bit(peripheral_capability::fifo) | capability_bit(peripheral_capability::asynchronous_uart) | capability_bit(peripheral_capability::simple_iic) | capability_bit(peripheral_capability::simple_spi), sci_source},
    instance_record{peripheral(peripheral_kind::sci, 1), 0x4007'0020U, static_cast<std::uint16_t>(clock_id::pclka), no_secondary_clock, 0x4004'7000U, 1U << 30U, capability_bit(peripheral_capability::fifo) | capability_bit(peripheral_capability::asynchronous_uart) | capability_bit(peripheral_capability::simple_iic) | capability_bit(peripheral_capability::simple_spi), sci_source},
    instance_record{peripheral(peripheral_kind::sci, 2), 0x4007'0040U, static_cast<std::uint16_t>(clock_id::pclka), no_secondary_clock, 0x4004'7000U, 1U << 29U, capability_bit(peripheral_capability::asynchronous_uart) | capability_bit(peripheral_capability::simple_iic) | capability_bit(peripheral_capability::simple_spi), sci_source},
    instance_record{peripheral(peripheral_kind::sci, 9), 0x4007'0120U, static_cast<std::uint16_t>(clock_id::pclka), no_secondary_clock, 0x4004'7000U, 1U << 22U, capability_bit(peripheral_capability::asynchronous_uart) | capability_bit(peripheral_capability::simple_iic) | capability_bit(peripheral_capability::simple_spi), sci_source},
    instance_record{peripheral(peripheral_kind::spi, 0), 0x4007'2000U, static_cast<std::uint16_t>(clock_id::pclka), no_secondary_clock, 0x4004'7000U, 1U << 19U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source},
    instance_record{peripheral(peripheral_kind::spi, 1), 0x4007'2100U, static_cast<std::uint16_t>(clock_id::pclka), no_secondary_clock, 0x4004'7000U, 1U << 18U, capability_bit(peripheral_capability::spi_master) | capability_bit(peripheral_capability::spi_slave), spi_source},
    instance_record{peripheral(peripheral_kind::adc14, 0), 0x4005'C000U, static_cast<std::uint16_t>(clock_id::pclkb), static_cast<std::uint16_t>(clock_id::pclkc), 0x4004'7008U, 1U << 16U, capability_bit(peripheral_capability::adc_14_bit), adc_source},
    instance_record{peripheral(peripheral_kind::dmac, 0), 0x4000'5000U, static_cast<std::uint16_t>(clock_id::iclk), no_secondary_clock, 0x4001'E01CU, 1U << 22U, capability_bit(peripheral_capability::dma), dmac_source},
    instance_record{peripheral(peripheral_kind::dmac, 1), 0x4000'5040U, static_cast<std::uint16_t>(clock_id::iclk), no_secondary_clock, 0x4001'E01CU, 1U << 22U, capability_bit(peripheral_capability::dma), dmac_source},
    instance_record{peripheral(peripheral_kind::dmac, 2), 0x4000'5080U, static_cast<std::uint16_t>(clock_id::iclk), no_secondary_clock, 0x4001'E01CU, 1U << 22U, capability_bit(peripheral_capability::dma), dmac_source},
    instance_record{peripheral(peripheral_kind::dmac, 3), 0x4000'50C0U, static_cast<std::uint16_t>(clock_id::iclk), no_secondary_clock, 0x4001'E01CU, 1U << 22U, capability_bit(peripheral_capability::dma), dmac_source},
    instance_record{peripheral(peripheral_kind::dtc, 0), 0x4000'5400U, static_cast<std::uint16_t>(clock_id::iclk), no_secondary_clock, 0x4001'E01CU, 1U << 22U, capability_bit(peripheral_capability::dtc), dtc_source},
};

inline constexpr std::array pclka_input{static_cast<std::uint16_t>(clock_id::pclka)};
inline constexpr std::array pclkb_input{static_cast<std::uint16_t>(clock_id::pclkb)};
inline constexpr std::array pclkbc_inputs{
    static_cast<std::uint16_t>(clock_id::pclkb),
    static_cast<std::uint16_t>(clock_id::pclkc)};
inline constexpr std::array pclkd_input{static_cast<std::uint16_t>(clock_id::pclkd)};
inline constexpr std::array<std::uint16_t, 0> no_resets{};
inline constexpr std::array<std::uint16_t, 0> no_dma{};
inline constexpr std::array gpt0_events{
    event_number(event::gpt0_compare_a), event_number(event::gpt0_compare_b),
    event_number(event::gpt0_overflow), event_number(event::gpt0_underflow)};
inline constexpr std::array sci0_events{
    event_number(event::sci0_rxi), event_number(event::sci0_txi),
    event_number(event::sci0_tei), event_number(event::sci0_eri)};
inline constexpr std::array iic0_events{
    event_number(event::iic0_rxi), event_number(event::iic0_txi),
    event_number(event::iic0_tei), event_number(event::iic0_eei)};
inline constexpr std::array spi0_events{
    event_number(event::spi0_rxi), event_number(event::spi0_txi),
    event_number(event::spi0_idle), event_number(event::spi0_error)};
inline constexpr std::array adc_events{
    event_number(event::adc_scan_end), event_number(event::adc_group_b)};
inline constexpr std::array dmac_events{
    event_number(event::dmac0_int), event_number(event::dmac1_int),
    event_number(event::dmac2_int), event_number(event::dmac3_int)};
inline constexpr std::array gpt0_dma{
    event_number(event::gpt0_compare_a), event_number(event::gpt0_compare_b)};
inline constexpr std::array sci0_dma{
    event_number(event::sci0_rxi), event_number(event::sci0_txi)};
inline constexpr std::array iic0_dma{
    event_number(event::iic0_rxi), event_number(event::iic0_txi)};
inline constexpr std::array spi0_dma{
    event_number(event::spi0_rxi), event_number(event::spi0_txi)};
inline constexpr std::array adc_dma{event_number(event::adc_scan_end)};

// Representative instance traits retain multiple event and DMA relationships.
// Other instances use the same block layout with the documented strides and
// event values exposed in gpt_instances and interrupt_sources.
inline constexpr std::array peripheral_instances{
    core::peripheral_metadata{peripheral(peripheral_kind::gpt, 0), pclkd_input, no_resets, gpt0_events, gpt0_dma, gpt_source},
    core::peripheral_metadata{peripheral(peripheral_kind::sci, 0), pclka_input, no_resets, sci0_events, sci0_dma, sci_source},
    core::peripheral_metadata{peripheral(peripheral_kind::iic, 0), pclkb_input, no_resets, iic0_events, iic0_dma, iic_source},
    core::peripheral_metadata{peripheral(peripheral_kind::spi, 0), pclka_input, no_resets, spi0_events, spi0_dma, spi_source},
    core::peripheral_metadata{peripheral(peripheral_kind::adc14, 0), pclkbc_inputs, no_resets, adc_events, adc_dma, adc_source},
    core::peripheral_metadata{peripheral(peripheral_kind::dmac, 0), std::span<const std::uint16_t>{}, no_resets, dmac_events, no_dma, dmac_source},
};

struct source_gap {
    std::string_view id;
    std::string_view state;
    std::string_view effect;
};

inline constexpr std::array source_gaps{
    source_gap{"GAP-RA4M1-001", "manual-revision-scope-accepted",
        "No separate device errata was exposed by the official part page; the gate accepts only the pinned Rev.1.10 scope and makes no silicon-wide claim."},
};

// Register denominators are manual subsection entries. Grouped arrays such as
// GTCCRA-F and ADDR0-14 count as their single numbered manual subsection.
inline constexpr std::array review_ledger{
    block_review{"identity/package", coverage_state::verified, 1, 1, "", identity_source},
    block_review{"address space", coverage_state::verified, 3, 3, "Full source-unit scope reconciled in complete_source_inventory", memory_source},
    block_review{"clock roots/domains", coverage_state::verified, 12, 12, "Full source-unit scope reconciled in complete_source_inventory", clock_source},
    block_review{"reset/protection", coverage_state::verified, 5, 5, "Full source-unit scope reconciled in complete_source_inventory", protections[0].provenance},
    block_review{"ICU initial events/routes", coverage_state::verified, interrupt_sources.size(), interrupt_sources.size(), "Full source-unit scope reconciled in complete_source_inventory", icu_source},
    block_review{"DMAC", coverage_state::verified, 12, dmac_registers.size(), "", dmac_source},
    block_review{"DTC constraints", coverage_state::verified, 10, 10, "Full source-unit scope reconciled in complete_source_inventory", dtc_source},
    block_review{"GPIO/PFS initial routes", coverage_state::verified, package_pins.size(), package_pins.size(), "Full source-unit scope reconciled in complete_source_inventory", pfs_source},
    block_review{"GPT", coverage_state::verified, 24, gpt_registers.size(), "", gpt_source},
    block_review{"SCI asynchronous UART slice", coverage_state::verified, 9, sci_uart_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", sci_source},
    block_review{"IIC", coverage_state::verified, 19, 19, "Three inaccessible/instance-specific entries are modeled by constraints, not MMIO aliases", iic_source},
    block_review{"SPI", coverage_state::verified, 12, 12, "", spi_source},
    block_review{"ADC14 initial scan slice", coverage_state::verified, 10, adc_registers.size(), "Full source-unit scope reconciled in complete_source_inventory", adc_source},
};

// The public RA4M1 description is declarative and has no constructors, target
// macros, MMIO execution, board dependency, ISR binding, or startup code.
// @req REQ-0005 REQ-0006 REQ-0007 REQ-0009 REQ-0010 REQ-0011 REQ-0012 REQ-0013 REQ-0040
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
    using pin_database_type = pin_database;
    using representative_register = p000pfs;
    using representative_field = psel;
};

static_assert(std::is_trivial_v<description>);

} // namespace mcuhw::mcu::ra4m1
