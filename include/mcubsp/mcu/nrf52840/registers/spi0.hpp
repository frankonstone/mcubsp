#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::spi0 {
struct events_ready : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ready_fields {
using events_ready = core::field_descriptor<events_ready, 0U, 1U>;
inline constexpr std::uint32_t events_ready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ready_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000004U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using ready = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000004U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using ready = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct psel_sck : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sck_fields {
using pin = core::field_descriptor<psel_sck, 0U, 5U>;
using port = core::field_descriptor<psel_sck, 5U, 1U>;
using connect = core::field_descriptor<psel_sck, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_mosi : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_mosi_fields {
using pin = core::field_descriptor<psel_mosi, 0U, 5U>;
using port = core::field_descriptor<psel_mosi, 5U, 1U>;
using connect = core::field_descriptor<psel_mosi, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_miso : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_miso_fields {
using pin = core::field_descriptor<psel_miso, 0U, 5U>;
using port = core::field_descriptor<psel_miso, 5U, 1U>;
using connect = core::field_descriptor<psel_miso, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct rxd : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_clear, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_fields {
using rxd = core::field_descriptor<rxd, 0U, 8U>;
}
struct txd : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_fields {
using txd = core::field_descriptor<txd, 0U, 8U>;
}
struct frequency : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x04000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace frequency_fields {
using frequency = core::field_descriptor<frequency, 0U, 32U>;
inline constexpr std::uint32_t frequency_k125 = 0x2000000U;
inline constexpr std::uint32_t frequency_k250 = 0x4000000U;
inline constexpr std::uint32_t frequency_k500 = 0x8000000U;
inline constexpr std::uint32_t frequency_m1 = 0x10000000U;
inline constexpr std::uint32_t frequency_m2 = 0x20000000U;
inline constexpr std::uint32_t frequency_m4 = 0x40000000U;
inline constexpr std::uint32_t frequency_m8 = 0x80000000U;
}
struct config : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using order = core::field_descriptor<config, 0U, 1U>;
inline constexpr std::uint32_t order_msbfirst = 0x0U;
inline constexpr std::uint32_t order_lsbfirst = 0x1U;
using cpha = core::field_descriptor<config, 1U, 1U>;
inline constexpr std::uint32_t cpha_leading = 0x0U;
inline constexpr std::uint32_t cpha_trailing = 0x1U;
using cpol = core::field_descriptor<config, 2U, 1U>;
inline constexpr std::uint32_t cpol_activehigh = 0x0U;
inline constexpr std::uint32_t cpol_activelow = 0x1U;
}
}
