#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/register.hpp>

namespace mcuhw::mcu::stm32h753 {

inline constexpr core::source_ref reference_manual{
    "RM0433", "Rev.8", "STM32H742/743/753/750 reference manual"};
inline constexpr core::source_ref datasheet{
    "DS12117", "Rev.10", "STM32H753xI datasheet"};
inline constexpr core::source_ref errata_sheet{
    "ES0392", "Rev.15", "STM32H742/743/750/753 device errata"};
inline constexpr core::source_ref correction_overlay{
    "ST reference-manual corrections", "reviewed 2026-09-13",
    "RM0433 Rev.8 updates: RCC Table 60, GPIO_AFRH, Table 3 bus interconnect"};

enum class coverage_state : std::uint8_t {
    described,
    verified,
    deferred,
    unavailable_in_source,
    not_applicable,
};

// A zero reset-known mask means unknown rather than an assumed reset of zero.
// @req REQ-0005 REQ-0006 REQ-0007 REQ-0033
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

// @req REQ-0005 REQ-0006 REQ-0007 REQ-0033
struct field_record {
    std::string_view register_name;
    std::string_view name;
    std::uint8_t position;
    std::uint8_t width;
    core::access_semantics access;
    std::uint32_t allowed_encoding_mask;
    core::source_ref provenance;
};

// @req REQ-0005 REQ-0006 REQ-0033
struct block_review {
    std::string_view block;
    coverage_state state;
    std::size_t register_denominator;
    std::size_t described_registers;
    std::string_view outstanding;
    core::source_ref provenance;
};

enum class package_variant : std::uint8_t {
    lqfp100,
    tfbga100,
    lqfp144,
    ufbga169,
    lqfp176,
    ufbga176_plus_25,
    lqfp208,
    tfbga240_plus_25,
};

[[nodiscard]] constexpr std::uint8_t package_bit(package_variant value) noexcept {
    return static_cast<std::uint8_t>(1U << static_cast<unsigned>(value));
}

struct package_record {
    package_variant id;
    std::string_view device_suffix;
    std::string_view package;
    std::uint16_t nominal_positions;
    core::source_ref provenance;
};

struct package_membership {
    core::pin_id pin;
    std::uint8_t package_mask;
    core::source_ref provenance;
};

[[nodiscard]] constexpr bool package_contains(
    package_variant package, core::pin_id pin,
    std::span<const package_membership> memberships) noexcept {
    for (const auto& membership : memberships) {
        if (membership.pin == pin
            && (membership.package_mask & package_bit(package)) != 0) {
            return true;
        }
    }
    return false;
}

enum class silicon_revision : std::uint8_t { yw, x, v };

struct silicon_record {
    silicon_revision revision;
    std::uint16_t rev_id;
    std::uint32_t maximum_core_hz;
    core::source_ref provenance;
};

enum class memory_domain : std::uint8_t { flash, tcm, d1, d2, d3, backup };

struct memory_properties {
    std::uint16_t region_id;
    memory_domain domain;
    std::uint8_t bus_width_bits;
    bool executable;
    bool default_cacheable;
    bool dma_requires_cache_maintenance;
    core::source_ref provenance;
};

struct cortex_m7_properties {
    std::uint32_t maximum_frequency_hz;
    std::size_t instruction_cache_bytes;
    std::size_t data_cache_bytes;
    std::uint8_t mpu_regions;
    bool double_precision_fpu;
    core::source_ref provenance;
};

enum class peripheral_kind : std::uint16_t {
    gpio = 1,
    timer,
    i2c,
    usart,
    spi,
    adc,
    dma,
    bdma,
    mdma,
    ethernet,
};

[[nodiscard]] constexpr core::peripheral_id peripheral(
    peripheral_kind kind, std::uint16_t instance) noexcept {
    return {static_cast<std::uint16_t>(kind), instance};
}

enum class clock_id : std::uint16_t {
    hsi,
    csi,
    hse,
    pll1_p,
    pll2_p,
    pll3_r,
    per_ck,
    sys_ck,
    hclk,
    pclk1,
    pclk2,
    pclk3,
    pclk4,
    timx_ker,
    timy_ker,
    adc_ker,
    eth_ker,
};

enum class signal : std::uint16_t {
    gpio_input,
    gpio_output,
    tim1_ch1,
    tim1_ch1n,
    tim1_ch2,
    tim1_ch3,
    tim1_ch4,
    tim2_ch1,
    i2c1_scl,
    i2c1_sda,
    usart1_tx,
    usart1_rx,
    usart3_tx,
    usart3_rx,
    spi1_nss,
    spi1_sck,
    spi1_miso,
    spi1_mosi,
    adc1_inp15,
    adc1_inp16,
    eth_rmii_ref_clk,
    eth_mdio,
    eth_mdc,
    eth_rmii_crs_dv,
    eth_rmii_rxd0,
    eth_rmii_rxd1,
    eth_rmii_tx_en,
    eth_rmii_txd0,
    eth_rmii_txd1,
};

[[nodiscard]] constexpr core::signal_id signal_id(signal value) noexcept {
    return {{0x5354U, 0U}, static_cast<std::uint16_t>(value)};
}

enum class timer_output : std::uint8_t {
    ch1,
    ch1n,
    ch2,
    ch2n,
    ch3,
    ch3n,
    ch4,
};

struct timer_instance {
    std::uint8_t instance;
    std::uint8_t counter_width;
    std::uintptr_t base;
    std::array<timer_output, 7> outputs;
    std::uint8_t output_count;
    bool center_aligned;
    bool dead_time;
    core::source_ref provenance;
};

// @req REQ-0004 REQ-0005 REQ-0007 REQ-0011 REQ-0033
struct instance_record {
    core::peripheral_id id;
    std::uintptr_t base;
    std::uint16_t bus_clock;
    std::uint16_t kernel_clock;
    std::uintptr_t clock_enable_register;
    std::uint32_t clock_enable_mask;
    std::uintptr_t reset_register;
    std::uint32_t reset_mask;
    std::uint32_t capability_bits;
    core::source_ref provenance;
};

struct erratum_record {
    std::string_view id;
    std::string_view block;
    std::uint8_t affected_revision_mask;
    std::string_view constraint;
    core::source_ref provenance;
};

struct source_gap {
    std::string_view id;
    std::string_view state;
    std::string_view effect;
};

} // namespace mcuhw::mcu::stm32h753
