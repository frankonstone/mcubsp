#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::lpcomp {
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
struct tasks_sample : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_sample_fields {
using tasks_sample = core::field_descriptor<tasks_sample, 0U, 1U>;
inline constexpr std::uint32_t tasks_sample_trigger = 0x1U;
}
struct events_ready : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ready_fields {
using events_ready = core::field_descriptor<events_ready, 0U, 1U>;
inline constexpr std::uint32_t events_ready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ready_generated = 0x1U;
}
struct events_down : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_down_fields {
using events_down = core::field_descriptor<events_down, 0U, 1U>;
inline constexpr std::uint32_t events_down_notgenerated = 0x0U;
inline constexpr std::uint32_t events_down_generated = 0x1U;
}
struct events_up : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_up_fields {
using events_up = core::field_descriptor<events_up, 0U, 1U>;
inline constexpr std::uint32_t events_up_notgenerated = 0x0U;
inline constexpr std::uint32_t events_up_generated = 0x1U;
}
struct events_cross : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_cross_fields {
using events_cross = core::field_descriptor<events_cross, 0U, 1U>;
inline constexpr std::uint32_t events_cross_notgenerated = 0x0U;
inline constexpr std::uint32_t events_cross_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using ready_sample = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t ready_sample_disabled = 0x0U;
inline constexpr std::uint32_t ready_sample_enabled = 0x1U;
using ready_stop = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t ready_stop_disabled = 0x0U;
inline constexpr std::uint32_t ready_stop_enabled = 0x1U;
using down_stop = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t down_stop_disabled = 0x0U;
inline constexpr std::uint32_t down_stop_enabled = 0x1U;
using up_stop = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t up_stop_disabled = 0x0U;
inline constexpr std::uint32_t up_stop_enabled = 0x1U;
using cross_stop = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t cross_stop_disabled = 0x0U;
inline constexpr std::uint32_t cross_stop_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using ready = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_set = 0x1U;
using down = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t down_disabled = 0x0U;
inline constexpr std::uint32_t down_enabled = 0x1U;
inline constexpr std::uint32_t down_set = 0x1U;
using up = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t up_disabled = 0x0U;
inline constexpr std::uint32_t up_enabled = 0x1U;
inline constexpr std::uint32_t up_set = 0x1U;
using cross = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t cross_disabled = 0x0U;
inline constexpr std::uint32_t cross_enabled = 0x1U;
inline constexpr std::uint32_t cross_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using ready = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_clear = 0x1U;
using down = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t down_disabled = 0x0U;
inline constexpr std::uint32_t down_enabled = 0x1U;
inline constexpr std::uint32_t down_clear = 0x1U;
using up = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t up_disabled = 0x0U;
inline constexpr std::uint32_t up_enabled = 0x1U;
inline constexpr std::uint32_t up_clear = 0x1U;
using cross = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t cross_disabled = 0x0U;
inline constexpr std::uint32_t cross_enabled = 0x1U;
inline constexpr std::uint32_t cross_clear = 0x1U;
}
struct result : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace result_fields {
using result = core::field_descriptor<result, 0U, 1U>;
inline constexpr std::uint32_t result_below = 0x0U;
inline constexpr std::uint32_t result_above = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 2U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct psel : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_fields {
using psel = core::field_descriptor<psel, 0U, 3U>;
inline constexpr std::uint32_t psel_analoginput0 = 0x0U;
inline constexpr std::uint32_t psel_analoginput1 = 0x1U;
inline constexpr std::uint32_t psel_analoginput2 = 0x2U;
inline constexpr std::uint32_t psel_analoginput3 = 0x3U;
inline constexpr std::uint32_t psel_analoginput4 = 0x4U;
inline constexpr std::uint32_t psel_analoginput5 = 0x5U;
inline constexpr std::uint32_t psel_analoginput6 = 0x6U;
inline constexpr std::uint32_t psel_analoginput7 = 0x7U;
}
struct refsel : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000004U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace refsel_fields {
using refsel = core::field_descriptor<refsel, 0U, 4U>;
inline constexpr std::uint32_t refsel_ref1_8vdd = 0x0U;
inline constexpr std::uint32_t refsel_ref2_8vdd = 0x1U;
inline constexpr std::uint32_t refsel_ref3_8vdd = 0x2U;
inline constexpr std::uint32_t refsel_ref4_8vdd = 0x3U;
inline constexpr std::uint32_t refsel_ref5_8vdd = 0x4U;
inline constexpr std::uint32_t refsel_ref6_8vdd = 0x5U;
inline constexpr std::uint32_t refsel_ref7_8vdd = 0x6U;
inline constexpr std::uint32_t refsel_aref = 0x7U;
inline constexpr std::uint32_t refsel_ref1_16vdd = 0x8U;
inline constexpr std::uint32_t refsel_ref3_16vdd = 0x9U;
inline constexpr std::uint32_t refsel_ref5_16vdd = 0xAU;
inline constexpr std::uint32_t refsel_ref7_16vdd = 0xBU;
inline constexpr std::uint32_t refsel_ref9_16vdd = 0xCU;
inline constexpr std::uint32_t refsel_ref11_16vdd = 0xDU;
inline constexpr std::uint32_t refsel_ref13_16vdd = 0xEU;
inline constexpr std::uint32_t refsel_ref15_16vdd = 0xFU;
}
struct extrefsel : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace extrefsel_fields {
using extrefsel = core::field_descriptor<extrefsel, 0U, 1U>;
inline constexpr std::uint32_t extrefsel_analogreference0 = 0x0U;
inline constexpr std::uint32_t extrefsel_analogreference1 = 0x1U;
}
struct anadetect : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace anadetect_fields {
using anadetect = core::field_descriptor<anadetect, 0U, 2U>;
inline constexpr std::uint32_t anadetect_cross = 0x0U;
inline constexpr std::uint32_t anadetect_up = 0x1U;
inline constexpr std::uint32_t anadetect_down = 0x2U;
}
struct hyst : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace hyst_fields {
using hyst = core::field_descriptor<hyst, 0U, 1U>;
inline constexpr std::uint32_t hyst_disabled = 0x0U;
inline constexpr std::uint32_t hyst_enabled = 0x1U;
}
}
