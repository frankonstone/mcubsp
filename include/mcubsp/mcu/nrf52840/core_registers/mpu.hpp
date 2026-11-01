#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::mpu {
inline constexpr std::uintptr_t base = 0xE000ED90U;
struct type : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace type_fields {
using iregion = core::field_descriptor<type, 16U, 8U, core::access_semantics::read_only>;
using dregion = core::field_descriptor<type, 8U, 8U, core::access_semantics::read_only>;
using separate = core::field_descriptor<type, 0U, 1U, core::access_semantics::read_only>;
}
struct ctrl : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_write, 0x7U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ctrl_fields {
using privdefena = core::field_descriptor<ctrl, 2U, 1U, core::access_semantics::read_write>;
using hfnmiena = core::field_descriptor<ctrl, 1U, 1U, core::access_semantics::read_write>;
using enable = core::field_descriptor<ctrl, 0U, 1U, core::access_semantics::read_write>;
}
struct rnr : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rnr_fields {
using region = core::field_descriptor<rnr, 0U, 8U, core::access_semantics::read_write>;
}
struct rbar : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rbar_fields {
using addr = core::field_descriptor<rbar, 5U, 27U, core::access_semantics::read_write>;
using valid = core::field_descriptor<rbar, 4U, 1U, core::access_semantics::read_write>;
using region = core::field_descriptor<rbar, 0U, 4U, core::access_semantics::read_write>;
}
struct rasr : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_write, 0xFFFFFF3FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rasr_fields {
using attrs = core::field_descriptor<rasr, 16U, 16U, core::access_semantics::read_write>;
using xn = core::field_descriptor<rasr, 28U, 1U, core::access_semantics::read_write>;
using ap = core::field_descriptor<rasr, 24U, 3U, core::access_semantics::read_write>;
using tex = core::field_descriptor<rasr, 19U, 3U, core::access_semantics::read_write>;
using s = core::field_descriptor<rasr, 18U, 1U, core::access_semantics::read_write>;
using c = core::field_descriptor<rasr, 17U, 1U, core::access_semantics::read_write>;
using b = core::field_descriptor<rasr, 16U, 1U, core::access_semantics::read_write>;
using srd = core::field_descriptor<rasr, 8U, 8U, core::access_semantics::read_write>;
using size = core::field_descriptor<rasr, 1U, 5U, core::access_semantics::read_write>;
using enable = core::field_descriptor<rasr, 0U, 1U, core::access_semantics::read_write>;
}
struct rbar_a1 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rbar_a1_fields {
using addr = core::field_descriptor<rbar_a1, 5U, 27U, core::access_semantics::read_write>;
using valid = core::field_descriptor<rbar_a1, 4U, 1U, core::access_semantics::read_write>;
using region = core::field_descriptor<rbar_a1, 0U, 4U, core::access_semantics::read_write>;
}
struct rasr_a1 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::read_write, 0xFFFFFF3FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rasr_a1_fields {
using attrs = core::field_descriptor<rasr_a1, 16U, 16U, core::access_semantics::read_write>;
using xn = core::field_descriptor<rasr_a1, 28U, 1U, core::access_semantics::read_write>;
using ap = core::field_descriptor<rasr_a1, 24U, 3U, core::access_semantics::read_write>;
using tex = core::field_descriptor<rasr_a1, 19U, 3U, core::access_semantics::read_write>;
using s = core::field_descriptor<rasr_a1, 18U, 1U, core::access_semantics::read_write>;
using c = core::field_descriptor<rasr_a1, 17U, 1U, core::access_semantics::read_write>;
using b = core::field_descriptor<rasr_a1, 16U, 1U, core::access_semantics::read_write>;
using srd = core::field_descriptor<rasr_a1, 8U, 8U, core::access_semantics::read_write>;
using size = core::field_descriptor<rasr_a1, 1U, 5U, core::access_semantics::read_write>;
using enable = core::field_descriptor<rasr_a1, 0U, 1U, core::access_semantics::read_write>;
}
struct rbar_a2 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rbar_a2_fields {
using addr = core::field_descriptor<rbar_a2, 5U, 27U, core::access_semantics::read_write>;
using valid = core::field_descriptor<rbar_a2, 4U, 1U, core::access_semantics::read_write>;
using region = core::field_descriptor<rbar_a2, 0U, 4U, core::access_semantics::read_write>;
}
struct rasr_a2 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::read_write, 0xFFFFFF3FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rasr_a2_fields {
using attrs = core::field_descriptor<rasr_a2, 16U, 16U, core::access_semantics::read_write>;
using xn = core::field_descriptor<rasr_a2, 28U, 1U, core::access_semantics::read_write>;
using ap = core::field_descriptor<rasr_a2, 24U, 3U, core::access_semantics::read_write>;
using tex = core::field_descriptor<rasr_a2, 19U, 3U, core::access_semantics::read_write>;
using s = core::field_descriptor<rasr_a2, 18U, 1U, core::access_semantics::read_write>;
using c = core::field_descriptor<rasr_a2, 17U, 1U, core::access_semantics::read_write>;
using b = core::field_descriptor<rasr_a2, 16U, 1U, core::access_semantics::read_write>;
using srd = core::field_descriptor<rasr_a2, 8U, 8U, core::access_semantics::read_write>;
using size = core::field_descriptor<rasr_a2, 1U, 5U, core::access_semantics::read_write>;
using enable = core::field_descriptor<rasr_a2, 0U, 1U, core::access_semantics::read_write>;
}
struct rbar_a3 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rbar_a3_fields {
using addr = core::field_descriptor<rbar_a3, 5U, 27U, core::access_semantics::read_write>;
using valid = core::field_descriptor<rbar_a3, 4U, 1U, core::access_semantics::read_write>;
using region = core::field_descriptor<rbar_a3, 0U, 4U, core::access_semantics::read_write>;
}
struct rasr_a3 : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::read_write, 0xFFFFFF3FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace rasr_a3_fields {
using attrs = core::field_descriptor<rasr_a3, 16U, 16U, core::access_semantics::read_write>;
using xn = core::field_descriptor<rasr_a3, 28U, 1U, core::access_semantics::read_write>;
using ap = core::field_descriptor<rasr_a3, 24U, 3U, core::access_semantics::read_write>;
using tex = core::field_descriptor<rasr_a3, 19U, 3U, core::access_semantics::read_write>;
using s = core::field_descriptor<rasr_a3, 18U, 1U, core::access_semantics::read_write>;
using c = core::field_descriptor<rasr_a3, 17U, 1U, core::access_semantics::read_write>;
using b = core::field_descriptor<rasr_a3, 16U, 1U, core::access_semantics::read_write>;
using srd = core::field_descriptor<rasr_a3, 8U, 8U, core::access_semantics::read_write>;
using size = core::field_descriptor<rasr_a3, 1U, 5U, core::access_semantics::read_write>;
using enable = core::field_descriptor<rasr_a3, 0U, 1U, core::access_semantics::read_write>;
}
}
