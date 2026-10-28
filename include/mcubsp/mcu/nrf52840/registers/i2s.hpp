#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::i2s {
struct tasks_start : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct events_rxptrupd : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxptrupd_fields {
using events_rxptrupd = core::field_descriptor<events_rxptrupd, 0U, 1U>;
inline constexpr std::uint32_t events_rxptrupd_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxptrupd_generated = 0x1U;
}
struct events_stopped : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_stopped_fields {
using events_stopped = core::field_descriptor<events_stopped, 0U, 1U>;
inline constexpr std::uint32_t events_stopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_stopped_generated = 0x1U;
}
struct events_txptrupd : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txptrupd_fields {
using events_txptrupd = core::field_descriptor<events_txptrupd, 0U, 1U>;
inline constexpr std::uint32_t events_txptrupd_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txptrupd_generated = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x00000026U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using rxptrupd = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t rxptrupd_disabled = 0x0U;
inline constexpr std::uint32_t rxptrupd_enabled = 0x1U;
using stopped = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
using txptrupd = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t txptrupd_disabled = 0x0U;
inline constexpr std::uint32_t txptrupd_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000026U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using rxptrupd = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t rxptrupd_disabled = 0x0U;
inline constexpr std::uint32_t rxptrupd_enabled = 0x1U;
inline constexpr std::uint32_t rxptrupd_set = 0x1U;
using stopped = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_set = 0x1U;
using txptrupd = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t txptrupd_disabled = 0x0U;
inline constexpr std::uint32_t txptrupd_enabled = 0x1U;
inline constexpr std::uint32_t txptrupd_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000026U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using rxptrupd = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t rxptrupd_disabled = 0x0U;
inline constexpr std::uint32_t rxptrupd_enabled = 0x1U;
inline constexpr std::uint32_t rxptrupd_clear = 0x1U;
using stopped = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t stopped_disabled = 0x0U;
inline constexpr std::uint32_t stopped_enabled = 0x1U;
inline constexpr std::uint32_t stopped_clear = 0x1U;
using txptrupd = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t txptrupd_disabled = 0x0U;
inline constexpr std::uint32_t txptrupd_enabled = 0x1U;
inline constexpr std::uint32_t txptrupd_clear = 0x1U;
}
struct enable : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace enable_fields {
using enable = core::field_descriptor<enable, 0U, 1U>;
inline constexpr std::uint32_t enable_disabled = 0x0U;
inline constexpr std::uint32_t enable_enabled = 0x1U;
}
struct config_mode : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_mode_fields {
using mode = core::field_descriptor<config_mode, 0U, 1U>;
inline constexpr std::uint32_t mode_master = 0x0U;
inline constexpr std::uint32_t mode_slave = 0x1U;
}
struct config_rxen : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_rxen_fields {
using rxen = core::field_descriptor<config_rxen, 0U, 1U>;
inline constexpr std::uint32_t rxen_disabled = 0x0U;
inline constexpr std::uint32_t rxen_enabled = 0x1U;
}
struct config_txen : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_txen_fields {
using txen = core::field_descriptor<config_txen, 0U, 1U>;
inline constexpr std::uint32_t txen_disabled = 0x0U;
inline constexpr std::uint32_t txen_enabled = 0x1U;
}
struct config_mcken : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_mcken_fields {
using mcken = core::field_descriptor<config_mcken, 0U, 1U>;
inline constexpr std::uint32_t mcken_disabled = 0x0U;
inline constexpr std::uint32_t mcken_enabled = 0x1U;
}
struct config_mckfreq : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x20000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_mckfreq_fields {
using mckfreq = core::field_descriptor<config_mckfreq, 0U, 32U>;
inline constexpr std::uint32_t mckfreq_reg_32mdiv8 = 0x20000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv10 = 0x18000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv11 = 0x16000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv15 = 0x11000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv16 = 0x10000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv21 = 0xC000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv23 = 0xB000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv30 = 0x8800000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv31 = 0x8400000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv32 = 0x8000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv42 = 0x6000000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv63 = 0x4100000U;
inline constexpr std::uint32_t mckfreq_reg_32mdiv125 = 0x20C0000U;
}
struct config_ratio : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000006U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_ratio_fields {
using ratio = core::field_descriptor<config_ratio, 0U, 4U>;
inline constexpr std::uint32_t ratio_reg_32x = 0x0U;
inline constexpr std::uint32_t ratio_reg_48x = 0x1U;
inline constexpr std::uint32_t ratio_reg_64x = 0x2U;
inline constexpr std::uint32_t ratio_reg_96x = 0x3U;
inline constexpr std::uint32_t ratio_reg_128x = 0x4U;
inline constexpr std::uint32_t ratio_reg_192x = 0x5U;
inline constexpr std::uint32_t ratio_reg_256x = 0x6U;
inline constexpr std::uint32_t ratio_reg_384x = 0x7U;
inline constexpr std::uint32_t ratio_reg_512x = 0x8U;
}
struct config_swidth : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_swidth_fields {
using swidth = core::field_descriptor<config_swidth, 0U, 2U>;
inline constexpr std::uint32_t swidth_reg_8bit = 0x0U;
inline constexpr std::uint32_t swidth_reg_16bit = 0x1U;
inline constexpr std::uint32_t swidth_reg_24bit = 0x2U;
}
struct config_align : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_align_fields {
using align = core::field_descriptor<config_align, 0U, 1U>;
inline constexpr std::uint32_t align_left = 0x0U;
inline constexpr std::uint32_t align_right = 0x1U;
}
struct config_format : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_format_fields {
using format = core::field_descriptor<config_format, 0U, 1U>;
inline constexpr std::uint32_t format_i2s = 0x0U;
inline constexpr std::uint32_t format_aligned = 0x1U;
}
struct config_channels : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0x00000003U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_channels_fields {
using channels = core::field_descriptor<config_channels, 0U, 2U>;
inline constexpr std::uint32_t channels_stereo = 0x0U;
inline constexpr std::uint32_t channels_left = 0x1U;
inline constexpr std::uint32_t channels_right = 0x2U;
}
struct rxd_ptr : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxd_ptr_fields {
using ptr = core::field_descriptor<rxd_ptr, 0U, 32U>;
}
struct txd_ptr : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txd_ptr_fields {
using ptr = core::field_descriptor<txd_ptr, 0U, 32U>;
}
struct rxtxd_maxcnt : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0x00003FFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxtxd_maxcnt_fields {
using maxcnt = core::field_descriptor<rxtxd_maxcnt, 0U, 14U>;
}
struct psel_mck : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_mck_fields {
using pin = core::field_descriptor<psel_mck, 0U, 5U>;
using port = core::field_descriptor<psel_mck, 5U, 1U>;
using connect = core::field_descriptor<psel_mck, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_sck : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sck_fields {
using pin = core::field_descriptor<psel_sck, 0U, 5U>;
using port = core::field_descriptor<psel_sck, 5U, 1U>;
using connect = core::field_descriptor<psel_sck, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_lrck : core::register_descriptor<std::uint32_t, 0x568U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_lrck_fields {
using pin = core::field_descriptor<psel_lrck, 0U, 5U>;
using port = core::field_descriptor<psel_lrck, 5U, 1U>;
using connect = core::field_descriptor<psel_lrck, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_sdin : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sdin_fields {
using pin = core::field_descriptor<psel_sdin, 0U, 5U>;
using port = core::field_descriptor<psel_sdin, 5U, 1U>;
using connect = core::field_descriptor<psel_sdin, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct psel_sdout : core::register_descriptor<std::uint32_t, 0x570U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace psel_sdout_fields {
using pin = core::field_descriptor<psel_sdout, 0U, 5U>;
using port = core::field_descriptor<psel_sdout, 5U, 1U>;
using connect = core::field_descriptor<psel_sdout, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
}
