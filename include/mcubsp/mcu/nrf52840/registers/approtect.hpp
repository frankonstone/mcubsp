#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::approtect {
struct forceprotect : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace forceprotect_fields {
using forceprotect = core::field_descriptor<forceprotect, 0U, 8U>;
inline constexpr std::uint32_t forceprotect_force = 0x0U;
}
struct disable : core::register_descriptor<std::uint32_t, 0x558U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace disable_fields {
using disable = core::field_descriptor<disable, 0U, 8U>;
inline constexpr std::uint32_t disable_swdisable = 0x5AU;
}
}
