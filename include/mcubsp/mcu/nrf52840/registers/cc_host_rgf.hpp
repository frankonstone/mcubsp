#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::cc_host_rgf {
struct host_cryptokey_sel : core::register_descriptor<std::uint32_t, 0x1A38U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_cryptokey_sel_fields {
using host_cryptokey_sel = core::field_descriptor<host_cryptokey_sel, 0U, 2U>;
inline constexpr std::uint32_t host_cryptokey_sel_k_dr = 0x0U;
inline constexpr std::uint32_t host_cryptokey_sel_k_prtl = 0x1U;
inline constexpr std::uint32_t host_cryptokey_sel_session = 0x2U;
}
struct host_iot_kprtl_lock : core::register_descriptor<std::uint32_t, 0x1A4CU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_iot_kprtl_lock_fields {
using host_iot_kprtl_lock = core::field_descriptor<host_iot_kprtl_lock, 0U, 1U>;
inline constexpr std::uint32_t host_iot_kprtl_lock_disabled = 0x0U;
inline constexpr std::uint32_t host_iot_kprtl_lock_enabled = 0x1U;
}
struct host_iot_kdr0 : core::register_descriptor<std::uint32_t, 0x1A50U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_iot_kdr0_fields {
using host_iot_kdr0 = core::field_descriptor<host_iot_kdr0, 0U, 32U>;
}
struct host_iot_kdr1 : core::register_descriptor<std::uint32_t, 0x1A54U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_iot_kdr1_fields {
using host_iot_kdr1 = core::field_descriptor<host_iot_kdr1, 0U, 32U>;
}
struct host_iot_kdr2 : core::register_descriptor<std::uint32_t, 0x1A58U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_iot_kdr2_fields {
using host_iot_kdr2 = core::field_descriptor<host_iot_kdr2, 0U, 32U>;
}
struct host_iot_kdr3 : core::register_descriptor<std::uint32_t, 0x1A5CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_iot_kdr3_fields {
using host_iot_kdr3 = core::field_descriptor<host_iot_kdr3, 0U, 32U>;
}
struct host_iot_lcs : core::register_descriptor<std::uint32_t, 0x1A60U,
    core::access_semantics::read_write, 0x00000107U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000002U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace host_iot_lcs_fields {
using lcs = core::field_descriptor<host_iot_lcs, 0U, 3U>;
inline constexpr std::uint32_t lcs_debug = 0x0U;
inline constexpr std::uint32_t lcs_secure = 0x2U;
using lcs_is_valid = core::field_descriptor<host_iot_lcs, 8U, 1U>;
inline constexpr std::uint32_t lcs_is_valid_invalid = 0x0U;
inline constexpr std::uint32_t lcs_is_valid_valid = 0x1U;
}
}
