#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::gpiote {
struct tasks_out0 : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out0_fields {
using tasks_out = core::field_descriptor<tasks_out0, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out1 : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out1_fields {
using tasks_out = core::field_descriptor<tasks_out1, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out2 : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out2_fields {
using tasks_out = core::field_descriptor<tasks_out2, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out3 : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out3_fields {
using tasks_out = core::field_descriptor<tasks_out3, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out4 : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out4_fields {
using tasks_out = core::field_descriptor<tasks_out4, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out5 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out5_fields {
using tasks_out = core::field_descriptor<tasks_out5, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out6 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out6_fields {
using tasks_out = core::field_descriptor<tasks_out6, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_out7 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_out7_fields {
using tasks_out = core::field_descriptor<tasks_out7, 0U, 1U>;
inline constexpr std::uint32_t tasks_out_trigger = 0x1U;
}
struct tasks_set0 : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set0_fields {
using tasks_set = core::field_descriptor<tasks_set0, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set1 : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set1_fields {
using tasks_set = core::field_descriptor<tasks_set1, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set2 : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set2_fields {
using tasks_set = core::field_descriptor<tasks_set2, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set3 : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set3_fields {
using tasks_set = core::field_descriptor<tasks_set3, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set4 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set4_fields {
using tasks_set = core::field_descriptor<tasks_set4, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set5 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set5_fields {
using tasks_set = core::field_descriptor<tasks_set5, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set6 : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set6_fields {
using tasks_set = core::field_descriptor<tasks_set6, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_set7 : core::register_descriptor<std::uint32_t, 0x4CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_set7_fields {
using tasks_set = core::field_descriptor<tasks_set7, 0U, 1U>;
inline constexpr std::uint32_t tasks_set_trigger = 0x1U;
}
struct tasks_clr0 : core::register_descriptor<std::uint32_t, 0x60U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr0_fields {
using tasks_clr = core::field_descriptor<tasks_clr0, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr1 : core::register_descriptor<std::uint32_t, 0x64U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr1_fields {
using tasks_clr = core::field_descriptor<tasks_clr1, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr2 : core::register_descriptor<std::uint32_t, 0x68U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr2_fields {
using tasks_clr = core::field_descriptor<tasks_clr2, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr3 : core::register_descriptor<std::uint32_t, 0x6CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr3_fields {
using tasks_clr = core::field_descriptor<tasks_clr3, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr4 : core::register_descriptor<std::uint32_t, 0x70U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr4_fields {
using tasks_clr = core::field_descriptor<tasks_clr4, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr5 : core::register_descriptor<std::uint32_t, 0x74U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr5_fields {
using tasks_clr = core::field_descriptor<tasks_clr5, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr6 : core::register_descriptor<std::uint32_t, 0x78U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr6_fields {
using tasks_clr = core::field_descriptor<tasks_clr6, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct tasks_clr7 : core::register_descriptor<std::uint32_t, 0x7CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_clr7_fields {
using tasks_clr = core::field_descriptor<tasks_clr7, 0U, 1U>;
inline constexpr std::uint32_t tasks_clr_trigger = 0x1U;
}
struct events_in0 : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in0_fields {
using events_in = core::field_descriptor<events_in0, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in1 : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in1_fields {
using events_in = core::field_descriptor<events_in1, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in2 : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in2_fields {
using events_in = core::field_descriptor<events_in2, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in3 : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in3_fields {
using events_in = core::field_descriptor<events_in3, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in4 : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in4_fields {
using events_in = core::field_descriptor<events_in4, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in5 : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in5_fields {
using events_in = core::field_descriptor<events_in5, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in6 : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in6_fields {
using events_in = core::field_descriptor<events_in6, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_in7 : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_in7_fields {
using events_in = core::field_descriptor<events_in7, 0U, 1U>;
inline constexpr std::uint32_t events_in_notgenerated = 0x0U;
inline constexpr std::uint32_t events_in_generated = 0x1U;
}
struct events_port : core::register_descriptor<std::uint32_t, 0x17CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_port_fields {
using events_port = core::field_descriptor<events_port, 0U, 1U>;
inline constexpr std::uint32_t events_port_notgenerated = 0x0U;
inline constexpr std::uint32_t events_port_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x800000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using in0 = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t in0_disabled = 0x0U;
inline constexpr std::uint32_t in0_enabled = 0x1U;
inline constexpr std::uint32_t in0_set = 0x1U;
using in1 = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t in1_disabled = 0x0U;
inline constexpr std::uint32_t in1_enabled = 0x1U;
inline constexpr std::uint32_t in1_set = 0x1U;
using in2 = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t in2_disabled = 0x0U;
inline constexpr std::uint32_t in2_enabled = 0x1U;
inline constexpr std::uint32_t in2_set = 0x1U;
using in3 = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t in3_disabled = 0x0U;
inline constexpr std::uint32_t in3_enabled = 0x1U;
inline constexpr std::uint32_t in3_set = 0x1U;
using in4 = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t in4_disabled = 0x0U;
inline constexpr std::uint32_t in4_enabled = 0x1U;
inline constexpr std::uint32_t in4_set = 0x1U;
using in5 = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t in5_disabled = 0x0U;
inline constexpr std::uint32_t in5_enabled = 0x1U;
inline constexpr std::uint32_t in5_set = 0x1U;
using in6 = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t in6_disabled = 0x0U;
inline constexpr std::uint32_t in6_enabled = 0x1U;
inline constexpr std::uint32_t in6_set = 0x1U;
using in7 = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t in7_disabled = 0x0U;
inline constexpr std::uint32_t in7_enabled = 0x1U;
inline constexpr std::uint32_t in7_set = 0x1U;
using port = core::field_descriptor<intenset, 31U, 1U>;
inline constexpr std::uint32_t port_disabled = 0x0U;
inline constexpr std::uint32_t port_enabled = 0x1U;
inline constexpr std::uint32_t port_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x800000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using in0 = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t in0_disabled = 0x0U;
inline constexpr std::uint32_t in0_enabled = 0x1U;
inline constexpr std::uint32_t in0_clear = 0x1U;
using in1 = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t in1_disabled = 0x0U;
inline constexpr std::uint32_t in1_enabled = 0x1U;
inline constexpr std::uint32_t in1_clear = 0x1U;
using in2 = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t in2_disabled = 0x0U;
inline constexpr std::uint32_t in2_enabled = 0x1U;
inline constexpr std::uint32_t in2_clear = 0x1U;
using in3 = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t in3_disabled = 0x0U;
inline constexpr std::uint32_t in3_enabled = 0x1U;
inline constexpr std::uint32_t in3_clear = 0x1U;
using in4 = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t in4_disabled = 0x0U;
inline constexpr std::uint32_t in4_enabled = 0x1U;
inline constexpr std::uint32_t in4_clear = 0x1U;
using in5 = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t in5_disabled = 0x0U;
inline constexpr std::uint32_t in5_enabled = 0x1U;
inline constexpr std::uint32_t in5_clear = 0x1U;
using in6 = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t in6_disabled = 0x0U;
inline constexpr std::uint32_t in6_enabled = 0x1U;
inline constexpr std::uint32_t in6_clear = 0x1U;
using in7 = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t in7_disabled = 0x0U;
inline constexpr std::uint32_t in7_enabled = 0x1U;
inline constexpr std::uint32_t in7_clear = 0x1U;
using port = core::field_descriptor<intenclr, 31U, 1U>;
inline constexpr std::uint32_t port_disabled = 0x0U;
inline constexpr std::uint32_t port_enabled = 0x1U;
inline constexpr std::uint32_t port_clear = 0x1U;
}
struct config0 : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config0_fields {
using mode = core::field_descriptor<config0, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config0, 8U, 5U>;
using port = core::field_descriptor<config0, 13U, 1U>;
using polarity = core::field_descriptor<config0, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config0, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config1 : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config1_fields {
using mode = core::field_descriptor<config1, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config1, 8U, 5U>;
using port = core::field_descriptor<config1, 13U, 1U>;
using polarity = core::field_descriptor<config1, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config1, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config2 : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config2_fields {
using mode = core::field_descriptor<config2, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config2, 8U, 5U>;
using port = core::field_descriptor<config2, 13U, 1U>;
using polarity = core::field_descriptor<config2, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config2, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config3 : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config3_fields {
using mode = core::field_descriptor<config3, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config3, 8U, 5U>;
using port = core::field_descriptor<config3, 13U, 1U>;
using polarity = core::field_descriptor<config3, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config3, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config4 : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config4_fields {
using mode = core::field_descriptor<config4, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config4, 8U, 5U>;
using port = core::field_descriptor<config4, 13U, 1U>;
using polarity = core::field_descriptor<config4, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config4, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config5 : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config5_fields {
using mode = core::field_descriptor<config5, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config5, 8U, 5U>;
using port = core::field_descriptor<config5, 13U, 1U>;
using polarity = core::field_descriptor<config5, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config5, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config6 : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config6_fields {
using mode = core::field_descriptor<config6, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config6, 8U, 5U>;
using port = core::field_descriptor<config6, 13U, 1U>;
using polarity = core::field_descriptor<config6, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config6, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
struct config7 : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0x00133F03U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config7_fields {
using mode = core::field_descriptor<config7, 0U, 2U>;
inline constexpr std::uint32_t mode_disabled = 0x0U;
inline constexpr std::uint32_t mode_event = 0x1U;
inline constexpr std::uint32_t mode_task = 0x3U;
using psel = core::field_descriptor<config7, 8U, 5U>;
using port = core::field_descriptor<config7, 13U, 1U>;
using polarity = core::field_descriptor<config7, 16U, 2U>;
inline constexpr std::uint32_t polarity_none = 0x0U;
inline constexpr std::uint32_t polarity_lotohi = 0x1U;
inline constexpr std::uint32_t polarity_hitolo = 0x2U;
inline constexpr std::uint32_t polarity_toggle = 0x3U;
using outinit = core::field_descriptor<config7, 20U, 1U>;
inline constexpr std::uint32_t outinit_low = 0x0U;
inline constexpr std::uint32_t outinit_high = 0x1U;
}
}
