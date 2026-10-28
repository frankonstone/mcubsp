#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::twim0 {
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
struct events_error : core::register_descriptor<std::uint32_t, 0x124U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_error_fields {
using events_error = core::field_descriptor<events_error, 0U, 1U>;
inline constexpr std::uint32_t events_error_notgenerated = 0x0U;
inline constexpr std::uint32_t events_error_generated = 0x1U;
}
struct events_suspended : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_suspended_fields {
using events_suspended = core::field_descriptor<events_suspended, 0U, 1U>;
inline constexpr std::uint32_t events_suspended_notgenerated = 0x0U;
inline constexpr std::uint32_t events_suspended_generated = 0x1U;
}
struct events_rxstarted : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxstarted_fields {
using events_rxstarted = core::field_descriptor<events_rxstarted, 0U, 1U>;
inline constexpr std::uint32_t events_rxstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxstarted_generated = 0x1U;
}
struct events_txstarted : core::register_descriptor<std::uint32_t, 0x150U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txstarted_fields {
using events_txstarted = core::field_descriptor<events_txstarted, 0U, 1U>;
inline constexpr std::uint32_t events_txstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txstarted_generated = 0x1U;
}
struct events_lastrx : core::register_descriptor<std::uint32_t, 0x15CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_lastrx_fields {
using events_lastrx = core::field_descriptor<events_lastrx, 0U, 1U>;
inline constexpr std::uint32_t events_lastrx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_lastrx_generated = 0x1U;
}
struct events_lasttx : core::register_descriptor<std::uint32_t, 0x160U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_lasttx_fields {
using events_lasttx = core::field_descriptor<events_lasttx, 0U, 1U>;
inline constexpr std::uint32_t events_lasttx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_lasttx_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00001F80U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using lasttx_startrx = core::field_descriptor<shorts, 7U, 1U>;
inline constexpr std::uint32_t lasttx_startrx_disabled = 0x0U;
inline constexpr std::uint32_t lasttx_startrx_enabled = 0x1U;
using lasttx_suspend = core::field_descriptor<shorts, 8U, 1U>;
inline constexpr std::uint32_t lasttx_suspend_disabled = 0x0U;
inline constexpr std::uint32_t lasttx_suspend_enabled = 0x1U;
using lasttx_stop = core::field_descriptor<shorts, 9U, 1U>;
inline constexpr std::uint32_t lasttx_stop_disabled = 0x0U;
inline constexpr std::uint32_t lasttx_stop_enabled = 0x1U;
using lastrx_starttx = core::field_descriptor<shorts, 10U, 1U>;
inline constexpr std::uint32_t lastrx_starttx_disabled = 0x0U;
inline constexpr std::uint32_t lastrx_starttx_enabled = 0x1U;
using lastrx_suspend = core::field_descriptor<shorts, 11U, 1U>;
inline constexpr std::uint32_t lastrx_suspend_disabled = 0x0U;
inline constexpr std::uint32_t lastrx_suspend_enabled = 0x1U;
using lastrx_stop = core::field_descriptor<shorts, 12U, 1U>;
inline constexpr std::uint32_t lastrx_stop_disabled = 0x0U;
inline constexpr std::uint32_t lastrx_stop_enabled = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x019C0202U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using stopped = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
using error = core::field_descriptor<inten, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
using suspended = core::field_descriptor<inten, 18U, 1U>;
inline constexpr std::uint32_t suspended_disabled = 0x0U;
inline constexpr std::uint32_t suspended_enabled = 0x1U;
using rxstarted = core::field_descriptor<inten, 19U, 1U>;
inline constexpr std::uint32_t rxstarted_disabled = 0x0U;
inline constexpr std::uint32_t rxstarted_enabled = 0x1U;
using txstarted = core::field_descriptor<inten, 20U, 1U>;
inline constexpr std::uint32_t txstarted_disabled = 0x0U;
inline constexpr std::uint32_t txstarted_enabled = 0x1U;
using lastrx = core::field_descriptor<inten, 23U, 1U>;
inline constexpr std::uint32_t lastrx_disabled = 0x0U;
inline constexpr std::uint32_t lastrx_enabled = 0x1U;
using lasttx = core::field_descriptor<inten, 24U, 1U>;
inline constexpr std::uint32_t lasttx_disabled = 0x0U;
inline constexpr std::uint32_t lasttx_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x019C0202U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using stopped = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using error = core::field_descriptor<intenset, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_set = 0x1U;
using suspended = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t suspended_disabled = 0x0U;
inline constexpr std::uint32_t suspended_enabled = 0x1U;
inline constexpr std::uint32_t suspended_set = 0x1U;
using rxstarted = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t rxstarted_disabled = 0x0U;
inline constexpr std::uint32_t rxstarted_enabled = 0x1U;
inline constexpr std::uint32_t rxstarted_set = 0x1U;
using txstarted = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t txstarted_disabled = 0x0U;
inline constexpr std::uint32_t txstarted_enabled = 0x1U;
inline constexpr std::uint32_t txstarted_set = 0x1U;
using lastrx = core::field_descriptor<intenset, 23U, 1U>;
inline constexpr std::uint32_t lastrx_disabled = 0x0U;
inline constexpr std::uint32_t lastrx_enabled = 0x1U;
inline constexpr std::uint32_t lastrx_set = 0x1U;
using lasttx = core::field_descriptor<intenset, 24U, 1U>;
inline constexpr std::uint32_t lasttx_disabled = 0x0U;
inline constexpr std::uint32_t lasttx_enabled = 0x1U;
inline constexpr std::uint32_t lasttx_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x019C0202U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using stopped = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using error = core::field_descriptor<intenclr, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_clear = 0x1U;
using suspended = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t suspended_disabled = 0x0U;
inline constexpr std::uint32_t suspended_enabled = 0x1U;
inline constexpr std::uint32_t suspended_clear = 0x1U;
using rxstarted = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t rxstarted_disabled = 0x0U;
inline constexpr std::uint32_t rxstarted_enabled = 0x1U;
inline constexpr std::uint32_t rxstarted_clear = 0x1U;
using txstarted = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t txstarted_disabled = 0x0U;
inline constexpr std::uint32_t txstarted_enabled = 0x1U;
inline constexpr std::uint32_t txstarted_clear = 0x1U;
using lastrx = core::field_descriptor<intenclr, 23U, 1U>;
inline constexpr std::uint32_t lastrx_disabled = 0x0U;
inline constexpr std::uint32_t lastrx_enabled = 0x1U;
inline constexpr std::uint32_t lastrx_clear = 0x1U;
using lasttx = core::field_descriptor<intenclr, 24U, 1U>;
inline constexpr std::uint32_t lasttx_disabled = 0x0U;
inline constexpr std::uint32_t lasttx_enabled = 0x1U;
inline constexpr std::uint32_t lasttx_clear = 0x1U;
}
struct errorsrc : core::register_descriptor<std::uint32_t, 0x4C4U,
    core::access_semantics::write_one_to_clear, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace errorsrc_fields {
using overrun = core::field_descriptor<errorsrc, 0U, 1U>;
inline constexpr std::uint32_t overrun_notreceived = 0x0U;
inline constexpr std::uint32_t overrun_received = 0x1U;
using anack = core::field_descriptor<errorsrc, 1U, 1U>;
inline constexpr std::uint32_t anack_notreceived = 0x0U;
inline constexpr std::uint32_t anack_received = 0x1U;
using dnack = core::field_descriptor<errorsrc, 2U, 1U>;
inline constexpr std::uint32_t dnack_notreceived = 0x0U;
inline constexpr std::uint32_t dnack_received = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x6U;
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
struct frequency : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x04000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace frequency_fields {
using frequency = core::field_descriptor<frequency, 0U, 32U>;
inline constexpr std::uint32_t frequency_k100 = 0x1980000U;
inline constexpr std::uint32_t frequency_k250 = 0x4000000U;
inline constexpr std::uint32_t frequency_k400 = 0x6400000U;
}
struct rxd_ptr : core::register_descriptor<std::uint32_t, 0x534U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_ptr_fields {
using ptr = core::field_descriptor<rxd_ptr, 0U, 32U>;
}
struct rxd_maxcnt : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_maxcnt_fields {
using maxcnt = core::field_descriptor<rxd_maxcnt, 0U, 16U>;
}
struct rxd_amount : core::register_descriptor<std::uint32_t, 0x53CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_amount_fields {
using amount = core::field_descriptor<rxd_amount, 0U, 16U>;
}
struct rxd_list : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_list_fields {
using list = core::field_descriptor<rxd_list, 0U, 3U>;
inline constexpr std::uint32_t list_disabled = 0x0U;
inline constexpr std::uint32_t list_arraylist = 0x1U;
}
struct txd_ptr : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_ptr_fields {
using ptr = core::field_descriptor<txd_ptr, 0U, 32U>;
}
struct txd_maxcnt : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_maxcnt_fields {
using maxcnt = core::field_descriptor<txd_maxcnt, 0U, 16U>;
}
struct txd_amount : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_amount_fields {
using amount = core::field_descriptor<txd_amount, 0U, 16U>;
}
struct txd_list : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_list_fields {
using list = core::field_descriptor<txd_list, 0U, 3U>;
inline constexpr std::uint32_t list_disabled = 0x0U;
inline constexpr std::uint32_t list_arraylist = 0x1U;
}
struct address : core::register_descriptor<std::uint32_t, 0x588U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace address_fields {
using address = core::field_descriptor<address, 0U, 7U>;
}
}
