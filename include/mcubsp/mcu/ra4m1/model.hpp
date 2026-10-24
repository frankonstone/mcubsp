#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <mcubsp/core/metadata.hpp>
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::ra4m1 {

inline constexpr core::source_ref hardware_manual{
    "R01UH0887EJ0110", "Rev.1.10", "RA4M1 Group hardware manual"};
inline constexpr core::source_ref datasheet{
    "R01DS0355EJ0110", "Rev.1.10", "Tables 1.13 and 1.14"};

enum class coverage_state : std::uint8_t {
    described,
    verified,
    deferred,
    unavailable_in_source,
    not_applicable,
};

// A compact, inspectable catalogue entry supplements the strongly typed
// register aliases. A zero reset-known mask means that this description does
// not claim a reset value; it never means an assumed reset of zero.
// @req REQ-0005 REQ-0006 REQ-0007
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

// Build documentary catalogue entries from the same definitions used by MMIO.
template<typename Register>
[[nodiscard]] consteval register_record describe_register(
    std::string_view name, std::uintptr_t base, core::source_ref provenance) {
    return {name, base + Register::offset,
            static_cast<std::uint8_t>(Register::width), Register::access,
            Register::writable_mask, Register::reset_value,
            Register::reset_known_mask, Register::reserved_bits, provenance};
}


// @req REQ-0005 REQ-0006 REQ-0007
struct field_record {
    std::string_view register_name;
    std::string_view name;
    std::uint8_t position;
    std::uint8_t width;
    core::access_semantics access;
    std::uint32_t allowed_encoding_mask;
    core::source_ref provenance;
};

// @req REQ-0005 REQ-0006
struct block_review {
    std::string_view block;
    coverage_state state;
    std::size_t register_denominator;
    std::size_t described_registers;
    std::string_view outstanding;
    core::source_ref provenance;
};

enum class peripheral_kind : std::uint16_t {
    gpio = 1,
    gpt,
    iic,
    sci,
    spi,
    adc14,
    dmac,
    dtc,
};

[[nodiscard]] constexpr core::peripheral_id peripheral(
    peripheral_kind kind, std::uint16_t instance) noexcept {
    return {static_cast<std::uint16_t>(kind), instance};
}

enum class clock_id : std::uint16_t {
    loco,
    moco,
    hoco,
    mosc,
    sosc,
    pll,
    iclk,
    pclka,
    pclkb,
    pclkc,
    pclkd,
    fclk,
};

enum class protection_group : std::uint16_t {
    none,
    prc0_clock,
    prc1_low_power,
    prc3_lvd,
    pfs,
    gpt,
};

enum class signal : std::uint16_t {
    gpio_input,
    gpio_output,
    gpt0_a,
    gpt0_b,
    gpt1_a,
    gpt1_b,
    gpt2_a,
    gpt2_b,
    gpt3_a,
    gpt3_b,
    gpt4_a,
    gpt4_b,
    gpt5_a,
    gpt5_b,
    gpt6_a,
    gpt6_b,
    gpt7_a,
    gpt7_b,
    sci0_rxd,
    sci0_txd,
    sci1_rxd,
    sci1_txd,
    sci2_rxd,
    sci2_txd,
    sci9_rxd,
    sci9_txd,
    iic0_scl,
    iic0_sda,
    iic1_scl,
    iic1_sda,
    spi0_miso,
    spi0_mosi,
    spi0_rspck,
    spi0_ssl0,
    spi1_miso,
    spi1_mosi,
    spi1_rspck,
    spi1_ssl0,
    adc_an000,
    adc_an001,
    adc_an002,
    adc_an003,
    adc_an004,
    adc_an005,
    adc_an006,
    adc_an007,
    adc_an008,
    adc_an009,
};

[[nodiscard]] constexpr core::signal_id signal_id(signal value) noexcept {
    const auto raw = static_cast<std::uint16_t>(value);
    return {{0x5241U, 0U}, raw};
}

enum class gpt_output : std::uint8_t { a, b };

struct gpt_instance {
    std::uint8_t instance;
    std::uint8_t counter_width;
    std::uintptr_t base;
    std::array<gpt_output, 2> outputs;
    core::source_ref provenance;
};

// @req REQ-0005 REQ-0007 REQ-0011
struct instance_record {
    core::peripheral_id id;
    std::uintptr_t base;
    std::uint16_t primary_clock;
    std::uint16_t secondary_clock;
    std::uintptr_t module_stop_register;
    std::uint32_t module_stop_mask;
    std::uint32_t capability_bits;
    core::source_ref provenance;
};

} // namespace mcubsp::mcu::ra4m1
