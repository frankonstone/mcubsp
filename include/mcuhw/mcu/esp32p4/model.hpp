#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/register.hpp>

namespace mcuhw::mcu::esp32p4 {

inline constexpr core::source_ref technical_reference_manual{
    "ESP32-P4 TRM", "pre-release v0.7 (2026-08-20)",
    "local esp32-p4_technical_reference_manual_en.pdf"};
inline constexpr core::source_ref series_datasheet{
    "ESP32-P4 Series Datasheet", "pre-release v0.7 (2026-07-14)",
    "Sections 1-2, 6 and Appendix A"};
inline constexpr core::source_ref soc_errata{
    "ESP32-P4 Series SoC Errata", "v1.3 (2026-07-10)",
    "ESP32-P4 chip revisions v3.0, v3.1 and v3.2"};
inline constexpr core::source_ref revision_guide{
    "ESP32-P4 Chip Revision v3.x User Guide", "v3.x",
    "production v3.0/v3.1/v3.2 revision applicability"};

enum class coverage_state : std::uint8_t {
    described,
    verified,
    deferred,
    unavailable_in_source,
    not_applicable,
};

// A zero reset-known mask records an unknown reset value; it never means that
// an undocumented reset of zero was assumed.
// @req REQ-0005 REQ-0006 REQ-0007 REQ-0034
struct register_record {
    std::string_view name;
    std::uintptr_t address;
    std::uint8_t width_bytes;
    core::access_semantics access;
    std::uint32_t writable_mask;
    std::uint32_t reset_value;
    std::uint32_t reset_known_mask;
    core::reserved_bit_policy reserved_bits;
    core::source_ref provenance;
};

struct field_record {
    std::string_view register_name;
    std::string_view name;
    std::uint8_t position;
    std::uint8_t width;
    core::access_semantics access;
    std::uint32_t allowed_encoding_mask;
    core::source_ref provenance;
};

struct block_review {
    std::string_view block;
    coverage_state state;
    std::size_t register_denominator;
    std::size_t described_registers;
    std::string_view outstanding;
    core::source_ref provenance;
};

enum class part_variant : std::uint8_t { nrw16x, nrw32x };
enum class silicon_revision : std::uint8_t { v3_0, v3_1, v3_2 };
enum class cpu_domain : std::uint8_t { hp_cpu0, hp_cpu1, lp_cpu };

struct part_record {
    part_variant id;
    std::string_view part_number;
    std::size_t in_package_psram_bytes;
    core::source_ref provenance;
};

struct silicon_record {
    silicon_revision revision;
    std::uint32_t maximum_hp_cpu_hz;
    bool production;
    core::source_ref provenance;
};

struct riscv_core_properties {
    cpu_domain owner;
    std::uint8_t xlen;
    std::uint32_t maximum_frequency_hz;
    std::uintptr_t reset_vector;
    bool held_in_reset_after_power_up;
    core::source_ref provenance;
};

enum class memory_domain : std::uint8_t {
    hp_spm,
    external_flash,
    external_ram,
    hp_rom,
    hp_l2mem,
    lp_rom,
    lp_sram,
    mipi_csi,
    mipi_dsi,
};

struct memory_properties {
    std::uint16_t region_id;
    memory_domain domain;
    std::uint16_t bus_width_bits;
    bool hp_cpu_access;
    bool lp_cpu_access;
    bool direct_uncached_alias;
    core::source_ref provenance;
};

enum class peripheral_kind : std::uint16_t {
    gpio_matrix = 1,
    io_mux,
    systimer,
    timer_group,
    ledc,
    mcpwm,
    i2c,
    uart,
    spi,
    adc,
    gdma_ahb,
    gdma_axi,
    vdma,
    dma_2d,
    i3c,
};

[[nodiscard]] constexpr core::peripheral_id peripheral(
    peripheral_kind kind, std::uint16_t instance) noexcept {
    return {static_cast<std::uint16_t>(kind), instance};
}

enum class clock_id : std::uint16_t {
    xtal,
    rc_fast,
    rc_slow,
    xtal32k,
    cpll,
    mpll,
    spll,
    root,
    cpu,
    mem,
    sys,
    apb,
    lp_fast,
    lp_slow,
    lp_peripheral,
    uart,
    i2c,
    gpspi,
    adc,
};

enum class signal : std::uint16_t {
    gpio_input,
    gpio_output,
    uart0_tx,
    uart0_rx,
    uart1_tx,
    uart1_rx,
    lp_uart_tx,
    lp_uart_rx,
    spi2_cs,
    spi2_sck,
    spi2_miso,
    spi2_mosi,
    i2c0_scl,
    i2c0_sda,
    ledc_ch0,
    mcpwm0_out0a,
    mcpwm0_out0b,
    adc1_ch0,
    adc2_ch0,
};

[[nodiscard]] constexpr core::signal_id signal_id(signal value) noexcept {
    return {{0x4553U, 0x5034U}, static_cast<std::uint16_t>(value)};
}

enum class timer_kind : std::uint8_t { system, timer_group, led_pwm, motor_pwm };

struct timer_instance {
    timer_kind kind;
    std::uint8_t instance;
    std::uint8_t timer_count;
    std::uint8_t counter_width;
    std::uint8_t output_count;
    bool complementary_pairs;
    bool dead_time;
    std::uintptr_t base;
    core::source_ref provenance;
};

struct instance_record {
    core::peripheral_id id;
    std::uintptr_t base;
    std::uint16_t bus_clock;
    std::uint16_t kernel_clock;
    std::uintptr_t clock_gate_register;
    std::uint32_t clock_gate_mask;
    std::uintptr_t reset_register;
    std::uint32_t reset_mask;
    std::uint32_t capability_bits;
    core::source_ref provenance;
};

struct source_gap {
    std::string_view id;
    std::string_view state;
    std::string_view effect;
};

} // namespace mcuhw::mcu::esp32p4
