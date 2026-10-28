#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::twis0 {
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
struct tasks_preparerx : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_preparerx_fields {
using tasks_preparerx = core::field_descriptor<tasks_preparerx, 0U, 1U>;
inline constexpr std::uint32_t tasks_preparerx_trigger = 0x1U;
}
struct tasks_preparetx : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_preparetx_fields {
using tasks_preparetx = core::field_descriptor<tasks_preparetx, 0U, 1U>;
inline constexpr std::uint32_t tasks_preparetx_trigger = 0x1U;
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
struct events_write : core::register_descriptor<std::uint32_t, 0x164U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_write_fields {
using events_write = core::field_descriptor<events_write, 0U, 1U>;
inline constexpr std::uint32_t events_write_notgenerated = 0x0U;
inline constexpr std::uint32_t events_write_generated = 0x1U;
}
struct events_read : core::register_descriptor<std::uint32_t, 0x168U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_read_fields {
using events_read = core::field_descriptor<events_read, 0U, 1U>;
inline constexpr std::uint32_t events_read_notgenerated = 0x0U;
inline constexpr std::uint32_t events_read_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00006000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using write_suspend = core::field_descriptor<shorts, 13U, 1U>;
inline constexpr std::uint32_t write_suspend_disabled = 0x0U;
inline constexpr std::uint32_t write_suspend_enabled = 0x1U;
using read_suspend = core::field_descriptor<shorts, 14U, 1U>;
inline constexpr std::uint32_t read_suspend_disabled = 0x0U;
inline constexpr std::uint32_t read_suspend_enabled = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x06180202U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using stopped = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
using error = core::field_descriptor<inten, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
using rxstarted = core::field_descriptor<inten, 19U, 1U>;
inline constexpr std::uint32_t rxstarted_disabled = 0x0U;
inline constexpr std::uint32_t rxstarted_enabled = 0x1U;
using txstarted = core::field_descriptor<inten, 20U, 1U>;
inline constexpr std::uint32_t txstarted_disabled = 0x0U;
inline constexpr std::uint32_t txstarted_enabled = 0x1U;
using write = core::field_descriptor<inten, 25U, 1U>;
inline constexpr std::uint32_t write_disabled = 0x0U;
inline constexpr std::uint32_t write_enabled = 0x1U;
using read = core::field_descriptor<inten, 26U, 1U>;
inline constexpr std::uint32_t read_disabled = 0x0U;
inline constexpr std::uint32_t read_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x06180202U, false,
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
using rxstarted = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t rxstarted_disabled = 0x0U;
inline constexpr std::uint32_t rxstarted_enabled = 0x1U;
inline constexpr std::uint32_t rxstarted_set = 0x1U;
using txstarted = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t txstarted_disabled = 0x0U;
inline constexpr std::uint32_t txstarted_enabled = 0x1U;
inline constexpr std::uint32_t txstarted_set = 0x1U;
using write = core::field_descriptor<intenset, 25U, 1U>;
inline constexpr std::uint32_t write_disabled = 0x0U;
inline constexpr std::uint32_t write_enabled = 0x1U;
inline constexpr std::uint32_t write_set = 0x1U;
using read = core::field_descriptor<intenset, 26U, 1U>;
inline constexpr std::uint32_t read_disabled = 0x0U;
inline constexpr std::uint32_t read_enabled = 0x1U;
inline constexpr std::uint32_t read_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x06180202U, false,
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
using rxstarted = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t rxstarted_disabled = 0x0U;
inline constexpr std::uint32_t rxstarted_enabled = 0x1U;
inline constexpr std::uint32_t rxstarted_clear = 0x1U;
using txstarted = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t txstarted_disabled = 0x0U;
inline constexpr std::uint32_t txstarted_enabled = 0x1U;
inline constexpr std::uint32_t txstarted_clear = 0x1U;
using write = core::field_descriptor<intenclr, 25U, 1U>;
inline constexpr std::uint32_t write_disabled = 0x0U;
inline constexpr std::uint32_t write_enabled = 0x1U;
inline constexpr std::uint32_t write_clear = 0x1U;
using read = core::field_descriptor<intenclr, 26U, 1U>;
inline constexpr std::uint32_t read_disabled = 0x0U;
inline constexpr std::uint32_t read_enabled = 0x1U;
inline constexpr std::uint32_t read_clear = 0x1U;
}
struct errorsrc : core::register_descriptor<std::uint32_t, 0x4D0U,
    core::access_semantics::write_one_to_clear, 0x0000000DU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace errorsrc_fields {
using overflow = core::field_descriptor<errorsrc, 0U, 1U>;
inline constexpr std::uint32_t overflow_notdetected = 0x0U;
inline constexpr std::uint32_t overflow_detected = 0x1U;
using dnack = core::field_descriptor<errorsrc, 2U, 1U>;
inline constexpr std::uint32_t dnack_notreceived = 0x0U;
inline constexpr std::uint32_t dnack_received = 0x1U;
using overread = core::field_descriptor<errorsrc, 3U, 1U>;
inline constexpr std::uint32_t overread_notdetected = 0x0U;
inline constexpr std::uint32_t overread_detected = 0x1U;
}
struct match : core::register_descriptor<std::uint32_t, 0x4D4U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace match_fields {
using match = core::field_descriptor<match, 0U, 1U>;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x9U;
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
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_list_fields {
using list = core::field_descriptor<rxd_list, 0U, 2U>;
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
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_list_fields {
using list = core::field_descriptor<txd_list, 0U, 2U>;
inline constexpr std::uint32_t list_disabled = 0x0U;
inline constexpr std::uint32_t list_arraylist = 0x1U;
}
struct address0 : core::register_descriptor<std::uint32_t, 0x588U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace address0_fields {
using address = core::field_descriptor<address0, 0U, 7U>;
}
struct address1 : core::register_descriptor<std::uint32_t, 0x58CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace address1_fields {
using address = core::field_descriptor<address1, 0U, 7U>;
}
struct config : core::register_descriptor<std::uint32_t, 0x594U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using address0 = core::field_descriptor<config, 0U, 1U>;
inline constexpr std::uint32_t address0_disabled = 0x0U;
inline constexpr std::uint32_t address0_enabled = 0x1U;
using address1 = core::field_descriptor<config, 1U, 1U>;
inline constexpr std::uint32_t address1_disabled = 0x0U;
inline constexpr std::uint32_t address1_enabled = 0x1U;
}
struct orc : core::register_descriptor<std::uint32_t, 0x5C0U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace orc_fields {
using orc = core::field_descriptor<orc, 0U, 8U>;
}
}
