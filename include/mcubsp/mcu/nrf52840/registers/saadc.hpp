#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::saadc {
struct tasks_start : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
}
struct tasks_sample : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_sample_fields {
using tasks_sample = core::field_descriptor<tasks_sample, 0U, 1U>;
inline constexpr std::uint32_t tasks_sample_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct tasks_calibrateoffset : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_calibrateoffset_fields {
using tasks_calibrateoffset = core::field_descriptor<tasks_calibrateoffset, 0U, 1U>;
inline constexpr std::uint32_t tasks_calibrateoffset_trigger = 0x1U;
}
struct events_started : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_started_fields {
using events_started = core::field_descriptor<events_started, 0U, 1U>;
inline constexpr std::uint32_t events_started_notgenerated = 0x0U;
inline constexpr std::uint32_t events_started_generated = 0x1U;
}
struct events_end : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_end_fields {
using events_end = core::field_descriptor<events_end, 0U, 1U>;
inline constexpr std::uint32_t events_end_notgenerated = 0x0U;
inline constexpr std::uint32_t events_end_generated = 0x1U;
}
struct events_done : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_done_fields {
using events_done = core::field_descriptor<events_done, 0U, 1U>;
inline constexpr std::uint32_t events_done_notgenerated = 0x0U;
inline constexpr std::uint32_t events_done_generated = 0x1U;
}
struct events_resultdone : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_resultdone_fields {
using events_resultdone = core::field_descriptor<events_resultdone, 0U, 1U>;
inline constexpr std::uint32_t events_resultdone_notgenerated = 0x0U;
inline constexpr std::uint32_t events_resultdone_generated = 0x1U;
}
struct events_calibratedone : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_calibratedone_fields {
using events_calibratedone = core::field_descriptor<events_calibratedone, 0U, 1U>;
inline constexpr std::uint32_t events_calibratedone_notgenerated = 0x0U;
inline constexpr std::uint32_t events_calibratedone_generated = 0x1U;
}
struct events_stopped : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_stopped_fields {
using events_stopped = core::field_descriptor<events_stopped, 0U, 1U>;
inline constexpr std::uint32_t events_stopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_stopped_generated = 0x1U;
}
struct events_ch0_limith : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch0_limith_fields {
using limith = core::field_descriptor<events_ch0_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch0_limitl : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch0_limitl_fields {
using limitl = core::field_descriptor<events_ch0_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch1_limith : core::register_descriptor<std::uint32_t, 0x120U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch1_limith_fields {
using limith = core::field_descriptor<events_ch1_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch1_limitl : core::register_descriptor<std::uint32_t, 0x124U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch1_limitl_fields {
using limitl = core::field_descriptor<events_ch1_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch2_limith : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch2_limith_fields {
using limith = core::field_descriptor<events_ch2_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch2_limitl : core::register_descriptor<std::uint32_t, 0x12CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch2_limitl_fields {
using limitl = core::field_descriptor<events_ch2_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch3_limith : core::register_descriptor<std::uint32_t, 0x130U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch3_limith_fields {
using limith = core::field_descriptor<events_ch3_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch3_limitl : core::register_descriptor<std::uint32_t, 0x134U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch3_limitl_fields {
using limitl = core::field_descriptor<events_ch3_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch4_limith : core::register_descriptor<std::uint32_t, 0x138U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch4_limith_fields {
using limith = core::field_descriptor<events_ch4_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch4_limitl : core::register_descriptor<std::uint32_t, 0x13CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch4_limitl_fields {
using limitl = core::field_descriptor<events_ch4_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch5_limith : core::register_descriptor<std::uint32_t, 0x140U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch5_limith_fields {
using limith = core::field_descriptor<events_ch5_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch5_limitl : core::register_descriptor<std::uint32_t, 0x144U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch5_limitl_fields {
using limitl = core::field_descriptor<events_ch5_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch6_limith : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch6_limith_fields {
using limith = core::field_descriptor<events_ch6_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch6_limitl : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch6_limitl_fields {
using limitl = core::field_descriptor<events_ch6_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct events_ch7_limith : core::register_descriptor<std::uint32_t, 0x150U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch7_limith_fields {
using limith = core::field_descriptor<events_ch7_limith, 0U, 1U>;
inline constexpr std::uint32_t limith_notgenerated = 0x0U;
inline constexpr std::uint32_t limith_generated = 0x1U;
}
struct events_ch7_limitl : core::register_descriptor<std::uint32_t, 0x154U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ch7_limitl_fields {
using limitl = core::field_descriptor<events_ch7_limitl, 0U, 1U>;
inline constexpr std::uint32_t limitl_notgenerated = 0x0U;
inline constexpr std::uint32_t limitl_generated = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x003FFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using started = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
using end = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
using done = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t done_disabled = 0x0U;
inline constexpr std::uint32_t done_enabled = 0x1U;
using resultdone = core::field_descriptor<inten, 3U, 1U>;
inline constexpr std::uint32_t resultdone_disabled = 0x0U;
inline constexpr std::uint32_t resultdone_enabled = 0x1U;
using calibratedone = core::field_descriptor<inten, 4U, 1U>;
inline constexpr std::uint32_t calibratedone_disabled = 0x0U;
inline constexpr std::uint32_t calibratedone_enabled = 0x1U;
using stopped = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
using ch0limith = core::field_descriptor<inten, 6U, 1U>;
inline constexpr std::uint32_t ch0limith_disabled = 0x0U;
inline constexpr std::uint32_t ch0limith_enabled = 0x1U;
using ch0limitl = core::field_descriptor<inten, 7U, 1U>;
inline constexpr std::uint32_t ch0limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch0limitl_enabled = 0x1U;
using ch1limith = core::field_descriptor<inten, 8U, 1U>;
inline constexpr std::uint32_t ch1limith_disabled = 0x0U;
inline constexpr std::uint32_t ch1limith_enabled = 0x1U;
using ch1limitl = core::field_descriptor<inten, 9U, 1U>;
inline constexpr std::uint32_t ch1limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch1limitl_enabled = 0x1U;
using ch2limith = core::field_descriptor<inten, 10U, 1U>;
inline constexpr std::uint32_t ch2limith_disabled = 0x0U;
inline constexpr std::uint32_t ch2limith_enabled = 0x1U;
using ch2limitl = core::field_descriptor<inten, 11U, 1U>;
inline constexpr std::uint32_t ch2limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch2limitl_enabled = 0x1U;
using ch3limith = core::field_descriptor<inten, 12U, 1U>;
inline constexpr std::uint32_t ch3limith_disabled = 0x0U;
inline constexpr std::uint32_t ch3limith_enabled = 0x1U;
using ch3limitl = core::field_descriptor<inten, 13U, 1U>;
inline constexpr std::uint32_t ch3limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch3limitl_enabled = 0x1U;
using ch4limith = core::field_descriptor<inten, 14U, 1U>;
inline constexpr std::uint32_t ch4limith_disabled = 0x0U;
inline constexpr std::uint32_t ch4limith_enabled = 0x1U;
using ch4limitl = core::field_descriptor<inten, 15U, 1U>;
inline constexpr std::uint32_t ch4limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch4limitl_enabled = 0x1U;
using ch5limith = core::field_descriptor<inten, 16U, 1U>;
inline constexpr std::uint32_t ch5limith_disabled = 0x0U;
inline constexpr std::uint32_t ch5limith_enabled = 0x1U;
using ch5limitl = core::field_descriptor<inten, 17U, 1U>;
inline constexpr std::uint32_t ch5limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch5limitl_enabled = 0x1U;
using ch6limith = core::field_descriptor<inten, 18U, 1U>;
inline constexpr std::uint32_t ch6limith_disabled = 0x0U;
inline constexpr std::uint32_t ch6limith_enabled = 0x1U;
using ch6limitl = core::field_descriptor<inten, 19U, 1U>;
inline constexpr std::uint32_t ch6limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch6limitl_enabled = 0x1U;
using ch7limith = core::field_descriptor<inten, 20U, 1U>;
inline constexpr std::uint32_t ch7limith_disabled = 0x0U;
inline constexpr std::uint32_t ch7limith_enabled = 0x1U;
using ch7limitl = core::field_descriptor<inten, 21U, 1U>;
inline constexpr std::uint32_t ch7limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch7limitl_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x003FFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using started = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_set = 0x1U;
using end = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_set = 0x1U;
using done = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t done_disabled = 0x0U;
inline constexpr std::uint32_t done_enabled = 0x1U;
inline constexpr std::uint32_t done_set = 0x1U;
using resultdone = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t resultdone_disabled = 0x0U;
inline constexpr std::uint32_t resultdone_enabled = 0x1U;
inline constexpr std::uint32_t resultdone_set = 0x1U;
using calibratedone = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t calibratedone_disabled = 0x0U;
inline constexpr std::uint32_t calibratedone_enabled = 0x1U;
inline constexpr std::uint32_t calibratedone_set = 0x1U;
using stopped = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using ch0limith = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t ch0limith_disabled = 0x0U;
inline constexpr std::uint32_t ch0limith_enabled = 0x1U;
inline constexpr std::uint32_t ch0limith_set = 0x1U;
using ch0limitl = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t ch0limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch0limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch0limitl_set = 0x1U;
using ch1limith = core::field_descriptor<intenset, 8U, 1U>;
inline constexpr std::uint32_t ch1limith_disabled = 0x0U;
inline constexpr std::uint32_t ch1limith_enabled = 0x1U;
inline constexpr std::uint32_t ch1limith_set = 0x1U;
using ch1limitl = core::field_descriptor<intenset, 9U, 1U>;
inline constexpr std::uint32_t ch1limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch1limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch1limitl_set = 0x1U;
using ch2limith = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t ch2limith_disabled = 0x0U;
inline constexpr std::uint32_t ch2limith_enabled = 0x1U;
inline constexpr std::uint32_t ch2limith_set = 0x1U;
using ch2limitl = core::field_descriptor<intenset, 11U, 1U>;
inline constexpr std::uint32_t ch2limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch2limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch2limitl_set = 0x1U;
using ch3limith = core::field_descriptor<intenset, 12U, 1U>;
inline constexpr std::uint32_t ch3limith_disabled = 0x0U;
inline constexpr std::uint32_t ch3limith_enabled = 0x1U;
inline constexpr std::uint32_t ch3limith_set = 0x1U;
using ch3limitl = core::field_descriptor<intenset, 13U, 1U>;
inline constexpr std::uint32_t ch3limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch3limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch3limitl_set = 0x1U;
using ch4limith = core::field_descriptor<intenset, 14U, 1U>;
inline constexpr std::uint32_t ch4limith_disabled = 0x0U;
inline constexpr std::uint32_t ch4limith_enabled = 0x1U;
inline constexpr std::uint32_t ch4limith_set = 0x1U;
using ch4limitl = core::field_descriptor<intenset, 15U, 1U>;
inline constexpr std::uint32_t ch4limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch4limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch4limitl_set = 0x1U;
using ch5limith = core::field_descriptor<intenset, 16U, 1U>;
inline constexpr std::uint32_t ch5limith_disabled = 0x0U;
inline constexpr std::uint32_t ch5limith_enabled = 0x1U;
inline constexpr std::uint32_t ch5limith_set = 0x1U;
using ch5limitl = core::field_descriptor<intenset, 17U, 1U>;
inline constexpr std::uint32_t ch5limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch5limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch5limitl_set = 0x1U;
using ch6limith = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t ch6limith_disabled = 0x0U;
inline constexpr std::uint32_t ch6limith_enabled = 0x1U;
inline constexpr std::uint32_t ch6limith_set = 0x1U;
using ch6limitl = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t ch6limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch6limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch6limitl_set = 0x1U;
using ch7limith = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t ch7limith_disabled = 0x0U;
inline constexpr std::uint32_t ch7limith_enabled = 0x1U;
inline constexpr std::uint32_t ch7limith_set = 0x1U;
using ch7limitl = core::field_descriptor<intenset, 21U, 1U>;
inline constexpr std::uint32_t ch7limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch7limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch7limitl_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x003FFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using started = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t started_disabled = 0x0U;
inline constexpr std::uint32_t started_enabled = 0x1U;
inline constexpr std::uint32_t started_clear = 0x1U;
using end = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_clear = 0x1U;
using done = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t done_disabled = 0x0U;
inline constexpr std::uint32_t done_enabled = 0x1U;
inline constexpr std::uint32_t done_clear = 0x1U;
using resultdone = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t resultdone_disabled = 0x0U;
inline constexpr std::uint32_t resultdone_enabled = 0x1U;
inline constexpr std::uint32_t resultdone_clear = 0x1U;
using calibratedone = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t calibratedone_disabled = 0x0U;
inline constexpr std::uint32_t calibratedone_enabled = 0x1U;
inline constexpr std::uint32_t calibratedone_clear = 0x1U;
using stopped = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using ch0limith = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t ch0limith_disabled = 0x0U;
inline constexpr std::uint32_t ch0limith_enabled = 0x1U;
inline constexpr std::uint32_t ch0limith_clear = 0x1U;
using ch0limitl = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t ch0limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch0limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch0limitl_clear = 0x1U;
using ch1limith = core::field_descriptor<intenclr, 8U, 1U>;
inline constexpr std::uint32_t ch1limith_disabled = 0x0U;
inline constexpr std::uint32_t ch1limith_enabled = 0x1U;
inline constexpr std::uint32_t ch1limith_clear = 0x1U;
using ch1limitl = core::field_descriptor<intenclr, 9U, 1U>;
inline constexpr std::uint32_t ch1limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch1limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch1limitl_clear = 0x1U;
using ch2limith = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t ch2limith_disabled = 0x0U;
inline constexpr std::uint32_t ch2limith_enabled = 0x1U;
inline constexpr std::uint32_t ch2limith_clear = 0x1U;
using ch2limitl = core::field_descriptor<intenclr, 11U, 1U>;
inline constexpr std::uint32_t ch2limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch2limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch2limitl_clear = 0x1U;
using ch3limith = core::field_descriptor<intenclr, 12U, 1U>;
inline constexpr std::uint32_t ch3limith_disabled = 0x0U;
inline constexpr std::uint32_t ch3limith_enabled = 0x1U;
inline constexpr std::uint32_t ch3limith_clear = 0x1U;
using ch3limitl = core::field_descriptor<intenclr, 13U, 1U>;
inline constexpr std::uint32_t ch3limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch3limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch3limitl_clear = 0x1U;
using ch4limith = core::field_descriptor<intenclr, 14U, 1U>;
inline constexpr std::uint32_t ch4limith_disabled = 0x0U;
inline constexpr std::uint32_t ch4limith_enabled = 0x1U;
inline constexpr std::uint32_t ch4limith_clear = 0x1U;
using ch4limitl = core::field_descriptor<intenclr, 15U, 1U>;
inline constexpr std::uint32_t ch4limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch4limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch4limitl_clear = 0x1U;
using ch5limith = core::field_descriptor<intenclr, 16U, 1U>;
inline constexpr std::uint32_t ch5limith_disabled = 0x0U;
inline constexpr std::uint32_t ch5limith_enabled = 0x1U;
inline constexpr std::uint32_t ch5limith_clear = 0x1U;
using ch5limitl = core::field_descriptor<intenclr, 17U, 1U>;
inline constexpr std::uint32_t ch5limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch5limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch5limitl_clear = 0x1U;
using ch6limith = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t ch6limith_disabled = 0x0U;
inline constexpr std::uint32_t ch6limith_enabled = 0x1U;
inline constexpr std::uint32_t ch6limith_clear = 0x1U;
using ch6limitl = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t ch6limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch6limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch6limitl_clear = 0x1U;
using ch7limith = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t ch7limith_disabled = 0x0U;
inline constexpr std::uint32_t ch7limith_enabled = 0x1U;
inline constexpr std::uint32_t ch7limith_clear = 0x1U;
using ch7limitl = core::field_descriptor<intenclr, 21U, 1U>;
inline constexpr std::uint32_t ch7limitl_disabled = 0x0U;
inline constexpr std::uint32_t ch7limitl_enabled = 0x1U;
inline constexpr std::uint32_t ch7limitl_clear = 0x1U;
}
struct status : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace status_fields {
using status = core::field_descriptor<status, 0U, 1U>;
inline constexpr std::uint32_t status_ready = 0x0U;
inline constexpr std::uint32_t status_busy = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct ch0_pselp : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch0_pselp_fields {
using pselp = core::field_descriptor<ch0_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch0_pseln : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch0_pseln_fields {
using pseln = core::field_descriptor<ch0_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch0_config : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch0_config_fields {
using resp = core::field_descriptor<ch0_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch0_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch0_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch0_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch0_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch0_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch0_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch0_limit : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch0_limit_fields {
using low = core::field_descriptor<ch0_limit, 0U, 16U>;
using high = core::field_descriptor<ch0_limit, 16U, 16U>;
}
struct ch1_pselp : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch1_pselp_fields {
using pselp = core::field_descriptor<ch1_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch1_pseln : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch1_pseln_fields {
using pseln = core::field_descriptor<ch1_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch1_config : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch1_config_fields {
using resp = core::field_descriptor<ch1_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch1_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch1_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch1_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch1_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch1_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch1_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch1_limit : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch1_limit_fields {
using low = core::field_descriptor<ch1_limit, 0U, 16U>;
using high = core::field_descriptor<ch1_limit, 16U, 16U>;
}
struct ch2_pselp : core::register_descriptor<std::uint32_t, 0x530U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch2_pselp_fields {
using pselp = core::field_descriptor<ch2_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch2_pseln : core::register_descriptor<std::uint32_t, 0x534U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch2_pseln_fields {
using pseln = core::field_descriptor<ch2_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch2_config : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch2_config_fields {
using resp = core::field_descriptor<ch2_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch2_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch2_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch2_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch2_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch2_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch2_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch2_limit : core::register_descriptor<std::uint32_t, 0x53CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch2_limit_fields {
using low = core::field_descriptor<ch2_limit, 0U, 16U>;
using high = core::field_descriptor<ch2_limit, 16U, 16U>;
}
struct ch3_pselp : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch3_pselp_fields {
using pselp = core::field_descriptor<ch3_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch3_pseln : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch3_pseln_fields {
using pseln = core::field_descriptor<ch3_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch3_config : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch3_config_fields {
using resp = core::field_descriptor<ch3_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch3_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch3_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch3_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch3_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch3_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch3_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch3_limit : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch3_limit_fields {
using low = core::field_descriptor<ch3_limit, 0U, 16U>;
using high = core::field_descriptor<ch3_limit, 16U, 16U>;
}
struct ch4_pselp : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch4_pselp_fields {
using pselp = core::field_descriptor<ch4_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch4_pseln : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch4_pseln_fields {
using pseln = core::field_descriptor<ch4_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch4_config : core::register_descriptor<std::uint32_t, 0x558U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch4_config_fields {
using resp = core::field_descriptor<ch4_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch4_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch4_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch4_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch4_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch4_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch4_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch4_limit : core::register_descriptor<std::uint32_t, 0x55CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch4_limit_fields {
using low = core::field_descriptor<ch4_limit, 0U, 16U>;
using high = core::field_descriptor<ch4_limit, 16U, 16U>;
}
struct ch5_pselp : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch5_pselp_fields {
using pselp = core::field_descriptor<ch5_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch5_pseln : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch5_pseln_fields {
using pseln = core::field_descriptor<ch5_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch5_config : core::register_descriptor<std::uint32_t, 0x568U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch5_config_fields {
using resp = core::field_descriptor<ch5_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch5_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch5_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch5_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch5_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch5_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch5_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch5_limit : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch5_limit_fields {
using low = core::field_descriptor<ch5_limit, 0U, 16U>;
using high = core::field_descriptor<ch5_limit, 16U, 16U>;
}
struct ch6_pselp : core::register_descriptor<std::uint32_t, 0x570U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch6_pselp_fields {
using pselp = core::field_descriptor<ch6_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch6_pseln : core::register_descriptor<std::uint32_t, 0x574U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch6_pseln_fields {
using pseln = core::field_descriptor<ch6_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch6_config : core::register_descriptor<std::uint32_t, 0x578U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch6_config_fields {
using resp = core::field_descriptor<ch6_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch6_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch6_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch6_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch6_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch6_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch6_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch6_limit : core::register_descriptor<std::uint32_t, 0x57CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch6_limit_fields {
using low = core::field_descriptor<ch6_limit, 0U, 16U>;
using high = core::field_descriptor<ch6_limit, 16U, 16U>;
}
struct ch7_pselp : core::register_descriptor<std::uint32_t, 0x580U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch7_pselp_fields {
using pselp = core::field_descriptor<ch7_pselp, 0U, 5U>;
inline constexpr std::uint32_t pselp_nc = 0x0U;
inline constexpr std::uint32_t pselp_analoginput0 = 0x1U;
inline constexpr std::uint32_t pselp_analoginput1 = 0x2U;
inline constexpr std::uint32_t pselp_analoginput2 = 0x3U;
inline constexpr std::uint32_t pselp_analoginput3 = 0x4U;
inline constexpr std::uint32_t pselp_analoginput4 = 0x5U;
inline constexpr std::uint32_t pselp_analoginput5 = 0x6U;
inline constexpr std::uint32_t pselp_analoginput6 = 0x7U;
inline constexpr std::uint32_t pselp_analoginput7 = 0x8U;
inline constexpr std::uint32_t pselp_vdd = 0x9U;
inline constexpr std::uint32_t pselp_vddhdiv5 = 0xDU;
}
struct ch7_pseln : core::register_descriptor<std::uint32_t, 0x584U,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch7_pseln_fields {
using pseln = core::field_descriptor<ch7_pseln, 0U, 5U>;
inline constexpr std::uint32_t pseln_nc = 0x0U;
inline constexpr std::uint32_t pseln_analoginput0 = 0x1U;
inline constexpr std::uint32_t pseln_analoginput1 = 0x2U;
inline constexpr std::uint32_t pseln_analoginput2 = 0x3U;
inline constexpr std::uint32_t pseln_analoginput3 = 0x4U;
inline constexpr std::uint32_t pseln_analoginput4 = 0x5U;
inline constexpr std::uint32_t pseln_analoginput5 = 0x6U;
inline constexpr std::uint32_t pseln_analoginput6 = 0x7U;
inline constexpr std::uint32_t pseln_analoginput7 = 0x8U;
inline constexpr std::uint32_t pseln_vdd = 0x9U;
inline constexpr std::uint32_t pseln_vddhdiv5 = 0xDU;
}
struct ch7_config : core::register_descriptor<std::uint32_t, 0x588U,
    core::access_semantics::read_write, 0x01171733U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00020000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch7_config_fields {
using resp = core::field_descriptor<ch7_config, 0U, 2U>;
inline constexpr std::uint32_t resp_bypass = 0x0U;
inline constexpr std::uint32_t resp_pulldown = 0x1U;
inline constexpr std::uint32_t resp_pullup = 0x2U;
inline constexpr std::uint32_t resp_vdd1_2 = 0x3U;
using resn = core::field_descriptor<ch7_config, 4U, 2U>;
inline constexpr std::uint32_t resn_bypass = 0x0U;
inline constexpr std::uint32_t resn_pulldown = 0x1U;
inline constexpr std::uint32_t resn_pullup = 0x2U;
inline constexpr std::uint32_t resn_vdd1_2 = 0x3U;
using gain = core::field_descriptor<ch7_config, 8U, 3U>;
inline constexpr std::uint32_t gain_gain1_6 = 0x0U;
inline constexpr std::uint32_t gain_gain1_5 = 0x1U;
inline constexpr std::uint32_t gain_gain1_4 = 0x2U;
inline constexpr std::uint32_t gain_gain1_3 = 0x3U;
inline constexpr std::uint32_t gain_gain1_2 = 0x4U;
inline constexpr std::uint32_t gain_gain1 = 0x5U;
inline constexpr std::uint32_t gain_gain2 = 0x6U;
inline constexpr std::uint32_t gain_gain4 = 0x7U;
using refsel = core::field_descriptor<ch7_config, 12U, 1U>;
inline constexpr std::uint32_t refsel_internal = 0x0U;
inline constexpr std::uint32_t refsel_vdd1_4 = 0x1U;
using tacq = core::field_descriptor<ch7_config, 16U, 3U>;
inline constexpr std::uint32_t tacq_reg_3us = 0x0U;
inline constexpr std::uint32_t tacq_reg_5us = 0x1U;
inline constexpr std::uint32_t tacq_reg_10us = 0x2U;
inline constexpr std::uint32_t tacq_reg_15us = 0x3U;
inline constexpr std::uint32_t tacq_reg_20us = 0x4U;
inline constexpr std::uint32_t tacq_reg_40us = 0x5U;
using mode = core::field_descriptor<ch7_config, 20U, 1U>;
inline constexpr std::uint32_t mode_se = 0x0U;
inline constexpr std::uint32_t mode_diff = 0x1U;
using burst = core::field_descriptor<ch7_config, 24U, 1U>;
inline constexpr std::uint32_t burst_disabled = 0x0U;
inline constexpr std::uint32_t burst_enabled = 0x1U;
}
struct ch7_limit : core::register_descriptor<std::uint32_t, 0x58CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x7FFF8000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch7_limit_fields {
using low = core::field_descriptor<ch7_limit, 0U, 16U>;
using high = core::field_descriptor<ch7_limit, 16U, 16U>;
}
struct resolution : core::register_descriptor<std::uint32_t, 0x5F0U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace resolution_fields {
using val = core::field_descriptor<resolution, 0U, 3U>;
inline constexpr std::uint32_t val_reg_8bit = 0x0U;
inline constexpr std::uint32_t val_reg_10bit = 0x1U;
inline constexpr std::uint32_t val_reg_12bit = 0x2U;
inline constexpr std::uint32_t val_reg_14bit = 0x3U;
}
struct oversample : core::register_descriptor<std::uint32_t, 0x5F4U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace oversample_fields {
using oversample = core::field_descriptor<oversample, 0U, 4U>;
inline constexpr std::uint32_t oversample_bypass = 0x0U;
inline constexpr std::uint32_t oversample_over2x = 0x1U;
inline constexpr std::uint32_t oversample_over4x = 0x2U;
inline constexpr std::uint32_t oversample_over8x = 0x3U;
inline constexpr std::uint32_t oversample_over16x = 0x4U;
inline constexpr std::uint32_t oversample_over32x = 0x5U;
inline constexpr std::uint32_t oversample_over64x = 0x6U;
inline constexpr std::uint32_t oversample_over128x = 0x7U;
inline constexpr std::uint32_t oversample_over256x = 0x8U;
}
struct samplerate : core::register_descriptor<std::uint32_t, 0x5F8U,
    core::access_semantics::read_write, 0x000017FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace samplerate_fields {
using cc = core::field_descriptor<samplerate, 0U, 11U>;
using mode = core::field_descriptor<samplerate, 12U, 1U>;
inline constexpr std::uint32_t mode_task = 0x0U;
inline constexpr std::uint32_t mode_timers = 0x1U;
}
struct result_ptr : core::register_descriptor<std::uint32_t, 0x62CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace result_ptr_fields {
using ptr = core::field_descriptor<result_ptr, 0U, 32U>;
}
struct result_maxcnt : core::register_descriptor<std::uint32_t, 0x630U,
    core::access_semantics::read_write, 0x00007FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace result_maxcnt_fields {
using maxcnt = core::field_descriptor<result_maxcnt, 0U, 15U>;
}
struct result_amount : core::register_descriptor<std::uint32_t, 0x634U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace result_amount_fields {
using amount = core::field_descriptor<result_amount, 0U, 15U>;
}
}
