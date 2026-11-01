#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::itm {
inline constexpr std::uintptr_t base = 0xE0000000U;
struct ter : core::register_descriptor<std::uint32_t, 0xE00U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ter_fields {
using value = core::field_descriptor<ter, 0U, 32U, core::access_semantics::read_write>;
}
struct tpr : core::register_descriptor<std::uint32_t, 0xE40U,
    core::access_semantics::read_write, 0xFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace tpr_fields {
using privmask = core::field_descriptor<tpr, 0U, 4U, core::access_semantics::read_write>;
}
struct tcr : core::register_descriptor<std::uint32_t, 0xE80U,
    core::access_semantics::read_write, 0x7F0F1FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace tcr_fields {
using busy = core::field_descriptor<tcr, 23U, 1U, core::access_semantics::read_only>;
using tracebusid = core::field_descriptor<tcr, 16U, 7U, core::access_semantics::read_write>;
using gtsfreq = core::field_descriptor<tcr, 10U, 2U, core::access_semantics::read_write>;
using tsprescale = core::field_descriptor<tcr, 8U, 2U, core::access_semantics::read_write>;
using swoena = core::field_descriptor<tcr, 4U, 1U, core::access_semantics::read_write>;
using dwtena = core::field_descriptor<tcr, 3U, 1U, core::access_semantics::read_write>;
using syncena = core::field_descriptor<tcr, 2U, 1U, core::access_semantics::read_write>;
using tsena = core::field_descriptor<tcr, 1U, 1U, core::access_semantics::read_write>;
using itmena = core::field_descriptor<tcr, 0U, 1U, core::access_semantics::read_write>;
}
struct lar : core::register_descriptor<std::uint32_t, 0xFB0U,
    core::access_semantics::command, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace lar_fields {
using value = core::field_descriptor<lar, 0U, 32U, core::access_semantics::command>;
}
struct lsr : core::register_descriptor<std::uint32_t, 0xFB4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace lsr_fields {
using byteacc = core::field_descriptor<lsr, 2U, 1U, core::access_semantics::read_only>;
using access = core::field_descriptor<lsr, 1U, 1U, core::access_semantics::read_only>;
using present = core::field_descriptor<lsr, 0U, 1U, core::access_semantics::read_only>;
}
struct pid4 : core::register_descriptor<std::uint32_t, 0xFD0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid4_fields {
using value = core::field_descriptor<pid4, 0U, 32U, core::access_semantics::read_only>;
}
struct pid5 : core::register_descriptor<std::uint32_t, 0xFD4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid5_fields {
using value = core::field_descriptor<pid5, 0U, 32U, core::access_semantics::read_only>;
}
struct pid6 : core::register_descriptor<std::uint32_t, 0xFD8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid6_fields {
using value = core::field_descriptor<pid6, 0U, 32U, core::access_semantics::read_only>;
}
struct pid7 : core::register_descriptor<std::uint32_t, 0xFDCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid7_fields {
using value = core::field_descriptor<pid7, 0U, 32U, core::access_semantics::read_only>;
}
struct pid0 : core::register_descriptor<std::uint32_t, 0xFE0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid0_fields {
using value = core::field_descriptor<pid0, 0U, 32U, core::access_semantics::read_only>;
}
struct pid1 : core::register_descriptor<std::uint32_t, 0xFE4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid1_fields {
using value = core::field_descriptor<pid1, 0U, 32U, core::access_semantics::read_only>;
}
struct pid2 : core::register_descriptor<std::uint32_t, 0xFE8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid2_fields {
using value = core::field_descriptor<pid2, 0U, 32U, core::access_semantics::read_only>;
}
struct pid3 : core::register_descriptor<std::uint32_t, 0xFECU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid3_fields {
using value = core::field_descriptor<pid3, 0U, 32U, core::access_semantics::read_only>;
}
struct cid0 : core::register_descriptor<std::uint32_t, 0xFF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid0_fields {
using value = core::field_descriptor<cid0, 0U, 32U, core::access_semantics::read_only>;
}
struct cid1 : core::register_descriptor<std::uint32_t, 0xFF4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid1_fields {
using value = core::field_descriptor<cid1, 0U, 32U, core::access_semantics::read_only>;
}
struct cid2 : core::register_descriptor<std::uint32_t, 0xFF8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid2_fields {
using value = core::field_descriptor<cid2, 0U, 32U, core::access_semantics::read_only>;
}
struct cid3 : core::register_descriptor<std::uint32_t, 0xFFCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid3_fields {
using value = core::field_descriptor<cid3, 0U, 32U, core::access_semantics::read_only>;
}
struct port0_u8 : core::register_descriptor<std::uint8_t, 0x0U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port0_u8_fields {
using data = core::field_descriptor<port0_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port0_u16 : core::register_descriptor<std::uint16_t, 0x0U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port0_u16_fields {
using data = core::field_descriptor<port0_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port0_u32 : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port0_u32_fields {
using data = core::field_descriptor<port0_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port0_status : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port0_status_fields {
using ready = core::field_descriptor<port0_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port1_u8 : core::register_descriptor<std::uint8_t, 0x4U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port1_u8_fields {
using data = core::field_descriptor<port1_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port1_u16 : core::register_descriptor<std::uint16_t, 0x4U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port1_u16_fields {
using data = core::field_descriptor<port1_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port1_u32 : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port1_u32_fields {
using data = core::field_descriptor<port1_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port1_status : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port1_status_fields {
using ready = core::field_descriptor<port1_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port2_u8 : core::register_descriptor<std::uint8_t, 0x8U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port2_u8_fields {
using data = core::field_descriptor<port2_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port2_u16 : core::register_descriptor<std::uint16_t, 0x8U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port2_u16_fields {
using data = core::field_descriptor<port2_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port2_u32 : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port2_u32_fields {
using data = core::field_descriptor<port2_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port2_status : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port2_status_fields {
using ready = core::field_descriptor<port2_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port3_u8 : core::register_descriptor<std::uint8_t, 0xCU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port3_u8_fields {
using data = core::field_descriptor<port3_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port3_u16 : core::register_descriptor<std::uint16_t, 0xCU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port3_u16_fields {
using data = core::field_descriptor<port3_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port3_u32 : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port3_u32_fields {
using data = core::field_descriptor<port3_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port3_status : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port3_status_fields {
using ready = core::field_descriptor<port3_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port4_u8 : core::register_descriptor<std::uint8_t, 0x10U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port4_u8_fields {
using data = core::field_descriptor<port4_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port4_u16 : core::register_descriptor<std::uint16_t, 0x10U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port4_u16_fields {
using data = core::field_descriptor<port4_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port4_u32 : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port4_u32_fields {
using data = core::field_descriptor<port4_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port4_status : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port4_status_fields {
using ready = core::field_descriptor<port4_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port5_u8 : core::register_descriptor<std::uint8_t, 0x14U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port5_u8_fields {
using data = core::field_descriptor<port5_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port5_u16 : core::register_descriptor<std::uint16_t, 0x14U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port5_u16_fields {
using data = core::field_descriptor<port5_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port5_u32 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port5_u32_fields {
using data = core::field_descriptor<port5_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port5_status : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port5_status_fields {
using ready = core::field_descriptor<port5_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port6_u8 : core::register_descriptor<std::uint8_t, 0x18U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port6_u8_fields {
using data = core::field_descriptor<port6_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port6_u16 : core::register_descriptor<std::uint16_t, 0x18U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port6_u16_fields {
using data = core::field_descriptor<port6_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port6_u32 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port6_u32_fields {
using data = core::field_descriptor<port6_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port6_status : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port6_status_fields {
using ready = core::field_descriptor<port6_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port7_u8 : core::register_descriptor<std::uint8_t, 0x1CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port7_u8_fields {
using data = core::field_descriptor<port7_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port7_u16 : core::register_descriptor<std::uint16_t, 0x1CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port7_u16_fields {
using data = core::field_descriptor<port7_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port7_u32 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port7_u32_fields {
using data = core::field_descriptor<port7_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port7_status : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port7_status_fields {
using ready = core::field_descriptor<port7_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port8_u8 : core::register_descriptor<std::uint8_t, 0x20U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port8_u8_fields {
using data = core::field_descriptor<port8_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port8_u16 : core::register_descriptor<std::uint16_t, 0x20U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port8_u16_fields {
using data = core::field_descriptor<port8_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port8_u32 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port8_u32_fields {
using data = core::field_descriptor<port8_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port8_status : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port8_status_fields {
using ready = core::field_descriptor<port8_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port9_u8 : core::register_descriptor<std::uint8_t, 0x24U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port9_u8_fields {
using data = core::field_descriptor<port9_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port9_u16 : core::register_descriptor<std::uint16_t, 0x24U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port9_u16_fields {
using data = core::field_descriptor<port9_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port9_u32 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port9_u32_fields {
using data = core::field_descriptor<port9_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port9_status : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port9_status_fields {
using ready = core::field_descriptor<port9_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port10_u8 : core::register_descriptor<std::uint8_t, 0x28U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port10_u8_fields {
using data = core::field_descriptor<port10_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port10_u16 : core::register_descriptor<std::uint16_t, 0x28U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port10_u16_fields {
using data = core::field_descriptor<port10_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port10_u32 : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port10_u32_fields {
using data = core::field_descriptor<port10_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port10_status : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port10_status_fields {
using ready = core::field_descriptor<port10_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port11_u8 : core::register_descriptor<std::uint8_t, 0x2CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port11_u8_fields {
using data = core::field_descriptor<port11_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port11_u16 : core::register_descriptor<std::uint16_t, 0x2CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port11_u16_fields {
using data = core::field_descriptor<port11_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port11_u32 : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port11_u32_fields {
using data = core::field_descriptor<port11_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port11_status : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port11_status_fields {
using ready = core::field_descriptor<port11_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port12_u8 : core::register_descriptor<std::uint8_t, 0x30U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port12_u8_fields {
using data = core::field_descriptor<port12_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port12_u16 : core::register_descriptor<std::uint16_t, 0x30U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port12_u16_fields {
using data = core::field_descriptor<port12_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port12_u32 : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port12_u32_fields {
using data = core::field_descriptor<port12_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port12_status : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port12_status_fields {
using ready = core::field_descriptor<port12_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port13_u8 : core::register_descriptor<std::uint8_t, 0x34U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port13_u8_fields {
using data = core::field_descriptor<port13_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port13_u16 : core::register_descriptor<std::uint16_t, 0x34U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port13_u16_fields {
using data = core::field_descriptor<port13_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port13_u32 : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port13_u32_fields {
using data = core::field_descriptor<port13_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port13_status : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port13_status_fields {
using ready = core::field_descriptor<port13_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port14_u8 : core::register_descriptor<std::uint8_t, 0x38U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port14_u8_fields {
using data = core::field_descriptor<port14_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port14_u16 : core::register_descriptor<std::uint16_t, 0x38U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port14_u16_fields {
using data = core::field_descriptor<port14_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port14_u32 : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port14_u32_fields {
using data = core::field_descriptor<port14_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port14_status : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port14_status_fields {
using ready = core::field_descriptor<port14_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port15_u8 : core::register_descriptor<std::uint8_t, 0x3CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port15_u8_fields {
using data = core::field_descriptor<port15_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port15_u16 : core::register_descriptor<std::uint16_t, 0x3CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port15_u16_fields {
using data = core::field_descriptor<port15_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port15_u32 : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port15_u32_fields {
using data = core::field_descriptor<port15_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port15_status : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port15_status_fields {
using ready = core::field_descriptor<port15_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port16_u8 : core::register_descriptor<std::uint8_t, 0x40U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port16_u8_fields {
using data = core::field_descriptor<port16_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port16_u16 : core::register_descriptor<std::uint16_t, 0x40U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port16_u16_fields {
using data = core::field_descriptor<port16_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port16_u32 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port16_u32_fields {
using data = core::field_descriptor<port16_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port16_status : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port16_status_fields {
using ready = core::field_descriptor<port16_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port17_u8 : core::register_descriptor<std::uint8_t, 0x44U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port17_u8_fields {
using data = core::field_descriptor<port17_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port17_u16 : core::register_descriptor<std::uint16_t, 0x44U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port17_u16_fields {
using data = core::field_descriptor<port17_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port17_u32 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port17_u32_fields {
using data = core::field_descriptor<port17_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port17_status : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port17_status_fields {
using ready = core::field_descriptor<port17_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port18_u8 : core::register_descriptor<std::uint8_t, 0x48U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port18_u8_fields {
using data = core::field_descriptor<port18_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port18_u16 : core::register_descriptor<std::uint16_t, 0x48U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port18_u16_fields {
using data = core::field_descriptor<port18_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port18_u32 : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port18_u32_fields {
using data = core::field_descriptor<port18_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port18_status : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port18_status_fields {
using ready = core::field_descriptor<port18_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port19_u8 : core::register_descriptor<std::uint8_t, 0x4CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port19_u8_fields {
using data = core::field_descriptor<port19_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port19_u16 : core::register_descriptor<std::uint16_t, 0x4CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port19_u16_fields {
using data = core::field_descriptor<port19_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port19_u32 : core::register_descriptor<std::uint32_t, 0x4CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port19_u32_fields {
using data = core::field_descriptor<port19_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port19_status : core::register_descriptor<std::uint32_t, 0x4CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port19_status_fields {
using ready = core::field_descriptor<port19_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port20_u8 : core::register_descriptor<std::uint8_t, 0x50U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port20_u8_fields {
using data = core::field_descriptor<port20_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port20_u16 : core::register_descriptor<std::uint16_t, 0x50U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port20_u16_fields {
using data = core::field_descriptor<port20_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port20_u32 : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port20_u32_fields {
using data = core::field_descriptor<port20_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port20_status : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port20_status_fields {
using ready = core::field_descriptor<port20_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port21_u8 : core::register_descriptor<std::uint8_t, 0x54U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port21_u8_fields {
using data = core::field_descriptor<port21_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port21_u16 : core::register_descriptor<std::uint16_t, 0x54U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port21_u16_fields {
using data = core::field_descriptor<port21_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port21_u32 : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port21_u32_fields {
using data = core::field_descriptor<port21_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port21_status : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port21_status_fields {
using ready = core::field_descriptor<port21_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port22_u8 : core::register_descriptor<std::uint8_t, 0x58U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port22_u8_fields {
using data = core::field_descriptor<port22_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port22_u16 : core::register_descriptor<std::uint16_t, 0x58U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port22_u16_fields {
using data = core::field_descriptor<port22_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port22_u32 : core::register_descriptor<std::uint32_t, 0x58U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port22_u32_fields {
using data = core::field_descriptor<port22_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port22_status : core::register_descriptor<std::uint32_t, 0x58U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port22_status_fields {
using ready = core::field_descriptor<port22_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port23_u8 : core::register_descriptor<std::uint8_t, 0x5CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port23_u8_fields {
using data = core::field_descriptor<port23_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port23_u16 : core::register_descriptor<std::uint16_t, 0x5CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port23_u16_fields {
using data = core::field_descriptor<port23_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port23_u32 : core::register_descriptor<std::uint32_t, 0x5CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port23_u32_fields {
using data = core::field_descriptor<port23_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port23_status : core::register_descriptor<std::uint32_t, 0x5CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port23_status_fields {
using ready = core::field_descriptor<port23_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port24_u8 : core::register_descriptor<std::uint8_t, 0x60U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port24_u8_fields {
using data = core::field_descriptor<port24_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port24_u16 : core::register_descriptor<std::uint16_t, 0x60U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port24_u16_fields {
using data = core::field_descriptor<port24_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port24_u32 : core::register_descriptor<std::uint32_t, 0x60U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port24_u32_fields {
using data = core::field_descriptor<port24_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port24_status : core::register_descriptor<std::uint32_t, 0x60U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port24_status_fields {
using ready = core::field_descriptor<port24_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port25_u8 : core::register_descriptor<std::uint8_t, 0x64U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port25_u8_fields {
using data = core::field_descriptor<port25_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port25_u16 : core::register_descriptor<std::uint16_t, 0x64U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port25_u16_fields {
using data = core::field_descriptor<port25_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port25_u32 : core::register_descriptor<std::uint32_t, 0x64U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port25_u32_fields {
using data = core::field_descriptor<port25_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port25_status : core::register_descriptor<std::uint32_t, 0x64U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port25_status_fields {
using ready = core::field_descriptor<port25_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port26_u8 : core::register_descriptor<std::uint8_t, 0x68U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port26_u8_fields {
using data = core::field_descriptor<port26_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port26_u16 : core::register_descriptor<std::uint16_t, 0x68U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port26_u16_fields {
using data = core::field_descriptor<port26_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port26_u32 : core::register_descriptor<std::uint32_t, 0x68U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port26_u32_fields {
using data = core::field_descriptor<port26_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port26_status : core::register_descriptor<std::uint32_t, 0x68U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port26_status_fields {
using ready = core::field_descriptor<port26_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port27_u8 : core::register_descriptor<std::uint8_t, 0x6CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port27_u8_fields {
using data = core::field_descriptor<port27_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port27_u16 : core::register_descriptor<std::uint16_t, 0x6CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port27_u16_fields {
using data = core::field_descriptor<port27_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port27_u32 : core::register_descriptor<std::uint32_t, 0x6CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port27_u32_fields {
using data = core::field_descriptor<port27_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port27_status : core::register_descriptor<std::uint32_t, 0x6CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port27_status_fields {
using ready = core::field_descriptor<port27_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port28_u8 : core::register_descriptor<std::uint8_t, 0x70U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port28_u8_fields {
using data = core::field_descriptor<port28_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port28_u16 : core::register_descriptor<std::uint16_t, 0x70U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port28_u16_fields {
using data = core::field_descriptor<port28_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port28_u32 : core::register_descriptor<std::uint32_t, 0x70U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port28_u32_fields {
using data = core::field_descriptor<port28_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port28_status : core::register_descriptor<std::uint32_t, 0x70U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port28_status_fields {
using ready = core::field_descriptor<port28_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port29_u8 : core::register_descriptor<std::uint8_t, 0x74U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port29_u8_fields {
using data = core::field_descriptor<port29_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port29_u16 : core::register_descriptor<std::uint16_t, 0x74U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port29_u16_fields {
using data = core::field_descriptor<port29_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port29_u32 : core::register_descriptor<std::uint32_t, 0x74U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port29_u32_fields {
using data = core::field_descriptor<port29_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port29_status : core::register_descriptor<std::uint32_t, 0x74U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port29_status_fields {
using ready = core::field_descriptor<port29_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port30_u8 : core::register_descriptor<std::uint8_t, 0x78U,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port30_u8_fields {
using data = core::field_descriptor<port30_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port30_u16 : core::register_descriptor<std::uint16_t, 0x78U,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port30_u16_fields {
using data = core::field_descriptor<port30_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port30_u32 : core::register_descriptor<std::uint32_t, 0x78U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port30_u32_fields {
using data = core::field_descriptor<port30_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port30_status : core::register_descriptor<std::uint32_t, 0x78U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port30_status_fields {
using ready = core::field_descriptor<port30_status, 0U, 1U, core::access_semantics::read_only>;
}
struct port31_u8 : core::register_descriptor<std::uint8_t, 0x7CU,
    core::access_semantics::write_only, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port31_u8_fields {
using data = core::field_descriptor<port31_u8, 0U, 8U, core::access_semantics::write_only>;
}
struct port31_u16 : core::register_descriptor<std::uint16_t, 0x7CU,
    core::access_semantics::write_only, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port31_u16_fields {
using data = core::field_descriptor<port31_u16, 0U, 16U, core::access_semantics::write_only>;
}
struct port31_u32 : core::register_descriptor<std::uint32_t, 0x7CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port31_u32_fields {
using data = core::field_descriptor<port31_u32, 0U, 32U, core::access_semantics::write_only>;
}
struct port31_status : core::register_descriptor<std::uint32_t, 0x7CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace port31_status_fields {
using ready = core::field_descriptor<port31_status, 0U, 1U, core::access_semantics::read_only>;
}
}
