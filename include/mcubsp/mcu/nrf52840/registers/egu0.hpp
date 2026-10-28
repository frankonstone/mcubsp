#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::egu0 {
struct tasks_trigger0 : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger0_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger0, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger1 : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger1_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger1, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger2 : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger2_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger2, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger3 : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger3_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger3, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger4 : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger4_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger4, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger5 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger5_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger5, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger6 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger6_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger6, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger7 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger7_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger7, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger8 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger8_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger8, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger9 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger9_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger9, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger10 : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger10_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger10, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger11 : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger11_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger11, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger12 : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger12_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger12, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger13 : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger13_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger13, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger14 : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger14_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger14, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct tasks_trigger15 : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_trigger15_fields {
using tasks_trigger = core::field_descriptor<tasks_trigger15, 0U, 1U>;
inline constexpr std::uint32_t tasks_trigger_trigger = 0x1U;
}
struct events_triggered0 : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered0_fields {
using events_triggered = core::field_descriptor<events_triggered0, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered1 : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered1_fields {
using events_triggered = core::field_descriptor<events_triggered1, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered2 : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered2_fields {
using events_triggered = core::field_descriptor<events_triggered2, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered3 : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered3_fields {
using events_triggered = core::field_descriptor<events_triggered3, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered4 : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered4_fields {
using events_triggered = core::field_descriptor<events_triggered4, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered5 : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered5_fields {
using events_triggered = core::field_descriptor<events_triggered5, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered6 : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered6_fields {
using events_triggered = core::field_descriptor<events_triggered6, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered7 : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered7_fields {
using events_triggered = core::field_descriptor<events_triggered7, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered8 : core::register_descriptor<std::uint32_t, 0x120U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered8_fields {
using events_triggered = core::field_descriptor<events_triggered8, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered9 : core::register_descriptor<std::uint32_t, 0x124U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered9_fields {
using events_triggered = core::field_descriptor<events_triggered9, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered10 : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered10_fields {
using events_triggered = core::field_descriptor<events_triggered10, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered11 : core::register_descriptor<std::uint32_t, 0x12CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered11_fields {
using events_triggered = core::field_descriptor<events_triggered11, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered12 : core::register_descriptor<std::uint32_t, 0x130U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered12_fields {
using events_triggered = core::field_descriptor<events_triggered12, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered13 : core::register_descriptor<std::uint32_t, 0x134U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered13_fields {
using events_triggered = core::field_descriptor<events_triggered13, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered14 : core::register_descriptor<std::uint32_t, 0x138U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered14_fields {
using events_triggered = core::field_descriptor<events_triggered14, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct events_triggered15 : core::register_descriptor<std::uint32_t, 0x13CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_triggered15_fields {
using events_triggered = core::field_descriptor<events_triggered15, 0U, 1U>;
inline constexpr std::uint32_t events_triggered_notgenerated = 0x0U;
inline constexpr std::uint32_t events_triggered_generated = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using triggered0 = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t triggered0_disabled = 0x0U;
inline constexpr std::uint32_t triggered0_enabled = 0x1U;
using triggered1 = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t triggered1_disabled = 0x0U;
inline constexpr std::uint32_t triggered1_enabled = 0x1U;
using triggered2 = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t triggered2_disabled = 0x0U;
inline constexpr std::uint32_t triggered2_enabled = 0x1U;
using triggered3 = core::field_descriptor<inten, 3U, 1U>;
inline constexpr std::uint32_t triggered3_disabled = 0x0U;
inline constexpr std::uint32_t triggered3_enabled = 0x1U;
using triggered4 = core::field_descriptor<inten, 4U, 1U>;
inline constexpr std::uint32_t triggered4_disabled = 0x0U;
inline constexpr std::uint32_t triggered4_enabled = 0x1U;
using triggered5 = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t triggered5_disabled = 0x0U;
inline constexpr std::uint32_t triggered5_enabled = 0x1U;
using triggered6 = core::field_descriptor<inten, 6U, 1U>;
inline constexpr std::uint32_t triggered6_disabled = 0x0U;
inline constexpr std::uint32_t triggered6_enabled = 0x1U;
using triggered7 = core::field_descriptor<inten, 7U, 1U>;
inline constexpr std::uint32_t triggered7_disabled = 0x0U;
inline constexpr std::uint32_t triggered7_enabled = 0x1U;
using triggered8 = core::field_descriptor<inten, 8U, 1U>;
inline constexpr std::uint32_t triggered8_disabled = 0x0U;
inline constexpr std::uint32_t triggered8_enabled = 0x1U;
using triggered9 = core::field_descriptor<inten, 9U, 1U>;
inline constexpr std::uint32_t triggered9_disabled = 0x0U;
inline constexpr std::uint32_t triggered9_enabled = 0x1U;
using triggered10 = core::field_descriptor<inten, 10U, 1U>;
inline constexpr std::uint32_t triggered10_disabled = 0x0U;
inline constexpr std::uint32_t triggered10_enabled = 0x1U;
using triggered11 = core::field_descriptor<inten, 11U, 1U>;
inline constexpr std::uint32_t triggered11_disabled = 0x0U;
inline constexpr std::uint32_t triggered11_enabled = 0x1U;
using triggered12 = core::field_descriptor<inten, 12U, 1U>;
inline constexpr std::uint32_t triggered12_disabled = 0x0U;
inline constexpr std::uint32_t triggered12_enabled = 0x1U;
using triggered13 = core::field_descriptor<inten, 13U, 1U>;
inline constexpr std::uint32_t triggered13_disabled = 0x0U;
inline constexpr std::uint32_t triggered13_enabled = 0x1U;
using triggered14 = core::field_descriptor<inten, 14U, 1U>;
inline constexpr std::uint32_t triggered14_disabled = 0x0U;
inline constexpr std::uint32_t triggered14_enabled = 0x1U;
using triggered15 = core::field_descriptor<inten, 15U, 1U>;
inline constexpr std::uint32_t triggered15_disabled = 0x0U;
inline constexpr std::uint32_t triggered15_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using triggered0 = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t triggered0_disabled = 0x0U;
inline constexpr std::uint32_t triggered0_enabled = 0x1U;
inline constexpr std::uint32_t triggered0_set = 0x1U;
using triggered1 = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t triggered1_disabled = 0x0U;
inline constexpr std::uint32_t triggered1_enabled = 0x1U;
inline constexpr std::uint32_t triggered1_set = 0x1U;
using triggered2 = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t triggered2_disabled = 0x0U;
inline constexpr std::uint32_t triggered2_enabled = 0x1U;
inline constexpr std::uint32_t triggered2_set = 0x1U;
using triggered3 = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t triggered3_disabled = 0x0U;
inline constexpr std::uint32_t triggered3_enabled = 0x1U;
inline constexpr std::uint32_t triggered3_set = 0x1U;
using triggered4 = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t triggered4_disabled = 0x0U;
inline constexpr std::uint32_t triggered4_enabled = 0x1U;
inline constexpr std::uint32_t triggered4_set = 0x1U;
using triggered5 = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t triggered5_disabled = 0x0U;
inline constexpr std::uint32_t triggered5_enabled = 0x1U;
inline constexpr std::uint32_t triggered5_set = 0x1U;
using triggered6 = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t triggered6_disabled = 0x0U;
inline constexpr std::uint32_t triggered6_enabled = 0x1U;
inline constexpr std::uint32_t triggered6_set = 0x1U;
using triggered7 = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t triggered7_disabled = 0x0U;
inline constexpr std::uint32_t triggered7_enabled = 0x1U;
inline constexpr std::uint32_t triggered7_set = 0x1U;
using triggered8 = core::field_descriptor<intenset, 8U, 1U>;
inline constexpr std::uint32_t triggered8_disabled = 0x0U;
inline constexpr std::uint32_t triggered8_enabled = 0x1U;
inline constexpr std::uint32_t triggered8_set = 0x1U;
using triggered9 = core::field_descriptor<intenset, 9U, 1U>;
inline constexpr std::uint32_t triggered9_disabled = 0x0U;
inline constexpr std::uint32_t triggered9_enabled = 0x1U;
inline constexpr std::uint32_t triggered9_set = 0x1U;
using triggered10 = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t triggered10_disabled = 0x0U;
inline constexpr std::uint32_t triggered10_enabled = 0x1U;
inline constexpr std::uint32_t triggered10_set = 0x1U;
using triggered11 = core::field_descriptor<intenset, 11U, 1U>;
inline constexpr std::uint32_t triggered11_disabled = 0x0U;
inline constexpr std::uint32_t triggered11_enabled = 0x1U;
inline constexpr std::uint32_t triggered11_set = 0x1U;
using triggered12 = core::field_descriptor<intenset, 12U, 1U>;
inline constexpr std::uint32_t triggered12_disabled = 0x0U;
inline constexpr std::uint32_t triggered12_enabled = 0x1U;
inline constexpr std::uint32_t triggered12_set = 0x1U;
using triggered13 = core::field_descriptor<intenset, 13U, 1U>;
inline constexpr std::uint32_t triggered13_disabled = 0x0U;
inline constexpr std::uint32_t triggered13_enabled = 0x1U;
inline constexpr std::uint32_t triggered13_set = 0x1U;
using triggered14 = core::field_descriptor<intenset, 14U, 1U>;
inline constexpr std::uint32_t triggered14_disabled = 0x0U;
inline constexpr std::uint32_t triggered14_enabled = 0x1U;
inline constexpr std::uint32_t triggered14_set = 0x1U;
using triggered15 = core::field_descriptor<intenset, 15U, 1U>;
inline constexpr std::uint32_t triggered15_disabled = 0x0U;
inline constexpr std::uint32_t triggered15_enabled = 0x1U;
inline constexpr std::uint32_t triggered15_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using triggered0 = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t triggered0_disabled = 0x0U;
inline constexpr std::uint32_t triggered0_enabled = 0x1U;
inline constexpr std::uint32_t triggered0_clear = 0x1U;
using triggered1 = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t triggered1_disabled = 0x0U;
inline constexpr std::uint32_t triggered1_enabled = 0x1U;
inline constexpr std::uint32_t triggered1_clear = 0x1U;
using triggered2 = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t triggered2_disabled = 0x0U;
inline constexpr std::uint32_t triggered2_enabled = 0x1U;
inline constexpr std::uint32_t triggered2_clear = 0x1U;
using triggered3 = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t triggered3_disabled = 0x0U;
inline constexpr std::uint32_t triggered3_enabled = 0x1U;
inline constexpr std::uint32_t triggered3_clear = 0x1U;
using triggered4 = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t triggered4_disabled = 0x0U;
inline constexpr std::uint32_t triggered4_enabled = 0x1U;
inline constexpr std::uint32_t triggered4_clear = 0x1U;
using triggered5 = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t triggered5_disabled = 0x0U;
inline constexpr std::uint32_t triggered5_enabled = 0x1U;
inline constexpr std::uint32_t triggered5_clear = 0x1U;
using triggered6 = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t triggered6_disabled = 0x0U;
inline constexpr std::uint32_t triggered6_enabled = 0x1U;
inline constexpr std::uint32_t triggered6_clear = 0x1U;
using triggered7 = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t triggered7_disabled = 0x0U;
inline constexpr std::uint32_t triggered7_enabled = 0x1U;
inline constexpr std::uint32_t triggered7_clear = 0x1U;
using triggered8 = core::field_descriptor<intenclr, 8U, 1U>;
inline constexpr std::uint32_t triggered8_disabled = 0x0U;
inline constexpr std::uint32_t triggered8_enabled = 0x1U;
inline constexpr std::uint32_t triggered8_clear = 0x1U;
using triggered9 = core::field_descriptor<intenclr, 9U, 1U>;
inline constexpr std::uint32_t triggered9_disabled = 0x0U;
inline constexpr std::uint32_t triggered9_enabled = 0x1U;
inline constexpr std::uint32_t triggered9_clear = 0x1U;
using triggered10 = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t triggered10_disabled = 0x0U;
inline constexpr std::uint32_t triggered10_enabled = 0x1U;
inline constexpr std::uint32_t triggered10_clear = 0x1U;
using triggered11 = core::field_descriptor<intenclr, 11U, 1U>;
inline constexpr std::uint32_t triggered11_disabled = 0x0U;
inline constexpr std::uint32_t triggered11_enabled = 0x1U;
inline constexpr std::uint32_t triggered11_clear = 0x1U;
using triggered12 = core::field_descriptor<intenclr, 12U, 1U>;
inline constexpr std::uint32_t triggered12_disabled = 0x0U;
inline constexpr std::uint32_t triggered12_enabled = 0x1U;
inline constexpr std::uint32_t triggered12_clear = 0x1U;
using triggered13 = core::field_descriptor<intenclr, 13U, 1U>;
inline constexpr std::uint32_t triggered13_disabled = 0x0U;
inline constexpr std::uint32_t triggered13_enabled = 0x1U;
inline constexpr std::uint32_t triggered13_clear = 0x1U;
using triggered14 = core::field_descriptor<intenclr, 14U, 1U>;
inline constexpr std::uint32_t triggered14_disabled = 0x0U;
inline constexpr std::uint32_t triggered14_enabled = 0x1U;
inline constexpr std::uint32_t triggered14_clear = 0x1U;
using triggered15 = core::field_descriptor<intenclr, 15U, 1U>;
inline constexpr std::uint32_t triggered15_disabled = 0x0U;
inline constexpr std::uint32_t triggered15_enabled = 0x1U;
inline constexpr std::uint32_t triggered15_clear = 0x1U;
}
}
