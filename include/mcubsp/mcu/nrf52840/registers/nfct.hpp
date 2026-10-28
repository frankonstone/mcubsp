#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::nfct {
struct tasks_activate : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_activate_fields {
using tasks_activate = core::field_descriptor<tasks_activate, 0U, 1U>;
inline constexpr std::uint32_t tasks_activate_trigger = 0x1U;
}
struct tasks_disable : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_disable_fields {
using tasks_disable = core::field_descriptor<tasks_disable, 0U, 1U>;
inline constexpr std::uint32_t tasks_disable_trigger = 0x1U;
}
struct tasks_sense : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_sense_fields {
using tasks_sense = core::field_descriptor<tasks_sense, 0U, 1U>;
inline constexpr std::uint32_t tasks_sense_trigger = 0x1U;
}
struct tasks_starttx : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_starttx_fields {
using tasks_starttx = core::field_descriptor<tasks_starttx, 0U, 1U>;
inline constexpr std::uint32_t tasks_starttx_trigger = 0x1U;
}
struct tasks_enablerxdata : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_enablerxdata_fields {
using tasks_enablerxdata = core::field_descriptor<tasks_enablerxdata, 0U, 1U>;
inline constexpr std::uint32_t tasks_enablerxdata_trigger = 0x1U;
}
struct tasks_goidle : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_goidle_fields {
using tasks_goidle = core::field_descriptor<tasks_goidle, 0U, 1U>;
inline constexpr std::uint32_t tasks_goidle_trigger = 0x1U;
}
struct tasks_gosleep : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_gosleep_fields {
using tasks_gosleep = core::field_descriptor<tasks_gosleep, 0U, 1U>;
inline constexpr std::uint32_t tasks_gosleep_trigger = 0x1U;
}
struct events_ready : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ready_fields {
using events_ready = core::field_descriptor<events_ready, 0U, 1U>;
inline constexpr std::uint32_t events_ready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ready_generated = 0x1U;
}
struct events_fielddetected : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_fielddetected_fields {
using events_fielddetected = core::field_descriptor<events_fielddetected, 0U, 1U>;
inline constexpr std::uint32_t events_fielddetected_notgenerated = 0x0U;
inline constexpr std::uint32_t events_fielddetected_generated = 0x1U;
}
struct events_fieldlost : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_fieldlost_fields {
using events_fieldlost = core::field_descriptor<events_fieldlost, 0U, 1U>;
inline constexpr std::uint32_t events_fieldlost_notgenerated = 0x0U;
inline constexpr std::uint32_t events_fieldlost_generated = 0x1U;
}
struct events_txframestart : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txframestart_fields {
using events_txframestart = core::field_descriptor<events_txframestart, 0U, 1U>;
inline constexpr std::uint32_t events_txframestart_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txframestart_generated = 0x1U;
}
struct events_txframeend : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txframeend_fields {
using events_txframeend = core::field_descriptor<events_txframeend, 0U, 1U>;
inline constexpr std::uint32_t events_txframeend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txframeend_generated = 0x1U;
}
struct events_rxframestart : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxframestart_fields {
using events_rxframestart = core::field_descriptor<events_rxframestart, 0U, 1U>;
inline constexpr std::uint32_t events_rxframestart_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxframestart_generated = 0x1U;
}
struct events_rxframeend : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxframeend_fields {
using events_rxframeend = core::field_descriptor<events_rxframeend, 0U, 1U>;
inline constexpr std::uint32_t events_rxframeend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxframeend_generated = 0x1U;
}
struct events_error : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_error_fields {
using events_error = core::field_descriptor<events_error, 0U, 1U>;
inline constexpr std::uint32_t events_error_notgenerated = 0x0U;
inline constexpr std::uint32_t events_error_generated = 0x1U;
}
struct events_rxerror : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxerror_fields {
using events_rxerror = core::field_descriptor<events_rxerror, 0U, 1U>;
inline constexpr std::uint32_t events_rxerror_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxerror_generated = 0x1U;
}
struct events_endrx : core::register_descriptor<std::uint32_t, 0x12CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endrx_fields {
using events_endrx = core::field_descriptor<events_endrx, 0U, 1U>;
inline constexpr std::uint32_t events_endrx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endrx_generated = 0x1U;
}
struct events_endtx : core::register_descriptor<std::uint32_t, 0x130U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endtx_fields {
using events_endtx = core::field_descriptor<events_endtx, 0U, 1U>;
inline constexpr std::uint32_t events_endtx_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endtx_generated = 0x1U;
}
struct events_autocolresstarted : core::register_descriptor<std::uint32_t, 0x138U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_autocolresstarted_fields {
using events_autocolresstarted = core::field_descriptor<events_autocolresstarted, 0U, 1U>;
inline constexpr std::uint32_t events_autocolresstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_autocolresstarted_generated = 0x1U;
}
struct events_collision : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_collision_fields {
using events_collision = core::field_descriptor<events_collision, 0U, 1U>;
inline constexpr std::uint32_t events_collision_notgenerated = 0x0U;
inline constexpr std::uint32_t events_collision_generated = 0x1U;
}
struct events_selected : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_selected_fields {
using events_selected = core::field_descriptor<events_selected, 0U, 1U>;
inline constexpr std::uint32_t events_selected_notgenerated = 0x0U;
inline constexpr std::uint32_t events_selected_generated = 0x1U;
}
struct events_started : core::register_descriptor<std::uint32_t, 0x150U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_started_fields {
using events_started = core::field_descriptor<events_started, 0U, 1U>;
inline constexpr std::uint32_t events_started_notgenerated = 0x0U;
inline constexpr std::uint32_t events_started_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00000023U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using fielddetected_activate = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t fielddetected_activate_disabled = 0x0U;
inline constexpr std::uint32_t fielddetected_activate_enabled = 0x1U;
using fieldlost_sense = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t fieldlost_sense_disabled = 0x0U;
inline constexpr std::uint32_t fieldlost_sense_enabled = 0x1U;
using txframeend_enablerxdata = core::field_descriptor<shorts, 5U, 1U>;
inline constexpr std::uint32_t txframeend_enablerxdata_disabled = 0x0U;
inline constexpr std::uint32_t txframeend_enablerxdata_enabled = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x001C5CFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using ready = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
using fielddetected = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t fielddetected_disabled = 0x0U;
inline constexpr std::uint32_t fielddetected_enabled = 0x1U;
using fieldlost = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t fieldlost_disabled = 0x0U;
inline constexpr std::uint32_t fieldlost_enabled = 0x1U;
using txframestart = core::field_descriptor<inten, 3U, 1U>;
inline constexpr std::uint32_t txframestart_disabled = 0x0U;
inline constexpr std::uint32_t txframestart_enabled = 0x1U;
using txframeend = core::field_descriptor<inten, 4U, 1U>;
inline constexpr std::uint32_t txframeend_disabled = 0x0U;
inline constexpr std::uint32_t txframeend_enabled = 0x1U;
using rxframestart = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t rxframestart_disabled = 0x0U;
inline constexpr std::uint32_t rxframestart_enabled = 0x1U;
using rxframeend = core::field_descriptor<inten, 6U, 1U>;
inline constexpr std::uint32_t rxframeend_disabled = 0x0U;
inline constexpr std::uint32_t rxframeend_enabled = 0x1U;
using error = core::field_descriptor<inten, 7U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
using rxerror = core::field_descriptor<inten, 10U, 1U>;
inline constexpr std::uint32_t rxerror_disabled = 0x0U;
inline constexpr std::uint32_t rxerror_enabled = 0x1U;
using endrx = core::field_descriptor<inten, 11U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
using endtx = core::field_descriptor<inten, 12U, 1U>;
inline constexpr std::uint32_t endtx_disabled = 0x0U;
inline constexpr std::uint32_t endtx_enabled = 0x1U;
using autocolresstarted = core::field_descriptor<inten, 14U, 1U>;
inline constexpr std::uint32_t autocolresstarted_disabled = 0x0U;
inline constexpr std::uint32_t autocolresstarted_enabled = 0x1U;
using collision = core::field_descriptor<inten, 18U, 1U>;
inline constexpr std::uint32_t collision_disabled = 0x0U;
inline constexpr std::uint32_t collision_enabled = 0x1U;
using selected = core::field_descriptor<inten, 19U, 1U>;
inline constexpr std::uint32_t selected_disabled = 0x0U;
inline constexpr std::uint32_t selected_enabled = 0x1U;
using started = core::field_descriptor<inten, 20U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x001C5CFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using ready = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_set = 0x1U;
using fielddetected = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t fielddetected_disabled = 0x0U;
inline constexpr std::uint32_t fielddetected_enabled = 0x1U;
inline constexpr std::uint32_t fielddetected_set = 0x1U;
using fieldlost = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t fieldlost_disabled = 0x0U;
inline constexpr std::uint32_t fieldlost_enabled = 0x1U;
inline constexpr std::uint32_t fieldlost_set = 0x1U;
using txframestart = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t txframestart_disabled = 0x0U;
inline constexpr std::uint32_t txframestart_enabled = 0x1U;
inline constexpr std::uint32_t txframestart_set = 0x1U;
using txframeend = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t txframeend_disabled = 0x0U;
inline constexpr std::uint32_t txframeend_enabled = 0x1U;
inline constexpr std::uint32_t txframeend_set = 0x1U;
using rxframestart = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t rxframestart_disabled = 0x0U;
inline constexpr std::uint32_t rxframestart_enabled = 0x1U;
inline constexpr std::uint32_t rxframestart_set = 0x1U;
using rxframeend = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t rxframeend_disabled = 0x0U;
inline constexpr std::uint32_t rxframeend_enabled = 0x1U;
inline constexpr std::uint32_t rxframeend_set = 0x1U;
using error = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_set = 0x1U;
using rxerror = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t rxerror_disabled = 0x0U;
inline constexpr std::uint32_t rxerror_enabled = 0x1U;
inline constexpr std::uint32_t rxerror_set = 0x1U;
using endrx = core::field_descriptor<intenset, 11U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
inline constexpr std::uint32_t endrx_set = 0x1U;
using endtx = core::field_descriptor<intenset, 12U, 1U>;
inline constexpr std::uint32_t endtx_disabled = 0x0U;
inline constexpr std::uint32_t endtx_enabled = 0x1U;
inline constexpr std::uint32_t endtx_set = 0x1U;
using autocolresstarted = core::field_descriptor<intenset, 14U, 1U>;
inline constexpr std::uint32_t autocolresstarted_disabled = 0x0U;
inline constexpr std::uint32_t autocolresstarted_enabled = 0x1U;
inline constexpr std::uint32_t autocolresstarted_set = 0x1U;
using collision = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t collision_disabled = 0x0U;
inline constexpr std::uint32_t collision_enabled = 0x1U;
inline constexpr std::uint32_t collision_set = 0x1U;
using selected = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t selected_disabled = 0x0U;
inline constexpr std::uint32_t selected_enabled = 0x1U;
inline constexpr std::uint32_t selected_set = 0x1U;
using started = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x001C5CFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using ready = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_clear = 0x1U;
using fielddetected = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t fielddetected_disabled = 0x0U;
inline constexpr std::uint32_t fielddetected_enabled = 0x1U;
inline constexpr std::uint32_t fielddetected_clear = 0x1U;
using fieldlost = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t fieldlost_disabled = 0x0U;
inline constexpr std::uint32_t fieldlost_enabled = 0x1U;
inline constexpr std::uint32_t fieldlost_clear = 0x1U;
using txframestart = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t txframestart_disabled = 0x0U;
inline constexpr std::uint32_t txframestart_enabled = 0x1U;
inline constexpr std::uint32_t txframestart_clear = 0x1U;
using txframeend = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t txframeend_disabled = 0x0U;
inline constexpr std::uint32_t txframeend_enabled = 0x1U;
inline constexpr std::uint32_t txframeend_clear = 0x1U;
using rxframestart = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t rxframestart_disabled = 0x0U;
inline constexpr std::uint32_t rxframestart_enabled = 0x1U;
inline constexpr std::uint32_t rxframestart_clear = 0x1U;
using rxframeend = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t rxframeend_disabled = 0x0U;
inline constexpr std::uint32_t rxframeend_enabled = 0x1U;
inline constexpr std::uint32_t rxframeend_clear = 0x1U;
using error = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_clear = 0x1U;
using rxerror = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t rxerror_disabled = 0x0U;
inline constexpr std::uint32_t rxerror_enabled = 0x1U;
inline constexpr std::uint32_t rxerror_clear = 0x1U;
using endrx = core::field_descriptor<intenclr, 11U, 1U>;
inline constexpr std::uint32_t endrx_disabled = 0x0U;
inline constexpr std::uint32_t endrx_enabled = 0x1U;
inline constexpr std::uint32_t endrx_clear = 0x1U;
using endtx = core::field_descriptor<intenclr, 12U, 1U>;
inline constexpr std::uint32_t endtx_disabled = 0x0U;
inline constexpr std::uint32_t endtx_enabled = 0x1U;
inline constexpr std::uint32_t endtx_clear = 0x1U;
using autocolresstarted = core::field_descriptor<intenclr, 14U, 1U>;
inline constexpr std::uint32_t autocolresstarted_disabled = 0x0U;
inline constexpr std::uint32_t autocolresstarted_enabled = 0x1U;
inline constexpr std::uint32_t autocolresstarted_clear = 0x1U;
using collision = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t collision_disabled = 0x0U;
inline constexpr std::uint32_t collision_enabled = 0x1U;
inline constexpr std::uint32_t collision_clear = 0x1U;
using selected = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t selected_disabled = 0x0U;
inline constexpr std::uint32_t selected_enabled = 0x1U;
inline constexpr std::uint32_t selected_clear = 0x1U;
using started = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_clear = 0x1U;
}
struct errorstatus : core::register_descriptor<std::uint32_t, 0x404U,
    core::access_semantics::write_one_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace errorstatus_fields {
using framedelaytimeout = core::field_descriptor<errorstatus, 0U, 1U>;
}
struct framestatus_rx : core::register_descriptor<std::uint32_t, 0x40CU,
    core::access_semantics::write_one_to_clear, 0x0000000DU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace framestatus_rx_fields {
using crcerror = core::field_descriptor<framestatus_rx, 0U, 1U>;
inline constexpr std::uint32_t crcerror_crccorrect = 0x0U;
inline constexpr std::uint32_t crcerror_crcerror = 0x1U;
using paritystatus = core::field_descriptor<framestatus_rx, 2U, 1U>;
inline constexpr std::uint32_t paritystatus_parityok = 0x0U;
inline constexpr std::uint32_t paritystatus_parityerror = 0x1U;
using overrun = core::field_descriptor<framestatus_rx, 3U, 1U>;
inline constexpr std::uint32_t overrun_nooverrun = 0x0U;
inline constexpr std::uint32_t overrun_overrun = 0x1U;
}
struct nfctagstate : core::register_descriptor<std::uint32_t, 0x410U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfctagstate_fields {
using nfctagstate = core::field_descriptor<nfctagstate, 0U, 3U>;
inline constexpr std::uint32_t nfctagstate_disabled = 0x0U;
inline constexpr std::uint32_t nfctagstate_rampup = 0x2U;
inline constexpr std::uint32_t nfctagstate_idle = 0x3U;
inline constexpr std::uint32_t nfctagstate_receive = 0x4U;
inline constexpr std::uint32_t nfctagstate_framedelay = 0x5U;
inline constexpr std::uint32_t nfctagstate_transmit = 0x6U;
}
struct sleepstate : core::register_descriptor<std::uint32_t, 0x420U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sleepstate_fields {
using sleepstate = core::field_descriptor<sleepstate, 0U, 1U>;
inline constexpr std::uint32_t sleepstate_idle = 0x0U;
inline constexpr std::uint32_t sleepstate_sleepa = 0x1U;
}
struct fieldpresent : core::register_descriptor<std::uint32_t, 0x43CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fieldpresent_fields {
using fieldpresent = core::field_descriptor<fieldpresent, 0U, 1U>;
inline constexpr std::uint32_t fieldpresent_nofield = 0x0U;
inline constexpr std::uint32_t fieldpresent_fieldpresent = 0x1U;
using lockdetect = core::field_descriptor<fieldpresent, 1U, 1U>;
inline constexpr std::uint32_t lockdetect_notlocked = 0x0U;
inline constexpr std::uint32_t lockdetect_locked = 0x1U;
}
struct framedelaymin : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000480U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace framedelaymin_fields {
using framedelaymin = core::field_descriptor<framedelaymin, 0U, 16U>;
}
struct framedelaymax : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x000FFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00001000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace framedelaymax_fields {
using framedelaymax = core::field_descriptor<framedelaymax, 0U, 20U>;
}
struct framedelaymode : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace framedelaymode_fields {
using framedelaymode = core::field_descriptor<framedelaymode, 0U, 2U>;
inline constexpr std::uint32_t framedelaymode_freerun = 0x0U;
inline constexpr std::uint32_t framedelaymode_window = 0x1U;
inline constexpr std::uint32_t framedelaymode_exactval = 0x2U;
inline constexpr std::uint32_t framedelaymode_windowgrid = 0x3U;
}
struct packetptr : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace packetptr_fields {
using ptr = core::field_descriptor<packetptr, 0U, 32U>;
}
struct maxlen : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x000001FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace maxlen_fields {
using maxlen = core::field_descriptor<maxlen, 0U, 9U>;
}
struct txd_frameconfig : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x00000017U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000017U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_frameconfig_fields {
using parity = core::field_descriptor<txd_frameconfig, 0U, 1U>;
inline constexpr std::uint32_t parity_noparity = 0x0U;
inline constexpr std::uint32_t parity_parity = 0x1U;
using discardmode = core::field_descriptor<txd_frameconfig, 1U, 1U>;
inline constexpr std::uint32_t discardmode_discardend = 0x0U;
inline constexpr std::uint32_t discardmode_discardstart = 0x1U;
using sof = core::field_descriptor<txd_frameconfig, 2U, 1U>;
inline constexpr std::uint32_t sof_nosof = 0x0U;
inline constexpr std::uint32_t sof_sof = 0x1U;
using crcmodetx = core::field_descriptor<txd_frameconfig, 4U, 1U>;
inline constexpr std::uint32_t crcmodetx_nocrctx = 0x0U;
inline constexpr std::uint32_t crcmodetx_crc16tx = 0x1U;
}
struct txd_amount : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x00000FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_amount_fields {
using txdatabits = core::field_descriptor<txd_amount, 0U, 3U>;
using txdatabytes = core::field_descriptor<txd_amount, 3U, 9U>;
}
struct rxd_frameconfig : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00000015U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000015U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_frameconfig_fields {
using parity = core::field_descriptor<rxd_frameconfig, 0U, 1U>;
inline constexpr std::uint32_t parity_noparity = 0x0U;
inline constexpr std::uint32_t parity_parity = 0x1U;
using sof = core::field_descriptor<rxd_frameconfig, 2U, 1U>;
inline constexpr std::uint32_t sof_nosof = 0x0U;
inline constexpr std::uint32_t sof_sof = 0x1U;
using crcmoderx = core::field_descriptor<rxd_frameconfig, 4U, 1U>;
inline constexpr std::uint32_t crcmoderx_nocrcrx = 0x0U;
inline constexpr std::uint32_t crcmoderx_crc16rx = 0x1U;
}
struct rxd_amount : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_amount_fields {
using rxdatabits = core::field_descriptor<rxd_amount, 0U, 3U>;
using rxdatabytes = core::field_descriptor<rxd_amount, 3U, 9U>;
}
struct nfcid1_last : core::register_descriptor<std::uint32_t, 0x590U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00006363U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfcid1_last_fields {
using nfcid1_z = core::field_descriptor<nfcid1_last, 0U, 8U>;
using nfcid1_y = core::field_descriptor<nfcid1_last, 8U, 8U>;
using nfcid1_x = core::field_descriptor<nfcid1_last, 16U, 8U>;
using nfcid1_w = core::field_descriptor<nfcid1_last, 24U, 8U>;
}
struct nfcid1_2nd_last : core::register_descriptor<std::uint32_t, 0x594U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfcid1_2nd_last_fields {
using nfcid1_v = core::field_descriptor<nfcid1_2nd_last, 0U, 8U>;
using nfcid1_u = core::field_descriptor<nfcid1_2nd_last, 8U, 8U>;
using nfcid1_t = core::field_descriptor<nfcid1_2nd_last, 16U, 8U>;
}
struct nfcid1_3rd_last : core::register_descriptor<std::uint32_t, 0x598U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfcid1_3rd_last_fields {
using nfcid1_s = core::field_descriptor<nfcid1_3rd_last, 0U, 8U>;
using nfcid1_r = core::field_descriptor<nfcid1_3rd_last, 8U, 8U>;
using nfcid1_q = core::field_descriptor<nfcid1_3rd_last, 16U, 8U>;
}
struct autocolresconfig : core::register_descriptor<std::uint32_t, 0x59CU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000002U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace autocolresconfig_fields {
using mode = core::field_descriptor<autocolresconfig, 0U, 1U>;
inline constexpr std::uint32_t mode_enabled = 0x0U;
inline constexpr std::uint32_t mode_disabled = 0x1U;
}
struct sensres : core::register_descriptor<std::uint32_t, 0x5A0U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sensres_fields {
using bitframesdd = core::field_descriptor<sensres, 0U, 5U>;
inline constexpr std::uint32_t bitframesdd_sdd00000 = 0x0U;
inline constexpr std::uint32_t bitframesdd_sdd00001 = 0x1U;
inline constexpr std::uint32_t bitframesdd_sdd00010 = 0x2U;
inline constexpr std::uint32_t bitframesdd_sdd00100 = 0x4U;
inline constexpr std::uint32_t bitframesdd_sdd01000 = 0x8U;
inline constexpr std::uint32_t bitframesdd_sdd10000 = 0x10U;
using rfu5 = core::field_descriptor<sensres, 5U, 1U>;
using nfcidsize = core::field_descriptor<sensres, 6U, 2U>;
inline constexpr std::uint32_t nfcidsize_nfcid1single = 0x0U;
inline constexpr std::uint32_t nfcidsize_nfcid1double = 0x1U;
inline constexpr std::uint32_t nfcidsize_nfcid1triple = 0x2U;
using platfconfig = core::field_descriptor<sensres, 8U, 4U>;
using rfu74 = core::field_descriptor<sensres, 12U, 4U>;
}
struct selres : core::register_descriptor<std::uint32_t, 0x5A4U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace selres_fields {
using rfu10 = core::field_descriptor<selres, 0U, 2U>;
using cascade = core::field_descriptor<selres, 2U, 1U>;
using rfu43 = core::field_descriptor<selres, 3U, 2U>;
using protocol = core::field_descriptor<selres, 5U, 2U>;
using rfu7 = core::field_descriptor<selres, 7U, 1U>;
}
}
