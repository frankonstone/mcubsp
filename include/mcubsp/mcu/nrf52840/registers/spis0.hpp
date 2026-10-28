#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::spis0 {
struct tasks_acquire : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_acquire_fields {
using tasks_acquire = core::field_descriptor<tasks_acquire, 0U, 1U>;
inline constexpr std::uint32_t tasks_acquire_trigger = 0x1U;
}
struct tasks_release : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_release_fields {
using tasks_release = core::field_descriptor<tasks_release, 0U, 1U>;
inline constexpr std::uint32_t tasks_release_trigger = 0x1U;
}
struct events_end : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_end_fields {
using events_end = core::field_descriptor<events_end, 0U, 1U>;
inline constexpr std::uint32_t events_end_notgenerated = 0x0U;
inline constexpr std::uint32_t events_end_generated = 0x1U;
}
struct events_endrx : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endrx_fields {
using events_endrx = core::field_descriptor<events_endrx, 0U, 1U>;
inline constexpr std::uint32_t events_endrx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endrx_generated = 0x1U;
}
struct events_acquired : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_acquired_fields {
using events_acquired = core::field_descriptor<events_acquired, 0U, 1U>;
inline constexpr std::uint32_t events_acquired_notgenerated = 0x0U;
inline constexpr std::uint32_t events_acquired_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00000004U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using end_acquire = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t end_acquire_disabled = 0x0U;
inline constexpr std::uint32_t end_acquire_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000412U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using end = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_set = 0x1U;
using endrx = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
inline constexpr std::uint32_t endrx_set = 0x1U;
using acquired = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t acquired_disabled = 0x0U;
inline constexpr std::uint32_t acquired_enabled = 0x1U;
inline constexpr std::uint32_t acquired_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000412U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using end = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_clear = 0x1U;
using endrx = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
inline constexpr std::uint32_t endrx_clear = 0x1U;
using acquired = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t acquired_disabled = 0x0U;
inline constexpr std::uint32_t acquired_enabled = 0x1U;
inline constexpr std::uint32_t acquired_clear = 0x1U;
}
struct semstat : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace semstat_fields {
using semstat = core::field_descriptor<semstat, 0U, 2U>;
inline constexpr std::uint32_t semstat_free = 0x0U;
inline constexpr std::uint32_t semstat_cpu = 0x1U;
inline constexpr std::uint32_t semstat_spis = 0x2U;
inline constexpr std::uint32_t semstat_cpupending = 0x3U;
}
struct status : core::register_descriptor<std::uint32_t, 0x440U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace status_fields {
using overread = core::field_descriptor<status, 0U, 1U>;
inline constexpr std::uint32_t overread_notpresent = 0x0U;
inline constexpr std::uint32_t overread_present = 0x1U;
inline constexpr std::uint32_t overread_clear = 0x1U;
using overflow = core::field_descriptor<status, 1U, 1U>;
inline constexpr std::uint32_t overflow_notpresent = 0x0U;
inline constexpr std::uint32_t overflow_present = 0x1U;
inline constexpr std::uint32_t overflow_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x2U;
}
struct psel_sck : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sck_fields {
using pin = core::field_descriptor<psel_sck, 0U, 5U>;
using port = core::field_descriptor<psel_sck, 5U, 1U>;
using connect = core::field_descriptor<psel_sck, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_miso : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_miso_fields {
using pin = core::field_descriptor<psel_miso, 0U, 5U>;
using port = core::field_descriptor<psel_miso, 5U, 1U>;
using connect = core::field_descriptor<psel_miso, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_mosi : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_mosi_fields {
using pin = core::field_descriptor<psel_mosi, 0U, 5U>;
using port = core::field_descriptor<psel_mosi, 5U, 1U>;
using connect = core::field_descriptor<psel_mosi, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_csn : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_csn_fields {
using pin = core::field_descriptor<psel_csn, 0U, 5U>;
using port = core::field_descriptor<psel_csn, 5U, 1U>;
using connect = core::field_descriptor<psel_csn, 31U, 1U>;
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
struct config : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using order = core::field_descriptor<config, 0U, 1U>;
inline constexpr std::uint32_t order_msbfirst = 0x0U;
inline constexpr std::uint32_t order_lsbfirst = 0x1U;
using cpha = core::field_descriptor<config, 1U, 1U>;
inline constexpr std::uint32_t cpha_leading = 0x0U;
inline constexpr std::uint32_t cpha_trailing = 0x1U;
using cpol = core::field_descriptor<config, 2U, 1U>;
inline constexpr std::uint32_t cpol_activehigh = 0x0U;
inline constexpr std::uint32_t cpol_activelow = 0x1U;
}
struct def : core::register_descriptor<std::uint32_t, 0x55CU,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace def_fields {
using def = core::field_descriptor<def, 0U, 8U>;
}
struct orc : core::register_descriptor<std::uint32_t, 0x5C0U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace orc_fields {
using orc = core::field_descriptor<orc, 0U, 8U>;
}
}
