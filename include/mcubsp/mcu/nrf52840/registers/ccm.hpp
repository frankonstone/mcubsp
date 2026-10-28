#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::ccm {
struct tasks_ksgen : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ksgen_fields {
using tasks_ksgen = core::field_descriptor<tasks_ksgen, 0U, 1U>;
inline constexpr std::uint32_t tasks_ksgen_trigger = 0x1U;
}
struct tasks_crypt : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_crypt_fields {
using tasks_crypt = core::field_descriptor<tasks_crypt, 0U, 1U>;
inline constexpr std::uint32_t tasks_crypt_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct tasks_rateoverride : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_rateoverride_fields {
using tasks_rateoverride = core::field_descriptor<tasks_rateoverride, 0U, 1U>;
inline constexpr std::uint32_t tasks_rateoverride_trigger = 0x1U;
}
struct events_endksgen : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endksgen_fields {
using events_endksgen = core::field_descriptor<events_endksgen, 0U, 1U>;
inline constexpr std::uint32_t events_endksgen_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endksgen_generated = 0x1U;
}
struct events_endcrypt : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_endcrypt_fields {
using events_endcrypt = core::field_descriptor<events_endcrypt, 0U, 1U>;
inline constexpr std::uint32_t events_endcrypt_notgenerated = 0x0U;
inline constexpr std::uint32_t events_endcrypt_generated = 0x1U;
}
struct events_error : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_error_fields {
using events_error = core::field_descriptor<events_error, 0U, 1U>;
inline constexpr std::uint32_t events_error_notgenerated = 0x0U;
inline constexpr std::uint32_t events_error_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using endksgen_crypt = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t endksgen_crypt_disabled = 0x0U;
inline constexpr std::uint32_t endksgen_crypt_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using endksgen = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t endksgen_disabled = 0x0U;
inline constexpr std::uint32_t endksgen_enabled = 0x1U;
inline constexpr std::uint32_t endksgen_set = 0x1U;
using endcrypt = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t endcrypt_disabled = 0x0U;
inline constexpr std::uint32_t endcrypt_enabled = 0x1U;
inline constexpr std::uint32_t endcrypt_set = 0x1U;
using error = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using endksgen = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t endksgen_disabled = 0x0U;
inline constexpr std::uint32_t endksgen_enabled = 0x1U;
inline constexpr std::uint32_t endksgen_clear = 0x1U;
using endcrypt = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t endcrypt_disabled = 0x0U;
inline constexpr std::uint32_t endcrypt_enabled = 0x1U;
inline constexpr std::uint32_t endcrypt_clear = 0x1U;
using error = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_clear = 0x1U;
}
struct micstatus : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace micstatus_fields {
using micstatus = core::field_descriptor<micstatus, 0U, 1U>;
inline constexpr std::uint32_t micstatus_checkfailed = 0x0U;
inline constexpr std::uint32_t micstatus_checkpassed = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 2U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x2U;
}
struct mode : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x01030001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mode_fields {
using mode = core::field_descriptor<mode, 0U, 1U>;
inline constexpr std::uint32_t mode_encryption = 0x0U;
inline constexpr std::uint32_t mode_decryption = 0x1U;
using datarate = core::field_descriptor<mode, 16U, 2U>;
inline constexpr std::uint32_t datarate_reg_1mbit = 0x0U;
inline constexpr std::uint32_t datarate_reg_2mbit = 0x1U;
inline constexpr std::uint32_t datarate_reg_125kbps = 0x2U;
inline constexpr std::uint32_t datarate_reg_500kbps = 0x3U;
using length = core::field_descriptor<mode, 24U, 1U>;
inline constexpr std::uint32_t length_reg_default = 0x0U;
inline constexpr std::uint32_t length_extended = 0x1U;
}
struct cnfptr : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace cnfptr_fields {
using cnfptr = core::field_descriptor<cnfptr, 0U, 32U>;
}
struct inptr : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inptr_fields {
using inptr = core::field_descriptor<inptr, 0U, 32U>;
}
struct outptr : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace outptr_fields {
using outptr = core::field_descriptor<outptr, 0U, 32U>;
}
struct scratchptr : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace scratchptr_fields {
using scratchptr = core::field_descriptor<scratchptr, 0U, 32U>;
}
struct maxpacketsize : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000000FBU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace maxpacketsize_fields {
using maxpacketsize = core::field_descriptor<maxpacketsize, 0U, 8U>;
}
struct rateoverride : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rateoverride_fields {
using rateoverride = core::field_descriptor<rateoverride, 0U, 2U>;
inline constexpr std::uint32_t rateoverride_reg_1mbit = 0x0U;
inline constexpr std::uint32_t rateoverride_reg_2mbit = 0x1U;
inline constexpr std::uint32_t rateoverride_reg_125kbps = 0x2U;
inline constexpr std::uint32_t rateoverride_reg_500kbps = 0x3U;
}
}
