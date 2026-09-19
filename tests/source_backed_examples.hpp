#pragma once

#include <cstdint>
#include <limits>

#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/register.hpp>

namespace mcuhw::contract_trials {

namespace ra4m1 {

inline constexpr core::source_ref io_source{
    "R01UH0887EJ0110", "Rev.1.10", "19.2.5, PDF pages 361-362"};
inline constexpr core::source_ref protection_source{
    "R01UH0887EJ0110", "Rev.1.10", "19.2.6 and 19.4.3, PDF pages 363-366"};
inline constexpr core::source_ref transfer_source{
    "R01UH0887EJ0110", "Rev.1.10", "Sections 16 and 17"};
inline constexpr core::source_ref clock_source{
    "R01UH0887EJ0110", "Rev.1.10", "Section 8, Clock Generation Circuit"};
inline constexpr core::source_ref interrupt_source{
    "R01UH0887EJ0110", "Rev.1.10", "Section 13, Interrupt Controller Unit"};

using p000pfs = core::register_descriptor<
    std::uint32_t, 0x000, core::access_semantics::read_write,
    std::numeric_limits<std::uint32_t>::max(), false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFDU>;
using p000pfs_ha = core::register_descriptor<
    std::uint16_t, 0x002, core::access_semantics::read_write,
    std::numeric_limits<std::uint16_t>::max(), false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFDU, p000pfs>;
using p000pfs_by = core::register_descriptor<
    std::uint8_t, 0x003, core::access_semantics::read_write,
    std::numeric_limits<std::uint8_t>::max(), false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFDU, p000pfs>;

using pidr = core::field_descriptor<
    p000pfs, 1, 1, core::access_semantics::read_only>;
using psel = core::field_descriptor<p000pfs, 24, 5>;

enum class p408_drive : std::uint8_t {
    low = 0,
    middle = 1,
    iic_fast = 2,
    prohibited = 3,
};

using p408_drive_bits = core::field_descriptor<p000pfs, 10, 2>;
using p408_drive_field = core::enum_field_descriptor<
    p408_drive_bits, p408_drive,
    p408_drive::low, p408_drive::middle, p408_drive::iic_fast>;

enum class gpt_output : std::uint8_t { a, b, c };
using gpt320_outputs = core::capability_set<
    gpt_output, gpt_output::a, gpt_output::b>;

inline constexpr core::pin_id trial_pin{0, 0};
inline constexpr core::signal_id trial_gpio_output{{1, 0}, 1};

struct trial_pin_database {
    template<core::pin_id Pin, core::signal_id Signal,
             core::electrical_mode Mode>
    static constexpr bool valid =
        Pin == trial_pin && Signal == trial_gpio_output
        && Mode == core::electrical_mode::push_pull;
};

} // namespace ra4m1

namespace stm32h753 {

inline constexpr core::source_ref gpio_source{
    "RM0433", "Rev.8", "11.3.5 and 11.4.7, PDF pages 534 and 543"};
inline constexpr core::source_ref exti_source{
    "RM0433", "Rev.8", "20.6.21, PDF page 783"};
inline constexpr core::source_ref dma_source{
    "RM0433", "Rev.8", "13.3.2, PDF page 602"};
inline constexpr core::source_ref clock_source{
    "RM0433", "Rev.8", "Section 8, Reset and Clock Control"};
inline constexpr core::source_ref memory_source{
    "RM0433", "Rev.8", "Section 2.4, Embedded SRAM"};

using gpio_odr = core::register_descriptor<
    std::uint16_t, 0x14, core::access_semantics::read_write,
    0xFFFFU, true, core::reserved_bit_policy::preserve,
    0U, 0U, 0xFFFFU>;
using gpio_bsrr = core::register_descriptor<
    std::uint32_t, 0x18, core::access_semantics::command>;
using exti_cpupr1 = core::register_descriptor<
    std::uint32_t, 0x88, core::access_semantics::write_one_to_clear,
    0x003F'FFFFU, false, core::reserved_bit_policy::write_zero,
    0U, 0U, 0U>;

} // namespace stm32h753

namespace esp32p4 {

inline constexpr core::source_ref gpio_source{
    "ESP32-P4 TRM", "pre-release v0.7", "10.19.1 and Registers 10.1-10.3, PDF pages 1018 and 1026-1027"};
inline constexpr core::source_ref routing_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 10, GPIO Matrix and IO MUX"};
inline constexpr core::source_ref dma_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 6, GDMA-AHB and GDMA-AXI"};
inline constexpr core::source_ref clock_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 11, Reset and Clock"};
inline constexpr core::source_ref interrupt_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 13, Interrupt Matrix"};

using gpio_out = core::register_descriptor<
    std::uint32_t, 0x04, core::access_semantics::read_write,
    0xFFFF'FFFFU, false, core::reserved_bit_policy::preserve,
    0U, 0U, 0xFFFF'FFFFU>;
using gpio_out_w1ts = core::register_descriptor<
    std::uint32_t, 0x08, core::access_semantics::set_alias,
    0xFFFF'FFFFU, false, core::reserved_bit_policy::write_zero,
    0U, 0U, 0xFFFF'FFFFU, gpio_out>;
using gpio_out_w1tc = core::register_descriptor<
    std::uint32_t, 0x0C, core::access_semantics::clear_alias,
    0xFFFF'FFFFU, false, core::reserved_bit_policy::write_zero,
    0U, 0U, 0xFFFF'FFFFU, gpio_out>;

} // namespace esp32p4

} // namespace mcuhw::contract_trials
