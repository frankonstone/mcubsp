#pragma once

#include <array>
#include <cstdint>

#include <mcubsp/mcu/esp32p4/system.hpp>

namespace mcubsp::mcu::esp32p4 {

inline constexpr core::source_ref package_source{
    "ESP32-P4 Series Datasheet", "pre-release v0.7",
    "Section 2 and QFN104 pin-layout/table"};
inline constexpr core::source_ref gpio_matrix_source{
    "ESP32-P4 TRM", "pre-release v0.7",
    "Chapter 10, GPIO Matrix/IO MUX, Registers 10.1-10.44"};

constexpr auto digital_modes = static_cast<std::uint16_t>(
    core::mode_mask(core::electrical_mode::input)
    | core::mode_mask(core::electrical_mode::push_pull)
    | core::mode_mask(core::electrical_mode::open_drain)
    | core::mode_mask(core::electrical_mode::pull_up)
    | core::mode_mask(core::electrical_mode::pull_down));
constexpr auto analog_modes = static_cast<std::uint16_t>(
    digital_modes | core::mode_mask(core::electrical_mode::analog));

inline constexpr std::array<std::uint16_t, 55> gpio_package_positions{
    104, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 22, 23, 24, 25, 52, 53, 55, 56, 57, 58, 60,
    61, 63, 64, 65, 66, 68, 69, 70, 80, 81, 82, 83, 84, 86, 87,
    88, 89, 90, 92, 93, 94, 95, 97, 98,
};

[[nodiscard]] constexpr bool is_analog_gpio(std::uint16_t gpio) noexcept {
    return (gpio >= 16U && gpio <= 23U) || (gpio >= 49U && gpio <= 54U);
}

consteval auto make_package_pins() {
    std::array<core::package_pin, 55> result{};
    for (std::uint16_t gpio = 0; gpio < result.size(); ++gpio) {
        result[gpio] = {{0, gpio}, gpio_package_positions[gpio],
                        is_analog_gpio(gpio) ? analog_modes : digital_modes,
                        package_source};
    }
    return result;
}

inline constexpr auto package_pins = make_package_pins();

struct pin_properties {
    core::pin_id pin;
    bool hp_io;
    bool lp_io;
    bool strapping;
    core::source_ref provenance;
};

consteval auto make_pin_properties() {
    std::array<pin_properties, 55> result{};
    for (std::uint16_t gpio = 0; gpio < result.size(); ++gpio) {
        result[gpio] = {{0, gpio}, true, gpio <= 15U,
                        gpio >= 34U && gpio <= 38U, package_source};
    }
    return result;
}

inline constexpr auto pin_details = make_pin_properties();

constexpr core::pin_route direct_route(std::uint16_t gpio, signal function,
                                       std::uint32_t io_mux_function) noexcept {
    return {{0, gpio}, signal_id(function), core::pin_route_kind::fixed,
            io_mux_function, package_source};
}

// These are direct IO-MUX functions. I2C, LEDC, MCPWM and ordinary GPIO use
// the programmable GPIO matrix and are validated generically below.
inline constexpr std::array pin_routes{
    direct_route(37, signal::uart0_tx, 0),
    direct_route(38, signal::uart0_rx, 0),
    direct_route(10, signal::uart1_tx, 2),
    direct_route(11, signal::uart1_rx, 2),
    direct_route(14, signal::lp_uart_tx, 0),
    direct_route(15, signal::lp_uart_rx, 0),
    direct_route(7, signal::spi2_cs, 3),
    direct_route(9, signal::spi2_sck, 3),
    direct_route(8, signal::spi2_mosi, 3),
    direct_route(10, signal::spi2_miso, 3),
    direct_route(28, signal::spi2_cs, 2),
    direct_route(30, signal::spi2_sck, 2),
    direct_route(29, signal::spi2_mosi, 2),
    direct_route(31, signal::spi2_miso, 2),
    core::pin_route{{0, 16}, signal_id(signal::adc1_ch0), core::pin_route_kind::fixed, 0, package_source},
    core::pin_route{{0, 49}, signal_id(signal::adc2_ch0), core::pin_route_kind::fixed, 0, package_source},
};

[[nodiscard]] constexpr bool matrix_signal(core::signal_id requested) noexcept {
    return requested == signal_id(signal::i2c0_scl)
        || requested == signal_id(signal::i2c0_sda)
        || requested == signal_id(signal::spi2_cs)
        || requested == signal_id(signal::spi2_sck)
        || requested == signal_id(signal::spi2_mosi)
        || requested == signal_id(signal::spi2_miso)
        || requested == signal_id(signal::i2s0_mclk)
        || requested == signal_id(signal::i2s0_bclk)
        || requested == signal_id(signal::i2s0_ws)
        || requested == signal_id(signal::i2s0_data_out)
        || requested == signal_id(signal::i2s0_data_in)
        || requested == signal_id(signal::ledc_ch0)
        || requested == signal_id(signal::mcpwm0_out0a)
        || requested == signal_id(signal::mcpwm0_out0b);
}

struct qfn104_pin_database {
    template<core::pin_id Pin, core::signal_id Signal,
             core::electrical_mode Mode>
    static constexpr bool valid = [] {
        if (Pin.port != 0U || Pin.index >= package_pins.size()
            || !core::package_contains(package_pins, Pin, Mode)) {
            return false;
        }
        if (Signal == signal_id(signal::gpio_input)) {
            return Mode == core::electrical_mode::input;
        }
        if (Signal == signal_id(signal::gpio_output)) {
            return Mode == core::electrical_mode::push_pull
                || Mode == core::electrical_mode::open_drain;
        }
        if (Signal == signal_id(signal::adc1_ch0)
            || Signal == signal_id(signal::adc2_ch0)) {
            return Mode == core::electrical_mode::analog
                && core::route_exists(pin_routes, Pin, Signal);
        }
        if (matrix_signal(Signal)) {
            return Mode == core::electrical_mode::input
                || Mode == core::electrical_mode::push_pull
                || Mode == core::electrical_mode::open_drain;
        }
        return core::route_exists(pin_routes, Pin, Signal);
    }();
};

inline constexpr std::uintptr_t gpio_matrix_base = 0x500E'0000U;
inline constexpr std::uintptr_t io_mux_base = 0x500E'1000U;
inline constexpr std::uintptr_t lp_gpio_matrix_base = 0x5012'A000U;
inline constexpr std::uintptr_t lp_io_mux_base = 0x5012'B000U;

using gpio_out = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;
using gpio_out_w1ts = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::set_alias, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU, gpio_out>;
using gpio_out_w1tc = core::register_descriptor<std::uint32_t, 0x0C,
    core::access_semantics::clear_alias, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU, gpio_out>;
using gpio_out1 = core::register_descriptor<std::uint32_t, 0x10,
    core::access_semantics::read_write, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0x01FF'FFFFU>;
using gpio_out1_w1ts = core::register_descriptor<std::uint32_t, 0x14,
    core::access_semantics::set_alias, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0x01FF'FFFFU, gpio_out1>;
using gpio_out1_w1tc = core::register_descriptor<std::uint32_t, 0x18,
    core::access_semantics::clear_alias, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0x01FF'FFFFU, gpio_out1>;
using gpio_enable = core::register_descriptor<std::uint32_t, 0x20,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;
using gpio_enable_w1ts = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::set_alias, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU, gpio_enable>;
using gpio_enable_w1tc = core::register_descriptor<std::uint32_t, 0x28,
    core::access_semantics::clear_alias, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU, gpio_enable>;
using gpio_enable1 = core::register_descriptor<std::uint32_t, 0x2C,
    core::access_semantics::read_write, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0x01FF'FFFFU>;
using gpio_enable1_w1ts = core::register_descriptor<std::uint32_t, 0x30,
    core::access_semantics::set_alias, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0x01FF'FFFFU, gpio_enable1>;
using gpio_enable1_w1tc = core::register_descriptor<std::uint32_t, 0x34,
    core::access_semantics::clear_alias, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0x01FF'FFFFU, gpio_enable1>;
using gpio_input = core::register_descriptor<std::uint32_t, 0x3C,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using gpio_input1 = core::register_descriptor<std::uint32_t, 0x40,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using gpio_pin0 = core::register_descriptor<std::uint32_t, 0x74,
    core::access_semantics::read_write, 0x0000'1FFFU, false,
    core::reserved_bit_policy::preserve>;
using gpio_func0_in_sel = core::register_descriptor<std::uint32_t, 0x15C,
    core::access_semantics::read_write, 0x0000'01FFU, false,
    core::reserved_bit_policy::preserve>;
using gpio_func0_out_sel = core::register_descriptor<std::uint32_t, 0x558,
    core::access_semantics::read_write, 0x0000'03FFU, false,
    core::reserved_bit_policy::preserve>;
using io_mux_gpio0 = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0x0000'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using gpio0_output = core::field_descriptor<gpio_out, 0, 1>;
using io_mux_gpio0_function = core::field_descriptor<io_mux_gpio0, 1, 2>;

inline constexpr std::array gpio_registers{
    register_record{"GPIO_OUT_REG", gpio_matrix_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_OUT_W1TS_REG", gpio_matrix_base + 0x08U, 4, core::access_semantics::set_alias, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_OUT_W1TC_REG", gpio_matrix_base + 0x0CU, 4, core::access_semantics::clear_alias, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_OUT1_REG", gpio_matrix_base + 0x10U, 4, core::access_semantics::read_write, 0x01FF'FFFFU, 0U, 0x01FF'FFFFU, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_OUT1_W1TS_REG", gpio_matrix_base + 0x14U, 4, core::access_semantics::set_alias, 0x01FF'FFFFU, 0U, 0x01FF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_OUT1_W1TC_REG", gpio_matrix_base + 0x18U, 4, core::access_semantics::clear_alias, 0x01FF'FFFFU, 0U, 0x01FF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_ENABLE_REG", gpio_matrix_base + 0x20U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_ENABLE_W1TS_REG", gpio_matrix_base + 0x24U, 4, core::access_semantics::set_alias, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_ENABLE_W1TC_REG", gpio_matrix_base + 0x28U, 4, core::access_semantics::clear_alias, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_ENABLE1_REG", gpio_matrix_base + 0x2CU, 4, core::access_semantics::read_write, 0x01FF'FFFFU, 0U, 0x01FF'FFFFU, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_ENABLE1_W1TS_REG", gpio_matrix_base + 0x30U, 4, core::access_semantics::set_alias, 0x01FF'FFFFU, 0U, 0x01FF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_ENABLE1_W1TC_REG", gpio_matrix_base + 0x34U, 4, core::access_semantics::clear_alias, 0x01FF'FFFFU, 0U, 0x01FF'FFFFU, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_IN_REG", gpio_matrix_base + 0x3CU, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_IN1_REG", gpio_matrix_base + 0x40U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"GPIO_PIN0_REG", gpio_matrix_base + 0x74U, 4, core::access_semantics::read_write, 0x0000'1FFFU, 0U, 0U, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_FUNC0_IN_SEL_CFG_REG", gpio_matrix_base + 0x15CU, 4, core::access_semantics::read_write, 0x0000'01FFU, 0U, 0U, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_FUNC0_OUT_SEL_CFG_REG", gpio_matrix_base + 0x558U, 4, core::access_semantics::read_write, 0x0000'03FFU, 0U, 0U, core::reserved_bit_policy::preserve, gpio_matrix_source},
    register_record{"GPIO_INT_CLR_REG", gpio_matrix_base + 0x70CU, 4, core::access_semantics::write_one_to_clear, 0x0000'000FU, 0U, 0U, core::reserved_bit_policy::write_zero, gpio_matrix_source},
    register_record{"IO_MUX_GPIO0_REG", io_mux_base + 0x04U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, gpio_matrix_source},
};

} // namespace mcubsp::mcu::esp32p4
