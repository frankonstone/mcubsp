#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <mcuhw/mcu/stm32h753/model.hpp>

namespace mcuhw::mcu::stm32h753 {

inline constexpr core::source_ref package_source{
    "DS12117", "Rev.10",
    "Figures 4-11 and Table 8, PDF pages 55-88"};
inline constexpr core::source_ref alternate_function_source{
    "DS12117 / RM0433 + ST correction overlay", "Rev.10 / Rev.8",
    "DS12117 Table 9, PDF pages 89-102; RM0433 11.3-11.4, PDF pages 530-548; corrected GPIO_AFRH offset"};

constexpr auto digital_modes = static_cast<std::uint16_t>(
    core::mode_mask(core::electrical_mode::input)
    | core::mode_mask(core::electrical_mode::push_pull)
    | core::mode_mask(core::electrical_mode::open_drain)
    | core::mode_mask(core::electrical_mode::pull_up)
    | core::mode_mask(core::electrical_mode::pull_down));
constexpr auto analog_modes = static_cast<std::uint16_t>(
    digital_modes | core::mode_mask(core::electrical_mode::analog));

constexpr core::package_pin pin(std::uint16_t port, std::uint16_t index,
                                std::uint16_t position,
                                bool analog = false) noexcept {
    return {{port, index}, position,
            static_cast<std::uint16_t>(analog ? analog_modes : digital_modes),
            package_source};
}

// Source-reviewed LQFP100 pins consumed by the initial peripheral routes.
// The full Table 8 bond-out remains a child item of the coverage inventory.
inline constexpr std::array package_pins{
    pin(0, 0, 22, true), pin(0, 1, 34, true), pin(0, 2, 35, true),
    pin(0, 3, 25, true), pin(0, 5, 29, true),
    pin(0, 6, 30, true),
    pin(0, 7, 31, true), pin(0, 8, 67, true), pin(0, 9, 68),
    pin(0, 10, 69), pin(1, 5, 91), pin(1, 6, 92), pin(1, 7, 93),
    pin(1, 8, 95), pin(1, 9, 96), pin(1, 13, 74),
    pin(2, 1, 26, true), pin(2, 4, 44, true), pin(2, 5, 45, true),
    pin(3, 8, 77), pin(3, 9, 78), pin(3, 14, 86), pin(4, 1, 142),
    pin(6, 11, 126), pin(6, 13, 128),
};

inline constexpr auto every_package = std::uint8_t{0xFFU};
inline constexpr std::array package_memberships{
    package_membership{{0, 0}, every_package, package_source},
    package_membership{{0, 1}, every_package, package_source},
    package_membership{{0, 2}, every_package, package_source},
    package_membership{{0, 3}, every_package, package_source},
    package_membership{{0, 5}, every_package, package_source},
    package_membership{{0, 6}, every_package, package_source},
    package_membership{{0, 7}, every_package, package_source},
    package_membership{{0, 8}, every_package, package_source},
    package_membership{{0, 9}, every_package, package_source},
    package_membership{{0, 10}, every_package, package_source},
    package_membership{{1, 5}, every_package, package_source},
    package_membership{{1, 6}, every_package, package_source},
    package_membership{{1, 7}, every_package, package_source},
    package_membership{{1, 8}, every_package, package_source},
    package_membership{{1, 9}, every_package, package_source},
    package_membership{{1, 13}, every_package, package_source},
    package_membership{{2, 1}, every_package, package_source},
    package_membership{{2, 4}, every_package, package_source},
    package_membership{{2, 5}, every_package, package_source},
    package_membership{{3, 8}, package_bit(package_variant::lqfp144),
                       package_source},
    package_membership{{3, 9}, package_bit(package_variant::lqfp144),
                       package_source},
    package_membership{{3, 14}, package_bit(package_variant::lqfp144),
                       package_source},
    package_membership{{4, 1}, package_bit(package_variant::lqfp144),
                       package_source},
    package_membership{{6, 11}, package_bit(package_variant::lqfp144),
                       package_source},
    package_membership{{6, 13}, package_bit(package_variant::lqfp144),
                       package_source},
    package_membership{{10, 0}, static_cast<std::uint8_t>(
        package_bit(package_variant::lqfp208)
        | package_bit(package_variant::tfbga240_plus_25)), package_source},
};

constexpr core::pin_route route(std::uint16_t port, std::uint16_t index,
                                signal function, std::uint32_t af) noexcept {
    return {{port, index}, signal_id(function),
            core::pin_route_kind::alternate_function, af,
            alternate_function_source};
}

inline constexpr std::array pin_routes{
    route(0, 8, signal::tim1_ch1, 1),
    route(0, 7, signal::tim1_ch1n, 1),
    route(0, 0, signal::tim2_ch1, 1),
    route(0, 5, signal::tim2_ch1, 1),
    route(1, 6, signal::i2c1_scl, 4),
    route(1, 7, signal::i2c1_sda, 4),
    route(1, 8, signal::i2c1_scl, 4),
    route(1, 9, signal::i2c1_sda, 4),
    route(0, 9, signal::usart1_tx, 7),
    route(0, 10, signal::usart1_rx, 7),
    route(3, 8, signal::usart3_tx, 7),
    route(3, 9, signal::usart3_rx, 7),
    route(0, 5, signal::spi1_sck, 5),
    route(0, 6, signal::spi1_miso, 5),
    route(0, 7, signal::spi1_mosi, 5),
    route(1, 5, signal::spi1_mosi, 5),
    route(0, 1, signal::eth_rmii_ref_clk, 11),
    route(0, 2, signal::eth_mdio, 11),
    route(2, 1, signal::eth_mdc, 11),
    route(0, 7, signal::eth_rmii_crs_dv, 11),
    route(2, 4, signal::eth_rmii_rxd0, 11),
    route(2, 5, signal::eth_rmii_rxd1, 11),
    route(6, 11, signal::eth_rmii_tx_en, 11),
    route(6, 13, signal::eth_rmii_txd0, 11),
    route(1, 13, signal::eth_rmii_txd1, 11),
    core::pin_route{{0, 3}, signal_id(signal::adc1_inp15),
        core::pin_route_kind::fixed, 15U, alternate_function_source},
    core::pin_route{{0, 0}, signal_id(signal::adc1_inp16),
        core::pin_route_kind::fixed, 16U, alternate_function_source},
};

template<package_variant Package>
struct pin_database {
    template<core::pin_id Pin, core::signal_id Signal,
             core::electrical_mode Mode>
    static constexpr bool valid = [] {
        if (!package_contains(Package, Pin, package_memberships)) {
            return false;
        }
        if (Signal == signal_id(signal::gpio_input)) {
            return Mode == core::electrical_mode::input;
        }
        if (Signal == signal_id(signal::gpio_output)) {
            return Mode == core::electrical_mode::push_pull
                || Mode == core::electrical_mode::open_drain;
        }
        if ((Signal == signal_id(signal::adc1_inp15)
             || Signal == signal_id(signal::adc1_inp16))
            && Mode != core::electrical_mode::analog) {
            return false;
        }
        return core::route_exists(pin_routes, Pin, Signal);
    }();
};

using lqfp100_pin_database = pin_database<package_variant::lqfp100>;
using lqfp144_pin_database = pin_database<package_variant::lqfp144>;

[[nodiscard]] constexpr std::uintptr_t gpio_base(std::uint16_t port) noexcept {
    return 0x5802'0000U + static_cast<std::uintptr_t>(port) * 0x400U;
}

using gpio_moder = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, true,
    core::reserved_bit_policy::write_zero, 0U, 0xABFF'FFFFU, 0xFFFF'FFFFU>;
using gpio_idr = core::register_descriptor<std::uint16_t, 0x10,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0U>;
using gpio_odr = core::register_descriptor<std::uint16_t, 0x14,
    core::access_semantics::read_write, 0xFFFFU, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFFU>;
using gpio_bsrr = core::register_descriptor<std::uint32_t, 0x18,
    core::access_semantics::command, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gpio_afrl = core::register_descriptor<std::uint32_t, 0x20,
    core::access_semantics::read_write, 0xFFFF'FFFFU, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gpio_afrh = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::read_write, 0xFFFF'FFFFU, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;

using gpio_mode0 = core::field_descriptor<gpio_moder, 0, 2>;
using gpio_af0 = core::field_descriptor<gpio_afrl, 0, 4>;

inline constexpr std::array gpio_registers{
    register_record{"GPIOx_MODER", gpio_base(0) + 0x00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0xABFF'FFFFU, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_OTYPER", gpio_base(0) + 0x04U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_OSPEEDR", gpio_base(0) + 0x08U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_PUPDR", gpio_base(0) + 0x0CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_IDR", gpio_base(0) + 0x10U, 2, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_ODR", gpio_base(0) + 0x14U, 2, core::access_semantics::read_write, 0xFFFFU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_BSRR", gpio_base(0) + 0x18U, 4, core::access_semantics::command, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_LCKR", gpio_base(0) + 0x1CU, 4, core::access_semantics::read_write, 0x0001'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_AFRL", gpio_base(0) + 0x20U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
    register_record{"GPIOx_AFRH", gpio_base(0) + 0x24U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, alternate_function_source},
};

} // namespace mcuhw::mcu::stm32h753
