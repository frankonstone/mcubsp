#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::acl {
struct acl0_addr : core::register_descriptor<std::uint32_t, 0x800U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl0_addr_fields {
using addr = core::field_descriptor<acl0_addr, 0U, 32U>;
}
struct acl0_size : core::register_descriptor<std::uint32_t, 0x804U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl0_size_fields {
using size = core::field_descriptor<acl0_size, 0U, 32U>;
}
struct acl0_perm : core::register_descriptor<std::uint32_t, 0x808U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl0_perm_fields {
using write = core::field_descriptor<acl0_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl0_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl1_addr : core::register_descriptor<std::uint32_t, 0x810U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl1_addr_fields {
using addr = core::field_descriptor<acl1_addr, 0U, 32U>;
}
struct acl1_size : core::register_descriptor<std::uint32_t, 0x814U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl1_size_fields {
using size = core::field_descriptor<acl1_size, 0U, 32U>;
}
struct acl1_perm : core::register_descriptor<std::uint32_t, 0x818U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl1_perm_fields {
using write = core::field_descriptor<acl1_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl1_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl2_addr : core::register_descriptor<std::uint32_t, 0x820U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl2_addr_fields {
using addr = core::field_descriptor<acl2_addr, 0U, 32U>;
}
struct acl2_size : core::register_descriptor<std::uint32_t, 0x824U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl2_size_fields {
using size = core::field_descriptor<acl2_size, 0U, 32U>;
}
struct acl2_perm : core::register_descriptor<std::uint32_t, 0x828U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl2_perm_fields {
using write = core::field_descriptor<acl2_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl2_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl3_addr : core::register_descriptor<std::uint32_t, 0x830U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl3_addr_fields {
using addr = core::field_descriptor<acl3_addr, 0U, 32U>;
}
struct acl3_size : core::register_descriptor<std::uint32_t, 0x834U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl3_size_fields {
using size = core::field_descriptor<acl3_size, 0U, 32U>;
}
struct acl3_perm : core::register_descriptor<std::uint32_t, 0x838U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl3_perm_fields {
using write = core::field_descriptor<acl3_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl3_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl4_addr : core::register_descriptor<std::uint32_t, 0x840U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl4_addr_fields {
using addr = core::field_descriptor<acl4_addr, 0U, 32U>;
}
struct acl4_size : core::register_descriptor<std::uint32_t, 0x844U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl4_size_fields {
using size = core::field_descriptor<acl4_size, 0U, 32U>;
}
struct acl4_perm : core::register_descriptor<std::uint32_t, 0x848U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl4_perm_fields {
using write = core::field_descriptor<acl4_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl4_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl5_addr : core::register_descriptor<std::uint32_t, 0x850U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl5_addr_fields {
using addr = core::field_descriptor<acl5_addr, 0U, 32U>;
}
struct acl5_size : core::register_descriptor<std::uint32_t, 0x854U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl5_size_fields {
using size = core::field_descriptor<acl5_size, 0U, 32U>;
}
struct acl5_perm : core::register_descriptor<std::uint32_t, 0x858U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl5_perm_fields {
using write = core::field_descriptor<acl5_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl5_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl6_addr : core::register_descriptor<std::uint32_t, 0x860U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl6_addr_fields {
using addr = core::field_descriptor<acl6_addr, 0U, 32U>;
}
struct acl6_size : core::register_descriptor<std::uint32_t, 0x864U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl6_size_fields {
using size = core::field_descriptor<acl6_size, 0U, 32U>;
}
struct acl6_perm : core::register_descriptor<std::uint32_t, 0x868U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl6_perm_fields {
using write = core::field_descriptor<acl6_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl6_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
struct acl7_addr : core::register_descriptor<std::uint32_t, 0x870U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl7_addr_fields {
using addr = core::field_descriptor<acl7_addr, 0U, 32U>;
}
struct acl7_size : core::register_descriptor<std::uint32_t, 0x874U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl7_size_fields {
using size = core::field_descriptor<acl7_size, 0U, 32U>;
}
struct acl7_perm : core::register_descriptor<std::uint32_t, 0x878U,
    core::access_semantics::read_write, 0x00000006U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = true; };
namespace acl7_perm_fields {
using write = core::field_descriptor<acl7_perm, 1U, 1U>;
inline constexpr std::uint32_t write_enable = 0x0U;
inline constexpr std::uint32_t write_disable = 0x1U;
using read = core::field_descriptor<acl7_perm, 2U, 1U>;
inline constexpr std::uint32_t read_enable = 0x0U;
inline constexpr std::uint32_t read_disable = 0x1U;
}
}
