#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::rng {
struct tasks_start : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct events_valrdy : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_valrdy_fields {
using events_valrdy = core::field_descriptor<events_valrdy, 0U, 1U>;
inline constexpr std::uint32_t events_valrdy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_valrdy_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using valrdy_stop = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t valrdy_stop_disabled = 0x0U;
inline constexpr std::uint32_t valrdy_stop_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using valrdy = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t valrdy_disabled = 0x0U;
inline constexpr std::uint32_t valrdy_enabled = 0x1U;
inline constexpr std::uint32_t valrdy_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using valrdy = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t valrdy_disabled = 0x0U;
inline constexpr std::uint32_t valrdy_enabled = 0x1U;
inline constexpr std::uint32_t valrdy_clear = 0x1U;
}
struct config : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using dercen = core::field_descriptor<config, 0U, 1U>;
inline constexpr std::uint32_t dercen_disabled = 0x0U;
inline constexpr std::uint32_t dercen_enabled = 0x1U;
}
struct value : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace value_fields {
using value = core::field_descriptor<value, 0U, 8U>;
}
}
