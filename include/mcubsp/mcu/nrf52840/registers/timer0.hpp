#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::timer0 {
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
struct tasks_count : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_count_fields {
using tasks_count = core::field_descriptor<tasks_count, 0U, 1U>;
inline constexpr std::uint32_t tasks_count_trigger = 0x1U;
}
struct tasks_clear : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clear_fields {
using tasks_clear = core::field_descriptor<tasks_clear, 0U, 1U>;
inline constexpr std::uint32_t tasks_clear_trigger = 0x1U;
}
struct tasks_shutdown : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_shutdown_fields {
using tasks_shutdown = core::field_descriptor<tasks_shutdown, 0U, 1U>;
inline constexpr std::uint32_t tasks_shutdown_trigger = 0x1U;
}
struct tasks_capture0 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_capture0_fields {
using tasks_capture = core::field_descriptor<tasks_capture0, 0U, 1U>;
inline constexpr std::uint32_t tasks_capture_trigger = 0x1U;
}
struct tasks_capture1 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_capture1_fields {
using tasks_capture = core::field_descriptor<tasks_capture1, 0U, 1U>;
inline constexpr std::uint32_t tasks_capture_trigger = 0x1U;
}
struct tasks_capture2 : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_capture2_fields {
using tasks_capture = core::field_descriptor<tasks_capture2, 0U, 1U>;
inline constexpr std::uint32_t tasks_capture_trigger = 0x1U;
}
struct tasks_capture3 : core::register_descriptor<std::uint32_t, 0x4CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_capture3_fields {
using tasks_capture = core::field_descriptor<tasks_capture3, 0U, 1U>;
inline constexpr std::uint32_t tasks_capture_trigger = 0x1U;
}
struct tasks_capture4 : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_capture4_fields {
using tasks_capture = core::field_descriptor<tasks_capture4, 0U, 1U>;
inline constexpr std::uint32_t tasks_capture_trigger = 0x1U;
}
struct tasks_capture5 : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_capture5_fields {
using tasks_capture = core::field_descriptor<tasks_capture5, 0U, 1U>;
inline constexpr std::uint32_t tasks_capture_trigger = 0x1U;
}
struct events_compare0 : core::register_descriptor<std::uint32_t, 0x140U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_compare0_fields {
using events_compare = core::field_descriptor<events_compare0, 0U, 1U>;
inline constexpr std::uint32_t events_compare_notgenerated = 0x0U;
inline constexpr std::uint32_t events_compare_generated = 0x1U;
}
struct events_compare1 : core::register_descriptor<std::uint32_t, 0x144U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_compare1_fields {
using events_compare = core::field_descriptor<events_compare1, 0U, 1U>;
inline constexpr std::uint32_t events_compare_notgenerated = 0x0U;
inline constexpr std::uint32_t events_compare_generated = 0x1U;
}
struct events_compare2 : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_compare2_fields {
using events_compare = core::field_descriptor<events_compare2, 0U, 1U>;
inline constexpr std::uint32_t events_compare_notgenerated = 0x0U;
inline constexpr std::uint32_t events_compare_generated = 0x1U;
}
struct events_compare3 : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_compare3_fields {
using events_compare = core::field_descriptor<events_compare3, 0U, 1U>;
inline constexpr std::uint32_t events_compare_notgenerated = 0x0U;
inline constexpr std::uint32_t events_compare_generated = 0x1U;
}
struct events_compare4 : core::register_descriptor<std::uint32_t, 0x150U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_compare4_fields {
using events_compare = core::field_descriptor<events_compare4, 0U, 1U>;
inline constexpr std::uint32_t events_compare_notgenerated = 0x0U;
inline constexpr std::uint32_t events_compare_generated = 0x1U;
}
struct events_compare5 : core::register_descriptor<std::uint32_t, 0x154U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_compare5_fields {
using events_compare = core::field_descriptor<events_compare5, 0U, 1U>;
inline constexpr std::uint32_t events_compare_notgenerated = 0x0U;
inline constexpr std::uint32_t events_compare_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00003F3FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using compare0_clear = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t compare0_clear_disabled = 0x0U;
inline constexpr std::uint32_t compare0_clear_enabled = 0x1U;
using compare1_clear = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t compare1_clear_disabled = 0x0U;
inline constexpr std::uint32_t compare1_clear_enabled = 0x1U;
using compare2_clear = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t compare2_clear_disabled = 0x0U;
inline constexpr std::uint32_t compare2_clear_enabled = 0x1U;
using compare3_clear = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t compare3_clear_disabled = 0x0U;
inline constexpr std::uint32_t compare3_clear_enabled = 0x1U;
using compare4_clear = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t compare4_clear_disabled = 0x0U;
inline constexpr std::uint32_t compare4_clear_enabled = 0x1U;
using compare5_clear = core::field_descriptor<shorts, 5U, 1U>;
inline constexpr std::uint32_t compare5_clear_disabled = 0x0U;
inline constexpr std::uint32_t compare5_clear_enabled = 0x1U;
using compare0_stop = core::field_descriptor<shorts, 8U, 1U>;
inline constexpr std::uint32_t compare0_stop_disabled = 0x0U;
inline constexpr std::uint32_t compare0_stop_enabled = 0x1U;
using compare1_stop = core::field_descriptor<shorts, 9U, 1U>;
inline constexpr std::uint32_t compare1_stop_disabled = 0x0U;
inline constexpr std::uint32_t compare1_stop_enabled = 0x1U;
using compare2_stop = core::field_descriptor<shorts, 10U, 1U>;
inline constexpr std::uint32_t compare2_stop_disabled = 0x0U;
inline constexpr std::uint32_t compare2_stop_enabled = 0x1U;
using compare3_stop = core::field_descriptor<shorts, 11U, 1U>;
inline constexpr std::uint32_t compare3_stop_disabled = 0x0U;
inline constexpr std::uint32_t compare3_stop_enabled = 0x1U;
using compare4_stop = core::field_descriptor<shorts, 12U, 1U>;
inline constexpr std::uint32_t compare4_stop_disabled = 0x0U;
inline constexpr std::uint32_t compare4_stop_enabled = 0x1U;
using compare5_stop = core::field_descriptor<shorts, 13U, 1U>;
inline constexpr std::uint32_t compare5_stop_disabled = 0x0U;
inline constexpr std::uint32_t compare5_stop_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x003F0000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using compare0 = core::field_descriptor<intenset, 16U, 1U>;
inline constexpr std::uint32_t compare0_disabled = 0x0U;
inline constexpr std::uint32_t compare0_enabled = 0x1U;
inline constexpr std::uint32_t compare0_set = 0x1U;
using compare1 = core::field_descriptor<intenset, 17U, 1U>;
inline constexpr std::uint32_t compare1_disabled = 0x0U;
inline constexpr std::uint32_t compare1_enabled = 0x1U;
inline constexpr std::uint32_t compare1_set = 0x1U;
using compare2 = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t compare2_disabled = 0x0U;
inline constexpr std::uint32_t compare2_enabled = 0x1U;
inline constexpr std::uint32_t compare2_set = 0x1U;
using compare3 = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t compare3_disabled = 0x0U;
inline constexpr std::uint32_t compare3_enabled = 0x1U;
inline constexpr std::uint32_t compare3_set = 0x1U;
using compare4 = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t compare4_disabled = 0x0U;
inline constexpr std::uint32_t compare4_enabled = 0x1U;
inline constexpr std::uint32_t compare4_set = 0x1U;
using compare5 = core::field_descriptor<intenset, 21U, 1U>;
inline constexpr std::uint32_t compare5_disabled = 0x0U;
inline constexpr std::uint32_t compare5_enabled = 0x1U;
inline constexpr std::uint32_t compare5_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x003F0000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using compare0 = core::field_descriptor<intenclr, 16U, 1U>;
inline constexpr std::uint32_t compare0_disabled = 0x0U;
inline constexpr std::uint32_t compare0_enabled = 0x1U;
inline constexpr std::uint32_t compare0_clear = 0x1U;
using compare1 = core::field_descriptor<intenclr, 17U, 1U>;
inline constexpr std::uint32_t compare1_disabled = 0x0U;
inline constexpr std::uint32_t compare1_enabled = 0x1U;
inline constexpr std::uint32_t compare1_clear = 0x1U;
using compare2 = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t compare2_disabled = 0x0U;
inline constexpr std::uint32_t compare2_enabled = 0x1U;
inline constexpr std::uint32_t compare2_clear = 0x1U;
using compare3 = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t compare3_disabled = 0x0U;
inline constexpr std::uint32_t compare3_enabled = 0x1U;
inline constexpr std::uint32_t compare3_clear = 0x1U;
using compare4 = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t compare4_disabled = 0x0U;
inline constexpr std::uint32_t compare4_enabled = 0x1U;
inline constexpr std::uint32_t compare4_clear = 0x1U;
using compare5 = core::field_descriptor<intenclr, 21U, 1U>;
inline constexpr std::uint32_t compare5_disabled = 0x0U;
inline constexpr std::uint32_t compare5_enabled = 0x1U;
inline constexpr std::uint32_t compare5_clear = 0x1U;
}
struct mode : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mode_fields {
using mode = core::field_descriptor<mode, 0U, 2U>;
inline constexpr std::uint32_t mode_timer = 0x0U;
inline constexpr std::uint32_t mode_counter = 0x1U;
inline constexpr std::uint32_t mode_lowpowercounter = 0x2U;
}
struct bitmode : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace bitmode_fields {
using bitmode = core::field_descriptor<bitmode, 0U, 2U>;
inline constexpr std::uint32_t bitmode_reg_16bit = 0x0U;
inline constexpr std::uint32_t bitmode_reg_08bit = 0x1U;
inline constexpr std::uint32_t bitmode_reg_24bit = 0x2U;
inline constexpr std::uint32_t bitmode_reg_32bit = 0x3U;
}
struct prescaler : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000004U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prescaler_fields {
using prescaler = core::field_descriptor<prescaler, 0U, 4U>;
}
struct cc0 : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc0_fields {
using cc = core::field_descriptor<cc0, 0U, 32U>;
}
struct cc1 : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc1_fields {
using cc = core::field_descriptor<cc1, 0U, 32U>;
}
struct cc2 : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc2_fields {
using cc = core::field_descriptor<cc2, 0U, 32U>;
}
struct cc3 : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc3_fields {
using cc = core::field_descriptor<cc3, 0U, 32U>;
}
struct cc4 : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc4_fields {
using cc = core::field_descriptor<cc4, 0U, 32U>;
}
struct cc5 : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc5_fields {
using cc = core::field_descriptor<cc5, 0U, 32U>;
}
}
