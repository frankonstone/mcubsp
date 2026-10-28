#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::rtc0 {
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
struct tasks_clear : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clear_fields {
using tasks_clear = core::field_descriptor<tasks_clear, 0U, 1U>;
inline constexpr std::uint32_t tasks_clear_trigger = 0x1U;
}
struct tasks_trigovrflw : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigovrflw_fields {
using tasks_trigovrflw = core::field_descriptor<tasks_trigovrflw, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigovrflw_trigger = 0x1U;
}
struct events_tick : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_tick_fields {
using events_tick = core::field_descriptor<events_tick, 0U, 1U>;
inline constexpr std::uint32_t events_tick_notgenerated = 0x0U;
inline constexpr std::uint32_t events_tick_generated = 0x1U;
}
struct events_ovrflw : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ovrflw_fields {
using events_ovrflw = core::field_descriptor<events_ovrflw, 0U, 1U>;
inline constexpr std::uint32_t events_ovrflw_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ovrflw_generated = 0x1U;
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
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x000F0003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using tick = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t tick_disabled = 0x0U;
inline constexpr std::uint32_t tick_enabled = 0x1U;
inline constexpr std::uint32_t tick_set = 0x1U;
using ovrflw = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t ovrflw_disabled = 0x0U;
inline constexpr std::uint32_t ovrflw_enabled = 0x1U;
inline constexpr std::uint32_t ovrflw_set = 0x1U;
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
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x000F0003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using tick = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t tick_disabled = 0x0U;
inline constexpr std::uint32_t tick_enabled = 0x1U;
inline constexpr std::uint32_t tick_clear = 0x1U;
using ovrflw = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t ovrflw_disabled = 0x0U;
inline constexpr std::uint32_t ovrflw_enabled = 0x1U;
inline constexpr std::uint32_t ovrflw_clear = 0x1U;
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
}
struct evten : core::register_descriptor<std::uint32_t, 0x340U,
    core::access_semantics::read_write, 0x000F0003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace evten_fields {
using tick = core::field_descriptor<evten, 0U, 1U>;
inline constexpr std::uint32_t tick_disabled = 0x0U;
inline constexpr std::uint32_t tick_enabled = 0x1U;
using ovrflw = core::field_descriptor<evten, 1U, 1U>;
inline constexpr std::uint32_t ovrflw_disabled = 0x0U;
inline constexpr std::uint32_t ovrflw_enabled = 0x1U;
using compare0 = core::field_descriptor<evten, 16U, 1U>;
inline constexpr std::uint32_t compare0_disabled = 0x0U;
inline constexpr std::uint32_t compare0_enabled = 0x1U;
using compare1 = core::field_descriptor<evten, 17U, 1U>;
inline constexpr std::uint32_t compare1_disabled = 0x0U;
inline constexpr std::uint32_t compare1_enabled = 0x1U;
using compare2 = core::field_descriptor<evten, 18U, 1U>;
inline constexpr std::uint32_t compare2_disabled = 0x0U;
inline constexpr std::uint32_t compare2_enabled = 0x1U;
using compare3 = core::field_descriptor<evten, 19U, 1U>;
inline constexpr std::uint32_t compare3_disabled = 0x0U;
inline constexpr std::uint32_t compare3_enabled = 0x1U;
}
struct evtenset : core::register_descriptor<std::uint32_t, 0x344U,
    core::access_semantics::set_alias, 0x000F0003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace evtenset_fields {
using tick = core::field_descriptor<evtenset, 0U, 1U>;
inline constexpr std::uint32_t tick_disabled = 0x0U;
inline constexpr std::uint32_t tick_enabled = 0x1U;
inline constexpr std::uint32_t tick_set = 0x1U;
using ovrflw = core::field_descriptor<evtenset, 1U, 1U>;
inline constexpr std::uint32_t ovrflw_disabled = 0x0U;
inline constexpr std::uint32_t ovrflw_enabled = 0x1U;
inline constexpr std::uint32_t ovrflw_set = 0x1U;
using compare0 = core::field_descriptor<evtenset, 16U, 1U>;
inline constexpr std::uint32_t compare0_disabled = 0x0U;
inline constexpr std::uint32_t compare0_enabled = 0x1U;
inline constexpr std::uint32_t compare0_set = 0x1U;
using compare1 = core::field_descriptor<evtenset, 17U, 1U>;
inline constexpr std::uint32_t compare1_disabled = 0x0U;
inline constexpr std::uint32_t compare1_enabled = 0x1U;
inline constexpr std::uint32_t compare1_set = 0x1U;
using compare2 = core::field_descriptor<evtenset, 18U, 1U>;
inline constexpr std::uint32_t compare2_disabled = 0x0U;
inline constexpr std::uint32_t compare2_enabled = 0x1U;
inline constexpr std::uint32_t compare2_set = 0x1U;
using compare3 = core::field_descriptor<evtenset, 19U, 1U>;
inline constexpr std::uint32_t compare3_disabled = 0x0U;
inline constexpr std::uint32_t compare3_enabled = 0x1U;
inline constexpr std::uint32_t compare3_set = 0x1U;
}
struct evtenclr : core::register_descriptor<std::uint32_t, 0x348U,
    core::access_semantics::clear_alias, 0x000F0003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace evtenclr_fields {
using tick = core::field_descriptor<evtenclr, 0U, 1U>;
inline constexpr std::uint32_t tick_disabled = 0x0U;
inline constexpr std::uint32_t tick_enabled = 0x1U;
inline constexpr std::uint32_t tick_clear = 0x1U;
using ovrflw = core::field_descriptor<evtenclr, 1U, 1U>;
inline constexpr std::uint32_t ovrflw_disabled = 0x0U;
inline constexpr std::uint32_t ovrflw_enabled = 0x1U;
inline constexpr std::uint32_t ovrflw_clear = 0x1U;
using compare0 = core::field_descriptor<evtenclr, 16U, 1U>;
inline constexpr std::uint32_t compare0_disabled = 0x0U;
inline constexpr std::uint32_t compare0_enabled = 0x1U;
inline constexpr std::uint32_t compare0_clear = 0x1U;
using compare1 = core::field_descriptor<evtenclr, 17U, 1U>;
inline constexpr std::uint32_t compare1_disabled = 0x0U;
inline constexpr std::uint32_t compare1_enabled = 0x1U;
inline constexpr std::uint32_t compare1_clear = 0x1U;
using compare2 = core::field_descriptor<evtenclr, 18U, 1U>;
inline constexpr std::uint32_t compare2_disabled = 0x0U;
inline constexpr std::uint32_t compare2_enabled = 0x1U;
inline constexpr std::uint32_t compare2_clear = 0x1U;
using compare3 = core::field_descriptor<evtenclr, 19U, 1U>;
inline constexpr std::uint32_t compare3_disabled = 0x0U;
inline constexpr std::uint32_t compare3_enabled = 0x1U;
inline constexpr std::uint32_t compare3_clear = 0x1U;
}
struct counter : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace counter_fields {
using counter = core::field_descriptor<counter, 0U, 24U>;
}
struct prescaler : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prescaler_fields {
using prescaler = core::field_descriptor<prescaler, 0U, 12U>;
}
struct cc0 : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc0_fields {
using compare = core::field_descriptor<cc0, 0U, 24U>;
}
struct cc1 : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc1_fields {
using compare = core::field_descriptor<cc1, 0U, 24U>;
}
struct cc2 : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc2_fields {
using compare = core::field_descriptor<cc2, 0U, 24U>;
}
struct cc3 : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cc3_fields {
using compare = core::field_descriptor<cc3, 0U, 24U>;
}
}
