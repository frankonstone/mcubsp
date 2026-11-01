#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::fpb {
inline constexpr std::uintptr_t base = 0xE0002000U;
struct ctrl : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_write, 0x1U, false,
    core::reserved_bit_policy::write_zero, 0x2U, 0x260U, 0xFFFFFFFFU> {};
namespace ctrl_fields {
using rev = core::field_descriptor<ctrl, 28U, 4U, core::access_semantics::read_only>;
using num_code_high = core::field_descriptor<ctrl, 12U, 3U, core::access_semantics::read_only>;
using num_lit = core::field_descriptor<ctrl, 8U, 4U, core::access_semantics::read_only>;
using num_code_low = core::field_descriptor<ctrl, 4U, 4U, core::access_semantics::read_only>;
using key = core::field_descriptor<ctrl, 1U, 1U, core::access_semantics::write_only>;
using enable = core::field_descriptor<ctrl, 0U, 1U, core::access_semantics::read_write>;
}
struct remap : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_write, 0x1FFFFFE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace remap_fields {
using rmpspt = core::field_descriptor<remap, 29U, 1U, core::access_semantics::read_only>;
using remap = core::field_descriptor<remap, 5U, 24U, core::access_semantics::read_write>;
}
struct comp0 : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0xDFFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp0_fields {
using replace = core::field_descriptor<comp0, 30U, 2U, core::access_semantics::read_write>;
using comp = core::field_descriptor<comp0, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp0, 0U, 1U, core::access_semantics::read_write>;
}
struct comp1 : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_write, 0xDFFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp1_fields {
using replace = core::field_descriptor<comp1, 30U, 2U, core::access_semantics::read_write>;
using comp = core::field_descriptor<comp1, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp1, 0U, 1U, core::access_semantics::read_write>;
}
struct comp2 : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_write, 0xDFFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp2_fields {
using replace = core::field_descriptor<comp2, 30U, 2U, core::access_semantics::read_write>;
using comp = core::field_descriptor<comp2, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp2, 0U, 1U, core::access_semantics::read_write>;
}
struct comp3 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_write, 0xDFFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp3_fields {
using replace = core::field_descriptor<comp3, 30U, 2U, core::access_semantics::read_write>;
using comp = core::field_descriptor<comp3, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp3, 0U, 1U, core::access_semantics::read_write>;
}
struct comp4 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::read_write, 0xDFFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp4_fields {
using replace = core::field_descriptor<comp4, 30U, 2U, core::access_semantics::read_write>;
using comp = core::field_descriptor<comp4, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp4, 0U, 1U, core::access_semantics::read_write>;
}
struct comp5 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::read_write, 0xDFFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp5_fields {
using replace = core::field_descriptor<comp5, 30U, 2U, core::access_semantics::read_write>;
using comp = core::field_descriptor<comp5, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp5, 0U, 1U, core::access_semantics::read_write>;
}
struct comp6 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::read_write, 0x1FFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp6_fields {
using comp = core::field_descriptor<comp6, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp6, 0U, 1U, core::access_semantics::read_write>;
}
struct comp7 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_write, 0x1FFFFFFDU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x1U> {};
namespace comp7_fields {
using comp = core::field_descriptor<comp7, 2U, 27U, core::access_semantics::read_write>;
using enable = core::field_descriptor<comp7, 0U, 1U, core::access_semantics::read_write>;
}
struct pid4 : core::register_descriptor<std::uint32_t, 0xFD0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid4_fields {
using id = core::field_descriptor<pid4, 0U, 8U, core::access_semantics::read_only>;
}
struct pid5 : core::register_descriptor<std::uint32_t, 0xFD4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid5_fields {
using id = core::field_descriptor<pid5, 0U, 8U, core::access_semantics::read_only>;
}
struct pid6 : core::register_descriptor<std::uint32_t, 0xFD8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid6_fields {
using id = core::field_descriptor<pid6, 0U, 8U, core::access_semantics::read_only>;
}
struct pid7 : core::register_descriptor<std::uint32_t, 0xFDCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid7_fields {
using id = core::field_descriptor<pid7, 0U, 8U, core::access_semantics::read_only>;
}
struct pid0 : core::register_descriptor<std::uint32_t, 0xFE0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid0_fields {
using id = core::field_descriptor<pid0, 0U, 8U, core::access_semantics::read_only>;
}
struct pid1 : core::register_descriptor<std::uint32_t, 0xFE4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid1_fields {
using id = core::field_descriptor<pid1, 0U, 8U, core::access_semantics::read_only>;
}
struct pid2 : core::register_descriptor<std::uint32_t, 0xFE8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid2_fields {
using id = core::field_descriptor<pid2, 0U, 8U, core::access_semantics::read_only>;
}
struct pid3 : core::register_descriptor<std::uint32_t, 0xFECU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pid3_fields {
using id = core::field_descriptor<pid3, 0U, 8U, core::access_semantics::read_only>;
}
struct cid0 : core::register_descriptor<std::uint32_t, 0xFF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid0_fields {
using id = core::field_descriptor<cid0, 0U, 8U, core::access_semantics::read_only>;
}
struct cid1 : core::register_descriptor<std::uint32_t, 0xFF4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid1_fields {
using id = core::field_descriptor<cid1, 0U, 8U, core::access_semantics::read_only>;
}
struct cid2 : core::register_descriptor<std::uint32_t, 0xFF8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid2_fields {
using id = core::field_descriptor<cid2, 0U, 8U, core::access_semantics::read_only>;
}
struct cid3 : core::register_descriptor<std::uint32_t, 0xFFCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cid3_fields {
using id = core::field_descriptor<cid3, 0U, 8U, core::access_semantics::read_only>;
}
}
