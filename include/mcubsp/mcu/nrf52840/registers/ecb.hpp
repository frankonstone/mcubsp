#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::ecb {
struct tasks_startecb : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startecb_fields {
using tasks_startecb = core::field_descriptor<tasks_startecb, 0U, 1U>;
inline constexpr std::uint32_t tasks_startecb_trigger = 0x1U;
}
struct tasks_stopecb : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stopecb_fields {
using tasks_stopecb = core::field_descriptor<tasks_stopecb, 0U, 1U>;
inline constexpr std::uint32_t tasks_stopecb_trigger = 0x1U;
}
struct events_endecb : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endecb_fields {
using events_endecb = core::field_descriptor<events_endecb, 0U, 1U>;
inline constexpr std::uint32_t events_endecb_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endecb_generated = 0x1U;
}
struct events_errorecb : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_errorecb_fields {
using events_errorecb = core::field_descriptor<events_errorecb, 0U, 1U>;
inline constexpr std::uint32_t events_errorecb_notgenerated = 0x0U;
inline constexpr std::uint32_t events_errorecb_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using endecb = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t endecb_disabled = 0x0U;
inline constexpr std::uint32_t endecb_enabled = 0x1U;
inline constexpr std::uint32_t endecb_set = 0x1U;
using errorecb = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t errorecb_disabled = 0x0U;
inline constexpr std::uint32_t errorecb_enabled = 0x1U;
inline constexpr std::uint32_t errorecb_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using endecb = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t endecb_disabled = 0x0U;
inline constexpr std::uint32_t endecb_enabled = 0x1U;
inline constexpr std::uint32_t endecb_clear = 0x1U;
using errorecb = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t errorecb_disabled = 0x0U;
inline constexpr std::uint32_t errorecb_enabled = 0x1U;
inline constexpr std::uint32_t errorecb_clear = 0x1U;
}
struct ecbdataptr : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ecbdataptr_fields {
using ecbdataptr = core::field_descriptor<ecbdataptr, 0U, 32U>;
}
}
