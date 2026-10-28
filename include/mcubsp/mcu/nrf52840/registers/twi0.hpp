#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::twi0 {
struct tasks_startrx : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startrx_fields {
using tasks_startrx = core::field_descriptor<tasks_startrx, 0U, 1U>;
inline constexpr std::uint32_t tasks_startrx_trigger = 0x1U;
}
struct tasks_starttx : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_starttx_fields {
using tasks_starttx = core::field_descriptor<tasks_starttx, 0U, 1U>;
inline constexpr std::uint32_t tasks_starttx_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct tasks_suspend : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_suspend_fields {
using tasks_suspend = core::field_descriptor<tasks_suspend, 0U, 1U>;
inline constexpr std::uint32_t tasks_suspend_trigger = 0x1U;
}
struct tasks_resume : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_resume_fields {
using tasks_resume = core::field_descriptor<tasks_resume, 0U, 1U>;
inline constexpr std::uint32_t tasks_resume_trigger = 0x1U;
}
struct events_stopped : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_stopped_fields {
using events_stopped = core::field_descriptor<events_stopped, 0U, 1U>;
inline constexpr std::uint32_t events_stopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_stopped_generated = 0x1U;
}
struct events_rxdready : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxdready_fields {
using events_rxdready = core::field_descriptor<events_rxdready, 0U, 1U>;
inline constexpr std::uint32_t events_rxdready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxdready_generated = 0x1U;
}
struct events_txdsent : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txdsent_fields {
using events_txdsent = core::field_descriptor<events_txdsent, 0U, 1U>;
inline constexpr std::uint32_t events_txdsent_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txdsent_generated = 0x1U;
}
struct events_error : core::register_descriptor<std::uint32_t, 0x124U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_error_fields {
using events_error = core::field_descriptor<events_error, 0U, 1U>;
inline constexpr std::uint32_t events_error_notgenerated = 0x0U;
inline constexpr std::uint32_t events_error_generated = 0x1U;
}
struct events_bb : core::register_descriptor<std::uint32_t, 0x138U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_bb_fields {
using events_bb = core::field_descriptor<events_bb, 0U, 1U>;
inline constexpr std::uint32_t events_bb_notgenerated = 0x0U;
inline constexpr std::uint32_t events_bb_generated = 0x1U;
}
struct events_suspended : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_suspended_fields {
using events_suspended = core::field_descriptor<events_suspended, 0U, 1U>;
inline constexpr std::uint32_t events_suspended_notgenerated = 0x0U;
inline constexpr std::uint32_t events_suspended_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using bb_suspend = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t bb_suspend_disabled = 0x0U;
inline constexpr std::uint32_t bb_suspend_enabled = 0x1U;
using bb_stop = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t bb_stop_disabled = 0x0U;
inline constexpr std::uint32_t bb_stop_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00044286U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using stopped = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using rxdready = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t rxdready_disabled = 0x0U;
inline constexpr std::uint32_t rxdready_enabled = 0x1U;
inline constexpr std::uint32_t rxdready_set = 0x1U;
using txdsent = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t txdsent_disabled = 0x0U;
inline constexpr std::uint32_t txdsent_enabled = 0x1U;
inline constexpr std::uint32_t txdsent_set = 0x1U;
using error = core::field_descriptor<intenset, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_set = 0x1U;
using bb = core::field_descriptor<intenset, 14U, 1U>;
inline constexpr std::uint32_t bb_disabled = 0x0U;
inline constexpr std::uint32_t bb_enabled = 0x1U;
inline constexpr std::uint32_t bb_set = 0x1U;
using suspended = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t suspended_disabled = 0x0U;
inline constexpr std::uint32_t suspended_enabled = 0x1U;
inline constexpr std::uint32_t suspended_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00044286U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using stopped = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using rxdready = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t rxdready_disabled = 0x0U;
inline constexpr std::uint32_t rxdready_enabled = 0x1U;
inline constexpr std::uint32_t rxdready_clear = 0x1U;
using txdsent = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t txdsent_disabled = 0x0U;
inline constexpr std::uint32_t txdsent_enabled = 0x1U;
inline constexpr std::uint32_t txdsent_clear = 0x1U;
using error = core::field_descriptor<intenclr, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_clear = 0x1U;
using bb = core::field_descriptor<intenclr, 14U, 1U>;
inline constexpr std::uint32_t bb_disabled = 0x0U;
inline constexpr std::uint32_t bb_enabled = 0x1U;
inline constexpr std::uint32_t bb_clear = 0x1U;
using suspended = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t suspended_disabled = 0x0U;
inline constexpr std::uint32_t suspended_enabled = 0x1U;
inline constexpr std::uint32_t suspended_clear = 0x1U;
}
struct errorsrc : core::register_descriptor<std::uint32_t, 0x4C4U,
    core::access_semantics::write_one_to_clear, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace errorsrc_fields {
using overrun = core::field_descriptor<errorsrc, 0U, 1U>;
inline constexpr std::uint32_t overrun_notpresent = 0x0U;
inline constexpr std::uint32_t overrun_present = 0x1U;
using anack = core::field_descriptor<errorsrc, 1U, 1U>;
inline constexpr std::uint32_t anack_notpresent = 0x0U;
inline constexpr std::uint32_t anack_present = 0x1U;
using dnack = core::field_descriptor<errorsrc, 2U, 1U>;
inline constexpr std::uint32_t dnack_notpresent = 0x0U;
inline constexpr std::uint32_t dnack_present = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x5U;
}
struct psel_scl : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_scl_fields {
using pin = core::field_descriptor<psel_scl, 0U, 5U>;
using port = core::field_descriptor<psel_scl, 5U, 1U>;
using connect = core::field_descriptor<psel_scl, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_sda : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sda_fields {
using pin = core::field_descriptor<psel_sda, 0U, 5U>;
using port = core::field_descriptor<psel_sda, 5U, 1U>;
using connect = core::field_descriptor<psel_sda, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct rxd : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_clear, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_fields {
using rxd = core::field_descriptor<rxd, 0U, 8U>;
}
struct txd : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_fields {
using txd = core::field_descriptor<txd, 0U, 8U>;
}
struct frequency : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x04000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace frequency_fields {
using frequency = core::field_descriptor<frequency, 0U, 32U>;
inline constexpr std::uint32_t frequency_k100 = 0x1980000U;
inline constexpr std::uint32_t frequency_k250 = 0x4000000U;
inline constexpr std::uint32_t frequency_k400 = 0x6680000U;
}
struct address : core::register_descriptor<std::uint32_t, 0x588U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace address_fields {
using address = core::field_descriptor<address, 0U, 7U>;
}
}
