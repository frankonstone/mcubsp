#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::temp {
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
struct events_datardy : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_datardy_fields {
using events_datardy = core::field_descriptor<events_datardy, 0U, 1U>;
inline constexpr std::uint32_t events_datardy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_datardy_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using datardy = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t datardy_disabled = 0x0U;
inline constexpr std::uint32_t datardy_enabled = 0x1U;
inline constexpr std::uint32_t datardy_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using datardy = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t datardy_disabled = 0x0U;
inline constexpr std::uint32_t datardy_enabled = 0x1U;
inline constexpr std::uint32_t datardy_clear = 0x1U;
}
struct temp : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_fields {
using temp = core::field_descriptor<temp, 0U, 32U>;
}
struct a0 : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000326U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace a0_fields {
using a0 = core::field_descriptor<a0, 0U, 12U>;
}
struct a1 : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000348U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace a1_fields {
using a1 = core::field_descriptor<a1, 0U, 12U>;
}
struct a2 : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000003AAU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace a2_fields {
using a2 = core::field_descriptor<a2, 0U, 12U>;
}
struct a3 : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x0000040EU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace a3_fields {
using a3 = core::field_descriptor<a3, 0U, 12U>;
}
struct a4 : core::register_descriptor<std::uint32_t, 0x530U,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000004BDU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace a4_fields {
using a4 = core::field_descriptor<a4, 0U, 12U>;
}
struct a5 : core::register_descriptor<std::uint32_t, 0x534U,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000005A3U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace a5_fields {
using a5 = core::field_descriptor<a5, 0U, 12U>;
}
struct b0 : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00003FEFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace b0_fields {
using b0 = core::field_descriptor<b0, 0U, 14U>;
}
struct b1 : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00003FBEU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace b1_fields {
using b1 = core::field_descriptor<b1, 0U, 14U>;
}
struct b2 : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00003FBEU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace b2_fields {
using b2 = core::field_descriptor<b2, 0U, 14U>;
}
struct b3 : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000012U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace b3_fields {
using b3 = core::field_descriptor<b3, 0U, 14U>;
}
struct b4 : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000124U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace b4_fields {
using b4 = core::field_descriptor<b4, 0U, 14U>;
}
struct b5 : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x0000027CU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace b5_fields {
using b5 = core::field_descriptor<b5, 0U, 14U>;
}
struct t0 : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000000E2U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace t0_fields {
using t0 = core::field_descriptor<t0, 0U, 8U>;
}
struct t1 : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace t1_fields {
using t1 = core::field_descriptor<t1, 0U, 8U>;
}
struct t2 : core::register_descriptor<std::uint32_t, 0x568U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000019U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace t2_fields {
using t2 = core::field_descriptor<t2, 0U, 8U>;
}
struct t3 : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x0000003CU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace t3_fields {
using t3 = core::field_descriptor<t3, 0U, 8U>;
}
struct t4 : core::register_descriptor<std::uint32_t, 0x570U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000050U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace t4_fields {
using t4 = core::field_descriptor<t4, 0U, 8U>;
}
}
