#pragma once

// @req-file REQ-0054
#include <mcubsp/mcu/nrf52840/model.hpp>
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840 {

inline constexpr core::source_ref twi_source{
    "Nordic nrfx MDK", "v3.2.0", "nrf52840.h / nrf52840_bitfields.h: TWI0/TWI1"};
inline constexpr std::uint32_t twi_clock_hz = 16'000'000U;
[[nodiscard]] constexpr std::uintptr_t twi_base(unsigned instance) noexcept {
    return 0x4000'3000U + static_cast<std::uintptr_t>(instance) * 0x1000U;
}
[[nodiscard]] constexpr std::uint32_t pin_select(core::pin_id pin) noexcept {
    return static_cast<std::uint32_t>(pin.port) * 32U + pin.index;
}

struct twi_registers {
    using task_startrx = core::register_descriptor<std::uint32_t, 0x000, core::access_semantics::command, 1U>;
    using task_starttx = core::register_descriptor<std::uint32_t, 0x008, core::access_semantics::command, 1U>;
    using task_stop = core::register_descriptor<std::uint32_t, 0x014, core::access_semantics::command, 1U>;
    using task_suspend = core::register_descriptor<std::uint32_t, 0x01C, core::access_semantics::command, 1U>;
    using task_resume = core::register_descriptor<std::uint32_t, 0x020, core::access_semantics::command, 1U>;
    using stopped = core::register_descriptor<std::uint32_t, 0x104, core::access_semantics::read_write, 1U>;
    using rxdready = core::register_descriptor<std::uint32_t, 0x108, core::access_semantics::read_write, 1U>;
    using txdsent = core::register_descriptor<std::uint32_t, 0x11C, core::access_semantics::read_write, 1U>;
    using error = core::register_descriptor<std::uint32_t, 0x124, core::access_semantics::read_write, 1U>;
    using bb = core::register_descriptor<std::uint32_t, 0x138, core::access_semantics::read_write, 1U>;
    using suspended = core::register_descriptor<std::uint32_t, 0x148, core::access_semantics::read_write, 1U>;
    using shorts = core::register_descriptor<std::uint32_t, 0x200, core::access_semantics::read_write, 3U>;
    using intenclr = core::register_descriptor<std::uint32_t, 0x308, core::access_semantics::clear_alias, 0x44286U>;
    using errorsrc = core::register_descriptor<std::uint32_t, 0x4C4, core::access_semantics::write_one_to_clear, 7U>;
    using enable = core::register_descriptor<std::uint32_t, 0x500, core::access_semantics::read_write, 0xFU>;
    using psel_scl = core::register_descriptor<std::uint32_t, 0x508, core::access_semantics::read_write, 0x8000'003FU>;
    using psel_sda = core::register_descriptor<std::uint32_t, 0x50C, core::access_semantics::read_write, 0x8000'003FU>;
    using rxd = core::register_descriptor<std::uint32_t, 0x518, core::access_semantics::read_only, 0U>;
    using txd = core::register_descriptor<std::uint32_t, 0x51C, core::access_semantics::read_write, 0xFFU>;
    using frequency = core::register_descriptor<std::uint32_t, 0x524, core::access_semantics::read_write>;
    using address = core::register_descriptor<std::uint32_t, 0x588, core::access_semantics::read_write, 0x7FU>;
};

inline constexpr std::uint32_t twi_bb_suspend = 1U;
inline constexpr std::uint32_t twi_bb_stop = 2U;

} // namespace mcubsp::mcu::nrf52840
