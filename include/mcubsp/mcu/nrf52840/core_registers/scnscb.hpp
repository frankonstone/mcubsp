#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::scnscb {
inline constexpr std::uintptr_t base = 0xE000E000U;
struct ictr : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ictr_fields {
using intlinesnum = core::field_descriptor<ictr, 0U, 4U, core::access_semantics::read_only>;
}
struct actlr : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0x307U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace actlr_fields {
using disoofp = core::field_descriptor<actlr, 9U, 1U, core::access_semantics::read_write>;
using disfpca = core::field_descriptor<actlr, 8U, 1U, core::access_semantics::read_write>;
using disfold = core::field_descriptor<actlr, 2U, 1U, core::access_semantics::read_write>;
using disdefwbuf = core::field_descriptor<actlr, 1U, 1U, core::access_semantics::read_write>;
using dismcycint = core::field_descriptor<actlr, 0U, 1U, core::access_semantics::read_write>;
}
}
