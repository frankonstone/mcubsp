#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <mcubsp/mcu/ra4m1/model.hpp>

namespace mcubsp::mcu::ra4m1 {

inline constexpr core::source_ref package_source{
    "R01UH0887EJ0110 / R01DS0355EJ0110", "Rev.1.10",
    "R01UH Figure 1.5 and Tables 19.5-19.15, PDF pages 66 and 369-379; R01DS Table 1.14"};
inline constexpr core::source_ref pfs_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "19.2.5-19.2.6 and Tables 19.5-19.15, PDF pages 361-379"};

constexpr auto digital_modes =
    core::mode_mask(core::electrical_mode::input)
    | core::mode_mask(core::electrical_mode::push_pull)
    | core::mode_mask(core::electrical_mode::open_drain)
    | core::mode_mask(core::electrical_mode::pull_up);
constexpr auto analog_modes =
    digital_modes | core::mode_mask(core::electrical_mode::analog);

constexpr core::package_pin pin(std::uint16_t port, std::uint16_t index,
                                std::uint16_t position,
                                bool analog = false) noexcept {
    return {{port, index}, position,
            static_cast<std::uint16_t>(analog ? analog_modes : digital_modes),
            package_source};
}

constexpr core::package_pin input_only_pin(std::uint16_t port,
                                           std::uint16_t index,
                                           std::uint16_t position) noexcept {
    return {{port, index}, position,
            core::mode_mask(core::electrical_mode::input), package_source};
}

// Every GPIO-capable bond-out in PLQP0064KB-C. Supply, reset, oscillator and
// USB-only pins intentionally do not appear in the MCU GPIO package set.
inline constexpr std::array package_pins{
    pin(4, 0, 1), pin(4, 1, 2), pin(4, 2, 3),
    pin(4, 11, 12), pin(4, 10, 13), pin(4, 9, 14), pin(4, 8, 15), pin(4, 7, 16),
    pin(3, 0, 17), pin(3, 1, 18), pin(3, 2, 19), pin(3, 3, 20), pin(3, 4, 21),
    input_only_pin(2, 0, 22), pin(2, 1, 23), pin(2, 4, 25),
    pin(2, 5, 26), pin(2, 6, 27), input_only_pin(2, 15, 6),
    input_only_pin(2, 14, 7), pin(2, 13, 9), pin(2, 12, 10),
    pin(9, 14, 30), pin(9, 15, 31),
    pin(1, 8, 33), pin(1, 9, 34), pin(1, 10, 35), pin(1, 11, 36),
    pin(1, 12, 37), pin(1, 13, 38), pin(1, 7, 41), pin(1, 6, 42),
    pin(1, 5, 43), pin(1, 4, 44), pin(1, 3, 45, true), pin(1, 2, 46, true),
    pin(1, 1, 47, true), pin(1, 0, 48, true),
    pin(5, 0, 49, true), pin(5, 1, 50, true), pin(5, 2, 51, true),
    pin(0, 15, 52, true), pin(0, 14, 53, true), pin(0, 13, 54, true),
    pin(0, 12, 55, true), pin(0, 11, 58, true), pin(0, 10, 59, true),
    pin(0, 4, 60, true), pin(0, 3, 61, true), pin(0, 2, 62, true),
    pin(0, 1, 63, true), pin(0, 0, 64, true),
};

constexpr core::pin_route route(std::uint16_t port, std::uint16_t index,
                                signal function, std::uint32_t psel) noexcept {
    return {{port, index}, signal_id(function),
            core::pin_route_kind::alternate_function, psel, pfs_source};
}

