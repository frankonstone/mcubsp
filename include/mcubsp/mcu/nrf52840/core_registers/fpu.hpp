#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::fpu {
inline constexpr std::uintptr_t base = 0xE000EF30U;
struct fpccr : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_write, 0xC000017BU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace fpccr_fields {
using aspen = core::field_descriptor<fpccr, 31U, 1U, core::access_semantics::read_write>;
using lspen = core::field_descriptor<fpccr, 30U, 1U, core::access_semantics::read_write>;
using monrdy = core::field_descriptor<fpccr, 8U, 1U, core::access_semantics::read_write>;
using bfrdy = core::field_descriptor<fpccr, 6U, 1U, core::access_semantics::read_write>;
using mmrdy = core::field_descriptor<fpccr, 5U, 1U, core::access_semantics::read_write>;
using hfrdy = core::field_descriptor<fpccr, 4U, 1U, core::access_semantics::read_write>;
using thread = core::field_descriptor<fpccr, 3U, 1U, core::access_semantics::read_write>;
using user = core::field_descriptor<fpccr, 1U, 1U, core::access_semantics::read_write>;
using lspact = core::field_descriptor<fpccr, 0U, 1U, core::access_semantics::read_write>;
}
struct fpcar : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0xFFFFFFF8U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace fpcar_fields {
using address = core::field_descriptor<fpcar, 3U, 29U, core::access_semantics::read_write>;
}
struct fpdscr : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_write, 0x7C00000U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace fpdscr_fields {
using ahp = core::field_descriptor<fpdscr, 26U, 1U, core::access_semantics::read_write>;
using dn = core::field_descriptor<fpdscr, 25U, 1U, core::access_semantics::read_write>;
using fz = core::field_descriptor<fpdscr, 24U, 1U, core::access_semantics::read_write>;
using rmode = core::field_descriptor<fpdscr, 22U, 2U, core::access_semantics::read_write>;
}
struct mvfr0 : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mvfr0_fields {
using fp_rounding_modes = core::field_descriptor<mvfr0, 28U, 4U, core::access_semantics::read_only>;
using short_vectors = core::field_descriptor<mvfr0, 24U, 4U, core::access_semantics::read_only>;
using square_root = core::field_descriptor<mvfr0, 20U, 4U, core::access_semantics::read_only>;
using divide = core::field_descriptor<mvfr0, 16U, 4U, core::access_semantics::read_only>;
using fp_excep_trapping = core::field_descriptor<mvfr0, 12U, 4U, core::access_semantics::read_only>;
using double_precision = core::field_descriptor<mvfr0, 8U, 4U, core::access_semantics::read_only>;
using single_precision = core::field_descriptor<mvfr0, 4U, 4U, core::access_semantics::read_only>;
using a_simd_registers = core::field_descriptor<mvfr0, 0U, 4U, core::access_semantics::read_only>;
}
struct mvfr1 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mvfr1_fields {
using fp_fused_mac = core::field_descriptor<mvfr1, 28U, 4U, core::access_semantics::read_only>;
using fp_hpfp = core::field_descriptor<mvfr1, 24U, 4U, core::access_semantics::read_only>;
using d_nan_mode = core::field_descriptor<mvfr1, 4U, 4U, core::access_semantics::read_only>;
using ftz_mode = core::field_descriptor<mvfr1, 0U, 4U, core::access_semantics::read_only>;
}
}
