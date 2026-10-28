#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::spim0 {
struct tasks_start : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
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
struct events_endrx : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endrx_fields {
using events_endrx = core::field_descriptor<events_endrx, 0U, 1U>;
inline constexpr std::uint32_t events_endrx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endrx_generated = 0x1U;
}
struct events_end : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_end_fields {
using events_end = core::field_descriptor<events_end, 0U, 1U>;
inline constexpr std::uint32_t events_end_notgenerated = 0x0U;
inline constexpr std::uint32_t events_end_generated = 0x1U;
}
struct events_endtx : core::register_descriptor<std::uint32_t, 0x120U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endtx_fields {
using events_endtx = core::field_descriptor<events_endtx, 0U, 1U>;
inline constexpr std::uint32_t events_endtx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endtx_generated = 0x1U;
}
struct events_started : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_started_fields {
using events_started = core::field_descriptor<events_started, 0U, 1U>;
inline constexpr std::uint32_t events_started_notgenerated = 0x0U;
inline constexpr std::uint32_t events_started_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00020000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using end_start = core::field_descriptor<shorts, 17U, 1U>;
inline constexpr std::uint32_t end_start_disabled = 0x0U;
inline constexpr std::uint32_t end_start_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00080152U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using stopped = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using endrx = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
inline constexpr std::uint32_t endrx_set = 0x1U;
using end = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_set = 0x1U;
using endtx = core::field_descriptor<intenset, 8U, 1U>;
inline constexpr std::uint32_t endtx_disabled = 0x0U;
inline constexpr std::uint32_t endtx_enabled = 0x1U;
inline constexpr std::uint32_t endtx_set = 0x1U;
using started = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00080152U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using stopped = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using endrx = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
inline constexpr std::uint32_t endrx_clear = 0x1U;
using end = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_clear = 0x1U;
using endtx = core::field_descriptor<intenclr, 8U, 1U>;
inline constexpr std::uint32_t endtx_disabled = 0x0U;
inline constexpr std::uint32_t endtx_enabled = 0x1U;
inline constexpr std::uint32_t endtx_clear = 0x1U;
using started = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_clear = 0x1U;
}
struct stallstat : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace stallstat_fields {
using tx = core::field_descriptor<stallstat, 0U, 1U>;
inline constexpr std::uint32_t tx_nostall = 0x0U;
inline constexpr std::uint32_t tx_stall = 0x1U;
using rx = core::field_descriptor<stallstat, 1U, 1U>;
inline constexpr std::uint32_t rx_nostall = 0x0U;
inline constexpr std::uint32_t rx_stall = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x7U;
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
struct psel_mosi : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_mosi_fields {
using pin = core::field_descriptor<psel_mosi, 0U, 5U>;
using port = core::field_descriptor<psel_mosi, 5U, 1U>;
using connect = core::field_descriptor<psel_mosi, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_miso : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_miso_fields {
using pin = core::field_descriptor<psel_miso, 0U, 5U>;
using port = core::field_descriptor<psel_miso, 5U, 1U>;
using connect = core::field_descriptor<psel_miso, 31U, 1U>;
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
struct frequency : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x04000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace frequency_fields {
using frequency = core::field_descriptor<frequency, 0U, 32U>;
inline constexpr std::uint32_t frequency_k125 = 0x2000000U;
inline constexpr std::uint32_t frequency_k250 = 0x4000000U;
inline constexpr std::uint32_t frequency_k500 = 0x8000000U;
inline constexpr std::uint32_t frequency_m1 = 0x10000000U;
inline constexpr std::uint32_t frequency_m2 = 0x20000000U;
inline constexpr std::uint32_t frequency_m4 = 0x40000000U;
inline constexpr std::uint32_t frequency_m8 = 0x80000000U;
inline constexpr std::uint32_t frequency_m16 = 0xA000000U;
inline constexpr std::uint32_t frequency_m32 = 0x14000000U;
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
struct iftiming_rxdelay : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000002U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace iftiming_rxdelay_fields {
using rxdelay = core::field_descriptor<iftiming_rxdelay, 0U, 3U>;
}
struct iftiming_csndur : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000002U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace iftiming_csndur_fields {
using csndur = core::field_descriptor<iftiming_csndur, 0U, 8U>;
}
struct csnpol : core::register_descriptor<std::uint32_t, 0x568U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace csnpol_fields {
using csnpol = core::field_descriptor<csnpol, 0U, 1U>;
inline constexpr std::uint32_t csnpol_low = 0x0U;
inline constexpr std::uint32_t csnpol_high = 0x1U;
}
struct pseldcx : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pseldcx_fields {
using pin = core::field_descriptor<pseldcx, 0U, 5U>;
using port = core::field_descriptor<pseldcx, 5U, 1U>;
using connect = core::field_descriptor<pseldcx, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct dcxcnt : core::register_descriptor<std::uint32_t, 0x570U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dcxcnt_fields {
using dcxcnt = core::field_descriptor<dcxcnt, 0U, 4U>;
}
struct orc : core::register_descriptor<std::uint32_t, 0x5C0U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace orc_fields {
using orc = core::field_descriptor<orc, 0U, 8U>;
}
}