// Strongly typed routes needed by the initial driver slice. The complete
// non-driver route denominator is accounted for in coverage.hpp.
inline constexpr std::array pin_routes{
    route(3, 0, signal::gpt0_a, 0x03), route(2, 12, signal::gpt0_b, 0x03),
    route(1, 5, signal::gpt1_a, 0x03), route(1, 4, signal::gpt1_b, 0x03),
    route(1, 3, signal::gpt2_a, 0x03), route(1, 2, signal::gpt2_b, 0x03),
    route(1, 11, signal::gpt3_a, 0x03), route(1, 12, signal::gpt3_b, 0x03),
    route(3, 2, signal::gpt4_a, 0x03), route(3, 1, signal::gpt4_b, 0x03),
    route(4, 9, signal::gpt5_a, 0x03), route(4, 8, signal::gpt5_b, 0x03),
    route(4, 0, signal::gpt6_a, 0x03), route(4, 1, signal::gpt6_b, 0x03),
    route(3, 4, signal::gpt7_a, 0x03), route(3, 3, signal::gpt7_b, 0x03),
    route(1, 0, signal::sci0_rxd, 0x04), route(1, 1, signal::sci0_txd, 0x04),
    route(4, 2, signal::sci1_rxd, 0x05), route(4, 1, signal::sci1_txd, 0x05),
    route(3, 1, signal::sci2_rxd, 0x04), route(3, 2, signal::sci2_txd, 0x04),
    route(4, 8, signal::sci9_rxd, 0x05), route(4, 9, signal::sci9_txd, 0x05),
    route(4, 0, signal::iic0_scl, 0x07), route(4, 1, signal::iic0_sda, 0x07),
    route(1, 0, signal::iic1_scl, 0x07), route(1, 1, signal::iic1_sda, 0x07),
    route(1, 0, signal::spi0_miso, 0x06), route(1, 1, signal::spi0_mosi, 0x06),
    route(1, 2, signal::spi0_rspck, 0x06), route(1, 3, signal::spi0_ssl0, 0x06),
    route(1, 10, signal::spi1_miso, 0x06), route(1, 9, signal::spi1_mosi, 0x06),
    route(1, 11, signal::spi1_rspck, 0x06), route(1, 8, signal::spi1_ssl0, 0x06),
    route(1, 12, signal::spi1_ssl0, 0x06),
    route(0, 0, signal::adc_an000, 1U << 15U),
    route(0, 1, signal::adc_an001, 1U << 15U),
    route(0, 2, signal::adc_an002, 1U << 15U),
    route(0, 3, signal::adc_an003, 1U << 15U),
    route(0, 4, signal::adc_an004, 1U << 15U),
    route(0, 10, signal::adc_an005, 1U << 15U),
    route(0, 11, signal::adc_an006, 1U << 15U),
    route(0, 12, signal::adc_an007, 1U << 15U),
    route(0, 13, signal::adc_an008, 1U << 15U),
    route(0, 14, signal::adc_an009, 1U << 15U),
};

struct pin_database {
    template<core::pin_id Pin, core::signal_id Signal,
             core::electrical_mode Mode>
    static constexpr bool valid = [] {
        if (!core::package_contains(package_pins, Pin, Mode)) {
            return false;
        }
        if (Signal == signal_id(signal::gpio_input)) {
            return Mode == core::electrical_mode::input;
        }
        if (Signal == signal_id(signal::gpio_output)) {
            return Mode == core::electrical_mode::push_pull
                || Mode == core::electrical_mode::open_drain;
        }
        return core::route_exists(pin_routes, Pin, Signal);
    }();
};

[[nodiscard]] constexpr std::uintptr_t pfs_address(core::pin_id value) noexcept {
    return 0x4004'0800U + value.port * 0x40U + value.index * 4U;
}

using p000pfs = core::register_descriptor<std::uint32_t, 0x000,
    core::access_semantics::read_write, 0xFFFF'FFFDU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFDU>;
using p000pfs_ha = core::register_descriptor<std::uint16_t, 0x002,
    core::access_semantics::read_write, 0xFFFDU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFDU, p000pfs>;
using p000pfs_by = core::register_descriptor<std::uint8_t, 0x003,
    core::access_semantics::read_write, 0xFDU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFDU, p000pfs>;
using pwpr = core::register_descriptor<std::uint8_t, 0xD03,
    core::access_semantics::read_write, 0xC0U, true,
    core::reserved_bit_policy::preserve, 0U, 0x80U, 0xFFU>;

using psel = core::field_descriptor<p000pfs, 24, 5>;
using pmr = core::field_descriptor<p000pfs, 16, 1>;
using asel = core::field_descriptor<p000pfs, 15, 1>;
using isel = core::field_descriptor<p000pfs, 14, 1>;
using pfs_pdr = core::field_descriptor<p000pfs, 2, 1>;
using pfs_pidr = core::field_descriptor<p000pfs, 1, 1, core::access_semantics::read_only>;
using pfs_podr = core::field_descriptor<p000pfs, 0, 1>;
using pcr = core::field_descriptor<p000pfs, 4, 1>;
using ncodr = core::field_descriptor<p000pfs, 6, 1>;
using dscr = core::field_descriptor<p000pfs, 10, 1>;
using dscr1 = core::field_descriptor<p000pfs, 11, 1>;

inline constexpr std::array pfs_registers{
    register_record{"P000PFS", 0x4004'0800U, 4, core::access_semantics::read_write, 0xFFFF'FFFDU, 0U, 0xFFFF'FFFDU, core::reserved_bit_policy::preserve, pfs_source},
    register_record{"P000PFS_HA", 0x4004'0802U, 2, core::access_semantics::read_write, 0xFFFDU, 0U, 0xFFFDU, core::reserved_bit_policy::preserve, pfs_source},
    register_record{"P000PFS_BY", 0x4004'0803U, 1, core::access_semantics::read_write, 0xFDU, 0U, 0xFDU, core::reserved_bit_policy::preserve, pfs_source},
    register_record{"PWPR", 0x4004'0D03U, 1, core::access_semantics::read_write, 0xC0U, 0x80U, 0xFFU, core::reserved_bit_policy::preserve, pfs_source},
};

} // namespace mcubsp::mcu::ra4m1
