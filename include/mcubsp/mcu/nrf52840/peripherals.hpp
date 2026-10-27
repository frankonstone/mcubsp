#pragma once

// @req-file REQ-0053
#include <mcubsp/core/register.hpp>
#include <mcubsp/mcu/nrf52840/model.hpp>

namespace mcubsp::mcu::nrf52840 {

[[nodiscard]] constexpr std::uintptr_t gpio_base(core::pin_id pin) noexcept {
    return 0x5000'0000U + static_cast<std::uintptr_t>(pin.port) * 0x300U;
}

struct gpio_registers {
    using out = core::register_descriptor<std::uint32_t, 0x504,
        core::access_semantics::read_write>;
    using outset = core::register_descriptor<std::uint32_t, 0x508,
        core::access_semantics::set_alias>;
    using outclr = core::register_descriptor<std::uint32_t, 0x50C,
        core::access_semantics::clear_alias>;
    using in = core::register_descriptor<std::uint32_t, 0x510,
        core::access_semantics::read_only, 0U>;
    template<unsigned Index> requires (Index < 32U)
    using pin_cnf = core::register_descriptor<std::uint32_t, 0x700U + Index * 4U,
        core::access_semantics::read_write, 0x0003'070FU>;
};

[[nodiscard]] constexpr std::uintptr_t pin_configuration(core::pin_id pin) noexcept {
    return gpio_base(pin) + 0x700U + static_cast<std::uintptr_t>(pin.index) * 4U;
}

} // namespace mcubsp::mcu::nrf52840
