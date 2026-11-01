#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::systick {
inline constexpr std::uintptr_t base = 0xE000E010U;
struct ctrl : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_write_side_effect, 0x7U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ctrl_fields {
using countflag = core::field_descriptor<ctrl, 16U, 1U, core::access_semantics::read_clear>;
using clksource = core::field_descriptor<ctrl, 2U, 1U, core::access_semantics::read_write_side_effect>;
using tickint = core::field_descriptor<ctrl, 1U, 1U, core::access_semantics::read_write_side_effect>;
using enable = core::field_descriptor<ctrl, 0U, 1U, core::access_semantics::read_write_side_effect>;
}
struct load : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_write, 0xFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace load_fields {
using reload = core::field_descriptor<load, 0U, 24U, core::access_semantics::read_write>;
}
struct val : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0xFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using val_status = core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, val>;
namespace val_fields {
using current = core::field_descriptor<val, 0U, 24U, core::access_semantics::command>;
}
struct calib : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace calib_fields {
using noref = core::field_descriptor<calib, 31U, 1U, core::access_semantics::read_only>;
using skew = core::field_descriptor<calib, 30U, 1U, core::access_semantics::read_only>;
using tenms = core::field_descriptor<calib, 0U, 24U, core::access_semantics::read_only>;
}
}
