#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::uart0 {
struct tasks_startrx : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_startrx_fields {
using tasks_startrx = core::field_descriptor<tasks_startrx, 0U, 1U>;
inline constexpr std::uint32_t tasks_startrx_trigger = 0x1U;
}
struct tasks_stoprx : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stoprx_fields {
using tasks_stoprx = core::field_descriptor<tasks_stoprx, 0U, 1U>;
inline constexpr std::uint32_t tasks_stoprx_trigger = 0x1U;
}
struct tasks_starttx : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_starttx_fields {
using tasks_starttx = core::field_descriptor<tasks_starttx, 0U, 1U>;
inline constexpr std::uint32_t tasks_starttx_trigger = 0x1U;
}
struct tasks_stoptx : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stoptx_fields {
using tasks_stoptx = core::field_descriptor<tasks_stoptx, 0U, 1U>;
inline constexpr std::uint32_t tasks_stoptx_trigger = 0x1U;
}
struct tasks_suspend : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_suspend_fields {
using tasks_suspend = core::field_descriptor<tasks_suspend, 0U, 1U>;
inline constexpr std::uint32_t tasks_suspend_trigger = 0x1U;
}
struct events_cts : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_cts_fields {
using events_cts = core::field_descriptor<events_cts, 0U, 1U>;
inline constexpr std::uint32_t events_cts_notgenerated = 0x0U;
inline constexpr std::uint32_t events_cts_generated = 0x1U;
}
struct events_ncts : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ncts_fields {
using events_ncts = core::field_descriptor<events_ncts, 0U, 1U>;
inline constexpr std::uint32_t events_ncts_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ncts_generated = 0x1U;
}
struct events_rxdrdy : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxdrdy_fields {
using events_rxdrdy = core::field_descriptor<events_rxdrdy, 0U, 1U>;
inline constexpr std::uint32_t events_rxdrdy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxdrdy_generated = 0x1U;
}
struct events_txdrdy : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txdrdy_fields {
using events_txdrdy = core::field_descriptor<events_txdrdy, 0U, 1U>;
inline constexpr std::uint32_t events_txdrdy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txdrdy_generated = 0x1U;
}
struct events_error : core::register_descriptor<std::uint32_t, 0x124U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_error_fields {
using events_error = core::field_descriptor<events_error, 0U, 1U>;
inline constexpr std::uint32_t events_error_notgenerated = 0x0U;
inline constexpr std::uint32_t events_error_generated = 0x1U;
}
struct events_rxto : core::register_descriptor<std::uint32_t, 0x144U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxto_fields {
using events_rxto = core::field_descriptor<events_rxto, 0U, 1U>;
inline constexpr std::uint32_t events_rxto_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxto_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x00000018U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using cts_startrx = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t cts_startrx_disabled = 0x0U;
inline constexpr std::uint32_t cts_startrx_enabled = 0x1U;
using ncts_stoprx = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t ncts_stoprx_disabled = 0x0U;
inline constexpr std::uint32_t ncts_stoprx_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00020287U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using cts = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t cts_disabled = 0x0U;
inline constexpr std::uint32_t cts_enabled = 0x1U;
inline constexpr std::uint32_t cts_set = 0x1U;
using ncts = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t ncts_disabled = 0x0U;
inline constexpr std::uint32_t ncts_enabled = 0x1U;
inline constexpr std::uint32_t ncts_set = 0x1U;
using rxdrdy = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t rxdrdy_disabled = 0x0U;
inline constexpr std::uint32_t rxdrdy_enabled = 0x1U;
inline constexpr std::uint32_t rxdrdy_set = 0x1U;
using txdrdy = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t txdrdy_disabled = 0x0U;
inline constexpr std::uint32_t txdrdy_enabled = 0x1U;
inline constexpr std::uint32_t txdrdy_set = 0x1U;
using error = core::field_descriptor<intenset, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_set = 0x1U;
using rxto = core::field_descriptor<intenset, 17U, 1U>;
inline constexpr std::uint32_t rxto_disabled = 0x0U;
inline constexpr std::uint32_t rxto_enabled = 0x1U;
inline constexpr std::uint32_t rxto_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00020287U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using cts = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t cts_disabled = 0x0U;
inline constexpr std::uint32_t cts_enabled = 0x1U;
inline constexpr std::uint32_t cts_clear = 0x1U;
using ncts = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t ncts_disabled = 0x0U;
inline constexpr std::uint32_t ncts_enabled = 0x1U;
inline constexpr std::uint32_t ncts_clear = 0x1U;
using rxdrdy = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t rxdrdy_disabled = 0x0U;
inline constexpr std::uint32_t rxdrdy_enabled = 0x1U;
inline constexpr std::uint32_t rxdrdy_clear = 0x1U;
using txdrdy = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t txdrdy_disabled = 0x0U;
inline constexpr std::uint32_t txdrdy_enabled = 0x1U;
inline constexpr std::uint32_t txdrdy_clear = 0x1U;
using error = core::field_descriptor<intenclr, 9U, 1U>;
inline constexpr std::uint32_t error_disabled = 0x0U;
inline constexpr std::uint32_t error_enabled = 0x1U;
inline constexpr std::uint32_t error_clear = 0x1U;
using rxto = core::field_descriptor<intenclr, 17U, 1U>;
inline constexpr std::uint32_t rxto_disabled = 0x0U;
inline constexpr std::uint32_t rxto_enabled = 0x1U;
inline constexpr std::uint32_t rxto_clear = 0x1U;
}
struct errorsrc : core::register_descriptor<std::uint32_t, 0x480U,
    core::access_semantics::write_one_to_clear, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace errorsrc_fields {
using overrun = core::field_descriptor<errorsrc, 0U, 1U>;
inline constexpr std::uint32_t overrun_notpresent = 0x0U;
inline constexpr std::uint32_t overrun_present = 0x1U;
using parity = core::field_descriptor<errorsrc, 1U, 1U>;
inline constexpr std::uint32_t parity_notpresent = 0x0U;
inline constexpr std::uint32_t parity_present = 0x1U;
using framing = core::field_descriptor<errorsrc, 2U, 1U>;
inline constexpr std::uint32_t framing_notpresent = 0x0U;
inline constexpr std::uint32_t framing_present = 0x1U;
using reg_break = core::field_descriptor<errorsrc, 3U, 1U>;
inline constexpr std::uint32_t reg_break_notpresent = 0x0U;
inline constexpr std::uint32_t reg_break_present = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 4U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x4U;
}
struct psel_rts : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_rts_fields {
using pin = core::field_descriptor<psel_rts, 0U, 5U>;
using port = core::field_descriptor<psel_rts, 5U, 1U>;
using connect = core::field_descriptor<psel_rts, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_txd : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_txd_fields {
using pin = core::field_descriptor<psel_txd, 0U, 5U>;
using port = core::field_descriptor<psel_txd, 5U, 1U>;
using connect = core::field_descriptor<psel_txd, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_cts : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_cts_fields {
using pin = core::field_descriptor<psel_cts, 0U, 5U>;
using port = core::field_descriptor<psel_cts, 5U, 1U>;
using connect = core::field_descriptor<psel_cts, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_rxd : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_rxd_fields {
using pin = core::field_descriptor<psel_rxd, 0U, 5U>;
using port = core::field_descriptor<psel_rxd, 5U, 1U>;
using connect = core::field_descriptor<psel_rxd, 31U, 1U>;
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
    core::access_semantics::write_only, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_fields {
using txd = core::field_descriptor<txd, 0U, 8U>;
}
struct baudrate : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x04000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace baudrate_fields {
using baudrate = core::field_descriptor<baudrate, 0U, 32U>;
inline constexpr std::uint32_t baudrate_baud1200 = 0x4F000U;
inline constexpr std::uint32_t baudrate_baud2400 = 0x9D000U;
inline constexpr std::uint32_t baudrate_baud4800 = 0x13B000U;
inline constexpr std::uint32_t baudrate_baud9600 = 0x275000U;
inline constexpr std::uint32_t baudrate_baud14400 = 0x3B0000U;
inline constexpr std::uint32_t baudrate_baud19200 = 0x4EA000U;
inline constexpr std::uint32_t baudrate_baud28800 = 0x75F000U;
inline constexpr std::uint32_t baudrate_baud31250 = 0x800000U;
inline constexpr std::uint32_t baudrate_baud38400 = 0x9D5000U;
inline constexpr std::uint32_t baudrate_baud56000 = 0xE50000U;
inline constexpr std::uint32_t baudrate_baud57600 = 0xEBF000U;
inline constexpr std::uint32_t baudrate_baud76800 = 0x13A9000U;
inline constexpr std::uint32_t baudrate_baud115200 = 0x1D7E000U;
inline constexpr std::uint32_t baudrate_baud230400 = 0x3AFB000U;
inline constexpr std::uint32_t baudrate_baud250000 = 0x4000000U;
inline constexpr std::uint32_t baudrate_baud460800 = 0x75F7000U;
inline constexpr std::uint32_t baudrate_baud921600 = 0xEBED000U;
inline constexpr std::uint32_t baudrate_baud1m = 0x10000000U;
}
struct config : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0x0000001FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using hwfc = core::field_descriptor<config, 0U, 1U>;
inline constexpr std::uint32_t hwfc_disabled = 0x0U;
inline constexpr std::uint32_t hwfc_enabled = 0x1U;
using parity = core::field_descriptor<config, 1U, 3U>;
inline constexpr std::uint32_t parity_excluded = 0x0U;
inline constexpr std::uint32_t parity_included = 0x7U;
using stop = core::field_descriptor<config, 4U, 1U>;
inline constexpr std::uint32_t stop_one = 0x0U;
inline constexpr std::uint32_t stop_two = 0x1U;
}
}
