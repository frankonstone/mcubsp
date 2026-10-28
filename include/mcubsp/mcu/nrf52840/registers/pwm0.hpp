#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::pwm0 {
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct tasks_seqstart0 : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_seqstart0_fields {
using tasks_seqstart = core::field_descriptor<tasks_seqstart0, 0U, 1U>;
inline constexpr std::uint32_t tasks_seqstart_trigger = 0x1U;
}
struct tasks_seqstart1 : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_seqstart1_fields {
using tasks_seqstart = core::field_descriptor<tasks_seqstart1, 0U, 1U>;
inline constexpr std::uint32_t tasks_seqstart_trigger = 0x1U;
}
struct tasks_nextstep : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_nextstep_fields {
using tasks_nextstep = core::field_descriptor<tasks_nextstep, 0U, 1U>;
inline constexpr std::uint32_t tasks_nextstep_trigger = 0x1U;
}
struct events_stopped : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_stopped_fields {
using events_stopped = core::field_descriptor<events_stopped, 0U, 1U>;
inline constexpr std::uint32_t events_stopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_stopped_generated = 0x1U;
}
struct events_seqstarted0 : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_seqstarted0_fields {
using events_seqstarted = core::field_descriptor<events_seqstarted0, 0U, 1U>;
inline constexpr std::uint32_t events_seqstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_seqstarted_generated = 0x1U;
}
struct events_seqstarted1 : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_seqstarted1_fields {
using events_seqstarted = core::field_descriptor<events_seqstarted1, 0U, 1U>;
inline constexpr std::uint32_t events_seqstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_seqstarted_generated = 0x1U;
}
struct events_seqend0 : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_seqend0_fields {
using events_seqend = core::field_descriptor<events_seqend0, 0U, 1U>;
inline constexpr std::uint32_t events_seqend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_seqend_generated = 0x1U;
}
struct events_seqend1 : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_seqend1_fields {
using events_seqend = core::field_descriptor<events_seqend1, 0U, 1U>;
inline constexpr std::uint32_t events_seqend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_seqend_generated = 0x1U;
}
struct events_pwmperiodend : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_pwmperiodend_fields {
using events_pwmperiodend = core::field_descriptor<events_pwmperiodend, 0U, 1U>;
inline constexpr std::uint32_t events_pwmperiodend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_pwmperiodend_generated = 0x1U;
}
struct events_loopsdone : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_loopsdone_fields {
using events_loopsdone = core::field_descriptor<events_loopsdone, 0U, 1U>;
inline constexpr std::uint32_t events_loopsdone_notgenerated = 0x0U;
inline constexpr std::uint32_t events_loopsdone_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using seqend0_stop = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t seqend0_stop_disabled = 0x0U;
inline constexpr std::uint32_t seqend0_stop_enabled = 0x1U;
using seqend1_stop = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t seqend1_stop_disabled = 0x0U;
inline constexpr std::uint32_t seqend1_stop_enabled = 0x1U;
using loopsdone_seqstart0 = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t loopsdone_seqstart0_disabled = 0x0U;
inline constexpr std::uint32_t loopsdone_seqstart0_enabled = 0x1U;
using loopsdone_seqstart1 = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t loopsdone_seqstart1_disabled = 0x0U;
inline constexpr std::uint32_t loopsdone_seqstart1_enabled = 0x1U;
using loopsdone_stop = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t loopsdone_stop_disabled = 0x0U;
inline constexpr std::uint32_t loopsdone_stop_enabled = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x000000FEU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using stopped = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
using seqstarted0 = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t seqstarted0_disabled = 0x0U;
inline constexpr std::uint32_t seqstarted0_enabled = 0x1U;
using seqstarted1 = core::field_descriptor<inten, 3U, 1U>;
inline constexpr std::uint32_t seqstarted1_disabled = 0x0U;
inline constexpr std::uint32_t seqstarted1_enabled = 0x1U;
using seqend0 = core::field_descriptor<inten, 4U, 1U>;
inline constexpr std::uint32_t seqend0_disabled = 0x0U;
inline constexpr std::uint32_t seqend0_enabled = 0x1U;
using seqend1 = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t seqend1_disabled = 0x0U;
inline constexpr std::uint32_t seqend1_enabled = 0x1U;
using pwmperiodend = core::field_descriptor<inten, 6U, 1U>;
inline constexpr std::uint32_t pwmperiodend_disabled = 0x0U;
inline constexpr std::uint32_t pwmperiodend_enabled = 0x1U;
using loopsdone = core::field_descriptor<inten, 7U, 1U>;
inline constexpr std::uint32_t loopsdone_disabled = 0x0U;
inline constexpr std::uint32_t loopsdone_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x000000FEU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using stopped = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using seqstarted0 = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t seqstarted0_disabled = 0x0U;
inline constexpr std::uint32_t seqstarted0_enabled = 0x1U;
inline constexpr std::uint32_t seqstarted0_set = 0x1U;
using seqstarted1 = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t seqstarted1_disabled = 0x0U;
inline constexpr std::uint32_t seqstarted1_enabled = 0x1U;
inline constexpr std::uint32_t seqstarted1_set = 0x1U;
using seqend0 = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t seqend0_disabled = 0x0U;
inline constexpr std::uint32_t seqend0_enabled = 0x1U;
inline constexpr std::uint32_t seqend0_set = 0x1U;
using seqend1 = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t seqend1_disabled = 0x0U;
inline constexpr std::uint32_t seqend1_enabled = 0x1U;
inline constexpr std::uint32_t seqend1_set = 0x1U;
using pwmperiodend = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t pwmperiodend_disabled = 0x0U;
inline constexpr std::uint32_t pwmperiodend_enabled = 0x1U;
inline constexpr std::uint32_t pwmperiodend_set = 0x1U;
using loopsdone = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t loopsdone_disabled = 0x0U;
inline constexpr std::uint32_t loopsdone_enabled = 0x1U;
inline constexpr std::uint32_t loopsdone_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x000000FEU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using stopped = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using seqstarted0 = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t seqstarted0_disabled = 0x0U;
inline constexpr std::uint32_t seqstarted0_enabled = 0x1U;
inline constexpr std::uint32_t seqstarted0_clear = 0x1U;
using seqstarted1 = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t seqstarted1_disabled = 0x0U;
inline constexpr std::uint32_t seqstarted1_enabled = 0x1U;
inline constexpr std::uint32_t seqstarted1_clear = 0x1U;
using seqend0 = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t seqend0_disabled = 0x0U;
inline constexpr std::uint32_t seqend0_enabled = 0x1U;
inline constexpr std::uint32_t seqend0_clear = 0x1U;
using seqend1 = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t seqend1_disabled = 0x0U;
inline constexpr std::uint32_t seqend1_enabled = 0x1U;
inline constexpr std::uint32_t seqend1_clear = 0x1U;
using pwmperiodend = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t pwmperiodend_disabled = 0x0U;
inline constexpr std::uint32_t pwmperiodend_enabled = 0x1U;
inline constexpr std::uint32_t pwmperiodend_clear = 0x1U;
using loopsdone = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t loopsdone_disabled = 0x0U;
inline constexpr std::uint32_t loopsdone_enabled = 0x1U;
inline constexpr std::uint32_t loopsdone_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct mode : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mode_fields {
using updown = core::field_descriptor<mode, 0U, 1U>;
inline constexpr std::uint32_t updown_up = 0x0U;
inline constexpr std::uint32_t updown_upanddown = 0x1U;
}
struct countertop : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x00007FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000003FFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace countertop_fields {
using countertop = core::field_descriptor<countertop, 0U, 15U>;
}
struct prescaler : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prescaler_fields {
using prescaler = core::field_descriptor<prescaler, 0U, 3U>;
inline constexpr std::uint32_t prescaler_div_1 = 0x0U;
inline constexpr std::uint32_t prescaler_div_2 = 0x1U;
inline constexpr std::uint32_t prescaler_div_4 = 0x2U;
inline constexpr std::uint32_t prescaler_div_8 = 0x3U;
inline constexpr std::uint32_t prescaler_div_16 = 0x4U;
inline constexpr std::uint32_t prescaler_div_32 = 0x5U;
inline constexpr std::uint32_t prescaler_div_64 = 0x6U;
inline constexpr std::uint32_t prescaler_div_128 = 0x7U;
}
struct decoder : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x00000103U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace decoder_fields {
using load = core::field_descriptor<decoder, 0U, 2U>;
inline constexpr std::uint32_t load_common = 0x0U;
inline constexpr std::uint32_t load_grouped = 0x1U;
inline constexpr std::uint32_t load_individual = 0x2U;
inline constexpr std::uint32_t load_waveform = 0x3U;
using mode = core::field_descriptor<decoder, 8U, 1U>;
inline constexpr std::uint32_t mode_refreshcount = 0x0U;
inline constexpr std::uint32_t mode_nextstep = 0x1U;
}
struct loop : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace loop_fields {
using cnt = core::field_descriptor<loop, 0U, 16U>;
inline constexpr std::uint32_t cnt_disabled = 0x0U;
}
struct seq0_ptr : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq0_ptr_fields {
using ptr = core::field_descriptor<seq0_ptr, 0U, 32U>;
}
struct seq0_cnt : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x00007FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq0_cnt_fields {
using cnt = core::field_descriptor<seq0_cnt, 0U, 15U>;
inline constexpr std::uint32_t cnt_disabled = 0x0U;
}
struct seq0_refresh : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq0_refresh_fields {
using cnt = core::field_descriptor<seq0_refresh, 0U, 24U>;
inline constexpr std::uint32_t cnt_continuous = 0x0U;
}
struct seq0_enddelay : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq0_enddelay_fields {
using cnt = core::field_descriptor<seq0_enddelay, 0U, 24U>;
}
struct seq1_ptr : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq1_ptr_fields {
using ptr = core::field_descriptor<seq1_ptr, 0U, 32U>;
}
struct seq1_cnt : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x00007FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq1_cnt_fields {
using cnt = core::field_descriptor<seq1_cnt, 0U, 15U>;
inline constexpr std::uint32_t cnt_disabled = 0x0U;
}
struct seq1_refresh : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq1_refresh_fields {
using cnt = core::field_descriptor<seq1_refresh, 0U, 24U>;
inline constexpr std::uint32_t cnt_continuous = 0x0U;
}
struct seq1_enddelay : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace seq1_enddelay_fields {
using cnt = core::field_descriptor<seq1_enddelay, 0U, 24U>;
}
struct psel_out0 : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_out0_fields {
using pin = core::field_descriptor<psel_out0, 0U, 5U>;
using port = core::field_descriptor<psel_out0, 5U, 1U>;
using connect = core::field_descriptor<psel_out0, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_out1 : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_out1_fields {
using pin = core::field_descriptor<psel_out1, 0U, 5U>;
using port = core::field_descriptor<psel_out1, 5U, 1U>;
using connect = core::field_descriptor<psel_out1, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_out2 : core::register_descriptor<std::uint32_t, 0x568U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_out2_fields {
using pin = core::field_descriptor<psel_out2, 0U, 5U>;
using port = core::field_descriptor<psel_out2, 5U, 1U>;
using connect = core::field_descriptor<psel_out2, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_out3 : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_out3_fields {
using pin = core::field_descriptor<psel_out3, 0U, 5U>;
using port = core::field_descriptor<psel_out3, 5U, 1U>;
using connect = core::field_descriptor<psel_out3, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
}
