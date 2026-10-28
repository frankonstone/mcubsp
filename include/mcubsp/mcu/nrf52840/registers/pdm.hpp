#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::pdm {
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
struct events_started : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_started_fields {
using events_started = core::field_descriptor<events_started, 0U, 1U>;
inline constexpr std::uint32_t events_started_notgenerated = 0x0U;
inline constexpr std::uint32_t events_started_generated = 0x1U;
}
struct events_stopped : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_stopped_fields {
using events_stopped = core::field_descriptor<events_stopped, 0U, 1U>;
inline constexpr std::uint32_t events_stopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_stopped_generated = 0x1U;
}
struct events_end : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_end_fields {
using events_end = core::field_descriptor<events_end, 0U, 1U>;
inline constexpr std::uint32_t events_end_notgenerated = 0x0U;
inline constexpr std::uint32_t events_end_generated = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using started = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
using stopped = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
using end = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using started = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_set = 0x1U;
using stopped = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using end = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using started = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_clear = 0x1U;
using stopped = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using end = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct pdmclkctrl : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x08400000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pdmclkctrl_fields {
using freq = core::field_descriptor<pdmclkctrl, 0U, 32U>;
inline constexpr std::uint32_t freq_reg_1000k = 0x8000000U;
inline constexpr std::uint32_t freq_reg_default = 0x8400000U;
inline constexpr std::uint32_t freq_reg_1067k = 0x8800000U;
inline constexpr std::uint32_t freq_reg_1231k = 0x9800000U;
inline constexpr std::uint32_t freq_reg_1280k = 0xA000000U;
inline constexpr std::uint32_t freq_reg_1333k = 0xA800000U;
}
struct mode : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mode_fields {
using operation = core::field_descriptor<mode, 0U, 1U>;
inline constexpr std::uint32_t operation_stereo = 0x0U;
inline constexpr std::uint32_t operation_mono = 0x1U;
using edge = core::field_descriptor<mode, 1U, 1U>;
inline constexpr std::uint32_t edge_leftfalling = 0x0U;
inline constexpr std::uint32_t edge_leftrising = 0x1U;
}
struct gainl : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000028U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace gainl_fields {
using gainl = core::field_descriptor<gainl, 0U, 7U>;
inline constexpr std::uint32_t gainl_mingain = 0x0U;
inline constexpr std::uint32_t gainl_defaultgain = 0x28U;
inline constexpr std::uint32_t gainl_maxgain = 0x50U;
}
struct gainr : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000028U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace gainr_fields {
using gainr = core::field_descriptor<gainr, 0U, 7U>;
inline constexpr std::uint32_t gainr_mingain = 0x0U;
inline constexpr std::uint32_t gainr_defaultgain = 0x28U;
inline constexpr std::uint32_t gainr_maxgain = 0x50U;
}
struct ratio : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ratio_fields {
using ratio = core::field_descriptor<ratio, 0U, 1U>;
inline constexpr std::uint32_t ratio_ratio64 = 0x0U;
inline constexpr std::uint32_t ratio_ratio80 = 0x1U;
}
struct psel_clk : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_clk_fields {
using pin = core::field_descriptor<psel_clk, 0U, 5U>;
using port = core::field_descriptor<psel_clk, 5U, 1U>;
using connect = core::field_descriptor<psel_clk, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_din : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_din_fields {
using pin = core::field_descriptor<psel_din, 0U, 5U>;
using port = core::field_descriptor<psel_din, 5U, 1U>;
using connect = core::field_descriptor<psel_din, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct sample_ptr : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sample_ptr_fields {
using sampleptr = core::field_descriptor<sample_ptr, 0U, 32U>;
}
struct sample_maxcnt : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0x00007FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sample_maxcnt_fields {
using buffsize = core::field_descriptor<sample_maxcnt, 0U, 15U>;
}
}
