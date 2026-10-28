#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::qspi {
struct tasks_activate : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_activate_fields {
using tasks_activate = core::field_descriptor<tasks_activate, 0U, 1U>;
inline constexpr std::uint32_t tasks_activate_trigger = 0x1U;
}
struct tasks_readstart : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_readstart_fields {
using tasks_readstart = core::field_descriptor<tasks_readstart, 0U, 1U>;
inline constexpr std::uint32_t tasks_readstart_trigger = 0x1U;
}
struct tasks_writestart : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_writestart_fields {
using tasks_writestart = core::field_descriptor<tasks_writestart, 0U, 1U>;
inline constexpr std::uint32_t tasks_writestart_trigger = 0x1U;
}
struct tasks_erasestart : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_erasestart_fields {
using tasks_erasestart = core::field_descriptor<tasks_erasestart, 0U, 1U>;
inline constexpr std::uint32_t tasks_erasestart_trigger = 0x1U;
}
struct tasks_deactivate : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_deactivate_fields {
using tasks_deactivate = core::field_descriptor<tasks_deactivate, 0U, 1U>;
inline constexpr std::uint32_t tasks_deactivate_trigger = 0x1U;
}
struct events_ready : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ready_fields {
using events_ready = core::field_descriptor<events_ready, 0U, 1U>;
inline constexpr std::uint32_t events_ready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ready_generated = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using ready = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using ready = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using ready = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct read_src : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace read_src_fields {
using src = core::field_descriptor<read_src, 0U, 32U>;
}
struct read_dst : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace read_dst_fields {
using dst = core::field_descriptor<read_dst, 0U, 32U>;
}
struct read_cnt : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x0003FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace read_cnt_fields {
using cnt = core::field_descriptor<read_cnt, 0U, 18U>;
}
struct write_dst : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace write_dst_fields {
using dst = core::field_descriptor<write_dst, 0U, 32U>;
}
struct write_src : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace write_src_fields {
using src = core::field_descriptor<write_src, 0U, 32U>;
}
struct write_cnt : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x0003FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace write_cnt_fields {
using cnt = core::field_descriptor<write_cnt, 0U, 18U>;
}
struct erase_ptr : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erase_ptr_fields {
using ptr = core::field_descriptor<erase_ptr, 0U, 32U>;
}
struct erase_len : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erase_len_fields {
using len = core::field_descriptor<erase_len, 0U, 2U>;
inline constexpr std::uint32_t len_reg_4kb = 0x0U;
inline constexpr std::uint32_t len_reg_64kb = 0x1U;
inline constexpr std::uint32_t len_all = 0x2U;
}
struct psel_sck : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sck_fields {
using pin = core::field_descriptor<psel_sck, 0U, 5U>;
using port = core::field_descriptor<psel_sck, 5U, 1U>;
using connect = core::field_descriptor<psel_sck, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_csn : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_csn_fields {
using pin = core::field_descriptor<psel_csn, 0U, 5U>;
using port = core::field_descriptor<psel_csn, 5U, 1U>;
using connect = core::field_descriptor<psel_csn, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_io0 : core::register_descriptor<std::uint32_t, 0x530U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_io0_fields {
using pin = core::field_descriptor<psel_io0, 0U, 5U>;
using port = core::field_descriptor<psel_io0, 5U, 1U>;
using connect = core::field_descriptor<psel_io0, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_io1 : core::register_descriptor<std::uint32_t, 0x534U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_io1_fields {
using pin = core::field_descriptor<psel_io1, 0U, 5U>;
using port = core::field_descriptor<psel_io1, 5U, 1U>;
using connect = core::field_descriptor<psel_io1, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_io2 : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_io2_fields {
using pin = core::field_descriptor<psel_io2, 0U, 5U>;
using port = core::field_descriptor<psel_io2, 5U, 1U>;
using connect = core::field_descriptor<psel_io2, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_io3 : core::register_descriptor<std::uint32_t, 0x53CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_io3_fields {
using pin = core::field_descriptor<psel_io3, 0U, 5U>;
using port = core::field_descriptor<psel_io3, 5U, 1U>;
using connect = core::field_descriptor<psel_io3, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct xipoffset : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace xipoffset_fields {
using xipoffset = core::field_descriptor<xipoffset, 0U, 32U>;
}
struct ifconfig0 : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x000010FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ifconfig0_fields {
using readoc = core::field_descriptor<ifconfig0, 0U, 3U>;
inline constexpr std::uint32_t readoc_fastread = 0x0U;
inline constexpr std::uint32_t readoc_read2o = 0x1U;
inline constexpr std::uint32_t readoc_read2io = 0x2U;
inline constexpr std::uint32_t readoc_read4o = 0x3U;
inline constexpr std::uint32_t readoc_read4io = 0x4U;
using writeoc = core::field_descriptor<ifconfig0, 3U, 3U>;
inline constexpr std::uint32_t writeoc_pp = 0x0U;
inline constexpr std::uint32_t writeoc_pp2o = 0x1U;
inline constexpr std::uint32_t writeoc_pp4o = 0x2U;
inline constexpr std::uint32_t writeoc_pp4io = 0x3U;
using addrmode = core::field_descriptor<ifconfig0, 6U, 1U>;
inline constexpr std::uint32_t addrmode_reg_24bit = 0x0U;
inline constexpr std::uint32_t addrmode_reg_32bit = 0x1U;
using dpmenable = core::field_descriptor<ifconfig0, 7U, 1U>;
inline constexpr std::uint32_t dpmenable_disable = 0x0U;
inline constexpr std::uint32_t dpmenable_enable = 0x1U;
using ppsize = core::field_descriptor<ifconfig0, 12U, 1U>;
inline constexpr std::uint32_t ppsize_reg_256bytes = 0x0U;
inline constexpr std::uint32_t ppsize_reg_512bytes = 0x1U;
}
struct ifconfig1 : core::register_descriptor<std::uint32_t, 0x600U,
    core::access_semantics::read_write, 0xF30000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00040480U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ifconfig1_fields {
using sckdelay = core::field_descriptor<ifconfig1, 0U, 8U>;
using dpmen = core::field_descriptor<ifconfig1, 24U, 1U>;
inline constexpr std::uint32_t dpmen_exit = 0x0U;
inline constexpr std::uint32_t dpmen_enter = 0x1U;
using spimode = core::field_descriptor<ifconfig1, 25U, 1U>;
inline constexpr std::uint32_t spimode_mode0 = 0x0U;
inline constexpr std::uint32_t spimode_mode3 = 0x1U;
using sckfreq = core::field_descriptor<ifconfig1, 28U, 4U>;
}
struct status : core::register_descriptor<std::uint32_t, 0x604U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace status_fields {
using dpm = core::field_descriptor<status, 2U, 1U>;
inline constexpr std::uint32_t dpm_disabled = 0x0U;
inline constexpr std::uint32_t dpm_enabled = 0x1U;
using ready = core::field_descriptor<status, 3U, 1U>;
inline constexpr std::uint32_t ready_ready = 0x1U;
inline constexpr std::uint32_t ready_busy = 0x0U;
using sreg = core::field_descriptor<status, 24U, 8U>;
}
struct dpmdur : core::register_descriptor<std::uint32_t, 0x614U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dpmdur_fields {
using enter = core::field_descriptor<dpmdur, 0U, 16U>;
using exit = core::field_descriptor<dpmdur, 16U, 16U>;
}
struct addrconf : core::register_descriptor<std::uint32_t, 0x624U,
    core::access_semantics::read_write, 0x0FFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000000B7U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace addrconf_fields {
using opcode = core::field_descriptor<addrconf, 0U, 8U>;
using byte0 = core::field_descriptor<addrconf, 8U, 8U>;
using byte1 = core::field_descriptor<addrconf, 16U, 8U>;
using mode = core::field_descriptor<addrconf, 24U, 2U>;
inline constexpr std::uint32_t mode_noinstr = 0x0U;
inline constexpr std::uint32_t mode_opcode = 0x1U;
inline constexpr std::uint32_t mode_opbyte0 = 0x2U;
inline constexpr std::uint32_t mode_all = 0x3U;
using wipwait = core::field_descriptor<addrconf, 26U, 1U>;
inline constexpr std::uint32_t wipwait_disable = 0x0U;
inline constexpr std::uint32_t wipwait_enable = 0x1U;
using wren = core::field_descriptor<addrconf, 27U, 1U>;
inline constexpr std::uint32_t wren_disable = 0x0U;
inline constexpr std::uint32_t wren_enable = 0x1U;
}
struct cinstrconf : core::register_descriptor<std::uint32_t, 0x634U,
    core::access_semantics::read_write, 0x0003FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00002000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cinstrconf_fields {
using opcode = core::field_descriptor<cinstrconf, 0U, 8U>;
using length = core::field_descriptor<cinstrconf, 8U, 4U>;
inline constexpr std::uint32_t length_reg_1b = 0x1U;
inline constexpr std::uint32_t length_reg_2b = 0x2U;
inline constexpr std::uint32_t length_reg_3b = 0x3U;
inline constexpr std::uint32_t length_reg_4b = 0x4U;
inline constexpr std::uint32_t length_reg_5b = 0x5U;
inline constexpr std::uint32_t length_reg_6b = 0x6U;
inline constexpr std::uint32_t length_reg_7b = 0x7U;
inline constexpr std::uint32_t length_reg_8b = 0x8U;
inline constexpr std::uint32_t length_reg_9b = 0x9U;
using lio2 = core::field_descriptor<cinstrconf, 12U, 1U>;
using lio3 = core::field_descriptor<cinstrconf, 13U, 1U>;
using wipwait = core::field_descriptor<cinstrconf, 14U, 1U>;
inline constexpr std::uint32_t wipwait_disable = 0x0U;
inline constexpr std::uint32_t wipwait_enable = 0x1U;
using wren = core::field_descriptor<cinstrconf, 15U, 1U>;
inline constexpr std::uint32_t wren_disable = 0x0U;
inline constexpr std::uint32_t wren_enable = 0x1U;
using lfen = core::field_descriptor<cinstrconf, 16U, 1U>;
inline constexpr std::uint32_t lfen_disable = 0x0U;
inline constexpr std::uint32_t lfen_enable = 0x1U;
using lfstop = core::field_descriptor<cinstrconf, 17U, 1U>;
inline constexpr std::uint32_t lfstop_stop = 0x1U;
}
struct cinstrdat0 : core::register_descriptor<std::uint32_t, 0x638U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cinstrdat0_fields {
using byte0 = core::field_descriptor<cinstrdat0, 0U, 8U>;
using byte1 = core::field_descriptor<cinstrdat0, 8U, 8U>;
using byte2 = core::field_descriptor<cinstrdat0, 16U, 8U>;
using byte3 = core::field_descriptor<cinstrdat0, 24U, 8U>;
}
struct cinstrdat1 : core::register_descriptor<std::uint32_t, 0x63CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cinstrdat1_fields {
using byte4 = core::field_descriptor<cinstrdat1, 0U, 8U>;
using byte5 = core::field_descriptor<cinstrdat1, 8U, 8U>;
using byte6 = core::field_descriptor<cinstrdat1, 16U, 8U>;
using byte7 = core::field_descriptor<cinstrdat1, 24U, 8U>;
}
struct iftiming : core::register_descriptor<std::uint32_t, 0x640U,
    core::access_semantics::read_write, 0x00000700U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000200U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace iftiming_fields {
using rxdelay = core::field_descriptor<iftiming, 8U, 3U>;
}
}
