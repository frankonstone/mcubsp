#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::qdec {
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
struct tasks_readclracc : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_readclracc_fields {
using tasks_readclracc = core::field_descriptor<tasks_readclracc, 0U, 1U>;
inline constexpr std::uint32_t tasks_readclracc_trigger = 0x1U;
}
struct tasks_rdclracc : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_rdclracc_fields {
using tasks_rdclracc = core::field_descriptor<tasks_rdclracc, 0U, 1U>;
inline constexpr std::uint32_t tasks_rdclracc_trigger = 0x1U;
}
struct tasks_rdclrdbl : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_rdclrdbl_fields {
using tasks_rdclrdbl = core::field_descriptor<tasks_rdclrdbl, 0U, 1U>;
inline constexpr std::uint32_t tasks_rdclrdbl_trigger = 0x1U;
}
struct events_samplerdy : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_samplerdy_fields {
using events_samplerdy = core::field_descriptor<events_samplerdy, 0U, 1U>;
inline constexpr std::uint32_t events_samplerdy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_samplerdy_generated = 0x1U;
}
struct events_reportrdy : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_reportrdy_fields {
using events_reportrdy = core::field_descriptor<events_reportrdy, 0U, 1U>;
inline constexpr std::uint32_t events_reportrdy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_reportrdy_generated = 0x1U;
}
struct events_accof : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_accof_fields {
using events_accof = core::field_descriptor<events_accof, 0U, 1U>;
inline constexpr std::uint32_t events_accof_notgenerated = 0x0U;
inline constexpr std::uint32_t events_accof_generated = 0x1U;
}
struct events_dblrdy : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_dblrdy_fields {
using events_dblrdy = core::field_descriptor<events_dblrdy, 0U, 1U>;
inline constexpr std::uint32_t events_dblrdy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_dblrdy_generated = 0x1U;
}
struct events_stopped : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_stopped_fields {
using events_stopped = core::field_descriptor<events_stopped, 0U, 1U>;
inline constexpr std::uint32_t events_stopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_stopped_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using reportrdy_readclracc = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t reportrdy_readclracc_disabled = 0x0U;
inline constexpr std::uint32_t reportrdy_readclracc_enabled = 0x1U;
using samplerdy_stop = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t samplerdy_stop_disabled = 0x0U;
inline constexpr std::uint32_t samplerdy_stop_enabled = 0x1U;
using reportrdy_rdclracc = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t reportrdy_rdclracc_disabled = 0x0U;
inline constexpr std::uint32_t reportrdy_rdclracc_enabled = 0x1U;
using reportrdy_stop = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t reportrdy_stop_disabled = 0x0U;
inline constexpr std::uint32_t reportrdy_stop_enabled = 0x1U;
using dblrdy_rdclrdbl = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t dblrdy_rdclrdbl_disabled = 0x0U;
inline constexpr std::uint32_t dblrdy_rdclrdbl_enabled = 0x1U;
using dblrdy_stop = core::field_descriptor<shorts, 5U, 1U>;
inline constexpr std::uint32_t dblrdy_stop_disabled = 0x0U;
inline constexpr std::uint32_t dblrdy_stop_enabled = 0x1U;
using samplerdy_readclracc = core::field_descriptor<shorts, 6U, 1U>;
inline constexpr std::uint32_t samplerdy_readclracc_disabled = 0x0U;
inline constexpr std::uint32_t samplerdy_readclracc_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using samplerdy = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t samplerdy_disabled = 0x0U;
inline constexpr std::uint32_t samplerdy_enabled = 0x1U;
inline constexpr std::uint32_t samplerdy_set = 0x1U;
using reportrdy = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t reportrdy_disabled = 0x0U;
inline constexpr std::uint32_t reportrdy_enabled = 0x1U;
inline constexpr std::uint32_t reportrdy_set = 0x1U;
using accof = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t accof_disabled = 0x0U;
inline constexpr std::uint32_t accof_enabled = 0x1U;
inline constexpr std::uint32_t accof_set = 0x1U;
using dblrdy = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t dblrdy_disabled = 0x0U;
inline constexpr std::uint32_t dblrdy_enabled = 0x1U;
inline constexpr std::uint32_t dblrdy_set = 0x1U;
using stopped = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using samplerdy = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t samplerdy_disabled = 0x0U;
inline constexpr std::uint32_t samplerdy_enabled = 0x1U;
inline constexpr std::uint32_t samplerdy_clear = 0x1U;
using reportrdy = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t reportrdy_disabled = 0x0U;
inline constexpr std::uint32_t reportrdy_enabled = 0x1U;
inline constexpr std::uint32_t reportrdy_clear = 0x1U;
using accof = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t accof_disabled = 0x0U;
inline constexpr std::uint32_t accof_enabled = 0x1U;
inline constexpr std::uint32_t accof_clear = 0x1U;
using dblrdy = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t dblrdy_disabled = 0x0U;
inline constexpr std::uint32_t dblrdy_enabled = 0x1U;
inline constexpr std::uint32_t dblrdy_clear = 0x1U;
using stopped = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct ledpol : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ledpol_fields {
using ledpol = core::field_descriptor<ledpol, 0U, 1U>;
inline constexpr std::uint32_t ledpol_activelow = 0x0U;
inline constexpr std::uint32_t ledpol_activehigh = 0x1U;
}
struct sampleper : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sampleper_fields {
using sampleper = core::field_descriptor<sampleper, 0U, 4U>;
inline constexpr std::uint32_t sampleper_reg_128us = 0x0U;
inline constexpr std::uint32_t sampleper_reg_256us = 0x1U;
inline constexpr std::uint32_t sampleper_reg_512us = 0x2U;
inline constexpr std::uint32_t sampleper_reg_1024us = 0x3U;
inline constexpr std::uint32_t sampleper_reg_2048us = 0x4U;
inline constexpr std::uint32_t sampleper_reg_4096us = 0x5U;
inline constexpr std::uint32_t sampleper_reg_8192us = 0x6U;
inline constexpr std::uint32_t sampleper_reg_16384us = 0x7U;
inline constexpr std::uint32_t sampleper_reg_32ms = 0x8U;
inline constexpr std::uint32_t sampleper_reg_65ms = 0x9U;
inline constexpr std::uint32_t sampleper_reg_131ms = 0xAU;
}
struct sample : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sample_fields {
using sample = core::field_descriptor<sample, 0U, 32U>;
}
struct reportper : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace reportper_fields {
using reportper = core::field_descriptor<reportper, 0U, 4U>;
inline constexpr std::uint32_t reportper_reg_10smpl = 0x0U;
inline constexpr std::uint32_t reportper_reg_40smpl = 0x1U;
inline constexpr std::uint32_t reportper_reg_80smpl = 0x2U;
inline constexpr std::uint32_t reportper_reg_120smpl = 0x3U;
inline constexpr std::uint32_t reportper_reg_160smpl = 0x4U;
inline constexpr std::uint32_t reportper_reg_200smpl = 0x5U;
inline constexpr std::uint32_t reportper_reg_240smpl = 0x6U;
inline constexpr std::uint32_t reportper_reg_280smpl = 0x7U;
inline constexpr std::uint32_t reportper_reg_1smpl = 0x8U;
}
struct acc : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace acc_fields {
using acc = core::field_descriptor<acc, 0U, 32U>;
}
struct accread : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace accread_fields {
using accread = core::field_descriptor<accread, 0U, 32U>;
}
struct psel_led : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_led_fields {
using pin = core::field_descriptor<psel_led, 0U, 5U>;
using port = core::field_descriptor<psel_led, 5U, 1U>;
using connect = core::field_descriptor<psel_led, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_a : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_a_fields {
using pin = core::field_descriptor<psel_a, 0U, 5U>;
using port = core::field_descriptor<psel_a, 5U, 1U>;
using connect = core::field_descriptor<psel_a, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_b : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_b_fields {
using pin = core::field_descriptor<psel_b, 0U, 5U>;
using port = core::field_descriptor<psel_b, 5U, 1U>;
using connect = core::field_descriptor<psel_b, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct dbfen : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dbfen_fields {
using dbfen = core::field_descriptor<dbfen, 0U, 1U>;
inline constexpr std::uint32_t dbfen_disabled = 0x0U;
inline constexpr std::uint32_t dbfen_enabled = 0x1U;
}
struct ledpre : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x000001FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000010U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ledpre_fields {
using ledpre = core::field_descriptor<ledpre, 0U, 9U>;
}
struct accdbl : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace accdbl_fields {
using accdbl = core::field_descriptor<accdbl, 0U, 4U>;
}
struct accdblread : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace accdblread_fields {
using accdblread = core::field_descriptor<accdblread, 0U, 4U>;
}
}
