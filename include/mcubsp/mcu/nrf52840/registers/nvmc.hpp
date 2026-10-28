#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::nvmc {
struct ready : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ready_fields {
using ready = core::field_descriptor<ready, 0U, 1U>;
inline constexpr std::uint32_t ready_busy = 0x0U;
inline constexpr std::uint32_t ready_ready = 0x1U;
}
struct readynext : core::register_descriptor<std::uint32_t, 0x408U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace readynext_fields {
using readynext = core::field_descriptor<readynext, 0U, 1U>;
inline constexpr std::uint32_t readynext_busy = 0x0U;
inline constexpr std::uint32_t readynext_ready = 0x1U;
}
struct config : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using wen = core::field_descriptor<config, 0U, 2U>;
inline constexpr std::uint32_t wen_ren = 0x0U;
inline constexpr std::uint32_t wen_wen = 0x1U;
inline constexpr std::uint32_t wen_een = 0x2U;
}
struct erasepage : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erasepage_fields {
using erasepage = core::field_descriptor<erasepage, 0U, 32U>;
}
struct erasepcr1 : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erasepcr1_fields {
using erasepcr1 = core::field_descriptor<erasepcr1, 0U, 32U>;
}
struct eraseall : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::write_only, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace eraseall_fields {
using eraseall = core::field_descriptor<eraseall, 0U, 1U>;
inline constexpr std::uint32_t eraseall_nooperation = 0x0U;
inline constexpr std::uint32_t eraseall_erase = 0x1U;
}
struct erasepcr0 : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erasepcr0_fields {
using erasepcr0 = core::field_descriptor<erasepcr0, 0U, 32U>;
}
struct eraseuicr : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::write_only, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace eraseuicr_fields {
using eraseuicr = core::field_descriptor<eraseuicr, 0U, 1U>;
inline constexpr std::uint32_t eraseuicr_nooperation = 0x0U;
inline constexpr std::uint32_t eraseuicr_erase = 0x1U;
}
struct erasepagepartial : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erasepagepartial_fields {
using erasepagepartial = core::field_descriptor<erasepagepartial, 0U, 32U>;
}
struct erasepagepartialcfg : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x0000000AU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace erasepagepartialcfg_fields {
using duration = core::field_descriptor<erasepagepartialcfg, 0U, 7U>;
}
struct icachecnf : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0x00000101U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace icachecnf_fields {
using cacheen = core::field_descriptor<icachecnf, 0U, 1U>;
inline constexpr std::uint32_t cacheen_disabled = 0x0U;
inline constexpr std::uint32_t cacheen_enabled = 0x1U;
using cacheprofen = core::field_descriptor<icachecnf, 8U, 1U>;
inline constexpr std::uint32_t cacheprofen_disabled = 0x0U;
inline constexpr std::uint32_t cacheprofen_enabled = 0x1U;
}
struct ihit : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ihit_fields {
using hits = core::field_descriptor<ihit, 0U, 32U>;
}
struct imiss : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace imiss_fields {
using misses = core::field_descriptor<imiss, 0U, 32U>;
}
}
