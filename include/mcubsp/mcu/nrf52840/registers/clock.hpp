#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::clock {
struct tasks_hfclkstart : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_hfclkstart_fields {
using tasks_hfclkstart = core::field_descriptor<tasks_hfclkstart, 0U, 1U>;
inline constexpr std::uint32_t tasks_hfclkstart_trigger = 0x1U;
}
struct tasks_hfclkstop : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_hfclkstop_fields {
using tasks_hfclkstop = core::field_descriptor<tasks_hfclkstop, 0U, 1U>;
inline constexpr std::uint32_t tasks_hfclkstop_trigger = 0x1U;
}
struct tasks_lfclkstart : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_lfclkstart_fields {
using tasks_lfclkstart = core::field_descriptor<tasks_lfclkstart, 0U, 1U>;
inline constexpr std::uint32_t tasks_lfclkstart_trigger = 0x1U;
}
struct tasks_lfclkstop : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_lfclkstop_fields {
using tasks_lfclkstop = core::field_descriptor<tasks_lfclkstop, 0U, 1U>;
inline constexpr std::uint32_t tasks_lfclkstop_trigger = 0x1U;
}
struct tasks_cal : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_cal_fields {
using tasks_cal = core::field_descriptor<tasks_cal, 0U, 1U>;
inline constexpr std::uint32_t tasks_cal_trigger = 0x1U;
}
struct tasks_ctstart : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ctstart_fields {
using tasks_ctstart = core::field_descriptor<tasks_ctstart, 0U, 1U>;
inline constexpr std::uint32_t tasks_ctstart_trigger = 0x1U;
}
struct tasks_ctstop : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ctstop_fields {
using tasks_ctstop = core::field_descriptor<tasks_ctstop, 0U, 1U>;
inline constexpr std::uint32_t tasks_ctstop_trigger = 0x1U;
}
struct events_hfclkstarted : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_hfclkstarted_fields {
using events_hfclkstarted = core::field_descriptor<events_hfclkstarted, 0U, 1U>;
inline constexpr std::uint32_t events_hfclkstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_hfclkstarted_generated = 0x1U;
}
struct events_lfclkstarted : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_lfclkstarted_fields {
using events_lfclkstarted = core::field_descriptor<events_lfclkstarted, 0U, 1U>;
inline constexpr std::uint32_t events_lfclkstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_lfclkstarted_generated = 0x1U;
}
struct events_done : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_done_fields {
using events_done = core::field_descriptor<events_done, 0U, 1U>;
inline constexpr std::uint32_t events_done_notgenerated = 0x0U;
inline constexpr std::uint32_t events_done_generated = 0x1U;
}
struct events_ctto : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ctto_fields {
using events_ctto = core::field_descriptor<events_ctto, 0U, 1U>;
inline constexpr std::uint32_t events_ctto_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ctto_generated = 0x1U;
}
struct events_ctstarted : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ctstarted_fields {
using events_ctstarted = core::field_descriptor<events_ctstarted, 0U, 1U>;
inline constexpr std::uint32_t events_ctstarted_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ctstarted_generated = 0x1U;
}
struct events_ctstopped : core::register_descriptor<std::uint32_t, 0x12CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ctstopped_fields {
using events_ctstopped = core::field_descriptor<events_ctstopped, 0U, 1U>;
inline constexpr std::uint32_t events_ctstopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ctstopped_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000C1BU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using hfclkstarted = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t hfclkstarted_disabled = 0x0U;
inline constexpr std::uint32_t hfclkstarted_enabled = 0x1U;
inline constexpr std::uint32_t hfclkstarted_set = 0x1U;
using lfclkstarted = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t lfclkstarted_disabled = 0x0U;
inline constexpr std::uint32_t lfclkstarted_enabled = 0x1U;
inline constexpr std::uint32_t lfclkstarted_set = 0x1U;
using done = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t done_disabled = 0x0U;
inline constexpr std::uint32_t done_enabled = 0x1U;
inline constexpr std::uint32_t done_set = 0x1U;
using ctto = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t ctto_disabled = 0x0U;
inline constexpr std::uint32_t ctto_enabled = 0x1U;
inline constexpr std::uint32_t ctto_set = 0x1U;
using ctstarted = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t ctstarted_disabled = 0x0U;
inline constexpr std::uint32_t ctstarted_enabled = 0x1U;
inline constexpr std::uint32_t ctstarted_set = 0x1U;
using ctstopped = core::field_descriptor<intenset, 11U, 1U>;
inline constexpr std::uint32_t ctstopped_disabled = 0x0U;
inline constexpr std::uint32_t ctstopped_enabled = 0x1U;
inline constexpr std::uint32_t ctstopped_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000C1BU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using hfclkstarted = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t hfclkstarted_disabled = 0x0U;
inline constexpr std::uint32_t hfclkstarted_enabled = 0x1U;
inline constexpr std::uint32_t hfclkstarted_clear = 0x1U;
using lfclkstarted = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t lfclkstarted_disabled = 0x0U;
inline constexpr std::uint32_t lfclkstarted_enabled = 0x1U;
inline constexpr std::uint32_t lfclkstarted_clear = 0x1U;
using done = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t done_disabled = 0x0U;
inline constexpr std::uint32_t done_enabled = 0x1U;
inline constexpr std::uint32_t done_clear = 0x1U;
using ctto = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t ctto_disabled = 0x0U;
inline constexpr std::uint32_t ctto_enabled = 0x1U;
inline constexpr std::uint32_t ctto_clear = 0x1U;
using ctstarted = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t ctstarted_disabled = 0x0U;
inline constexpr std::uint32_t ctstarted_enabled = 0x1U;
inline constexpr std::uint32_t ctstarted_clear = 0x1U;
using ctstopped = core::field_descriptor<intenclr, 11U, 1U>;
inline constexpr std::uint32_t ctstopped_disabled = 0x0U;
inline constexpr std::uint32_t ctstopped_enabled = 0x1U;
inline constexpr std::uint32_t ctstopped_clear = 0x1U;
}
struct hfclkrun : core::register_descriptor<std::uint32_t, 0x408U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace hfclkrun_fields {
using status = core::field_descriptor<hfclkrun, 0U, 1U>;
inline constexpr std::uint32_t status_nottriggered = 0x0U;
inline constexpr std::uint32_t status_triggered = 0x1U;
}
struct hfclkstat : core::register_descriptor<std::uint32_t, 0x40CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace hfclkstat_fields {
using src = core::field_descriptor<hfclkstat, 0U, 1U>;
inline constexpr std::uint32_t src_rc = 0x0U;
inline constexpr std::uint32_t src_xtal = 0x1U;
using state = core::field_descriptor<hfclkstat, 16U, 1U>;
inline constexpr std::uint32_t state_notrunning = 0x0U;
inline constexpr std::uint32_t state_running = 0x1U;
}
struct lfclkrun : core::register_descriptor<std::uint32_t, 0x414U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace lfclkrun_fields {
using status = core::field_descriptor<lfclkrun, 0U, 1U>;
inline constexpr std::uint32_t status_nottriggered = 0x0U;
inline constexpr std::uint32_t status_triggered = 0x1U;
}
struct lfclkstat : core::register_descriptor<std::uint32_t, 0x418U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace lfclkstat_fields {
using src = core::field_descriptor<lfclkstat, 0U, 2U>;
inline constexpr std::uint32_t src_rc = 0x0U;
inline constexpr std::uint32_t src_xtal = 0x1U;
inline constexpr std::uint32_t src_synth = 0x2U;
using state = core::field_descriptor<lfclkstat, 16U, 1U>;
inline constexpr std::uint32_t state_notrunning = 0x0U;
inline constexpr std::uint32_t state_running = 0x1U;
}
struct lfclksrccopy : core::register_descriptor<std::uint32_t, 0x41CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace lfclksrccopy_fields {
using src = core::field_descriptor<lfclksrccopy, 0U, 2U>;
inline constexpr std::uint32_t src_rc = 0x0U;
inline constexpr std::uint32_t src_xtal = 0x1U;
inline constexpr std::uint32_t src_synth = 0x2U;
}
struct lfclksrc : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x00030003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace lfclksrc_fields {
using src = core::field_descriptor<lfclksrc, 0U, 2U>;
inline constexpr std::uint32_t src_rc = 0x0U;
inline constexpr std::uint32_t src_xtal = 0x1U;
inline constexpr std::uint32_t src_synth = 0x2U;
using bypass = core::field_descriptor<lfclksrc, 16U, 1U>;
inline constexpr std::uint32_t bypass_disabled = 0x0U;
inline constexpr std::uint32_t bypass_enabled = 0x1U;
using external = core::field_descriptor<lfclksrc, 17U, 1U>;
inline constexpr std::uint32_t external_disabled = 0x0U;
inline constexpr std::uint32_t external_enabled = 0x1U;
}
struct hfxodebounce : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000010U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace hfxodebounce_fields {
using hfxodebounce = core::field_descriptor<hfxodebounce, 0U, 8U>;
inline constexpr std::uint32_t hfxodebounce_db256us = 0x10U;
inline constexpr std::uint32_t hfxodebounce_db1024us = 0x40U;
}
struct ctiv : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ctiv_fields {
using ctiv = core::field_descriptor<ctiv, 0U, 7U>;
}
struct traceconfig : core::register_descriptor<std::uint32_t, 0x55CU,
    core::access_semantics::read_write, 0x00030003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace traceconfig_fields {
using traceportspeed = core::field_descriptor<traceconfig, 0U, 2U>;
inline constexpr std::uint32_t traceportspeed_reg_32mhz = 0x0U;
inline constexpr std::uint32_t traceportspeed_reg_16mhz = 0x1U;
inline constexpr std::uint32_t traceportspeed_reg_8mhz = 0x2U;
inline constexpr std::uint32_t traceportspeed_reg_4mhz = 0x3U;
using tracemux = core::field_descriptor<traceconfig, 16U, 2U>;
inline constexpr std::uint32_t tracemux_gpio = 0x0U;
inline constexpr std::uint32_t tracemux_serial = 0x1U;
inline constexpr std::uint32_t tracemux_parallel = 0x2U;
}
struct lfrcmode : core::register_descriptor<std::uint32_t, 0x5B4U,
    core::access_semantics::read_write, 0x00010001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace lfrcmode_fields {
using mode = core::field_descriptor<lfrcmode, 0U, 1U>;
inline constexpr std::uint32_t mode_normal = 0x0U;
inline constexpr std::uint32_t mode_ulp = 0x1U;
using status = core::field_descriptor<lfrcmode, 16U, 1U>;
inline constexpr std::uint32_t status_normal = 0x0U;
inline constexpr std::uint32_t status_ulp = 0x1U;
}
}
