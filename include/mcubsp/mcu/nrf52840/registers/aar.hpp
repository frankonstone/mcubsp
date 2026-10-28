#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::aar {
struct tasks_start : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct events_end : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_end_fields {
using events_end = core::field_descriptor<events_end, 0U, 1U>;
inline constexpr std::uint32_t events_end_notgenerated = 0x0U;
inline constexpr std::uint32_t events_end_generated = 0x1U;
}
struct events_resolved : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_resolved_fields {
using events_resolved = core::field_descriptor<events_resolved, 0U, 1U>;
inline constexpr std::uint32_t events_resolved_notgenerated = 0x0U;
inline constexpr std::uint32_t events_resolved_generated = 0x1U;
}
struct events_notresolved : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_notresolved_fields {
using events_notresolved = core::field_descriptor<events_notresolved, 0U, 1U>;
inline constexpr std::uint32_t events_notresolved_notgenerated = 0x0U;
inline constexpr std::uint32_t events_notresolved_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using end = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_set = 0x1U;
using resolved = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t resolved_disabled = 0x0U;
inline constexpr std::uint32_t resolved_enabled = 0x1U;
inline constexpr std::uint32_t resolved_set = 0x1U;
using notresolved = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t notresolved_disabled = 0x0U;
inline constexpr std::uint32_t notresolved_enabled = 0x1U;
inline constexpr std::uint32_t notresolved_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using end = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_clear = 0x1U;
using resolved = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t resolved_disabled = 0x0U;
inline constexpr std::uint32_t resolved_enabled = 0x1U;
inline constexpr std::uint32_t resolved_clear = 0x1U;
using notresolved = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t notresolved_disabled = 0x0U;
inline constexpr std::uint32_t notresolved_enabled = 0x1U;
inline constexpr std::uint32_t notresolved_clear = 0x1U;
}
struct status : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace status_fields {
using status = core::field_descriptor<status, 0U, 4U>;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 2U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x3U;
}
struct nirk : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nirk_fields {
using nirk = core::field_descriptor<nirk, 0U, 5U>;
}
struct irkptr : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace irkptr_fields {
using irkptr = core::field_descriptor<irkptr, 0U, 32U>;
}
struct addrptr : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace addrptr_fields {
using addrptr = core::field_descriptor<addrptr, 0U, 32U>;
}
struct scratchptr : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace scratchptr_fields {
using scratchptr = core::field_descriptor<scratchptr, 0U, 32U>;
}
}
