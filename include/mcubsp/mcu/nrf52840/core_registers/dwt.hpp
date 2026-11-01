#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::dwt {
inline constexpr std::uintptr_t base = 0xE0001000U;
struct ctrl : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_write, 0x7F1FFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ctrl_fields {
using numcomp = core::field_descriptor<ctrl, 28U, 4U, core::access_semantics::read_only>;
using notrcpkt = core::field_descriptor<ctrl, 27U, 1U, core::access_semantics::read_only>;
using noexttrig = core::field_descriptor<ctrl, 26U, 1U, core::access_semantics::read_only>;
using nocyccnt = core::field_descriptor<ctrl, 25U, 1U, core::access_semantics::read_only>;
using noprfcnt = core::field_descriptor<ctrl, 24U, 1U, core::access_semantics::read_only>;
using cycevtena = core::field_descriptor<ctrl, 22U, 1U, core::access_semantics::read_write>;
using foldevtena = core::field_descriptor<ctrl, 21U, 1U, core::access_semantics::read_write>;
using lsuevtena = core::field_descriptor<ctrl, 20U, 1U, core::access_semantics::read_write>;
using sleepevtena = core::field_descriptor<ctrl, 19U, 1U, core::access_semantics::read_write>;
using excevtena = core::field_descriptor<ctrl, 18U, 1U, core::access_semantics::read_write>;
using cpievtena = core::field_descriptor<ctrl, 17U, 1U, core::access_semantics::read_write>;
using exctrcena = core::field_descriptor<ctrl, 16U, 1U, core::access_semantics::read_write>;
using pcsamplena = core::field_descriptor<ctrl, 12U, 1U, core::access_semantics::read_write>;
using synctap = core::field_descriptor<ctrl, 10U, 2U, core::access_semantics::read_write>;
using cyctap = core::field_descriptor<ctrl, 9U, 1U, core::access_semantics::read_write>;
using postinit = core::field_descriptor<ctrl, 5U, 4U, core::access_semantics::read_write>;
using postpreset = core::field_descriptor<ctrl, 1U, 4U, core::access_semantics::read_write>;
using cyccntena = core::field_descriptor<ctrl, 0U, 1U, core::access_semantics::read_write>;
}
struct cyccnt : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cyccnt_fields {
using value = core::field_descriptor<cyccnt, 0U, 32U, core::access_semantics::read_write>;
}
struct cpicnt : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cpicnt_fields {
using cpicnt = core::field_descriptor<cpicnt, 0U, 8U, core::access_semantics::read_write>;
}
struct exccnt : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace exccnt_fields {
using exccnt = core::field_descriptor<exccnt, 0U, 8U, core::access_semantics::read_write>;
}
struct sleepcnt : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace sleepcnt_fields {
using sleepcnt = core::field_descriptor<sleepcnt, 0U, 8U, core::access_semantics::read_write>;
}
struct lsucnt : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace lsucnt_fields {
using lsucnt = core::field_descriptor<lsucnt, 0U, 8U, core::access_semantics::read_write>;
}
struct foldcnt : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace foldcnt_fields {
using foldcnt = core::field_descriptor<foldcnt, 0U, 8U, core::access_semantics::read_write>;
}
struct pcsr : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pcsr_fields {
using value = core::field_descriptor<pcsr, 0U, 32U, core::access_semantics::read_only>;
}
struct comp0 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace comp0_fields {
using value = core::field_descriptor<comp0, 0U, 32U, core::access_semantics::read_write>;
}
struct mask0 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_write, 0x1FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mask0_fields {
using mask = core::field_descriptor<mask0, 0U, 5U, core::access_semantics::read_write>;
}
struct function0 : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::read_write_side_effect, 0xAFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace function0_fields {
using matched = core::field_descriptor<function0, 24U, 1U, core::access_semantics::read_clear>;
using cycmatch = core::field_descriptor<function0, 7U, 1U, core::access_semantics::read_write_side_effect>;
using emitrange = core::field_descriptor<function0, 5U, 1U, core::access_semantics::read_write_side_effect>;
using function = core::field_descriptor<function0, 0U, 4U, core::access_semantics::read_write_side_effect>;
}
struct comp1 : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace comp1_fields {
using value = core::field_descriptor<comp1, 0U, 32U, core::access_semantics::read_write>;
}
struct mask1 : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::read_write, 0x1FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mask1_fields {
using mask = core::field_descriptor<mask1, 0U, 5U, core::access_semantics::read_write>;
}
struct function1 : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::read_write_side_effect, 0xFFD2FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace function1_fields {
using matched = core::field_descriptor<function1, 24U, 1U, core::access_semantics::read_clear>;
using datavaddr1 = core::field_descriptor<function1, 16U, 4U, core::access_semantics::read_write_side_effect>;
using datavaddr0 = core::field_descriptor<function1, 12U, 4U, core::access_semantics::read_write_side_effect>;
using datavsize = core::field_descriptor<function1, 10U, 2U, core::access_semantics::read_write_side_effect>;
using lnk1ena = core::field_descriptor<function1, 9U, 1U, core::access_semantics::read_only>;
using datavmatch = core::field_descriptor<function1, 8U, 1U, core::access_semantics::read_write_side_effect>;
using emitrange = core::field_descriptor<function1, 5U, 1U, core::access_semantics::read_write_side_effect>;
using function = core::field_descriptor<function1, 0U, 4U, core::access_semantics::read_write_side_effect>;
}
struct comp2 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace comp2_fields {
using value = core::field_descriptor<comp2, 0U, 32U, core::access_semantics::read_write>;
}
struct mask2 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::read_write, 0x1FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mask2_fields {
using mask = core::field_descriptor<mask2, 0U, 5U, core::access_semantics::read_write>;
}
struct function2 : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::read_write_side_effect, 0x2FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace function2_fields {
using matched = core::field_descriptor<function2, 24U, 1U, core::access_semantics::read_clear>;
using emitrange = core::field_descriptor<function2, 5U, 1U, core::access_semantics::read_write_side_effect>;
using function = core::field_descriptor<function2, 0U, 4U, core::access_semantics::read_write_side_effect>;
}
struct comp3 : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace comp3_fields {
using value = core::field_descriptor<comp3, 0U, 32U, core::access_semantics::read_write>;
}
struct mask3 : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::read_write, 0x1FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mask3_fields {
using mask = core::field_descriptor<mask3, 0U, 5U, core::access_semantics::read_write>;
}
struct function3 : core::register_descriptor<std::uint32_t, 0x58U,
    core::access_semantics::read_write_side_effect, 0x2FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace function3_fields {
using matched = core::field_descriptor<function3, 24U, 1U, core::access_semantics::read_clear>;
using emitrange = core::field_descriptor<function3, 5U, 1U, core::access_semantics::read_write_side_effect>;
using function = core::field_descriptor<function3, 0U, 4U, core::access_semantics::read_write_side_effect>;
}
struct pid4 : core::register_descriptor<std::uint32_t, 0xFD0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x4U, 0xFFFFFFFFU> {};
namespace pid4_fields {
using id = core::field_descriptor<pid4, 0U, 8U, core::access_semantics::read_only>;
}
struct pid5 : core::register_descriptor<std::uint32_t, 0xFD4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid5_fields {
using id = core::field_descriptor<pid5, 0U, 8U, core::access_semantics::read_only>;
}
struct pid6 : core::register_descriptor<std::uint32_t, 0xFD8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid6_fields {
using id = core::field_descriptor<pid6, 0U, 8U, core::access_semantics::read_only>;
}
struct pid7 : core::register_descriptor<std::uint32_t, 0xFDCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid7_fields {
using id = core::field_descriptor<pid7, 0U, 8U, core::access_semantics::read_only>;
}
struct pid0 : core::register_descriptor<std::uint32_t, 0xFE0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x2U, 0xFFFFFFFFU> {};
namespace pid0_fields {
using id = core::field_descriptor<pid0, 0U, 8U, core::access_semantics::read_only>;
}
struct pid1 : core::register_descriptor<std::uint32_t, 0xFE4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xB0U, 0xFFFFFFFFU> {};
namespace pid1_fields {
using id = core::field_descriptor<pid1, 0U, 8U, core::access_semantics::read_only>;
}
struct pid2 : core::register_descriptor<std::uint32_t, 0xFE8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x3BU, 0xFFFFFFFFU> {};
namespace pid2_fields {
using id = core::field_descriptor<pid2, 0U, 8U, core::access_semantics::read_only>;
}
struct pid3 : core::register_descriptor<std::uint32_t, 0xFECU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid3_fields {
using id = core::field_descriptor<pid3, 0U, 8U, core::access_semantics::read_only>;
}
struct cid0 : core::register_descriptor<std::uint32_t, 0xFF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xDU, 0xFFFFFFFFU> {};
namespace cid0_fields {
using id = core::field_descriptor<cid0, 0U, 8U, core::access_semantics::read_only>;
}
struct cid1 : core::register_descriptor<std::uint32_t, 0xFF4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xE0U, 0xFFFFFFFFU> {};
namespace cid1_fields {
using id = core::field_descriptor<cid1, 0U, 8U, core::access_semantics::read_only>;
}
struct cid2 : core::register_descriptor<std::uint32_t, 0xFF8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x5U, 0xFFFFFFFFU> {};
namespace cid2_fields {
using id = core::field_descriptor<cid2, 0U, 8U, core::access_semantics::read_only>;
}
struct cid3 : core::register_descriptor<std::uint32_t, 0xFFCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xB1U, 0xFFFFFFFFU> {};
namespace cid3_fields {
using id = core::field_descriptor<cid3, 0U, 8U, core::access_semantics::read_only>;
}
}
