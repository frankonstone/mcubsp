#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::debug {
inline constexpr std::uintptr_t base = 0xE000EDF0U;
struct dhcsr : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_write_side_effect, 0x2FU, false,
    core::reserved_bit_policy::write_zero, 0xA05F0000U, 0x0U, 0x0U> {};
namespace dhcsr_fields {
using dbgkey = core::field_descriptor<dhcsr, 16U, 16U, core::access_semantics::read_write_side_effect>;
using s_reset_st = core::field_descriptor<dhcsr, 25U, 1U, core::access_semantics::read_clear>;
using s_retire_st = core::field_descriptor<dhcsr, 24U, 1U, core::access_semantics::read_clear>;
using s_lockup = core::field_descriptor<dhcsr, 19U, 1U, core::access_semantics::read_only>;
using s_sleep = core::field_descriptor<dhcsr, 18U, 1U, core::access_semantics::read_only>;
using s_halt = core::field_descriptor<dhcsr, 17U, 1U, core::access_semantics::read_only>;
using s_regrdy = core::field_descriptor<dhcsr, 16U, 1U, core::access_semantics::read_only>;
using c_snapstall = core::field_descriptor<dhcsr, 5U, 1U, core::access_semantics::read_write_side_effect>;
using c_maskints = core::field_descriptor<dhcsr, 3U, 1U, core::access_semantics::read_write_side_effect>;
using c_step = core::field_descriptor<dhcsr, 2U, 1U, core::access_semantics::read_write_side_effect>;
using c_halt = core::field_descriptor<dhcsr, 1U, 1U, core::access_semantics::read_write_side_effect>;
using c_debugen = core::field_descriptor<dhcsr, 0U, 1U, core::access_semantics::read_write_side_effect>;
}
struct dcrsr : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x1007FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace dcrsr_fields {
using regwnr = core::field_descriptor<dcrsr, 16U, 1U, core::access_semantics::command>;
using regsel = core::field_descriptor<dcrsr, 0U, 7U, core::access_semantics::command>;
}
struct dcrdr : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace dcrdr_fields {
using value = core::field_descriptor<dcrdr, 0U, 32U, core::access_semantics::read_write>;
}
struct demcr : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_write, 0x10F07F1U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace demcr_fields {
using trcena = core::field_descriptor<demcr, 24U, 1U, core::access_semantics::read_write>;
using mon_req = core::field_descriptor<demcr, 19U, 1U, core::access_semantics::read_write>;
using mon_step = core::field_descriptor<demcr, 18U, 1U, core::access_semantics::read_write>;
using mon_pend = core::field_descriptor<demcr, 17U, 1U, core::access_semantics::read_write>;
using mon_en = core::field_descriptor<demcr, 16U, 1U, core::access_semantics::read_write>;
using vc_harderr = core::field_descriptor<demcr, 10U, 1U, core::access_semantics::read_write>;
using vc_interr = core::field_descriptor<demcr, 9U, 1U, core::access_semantics::read_write>;
using vc_buserr = core::field_descriptor<demcr, 8U, 1U, core::access_semantics::read_write>;
using vc_staterr = core::field_descriptor<demcr, 7U, 1U, core::access_semantics::read_write>;
using vc_chkerr = core::field_descriptor<demcr, 6U, 1U, core::access_semantics::read_write>;
using vc_nocperr = core::field_descriptor<demcr, 5U, 1U, core::access_semantics::read_write>;
using vc_mmerr = core::field_descriptor<demcr, 4U, 1U, core::access_semantics::read_write>;
using vc_corereset = core::field_descriptor<demcr, 0U, 1U, core::access_semantics::read_write>;
}
}
