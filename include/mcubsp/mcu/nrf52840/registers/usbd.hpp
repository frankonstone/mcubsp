#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::usbd {
struct tasks_startepin0 : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin0_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin0, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin1 : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin1_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin1, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin2 : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin2_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin2, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin3 : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin3_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin3, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin4 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin4_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin4, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin5 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin5_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin5, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin6 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin6_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin6, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startepin7 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepin7_fields {
using tasks_startepin = core::field_descriptor<tasks_startepin7, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepin_trigger = 0x1U;
}
struct tasks_startisoin : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startisoin_fields {
using tasks_startisoin = core::field_descriptor<tasks_startisoin, 0U, 1U>;
inline constexpr std::uint32_t tasks_startisoin_trigger = 0x1U;
}
struct tasks_startepout0 : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout0_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout0, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout1 : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout1_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout1, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout2 : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout2_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout2, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout3 : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout3_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout3, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout4 : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout4_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout4, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout5 : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout5_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout5, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout6 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout6_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout6, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startepout7 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startepout7_fields {
using tasks_startepout = core::field_descriptor<tasks_startepout7, 0U, 1U>;
inline constexpr std::uint32_t tasks_startepout_trigger = 0x1U;
}
struct tasks_startisoout : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startisoout_fields {
using tasks_startisoout = core::field_descriptor<tasks_startisoout, 0U, 1U>;
inline constexpr std::uint32_t tasks_startisoout_trigger = 0x1U;
}
struct tasks_ep0rcvout : core::register_descriptor<std::uint32_t, 0x4CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ep0rcvout_fields {
using tasks_ep0rcvout = core::field_descriptor<tasks_ep0rcvout, 0U, 1U>;
inline constexpr std::uint32_t tasks_ep0rcvout_trigger = 0x1U;
}
struct tasks_ep0status : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ep0status_fields {
using tasks_ep0status = core::field_descriptor<tasks_ep0status, 0U, 1U>;
inline constexpr std::uint32_t tasks_ep0status_trigger = 0x1U;
}
struct tasks_ep0stall : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ep0stall_fields {
using tasks_ep0stall = core::field_descriptor<tasks_ep0stall, 0U, 1U>;
inline constexpr std::uint32_t tasks_ep0stall_trigger = 0x1U;
}
struct tasks_dpdmdrive : core::register_descriptor<std::uint32_t, 0x58U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_dpdmdrive_fields {
using tasks_dpdmdrive = core::field_descriptor<tasks_dpdmdrive, 0U, 1U>;
inline constexpr std::uint32_t tasks_dpdmdrive_trigger = 0x1U;
}
struct tasks_dpdmnodrive : core::register_descriptor<std::uint32_t, 0x5CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_dpdmnodrive_fields {
using tasks_dpdmnodrive = core::field_descriptor<tasks_dpdmnodrive, 0U, 1U>;
inline constexpr std::uint32_t tasks_dpdmnodrive_trigger = 0x1U;
}
struct events_usbreset : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_usbreset_fields {
using events_usbreset = core::field_descriptor<events_usbreset, 0U, 1U>;
inline constexpr std::uint32_t events_usbreset_notgenerated = 0x0U;
inline constexpr std::uint32_t events_usbreset_generated = 0x1U;
}
struct events_started : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_started_fields {
using events_started = core::field_descriptor<events_started, 0U, 1U>;
inline constexpr std::uint32_t events_started_notgenerated = 0x0U;
inline constexpr std::uint32_t events_started_generated = 0x1U;
}
struct events_endepin0 : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin0_fields {
using events_endepin = core::field_descriptor<events_endepin0, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin1 : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin1_fields {
using events_endepin = core::field_descriptor<events_endepin1, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin2 : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin2_fields {
using events_endepin = core::field_descriptor<events_endepin2, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin3 : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin3_fields {
using events_endepin = core::field_descriptor<events_endepin3, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin4 : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin4_fields {
using events_endepin = core::field_descriptor<events_endepin4, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin5 : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin5_fields {
using events_endepin = core::field_descriptor<events_endepin5, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin6 : core::register_descriptor<std::uint32_t, 0x120U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin6_fields {
using events_endepin = core::field_descriptor<events_endepin6, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_endepin7 : core::register_descriptor<std::uint32_t, 0x124U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepin7_fields {
using events_endepin = core::field_descriptor<events_endepin7, 0U, 1U>;
inline constexpr std::uint32_t events_endepin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepin_generated = 0x1U;
}
struct events_ep0datadone : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ep0datadone_fields {
using events_ep0datadone = core::field_descriptor<events_ep0datadone, 0U, 1U>;
inline constexpr std::uint32_t events_ep0datadone_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ep0datadone_generated = 0x1U;
}
struct events_endisoin : core::register_descriptor<std::uint32_t, 0x12CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endisoin_fields {
using events_endisoin = core::field_descriptor<events_endisoin, 0U, 1U>;
inline constexpr std::uint32_t events_endisoin_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endisoin_generated = 0x1U;
}
struct events_endepout0 : core::register_descriptor<std::uint32_t, 0x130U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout0_fields {
using events_endepout = core::field_descriptor<events_endepout0, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout1 : core::register_descriptor<std::uint32_t, 0x134U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout1_fields {
using events_endepout = core::field_descriptor<events_endepout1, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout2 : core::register_descriptor<std::uint32_t, 0x138U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout2_fields {
using events_endepout = core::field_descriptor<events_endepout2, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout3 : core::register_descriptor<std::uint32_t, 0x13CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout3_fields {
using events_endepout = core::field_descriptor<events_endepout3, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout4 : core::register_descriptor<std::uint32_t, 0x140U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout4_fields {
using events_endepout = core::field_descriptor<events_endepout4, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout5 : core::register_descriptor<std::uint32_t, 0x144U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout5_fields {
using events_endepout = core::field_descriptor<events_endepout5, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout6 : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout6_fields {
using events_endepout = core::field_descriptor<events_endepout6, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endepout7 : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endepout7_fields {
using events_endepout = core::field_descriptor<events_endepout7, 0U, 1U>;
inline constexpr std::uint32_t events_endepout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endepout_generated = 0x1U;
}
struct events_endisoout : core::register_descriptor<std::uint32_t, 0x150U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endisoout_fields {
using events_endisoout = core::field_descriptor<events_endisoout, 0U, 1U>;
inline constexpr std::uint32_t events_endisoout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endisoout_generated = 0x1U;
}
struct events_sof : core::register_descriptor<std::uint32_t, 0x154U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_sof_fields {
using events_sof = core::field_descriptor<events_sof, 0U, 1U>;
inline constexpr std::uint32_t events_sof_notgenerated = 0x0U;
inline constexpr std::uint32_t events_sof_generated = 0x1U;
}
struct events_usbevent : core::register_descriptor<std::uint32_t, 0x158U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_usbevent_fields {
using events_usbevent = core::field_descriptor<events_usbevent, 0U, 1U>;
inline constexpr std::uint32_t events_usbevent_notgenerated = 0x0U;
inline constexpr std::uint32_t events_usbevent_generated = 0x1U;
}
struct events_ep0setup : core::register_descriptor<std::uint32_t, 0x15CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ep0setup_fields {
using events_ep0setup = core::field_descriptor<events_ep0setup, 0U, 1U>;
inline constexpr std::uint32_t events_ep0setup_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ep0setup_generated = 0x1U;
}
struct events_epdata : core::register_descriptor<std::uint32_t, 0x160U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_epdata_fields {
using events_epdata = core::field_descriptor<events_epdata, 0U, 1U>;
inline constexpr std::uint32_t events_epdata_notgenerated = 0x0U;
inline constexpr std::uint32_t events_epdata_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using ep0datadone_startepin0 = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t ep0datadone_startepin0_disabled = 0x0U;
inline constexpr std::uint32_t ep0datadone_startepin0_enabled = 0x1U;
using ep0datadone_startepout0 = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t ep0datadone_startepout0_disabled = 0x0U;
inline constexpr std::uint32_t ep0datadone_startepout0_enabled = 0x1U;
using ep0datadone_ep0status = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t ep0datadone_ep0status_disabled = 0x0U;
inline constexpr std::uint32_t ep0datadone_ep0status_enabled = 0x1U;
using endepout0_ep0status = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t endepout0_ep0status_disabled = 0x0U;
inline constexpr std::uint32_t endepout0_ep0status_enabled = 0x1U;
using endepout0_ep0rcvout = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t endepout0_ep0rcvout_disabled = 0x0U;
inline constexpr std::uint32_t endepout0_ep0rcvout_enabled = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x01FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using usbreset = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t usbreset_disabled = 0x0U;
inline constexpr std::uint32_t usbreset_enabled = 0x1U;
using started = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
using endepin0 = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t endepin0_disabled = 0x0U;
inline constexpr std::uint32_t endepin0_enabled = 0x1U;
using endepin1 = core::field_descriptor<inten, 3U, 1U>;
inline constexpr std::uint32_t endepin1_disabled = 0x0U;
inline constexpr std::uint32_t endepin1_enabled = 0x1U;
using endepin2 = core::field_descriptor<inten, 4U, 1U>;
inline constexpr std::uint32_t endepin2_disabled = 0x0U;
inline constexpr std::uint32_t endepin2_enabled = 0x1U;
using endepin3 = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t endepin3_disabled = 0x0U;
inline constexpr std::uint32_t endepin3_enabled = 0x1U;
using endepin4 = core::field_descriptor<inten, 6U, 1U>;
inline constexpr std::uint32_t endepin4_disabled = 0x0U;
inline constexpr std::uint32_t endepin4_enabled = 0x1U;
using endepin5 = core::field_descriptor<inten, 7U, 1U>;
inline constexpr std::uint32_t endepin5_disabled = 0x0U;
inline constexpr std::uint32_t endepin5_enabled = 0x1U;
using endepin6 = core::field_descriptor<inten, 8U, 1U>;
inline constexpr std::uint32_t endepin6_disabled = 0x0U;
inline constexpr std::uint32_t endepin6_enabled = 0x1U;
using endepin7 = core::field_descriptor<inten, 9U, 1U>;
inline constexpr std::uint32_t endepin7_disabled = 0x0U;
inline constexpr std::uint32_t endepin7_enabled = 0x1U;
using ep0datadone = core::field_descriptor<inten, 10U, 1U>;
inline constexpr std::uint32_t ep0datadone_disabled = 0x0U;
inline constexpr std::uint32_t ep0datadone_enabled = 0x1U;
using endisoin = core::field_descriptor<inten, 11U, 1U>;
inline constexpr std::uint32_t endisoin_disabled = 0x0U;
inline constexpr std::uint32_t endisoin_enabled = 0x1U;
using endepout0 = core::field_descriptor<inten, 12U, 1U>;
inline constexpr std::uint32_t endepout0_disabled = 0x0U;
inline constexpr std::uint32_t endepout0_enabled = 0x1U;
using endepout1 = core::field_descriptor<inten, 13U, 1U>;
inline constexpr std::uint32_t endepout1_disabled = 0x0U;
inline constexpr std::uint32_t endepout1_enabled = 0x1U;
using endepout2 = core::field_descriptor<inten, 14U, 1U>;
inline constexpr std::uint32_t endepout2_disabled = 0x0U;
inline constexpr std::uint32_t endepout2_enabled = 0x1U;
using endepout3 = core::field_descriptor<inten, 15U, 1U>;
inline constexpr std::uint32_t endepout3_disabled = 0x0U;
inline constexpr std::uint32_t endepout3_enabled = 0x1U;
using endepout4 = core::field_descriptor<inten, 16U, 1U>;
inline constexpr std::uint32_t endepout4_disabled = 0x0U;
inline constexpr std::uint32_t endepout4_enabled = 0x1U;
using endepout5 = core::field_descriptor<inten, 17U, 1U>;
inline constexpr std::uint32_t endepout5_disabled = 0x0U;
inline constexpr std::uint32_t endepout5_enabled = 0x1U;
using endepout6 = core::field_descriptor<inten, 18U, 1U>;
inline constexpr std::uint32_t endepout6_disabled = 0x0U;
inline constexpr std::uint32_t endepout6_enabled = 0x1U;
using endepout7 = core::field_descriptor<inten, 19U, 1U>;
inline constexpr std::uint32_t endepout7_disabled = 0x0U;
inline constexpr std::uint32_t endepout7_enabled = 0x1U;
using endisoout = core::field_descriptor<inten, 20U, 1U>;
inline constexpr std::uint32_t endisoout_disabled = 0x0U;
inline constexpr std::uint32_t endisoout_enabled = 0x1U;
using sof = core::field_descriptor<inten, 21U, 1U>;
inline constexpr std::uint32_t sof_disabled = 0x0U;
inline constexpr std::uint32_t sof_enabled = 0x1U;
using usbevent = core::field_descriptor<inten, 22U, 1U>;
inline constexpr std::uint32_t usbevent_disabled = 0x0U;
inline constexpr std::uint32_t usbevent_enabled = 0x1U;
using ep0setup = core::field_descriptor<inten, 23U, 1U>;
inline constexpr std::uint32_t ep0setup_disabled = 0x0U;
inline constexpr std::uint32_t ep0setup_enabled = 0x1U;
using epdata = core::field_descriptor<inten, 24U, 1U>;
inline constexpr std::uint32_t epdata_disabled = 0x0U;
inline constexpr std::uint32_t epdata_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x01FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using usbreset = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t usbreset_disabled = 0x0U;
inline constexpr std::uint32_t usbreset_enabled = 0x1U;
inline constexpr std::uint32_t usbreset_set = 0x1U;
using started = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_set = 0x1U;
using endepin0 = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t endepin0_disabled = 0x0U;
inline constexpr std::uint32_t endepin0_enabled = 0x1U;
inline constexpr std::uint32_t endepin0_set = 0x1U;
using endepin1 = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t endepin1_disabled = 0x0U;
inline constexpr std::uint32_t endepin1_enabled = 0x1U;
inline constexpr std::uint32_t endepin1_set = 0x1U;
using endepin2 = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t endepin2_disabled = 0x0U;
inline constexpr std::uint32_t endepin2_enabled = 0x1U;
inline constexpr std::uint32_t endepin2_set = 0x1U;
using endepin3 = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t endepin3_disabled = 0x0U;
inline constexpr std::uint32_t endepin3_enabled = 0x1U;
inline constexpr std::uint32_t endepin3_set = 0x1U;
using endepin4 = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t endepin4_disabled = 0x0U;
inline constexpr std::uint32_t endepin4_enabled = 0x1U;
inline constexpr std::uint32_t endepin4_set = 0x1U;
using endepin5 = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t endepin5_disabled = 0x0U;
inline constexpr std::uint32_t endepin5_enabled = 0x1U;
inline constexpr std::uint32_t endepin5_set = 0x1U;
using endepin6 = core::field_descriptor<intenset, 8U, 1U>;
inline constexpr std::uint32_t endepin6_disabled = 0x0U;
inline constexpr std::uint32_t endepin6_enabled = 0x1U;
inline constexpr std::uint32_t endepin6_set = 0x1U;
using endepin7 = core::field_descriptor<intenset, 9U, 1U>;
inline constexpr std::uint32_t endepin7_disabled = 0x0U;
inline constexpr std::uint32_t endepin7_enabled = 0x1U;
inline constexpr std::uint32_t endepin7_set = 0x1U;
using ep0datadone = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t ep0datadone_disabled = 0x0U;
inline constexpr std::uint32_t ep0datadone_enabled = 0x1U;
inline constexpr std::uint32_t ep0datadone_set = 0x1U;
using endisoin = core::field_descriptor<intenset, 11U, 1U>;
inline constexpr std::uint32_t endisoin_disabled = 0x0U;
inline constexpr std::uint32_t endisoin_enabled = 0x1U;
inline constexpr std::uint32_t endisoin_set = 0x1U;
using endepout0 = core::field_descriptor<intenset, 12U, 1U>;
inline constexpr std::uint32_t endepout0_disabled = 0x0U;
inline constexpr std::uint32_t endepout0_enabled = 0x1U;
inline constexpr std::uint32_t endepout0_set = 0x1U;
using endepout1 = core::field_descriptor<intenset, 13U, 1U>;
inline constexpr std::uint32_t endepout1_disabled = 0x0U;
inline constexpr std::uint32_t endepout1_enabled = 0x1U;
inline constexpr std::uint32_t endepout1_set = 0x1U;
using endepout2 = core::field_descriptor<intenset, 14U, 1U>;
inline constexpr std::uint32_t endepout2_disabled = 0x0U;
inline constexpr std::uint32_t endepout2_enabled = 0x1U;
inline constexpr std::uint32_t endepout2_set = 0x1U;
using endepout3 = core::field_descriptor<intenset, 15U, 1U>;
inline constexpr std::uint32_t endepout3_disabled = 0x0U;
inline constexpr std::uint32_t endepout3_enabled = 0x1U;
inline constexpr std::uint32_t endepout3_set = 0x1U;
using endepout4 = core::field_descriptor<intenset, 16U, 1U>;
inline constexpr std::uint32_t endepout4_disabled = 0x0U;
inline constexpr std::uint32_t endepout4_enabled = 0x1U;
inline constexpr std::uint32_t endepout4_set = 0x1U;
using endepout5 = core::field_descriptor<intenset, 17U, 1U>;
inline constexpr std::uint32_t endepout5_disabled = 0x0U;
inline constexpr std::uint32_t endepout5_enabled = 0x1U;
inline constexpr std::uint32_t endepout5_set = 0x1U;
using endepout6 = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t endepout6_disabled = 0x0U;
inline constexpr std::uint32_t endepout6_enabled = 0x1U;
inline constexpr std::uint32_t endepout6_set = 0x1U;
using endepout7 = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t endepout7_disabled = 0x0U;
inline constexpr std::uint32_t endepout7_enabled = 0x1U;
inline constexpr std::uint32_t endepout7_set = 0x1U;
using endisoout = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t endisoout_disabled = 0x0U;
inline constexpr std::uint32_t endisoout_enabled = 0x1U;
inline constexpr std::uint32_t endisoout_set = 0x1U;
using sof = core::field_descriptor<intenset, 21U, 1U>;
inline constexpr std::uint32_t sof_disabled = 0x0U;
inline constexpr std::uint32_t sof_enabled = 0x1U;
inline constexpr std::uint32_t sof_set = 0x1U;
using usbevent = core::field_descriptor<intenset, 22U, 1U>;
inline constexpr std::uint32_t usbevent_disabled = 0x0U;
inline constexpr std::uint32_t usbevent_enabled = 0x1U;
inline constexpr std::uint32_t usbevent_set = 0x1U;
using ep0setup = core::field_descriptor<intenset, 23U, 1U>;
inline constexpr std::uint32_t ep0setup_disabled = 0x0U;
inline constexpr std::uint32_t ep0setup_enabled = 0x1U;
inline constexpr std::uint32_t ep0setup_set = 0x1U;
using epdata = core::field_descriptor<intenset, 24U, 1U>;
inline constexpr std::uint32_t epdata_disabled = 0x0U;
inline constexpr std::uint32_t epdata_enabled = 0x1U;
inline constexpr std::uint32_t epdata_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x01FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using usbreset = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t usbreset_disabled = 0x0U;
inline constexpr std::uint32_t usbreset_enabled = 0x1U;
inline constexpr std::uint32_t usbreset_clear = 0x1U;
using started = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_clear = 0x1U;
using endepin0 = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t endepin0_disabled = 0x0U;
inline constexpr std::uint32_t endepin0_enabled = 0x1U;
inline constexpr std::uint32_t endepin0_clear = 0x1U;
using endepin1 = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t endepin1_disabled = 0x0U;
inline constexpr std::uint32_t endepin1_enabled = 0x1U;
inline constexpr std::uint32_t endepin1_clear = 0x1U;
using endepin2 = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t endepin2_disabled = 0x0U;
inline constexpr std::uint32_t endepin2_enabled = 0x1U;
inline constexpr std::uint32_t endepin2_clear = 0x1U;
using endepin3 = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t endepin3_disabled = 0x0U;
inline constexpr std::uint32_t endepin3_enabled = 0x1U;
inline constexpr std::uint32_t endepin3_clear = 0x1U;
using endepin4 = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t endepin4_disabled = 0x0U;
inline constexpr std::uint32_t endepin4_enabled = 0x1U;
inline constexpr std::uint32_t endepin4_clear = 0x1U;
using endepin5 = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t endepin5_disabled = 0x0U;
inline constexpr std::uint32_t endepin5_enabled = 0x1U;
inline constexpr std::uint32_t endepin5_clear = 0x1U;
using endepin6 = core::field_descriptor<intenclr, 8U, 1U>;
inline constexpr std::uint32_t endepin6_disabled = 0x0U;
inline constexpr std::uint32_t endepin6_enabled = 0x1U;
inline constexpr std::uint32_t endepin6_clear = 0x1U;
using endepin7 = core::field_descriptor<intenclr, 9U, 1U>;
inline constexpr std::uint32_t endepin7_disabled = 0x0U;
inline constexpr std::uint32_t endepin7_enabled = 0x1U;
inline constexpr std::uint32_t endepin7_clear = 0x1U;
using ep0datadone = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t ep0datadone_disabled = 0x0U;
inline constexpr std::uint32_t ep0datadone_enabled = 0x1U;
inline constexpr std::uint32_t ep0datadone_clear = 0x1U;
using endisoin = core::field_descriptor<intenclr, 11U, 1U>;
inline constexpr std::uint32_t endisoin_disabled = 0x0U;
inline constexpr std::uint32_t endisoin_enabled = 0x1U;
inline constexpr std::uint32_t endisoin_clear = 0x1U;
using endepout0 = core::field_descriptor<intenclr, 12U, 1U>;
inline constexpr std::uint32_t endepout0_disabled = 0x0U;
inline constexpr std::uint32_t endepout0_enabled = 0x1U;
inline constexpr std::uint32_t endepout0_clear = 0x1U;
using endepout1 = core::field_descriptor<intenclr, 13U, 1U>;
inline constexpr std::uint32_t endepout1_disabled = 0x0U;
inline constexpr std::uint32_t endepout1_enabled = 0x1U;
inline constexpr std::uint32_t endepout1_clear = 0x1U;
using endepout2 = core::field_descriptor<intenclr, 14U, 1U>;
inline constexpr std::uint32_t endepout2_disabled = 0x0U;
inline constexpr std::uint32_t endepout2_enabled = 0x1U;
inline constexpr std::uint32_t endepout2_clear = 0x1U;
using endepout3 = core::field_descriptor<intenclr, 15U, 1U>;
inline constexpr std::uint32_t endepout3_disabled = 0x0U;
inline constexpr std::uint32_t endepout3_enabled = 0x1U;
inline constexpr std::uint32_t endepout3_clear = 0x1U;
using endepout4 = core::field_descriptor<intenclr, 16U, 1U>;
inline constexpr std::uint32_t endepout4_disabled = 0x0U;
inline constexpr std::uint32_t endepout4_enabled = 0x1U;
inline constexpr std::uint32_t endepout4_clear = 0x1U;
using endepout5 = core::field_descriptor<intenclr, 17U, 1U>;
inline constexpr std::uint32_t endepout5_disabled = 0x0U;
inline constexpr std::uint32_t endepout5_enabled = 0x1U;
inline constexpr std::uint32_t endepout5_clear = 0x1U;
using endepout6 = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t endepout6_disabled = 0x0U;
inline constexpr std::uint32_t endepout6_enabled = 0x1U;
inline constexpr std::uint32_t endepout6_clear = 0x1U;
using endepout7 = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t endepout7_disabled = 0x0U;
inline constexpr std::uint32_t endepout7_enabled = 0x1U;
inline constexpr std::uint32_t endepout7_clear = 0x1U;
using endisoout = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t endisoout_disabled = 0x0U;
inline constexpr std::uint32_t endisoout_enabled = 0x1U;
inline constexpr std::uint32_t endisoout_clear = 0x1U;
using sof = core::field_descriptor<intenclr, 21U, 1U>;
inline constexpr std::uint32_t sof_disabled = 0x0U;
inline constexpr std::uint32_t sof_enabled = 0x1U;
inline constexpr std::uint32_t sof_clear = 0x1U;
using usbevent = core::field_descriptor<intenclr, 22U, 1U>;
inline constexpr std::uint32_t usbevent_disabled = 0x0U;
inline constexpr std::uint32_t usbevent_enabled = 0x1U;
inline constexpr std::uint32_t usbevent_clear = 0x1U;
using ep0setup = core::field_descriptor<intenclr, 23U, 1U>;
inline constexpr std::uint32_t ep0setup_disabled = 0x0U;
inline constexpr std::uint32_t ep0setup_enabled = 0x1U;
inline constexpr std::uint32_t ep0setup_clear = 0x1U;
using epdata = core::field_descriptor<intenclr, 24U, 1U>;
inline constexpr std::uint32_t epdata_disabled = 0x0U;
inline constexpr std::uint32_t epdata_enabled = 0x1U;
inline constexpr std::uint32_t epdata_clear = 0x1U;
}
struct eventcause : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::write_one_to_clear, 0x00000F01U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace eventcause_fields {
using isooutcrc = core::field_descriptor<eventcause, 0U, 1U>;
inline constexpr std::uint32_t isooutcrc_notdetected = 0x0U;
inline constexpr std::uint32_t isooutcrc_detected = 0x1U;
using suspend = core::field_descriptor<eventcause, 8U, 1U>;
inline constexpr std::uint32_t suspend_notdetected = 0x0U;
inline constexpr std::uint32_t suspend_detected = 0x1U;
using resume = core::field_descriptor<eventcause, 9U, 1U>;
inline constexpr std::uint32_t resume_notdetected = 0x0U;
inline constexpr std::uint32_t resume_detected = 0x1U;
using usbwuallowed = core::field_descriptor<eventcause, 10U, 1U>;
inline constexpr std::uint32_t usbwuallowed_notallowed = 0x0U;
inline constexpr std::uint32_t usbwuallowed_allowed = 0x1U;
using ready = core::field_descriptor<eventcause, 11U, 1U>;
inline constexpr std::uint32_t ready_notdetected = 0x0U;
inline constexpr std::uint32_t ready_ready = 0x1U;
}
struct halted_epin0 : core::register_descriptor<std::uint32_t, 0x420U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin0_fields {
using getstatus = core::field_descriptor<halted_epin0, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin1 : core::register_descriptor<std::uint32_t, 0x424U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin1_fields {
using getstatus = core::field_descriptor<halted_epin1, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin2 : core::register_descriptor<std::uint32_t, 0x428U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin2_fields {
using getstatus = core::field_descriptor<halted_epin2, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin3 : core::register_descriptor<std::uint32_t, 0x42CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin3_fields {
using getstatus = core::field_descriptor<halted_epin3, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin4 : core::register_descriptor<std::uint32_t, 0x430U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin4_fields {
using getstatus = core::field_descriptor<halted_epin4, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin5 : core::register_descriptor<std::uint32_t, 0x434U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin5_fields {
using getstatus = core::field_descriptor<halted_epin5, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin6 : core::register_descriptor<std::uint32_t, 0x438U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin6_fields {
using getstatus = core::field_descriptor<halted_epin6, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epin7 : core::register_descriptor<std::uint32_t, 0x43CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epin7_fields {
using getstatus = core::field_descriptor<halted_epin7, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout0 : core::register_descriptor<std::uint32_t, 0x444U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout0_fields {
using getstatus = core::field_descriptor<halted_epout0, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout1 : core::register_descriptor<std::uint32_t, 0x448U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout1_fields {
using getstatus = core::field_descriptor<halted_epout1, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout2 : core::register_descriptor<std::uint32_t, 0x44CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout2_fields {
using getstatus = core::field_descriptor<halted_epout2, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout3 : core::register_descriptor<std::uint32_t, 0x450U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout3_fields {
using getstatus = core::field_descriptor<halted_epout3, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout4 : core::register_descriptor<std::uint32_t, 0x454U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout4_fields {
using getstatus = core::field_descriptor<halted_epout4, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout5 : core::register_descriptor<std::uint32_t, 0x458U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout5_fields {
using getstatus = core::field_descriptor<halted_epout5, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout6 : core::register_descriptor<std::uint32_t, 0x45CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout6_fields {
using getstatus = core::field_descriptor<halted_epout6, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct halted_epout7 : core::register_descriptor<std::uint32_t, 0x460U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace halted_epout7_fields {
using getstatus = core::field_descriptor<halted_epout7, 0U, 16U>;
inline constexpr std::uint32_t getstatus_nothalted = 0x0U;
inline constexpr std::uint32_t getstatus_halted = 0x1U;
}
struct epstatus : core::register_descriptor<std::uint32_t, 0x468U,
    core::access_semantics::write_one_to_clear, 0x01FF01FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epstatus_fields {
using epin0 = core::field_descriptor<epstatus, 0U, 1U>;
inline constexpr std::uint32_t epin0_nodata = 0x0U;
inline constexpr std::uint32_t epin0_datadone = 0x1U;
using epin1 = core::field_descriptor<epstatus, 1U, 1U>;
inline constexpr std::uint32_t epin1_nodata = 0x0U;
inline constexpr std::uint32_t epin1_datadone = 0x1U;
using epin2 = core::field_descriptor<epstatus, 2U, 1U>;
inline constexpr std::uint32_t epin2_nodata = 0x0U;
inline constexpr std::uint32_t epin2_datadone = 0x1U;
using epin3 = core::field_descriptor<epstatus, 3U, 1U>;
inline constexpr std::uint32_t epin3_nodata = 0x0U;
inline constexpr std::uint32_t epin3_datadone = 0x1U;
using epin4 = core::field_descriptor<epstatus, 4U, 1U>;
inline constexpr std::uint32_t epin4_nodata = 0x0U;
inline constexpr std::uint32_t epin4_datadone = 0x1U;
using epin5 = core::field_descriptor<epstatus, 5U, 1U>;
inline constexpr std::uint32_t epin5_nodata = 0x0U;
inline constexpr std::uint32_t epin5_datadone = 0x1U;
using epin6 = core::field_descriptor<epstatus, 6U, 1U>;
inline constexpr std::uint32_t epin6_nodata = 0x0U;
inline constexpr std::uint32_t epin6_datadone = 0x1U;
using epin7 = core::field_descriptor<epstatus, 7U, 1U>;
inline constexpr std::uint32_t epin7_nodata = 0x0U;
inline constexpr std::uint32_t epin7_datadone = 0x1U;
using epin8 = core::field_descriptor<epstatus, 8U, 1U>;
inline constexpr std::uint32_t epin8_nodata = 0x0U;
inline constexpr std::uint32_t epin8_datadone = 0x1U;
using epout0 = core::field_descriptor<epstatus, 16U, 1U>;
inline constexpr std::uint32_t epout0_nodata = 0x0U;
inline constexpr std::uint32_t epout0_datadone = 0x1U;
using epout1 = core::field_descriptor<epstatus, 17U, 1U>;
inline constexpr std::uint32_t epout1_nodata = 0x0U;
inline constexpr std::uint32_t epout1_datadone = 0x1U;
using epout2 = core::field_descriptor<epstatus, 18U, 1U>;
inline constexpr std::uint32_t epout2_nodata = 0x0U;
inline constexpr std::uint32_t epout2_datadone = 0x1U;
using epout3 = core::field_descriptor<epstatus, 19U, 1U>;
inline constexpr std::uint32_t epout3_nodata = 0x0U;
inline constexpr std::uint32_t epout3_datadone = 0x1U;
using epout4 = core::field_descriptor<epstatus, 20U, 1U>;
inline constexpr std::uint32_t epout4_nodata = 0x0U;
inline constexpr std::uint32_t epout4_datadone = 0x1U;
using epout5 = core::field_descriptor<epstatus, 21U, 1U>;
inline constexpr std::uint32_t epout5_nodata = 0x0U;
inline constexpr std::uint32_t epout5_datadone = 0x1U;
using epout6 = core::field_descriptor<epstatus, 22U, 1U>;
inline constexpr std::uint32_t epout6_nodata = 0x0U;
inline constexpr std::uint32_t epout6_datadone = 0x1U;
using epout7 = core::field_descriptor<epstatus, 23U, 1U>;
inline constexpr std::uint32_t epout7_nodata = 0x0U;
inline constexpr std::uint32_t epout7_datadone = 0x1U;
using epout8 = core::field_descriptor<epstatus, 24U, 1U>;
inline constexpr std::uint32_t epout8_nodata = 0x0U;
inline constexpr std::uint32_t epout8_datadone = 0x1U;
}
struct epdatastatus : core::register_descriptor<std::uint32_t, 0x46CU,
    core::access_semantics::write_one_to_clear, 0x00FE00FEU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epdatastatus_fields {
using epin1 = core::field_descriptor<epdatastatus, 1U, 1U>;
inline constexpr std::uint32_t epin1_notdone = 0x0U;
inline constexpr std::uint32_t epin1_datadone = 0x1U;
using epin2 = core::field_descriptor<epdatastatus, 2U, 1U>;
inline constexpr std::uint32_t epin2_notdone = 0x0U;
inline constexpr std::uint32_t epin2_datadone = 0x1U;
using epin3 = core::field_descriptor<epdatastatus, 3U, 1U>;
inline constexpr std::uint32_t epin3_notdone = 0x0U;
inline constexpr std::uint32_t epin3_datadone = 0x1U;
using epin4 = core::field_descriptor<epdatastatus, 4U, 1U>;
inline constexpr std::uint32_t epin4_notdone = 0x0U;
inline constexpr std::uint32_t epin4_datadone = 0x1U;
using epin5 = core::field_descriptor<epdatastatus, 5U, 1U>;
inline constexpr std::uint32_t epin5_notdone = 0x0U;
inline constexpr std::uint32_t epin5_datadone = 0x1U;
using epin6 = core::field_descriptor<epdatastatus, 6U, 1U>;
inline constexpr std::uint32_t epin6_notdone = 0x0U;
inline constexpr std::uint32_t epin6_datadone = 0x1U;
using epin7 = core::field_descriptor<epdatastatus, 7U, 1U>;
inline constexpr std::uint32_t epin7_notdone = 0x0U;
inline constexpr std::uint32_t epin7_datadone = 0x1U;
using epout1 = core::field_descriptor<epdatastatus, 17U, 1U>;
inline constexpr std::uint32_t epout1_notstarted = 0x0U;
inline constexpr std::uint32_t epout1_started = 0x1U;
using epout2 = core::field_descriptor<epdatastatus, 18U, 1U>;
inline constexpr std::uint32_t epout2_notstarted = 0x0U;
inline constexpr std::uint32_t epout2_started = 0x1U;
using epout3 = core::field_descriptor<epdatastatus, 19U, 1U>;
inline constexpr std::uint32_t epout3_notstarted = 0x0U;
inline constexpr std::uint32_t epout3_started = 0x1U;
using epout4 = core::field_descriptor<epdatastatus, 20U, 1U>;
inline constexpr std::uint32_t epout4_notstarted = 0x0U;
inline constexpr std::uint32_t epout4_started = 0x1U;
using epout5 = core::field_descriptor<epdatastatus, 21U, 1U>;
inline constexpr std::uint32_t epout5_notstarted = 0x0U;
inline constexpr std::uint32_t epout5_started = 0x1U;
using epout6 = core::field_descriptor<epdatastatus, 22U, 1U>;
inline constexpr std::uint32_t epout6_notstarted = 0x0U;
inline constexpr std::uint32_t epout6_started = 0x1U;
using epout7 = core::field_descriptor<epdatastatus, 23U, 1U>;
inline constexpr std::uint32_t epout7_notstarted = 0x0U;
inline constexpr std::uint32_t epout7_started = 0x1U;
}
struct usbaddr : core::register_descriptor<std::uint32_t, 0x470U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace usbaddr_fields {
using addr = core::field_descriptor<usbaddr, 0U, 7U>;
}
struct bmrequesttype : core::register_descriptor<std::uint32_t, 0x480U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace bmrequesttype_fields {
using recipient = core::field_descriptor<bmrequesttype, 0U, 5U>;
inline constexpr std::uint32_t recipient_device = 0x0U;
inline constexpr std::uint32_t recipient_interface = 0x1U;
inline constexpr std::uint32_t recipient_endpoint = 0x2U;
inline constexpr std::uint32_t recipient_other = 0x3U;
using type = core::field_descriptor<bmrequesttype, 5U, 2U>;
inline constexpr std::uint32_t type_standard = 0x0U;
inline constexpr std::uint32_t type_reg_class = 0x1U;
inline constexpr std::uint32_t type_vendor = 0x2U;
using direction = core::field_descriptor<bmrequesttype, 7U, 1U>;
inline constexpr std::uint32_t direction_hosttodevice = 0x0U;
inline constexpr std::uint32_t direction_devicetohost = 0x1U;
}
struct brequest : core::register_descriptor<std::uint32_t, 0x484U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace brequest_fields {
using brequest = core::field_descriptor<brequest, 0U, 8U>;
inline constexpr std::uint32_t brequest_std_get_status = 0x0U;
inline constexpr std::uint32_t brequest_std_clear_feature = 0x1U;
inline constexpr std::uint32_t brequest_std_set_feature = 0x3U;
inline constexpr std::uint32_t brequest_std_set_address = 0x5U;
inline constexpr std::uint32_t brequest_std_get_descriptor = 0x6U;
inline constexpr std::uint32_t brequest_std_set_descriptor = 0x7U;
inline constexpr std::uint32_t brequest_std_get_configuration = 0x8U;
inline constexpr std::uint32_t brequest_std_set_configuration = 0x9U;
inline constexpr std::uint32_t brequest_std_get_interface = 0xAU;
inline constexpr std::uint32_t brequest_std_set_interface = 0xBU;
inline constexpr std::uint32_t brequest_std_synch_frame = 0xCU;
}
struct wvaluel : core::register_descriptor<std::uint32_t, 0x488U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace wvaluel_fields {
using wvaluel = core::field_descriptor<wvaluel, 0U, 8U>;
}
struct wvalueh : core::register_descriptor<std::uint32_t, 0x48CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace wvalueh_fields {
using wvalueh = core::field_descriptor<wvalueh, 0U, 8U>;
}
struct windexl : core::register_descriptor<std::uint32_t, 0x490U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace windexl_fields {
using windexl = core::field_descriptor<windexl, 0U, 8U>;
}
struct windexh : core::register_descriptor<std::uint32_t, 0x494U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace windexh_fields {
using windexh = core::field_descriptor<windexh, 0U, 8U>;
}
struct wlengthl : core::register_descriptor<std::uint32_t, 0x498U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace wlengthl_fields {
using wlengthl = core::field_descriptor<wlengthl, 0U, 8U>;
}
struct wlengthh : core::register_descriptor<std::uint32_t, 0x49CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace wlengthh_fields {
using wlengthh = core::field_descriptor<wlengthh, 0U, 8U>;
}
struct size_epout0 : core::register_descriptor<std::uint32_t, 0x4A0U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout0_fields {
using size = core::field_descriptor<size_epout0, 0U, 7U>;
}
struct size_epout1 : core::register_descriptor<std::uint32_t, 0x4A4U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout1_fields {
using size = core::field_descriptor<size_epout1, 0U, 7U>;
}
struct size_epout2 : core::register_descriptor<std::uint32_t, 0x4A8U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout2_fields {
using size = core::field_descriptor<size_epout2, 0U, 7U>;
}
struct size_epout3 : core::register_descriptor<std::uint32_t, 0x4ACU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout3_fields {
using size = core::field_descriptor<size_epout3, 0U, 7U>;
}
struct size_epout4 : core::register_descriptor<std::uint32_t, 0x4B0U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout4_fields {
using size = core::field_descriptor<size_epout4, 0U, 7U>;
}
struct size_epout5 : core::register_descriptor<std::uint32_t, 0x4B4U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout5_fields {
using size = core::field_descriptor<size_epout5, 0U, 7U>;
}
struct size_epout6 : core::register_descriptor<std::uint32_t, 0x4B8U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout6_fields {
using size = core::field_descriptor<size_epout6, 0U, 7U>;
}
struct size_epout7 : core::register_descriptor<std::uint32_t, 0x4BCU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_epout7_fields {
using size = core::field_descriptor<size_epout7, 0U, 7U>;
}
struct size_isoout : core::register_descriptor<std::uint32_t, 0x4C0U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00010000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace size_isoout_fields {
using size = core::field_descriptor<size_isoout, 0U, 10U>;
using zero = core::field_descriptor<size_isoout, 16U, 1U>;
inline constexpr std::uint32_t zero_normal = 0x0U;
inline constexpr std::uint32_t zero_zerodata = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct usbpullup : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace usbpullup_fields {
using connect = core::field_descriptor<usbpullup, 0U, 1U>;
inline constexpr std::uint32_t connect_disabled = 0x0U;
inline constexpr std::uint32_t connect_enabled = 0x1U;
}
struct dpdmvalue : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dpdmvalue_fields {
using state = core::field_descriptor<dpdmvalue, 0U, 5U>;
inline constexpr std::uint32_t state_resume = 0x1U;
inline constexpr std::uint32_t state_j = 0x2U;
inline constexpr std::uint32_t state_k = 0x4U;
}
struct dtoggle : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x00000387U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000100U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dtoggle_fields {
using ep = core::field_descriptor<dtoggle, 0U, 3U>;
using io = core::field_descriptor<dtoggle, 7U, 1U>;
inline constexpr std::uint32_t io_reg_out = 0x0U;
inline constexpr std::uint32_t io_reg_in = 0x1U;
using value = core::field_descriptor<dtoggle, 8U, 2U>;
inline constexpr std::uint32_t value_nop = 0x0U;
inline constexpr std::uint32_t value_data0 = 0x1U;
inline constexpr std::uint32_t value_data1 = 0x2U;
}
struct epinen : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x000001FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epinen_fields {
using in0 = core::field_descriptor<epinen, 0U, 1U>;
inline constexpr std::uint32_t in0_disable = 0x0U;
inline constexpr std::uint32_t in0_enable = 0x1U;
using in1 = core::field_descriptor<epinen, 1U, 1U>;
inline constexpr std::uint32_t in1_disable = 0x0U;
inline constexpr std::uint32_t in1_enable = 0x1U;
using in2 = core::field_descriptor<epinen, 2U, 1U>;
inline constexpr std::uint32_t in2_disable = 0x0U;
inline constexpr std::uint32_t in2_enable = 0x1U;
using in3 = core::field_descriptor<epinen, 3U, 1U>;
inline constexpr std::uint32_t in3_disable = 0x0U;
inline constexpr std::uint32_t in3_enable = 0x1U;
using in4 = core::field_descriptor<epinen, 4U, 1U>;
inline constexpr std::uint32_t in4_disable = 0x0U;
inline constexpr std::uint32_t in4_enable = 0x1U;
using in5 = core::field_descriptor<epinen, 5U, 1U>;
inline constexpr std::uint32_t in5_disable = 0x0U;
inline constexpr std::uint32_t in5_enable = 0x1U;
using in6 = core::field_descriptor<epinen, 6U, 1U>;
inline constexpr std::uint32_t in6_disable = 0x0U;
inline constexpr std::uint32_t in6_enable = 0x1U;
using in7 = core::field_descriptor<epinen, 7U, 1U>;
inline constexpr std::uint32_t in7_disable = 0x0U;
inline constexpr std::uint32_t in7_enable = 0x1U;
using isoin = core::field_descriptor<epinen, 8U, 1U>;
inline constexpr std::uint32_t isoin_disable = 0x0U;
inline constexpr std::uint32_t isoin_enable = 0x1U;
}
struct epouten : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x000001FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epouten_fields {
using out0 = core::field_descriptor<epouten, 0U, 1U>;
inline constexpr std::uint32_t out0_disable = 0x0U;
inline constexpr std::uint32_t out0_enable = 0x1U;
using out1 = core::field_descriptor<epouten, 1U, 1U>;
inline constexpr std::uint32_t out1_disable = 0x0U;
inline constexpr std::uint32_t out1_enable = 0x1U;
using out2 = core::field_descriptor<epouten, 2U, 1U>;
inline constexpr std::uint32_t out2_disable = 0x0U;
inline constexpr std::uint32_t out2_enable = 0x1U;
using out3 = core::field_descriptor<epouten, 3U, 1U>;
inline constexpr std::uint32_t out3_disable = 0x0U;
inline constexpr std::uint32_t out3_enable = 0x1U;
using out4 = core::field_descriptor<epouten, 4U, 1U>;
inline constexpr std::uint32_t out4_disable = 0x0U;
inline constexpr std::uint32_t out4_enable = 0x1U;
using out5 = core::field_descriptor<epouten, 5U, 1U>;
inline constexpr std::uint32_t out5_disable = 0x0U;
inline constexpr std::uint32_t out5_enable = 0x1U;
using out6 = core::field_descriptor<epouten, 6U, 1U>;
inline constexpr std::uint32_t out6_disable = 0x0U;
inline constexpr std::uint32_t out6_enable = 0x1U;
using out7 = core::field_descriptor<epouten, 7U, 1U>;
inline constexpr std::uint32_t out7_disable = 0x0U;
inline constexpr std::uint32_t out7_enable = 0x1U;
using isoout = core::field_descriptor<epouten, 8U, 1U>;
inline constexpr std::uint32_t isoout_disable = 0x0U;
inline constexpr std::uint32_t isoout_enable = 0x1U;
}
struct epstall : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::write_only, 0x00000187U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epstall_fields {
using ep = core::field_descriptor<epstall, 0U, 3U>;
using io = core::field_descriptor<epstall, 7U, 1U>;
inline constexpr std::uint32_t io_reg_out = 0x0U;
inline constexpr std::uint32_t io_reg_in = 0x1U;
using stall = core::field_descriptor<epstall, 8U, 1U>;
inline constexpr std::uint32_t stall_unstall = 0x0U;
inline constexpr std::uint32_t stall_stall = 0x1U;
}
struct isosplit : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isosplit_fields {
using split = core::field_descriptor<isosplit, 0U, 16U>;
inline constexpr std::uint32_t split_onedir = 0x0U;
inline constexpr std::uint32_t split_halfin = 0x80U;
}
struct framecntr : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace framecntr_fields {
using framecntr = core::field_descriptor<framecntr, 0U, 11U>;
}
struct lowpower : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace lowpower_fields {
using lowpower = core::field_descriptor<lowpower, 0U, 1U>;
inline constexpr std::uint32_t lowpower_forcenormal = 0x0U;
inline constexpr std::uint32_t lowpower_lowpower = 0x1U;
}
struct isoinconfig : core::register_descriptor<std::uint32_t, 0x530U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoinconfig_fields {
using response = core::field_descriptor<isoinconfig, 0U, 1U>;
inline constexpr std::uint32_t response_noresp = 0x0U;
inline constexpr std::uint32_t response_zerodata = 0x1U;
}
struct epin0_ptr : core::register_descriptor<std::uint32_t, 0x600U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin0_ptr_fields {
using ptr = core::field_descriptor<epin0_ptr, 0U, 32U>;
}
struct epin0_maxcnt : core::register_descriptor<std::uint32_t, 0x604U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin0_maxcnt_fields {
using maxcnt = core::field_descriptor<epin0_maxcnt, 0U, 7U>;
}
struct epin0_amount : core::register_descriptor<std::uint32_t, 0x608U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin0_amount_fields {
using amount = core::field_descriptor<epin0_amount, 0U, 7U>;
}
struct epin1_ptr : core::register_descriptor<std::uint32_t, 0x614U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin1_ptr_fields {
using ptr = core::field_descriptor<epin1_ptr, 0U, 32U>;
}
struct epin1_maxcnt : core::register_descriptor<std::uint32_t, 0x618U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin1_maxcnt_fields {
using maxcnt = core::field_descriptor<epin1_maxcnt, 0U, 7U>;
}
struct epin1_amount : core::register_descriptor<std::uint32_t, 0x61CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin1_amount_fields {
using amount = core::field_descriptor<epin1_amount, 0U, 7U>;
}
struct epin2_ptr : core::register_descriptor<std::uint32_t, 0x628U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin2_ptr_fields {
using ptr = core::field_descriptor<epin2_ptr, 0U, 32U>;
}
struct epin2_maxcnt : core::register_descriptor<std::uint32_t, 0x62CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin2_maxcnt_fields {
using maxcnt = core::field_descriptor<epin2_maxcnt, 0U, 7U>;
}
struct epin2_amount : core::register_descriptor<std::uint32_t, 0x630U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin2_amount_fields {
using amount = core::field_descriptor<epin2_amount, 0U, 7U>;
}
struct epin3_ptr : core::register_descriptor<std::uint32_t, 0x63CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin3_ptr_fields {
using ptr = core::field_descriptor<epin3_ptr, 0U, 32U>;
}
struct epin3_maxcnt : core::register_descriptor<std::uint32_t, 0x640U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin3_maxcnt_fields {
using maxcnt = core::field_descriptor<epin3_maxcnt, 0U, 7U>;
}
struct epin3_amount : core::register_descriptor<std::uint32_t, 0x644U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin3_amount_fields {
using amount = core::field_descriptor<epin3_amount, 0U, 7U>;
}
struct epin4_ptr : core::register_descriptor<std::uint32_t, 0x650U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin4_ptr_fields {
using ptr = core::field_descriptor<epin4_ptr, 0U, 32U>;
}
struct epin4_maxcnt : core::register_descriptor<std::uint32_t, 0x654U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin4_maxcnt_fields {
using maxcnt = core::field_descriptor<epin4_maxcnt, 0U, 7U>;
}
struct epin4_amount : core::register_descriptor<std::uint32_t, 0x658U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin4_amount_fields {
using amount = core::field_descriptor<epin4_amount, 0U, 7U>;
}
struct epin5_ptr : core::register_descriptor<std::uint32_t, 0x664U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin5_ptr_fields {
using ptr = core::field_descriptor<epin5_ptr, 0U, 32U>;
}
struct epin5_maxcnt : core::register_descriptor<std::uint32_t, 0x668U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin5_maxcnt_fields {
using maxcnt = core::field_descriptor<epin5_maxcnt, 0U, 7U>;
}
struct epin5_amount : core::register_descriptor<std::uint32_t, 0x66CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin5_amount_fields {
using amount = core::field_descriptor<epin5_amount, 0U, 7U>;
}
struct epin6_ptr : core::register_descriptor<std::uint32_t, 0x678U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin6_ptr_fields {
using ptr = core::field_descriptor<epin6_ptr, 0U, 32U>;
}
struct epin6_maxcnt : core::register_descriptor<std::uint32_t, 0x67CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin6_maxcnt_fields {
using maxcnt = core::field_descriptor<epin6_maxcnt, 0U, 7U>;
}
struct epin6_amount : core::register_descriptor<std::uint32_t, 0x680U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin6_amount_fields {
using amount = core::field_descriptor<epin6_amount, 0U, 7U>;
}
struct epin7_ptr : core::register_descriptor<std::uint32_t, 0x68CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin7_ptr_fields {
using ptr = core::field_descriptor<epin7_ptr, 0U, 32U>;
}
struct epin7_maxcnt : core::register_descriptor<std::uint32_t, 0x690U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin7_maxcnt_fields {
using maxcnt = core::field_descriptor<epin7_maxcnt, 0U, 7U>;
}
struct epin7_amount : core::register_descriptor<std::uint32_t, 0x694U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epin7_amount_fields {
using amount = core::field_descriptor<epin7_amount, 0U, 7U>;
}
struct isoin_ptr : core::register_descriptor<std::uint32_t, 0x6A0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoin_ptr_fields {
using ptr = core::field_descriptor<isoin_ptr, 0U, 32U>;
}
struct isoin_maxcnt : core::register_descriptor<std::uint32_t, 0x6A4U,
    core::access_semantics::read_write, 0x000003FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoin_maxcnt_fields {
using maxcnt = core::field_descriptor<isoin_maxcnt, 0U, 10U>;
}
struct isoin_amount : core::register_descriptor<std::uint32_t, 0x6A8U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoin_amount_fields {
using amount = core::field_descriptor<isoin_amount, 0U, 10U>;
}
struct epout0_ptr : core::register_descriptor<std::uint32_t, 0x700U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout0_ptr_fields {
using ptr = core::field_descriptor<epout0_ptr, 0U, 32U>;
}
struct epout0_maxcnt : core::register_descriptor<std::uint32_t, 0x704U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout0_maxcnt_fields {
using maxcnt = core::field_descriptor<epout0_maxcnt, 0U, 7U>;
}
struct epout0_amount : core::register_descriptor<std::uint32_t, 0x708U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout0_amount_fields {
using amount = core::field_descriptor<epout0_amount, 0U, 7U>;
}
struct epout1_ptr : core::register_descriptor<std::uint32_t, 0x714U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout1_ptr_fields {
using ptr = core::field_descriptor<epout1_ptr, 0U, 32U>;
}
struct epout1_maxcnt : core::register_descriptor<std::uint32_t, 0x718U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout1_maxcnt_fields {
using maxcnt = core::field_descriptor<epout1_maxcnt, 0U, 7U>;
}
struct epout1_amount : core::register_descriptor<std::uint32_t, 0x71CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout1_amount_fields {
using amount = core::field_descriptor<epout1_amount, 0U, 7U>;
}
struct epout2_ptr : core::register_descriptor<std::uint32_t, 0x728U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout2_ptr_fields {
using ptr = core::field_descriptor<epout2_ptr, 0U, 32U>;
}
struct epout2_maxcnt : core::register_descriptor<std::uint32_t, 0x72CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout2_maxcnt_fields {
using maxcnt = core::field_descriptor<epout2_maxcnt, 0U, 7U>;
}
struct epout2_amount : core::register_descriptor<std::uint32_t, 0x730U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout2_amount_fields {
using amount = core::field_descriptor<epout2_amount, 0U, 7U>;
}
struct epout3_ptr : core::register_descriptor<std::uint32_t, 0x73CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout3_ptr_fields {
using ptr = core::field_descriptor<epout3_ptr, 0U, 32U>;
}
struct epout3_maxcnt : core::register_descriptor<std::uint32_t, 0x740U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout3_maxcnt_fields {
using maxcnt = core::field_descriptor<epout3_maxcnt, 0U, 7U>;
}
struct epout3_amount : core::register_descriptor<std::uint32_t, 0x744U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout3_amount_fields {
using amount = core::field_descriptor<epout3_amount, 0U, 7U>;
}
struct epout4_ptr : core::register_descriptor<std::uint32_t, 0x750U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout4_ptr_fields {
using ptr = core::field_descriptor<epout4_ptr, 0U, 32U>;
}
struct epout4_maxcnt : core::register_descriptor<std::uint32_t, 0x754U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout4_maxcnt_fields {
using maxcnt = core::field_descriptor<epout4_maxcnt, 0U, 7U>;
}
struct epout4_amount : core::register_descriptor<std::uint32_t, 0x758U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout4_amount_fields {
using amount = core::field_descriptor<epout4_amount, 0U, 7U>;
}
struct epout5_ptr : core::register_descriptor<std::uint32_t, 0x764U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout5_ptr_fields {
using ptr = core::field_descriptor<epout5_ptr, 0U, 32U>;
}
struct epout5_maxcnt : core::register_descriptor<std::uint32_t, 0x768U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout5_maxcnt_fields {
using maxcnt = core::field_descriptor<epout5_maxcnt, 0U, 7U>;
}
struct epout5_amount : core::register_descriptor<std::uint32_t, 0x76CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout5_amount_fields {
using amount = core::field_descriptor<epout5_amount, 0U, 7U>;
}
struct epout6_ptr : core::register_descriptor<std::uint32_t, 0x778U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout6_ptr_fields {
using ptr = core::field_descriptor<epout6_ptr, 0U, 32U>;
}
struct epout6_maxcnt : core::register_descriptor<std::uint32_t, 0x77CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout6_maxcnt_fields {
using maxcnt = core::field_descriptor<epout6_maxcnt, 0U, 7U>;
}
struct epout6_amount : core::register_descriptor<std::uint32_t, 0x780U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout6_amount_fields {
using amount = core::field_descriptor<epout6_amount, 0U, 7U>;
}
struct epout7_ptr : core::register_descriptor<std::uint32_t, 0x78CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout7_ptr_fields {
using ptr = core::field_descriptor<epout7_ptr, 0U, 32U>;
}
struct epout7_maxcnt : core::register_descriptor<std::uint32_t, 0x790U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout7_maxcnt_fields {
using maxcnt = core::field_descriptor<epout7_maxcnt, 0U, 7U>;
}
struct epout7_amount : core::register_descriptor<std::uint32_t, 0x794U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace epout7_amount_fields {
using amount = core::field_descriptor<epout7_amount, 0U, 7U>;
}
struct isoout_ptr : core::register_descriptor<std::uint32_t, 0x7A0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoout_ptr_fields {
using ptr = core::field_descriptor<isoout_ptr, 0U, 32U>;
}
struct isoout_maxcnt : core::register_descriptor<std::uint32_t, 0x7A4U,
    core::access_semantics::read_write, 0x000003FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoout_maxcnt_fields {
using maxcnt = core::field_descriptor<isoout_maxcnt, 0U, 10U>;
}
struct isoout_amount : core::register_descriptor<std::uint32_t, 0x7A8U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace isoout_amount_fields {
using amount = core::field_descriptor<isoout_amount, 0U, 10U>;
}
}
