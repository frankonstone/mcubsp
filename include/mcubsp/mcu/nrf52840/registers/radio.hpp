#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::radio {
struct tasks_txen : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_txen_fields {
using tasks_txen = core::field_descriptor<tasks_txen, 0U, 1U>;
inline constexpr std::uint32_t tasks_txen_trigger = 0x1U;
}
struct tasks_rxen : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_rxen_fields {
using tasks_rxen = core::field_descriptor<tasks_rxen, 0U, 1U>;
inline constexpr std::uint32_t tasks_rxen_trigger = 0x1U;
}
struct tasks_start : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
}
struct tasks_stop : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_stop_fields {
using tasks_stop = core::field_descriptor<tasks_stop, 0U, 1U>;
inline constexpr std::uint32_t tasks_stop_trigger = 0x1U;
}
struct tasks_disable : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_disable_fields {
using tasks_disable = core::field_descriptor<tasks_disable, 0U, 1U>;
inline constexpr std::uint32_t tasks_disable_trigger = 0x1U;
}
struct tasks_rssistart : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_rssistart_fields {
using tasks_rssistart = core::field_descriptor<tasks_rssistart, 0U, 1U>;
inline constexpr std::uint32_t tasks_rssistart_trigger = 0x1U;
}
struct tasks_rssistop : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_rssistop_fields {
using tasks_rssistop = core::field_descriptor<tasks_rssistop, 0U, 1U>;
inline constexpr std::uint32_t tasks_rssistop_trigger = 0x1U;
}
struct tasks_bcstart : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_bcstart_fields {
using tasks_bcstart = core::field_descriptor<tasks_bcstart, 0U, 1U>;
inline constexpr std::uint32_t tasks_bcstart_trigger = 0x1U;
}
struct tasks_bcstop : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_bcstop_fields {
using tasks_bcstop = core::field_descriptor<tasks_bcstop, 0U, 1U>;
inline constexpr std::uint32_t tasks_bcstop_trigger = 0x1U;
}
struct tasks_edstart : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_edstart_fields {
using tasks_edstart = core::field_descriptor<tasks_edstart, 0U, 1U>;
inline constexpr std::uint32_t tasks_edstart_trigger = 0x1U;
}
struct tasks_edstop : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_edstop_fields {
using tasks_edstop = core::field_descriptor<tasks_edstop, 0U, 1U>;
inline constexpr std::uint32_t tasks_edstop_trigger = 0x1U;
}
struct tasks_ccastart : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ccastart_fields {
using tasks_ccastart = core::field_descriptor<tasks_ccastart, 0U, 1U>;
inline constexpr std::uint32_t tasks_ccastart_trigger = 0x1U;
}
struct tasks_ccastop : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_ccastop_fields {
using tasks_ccastop = core::field_descriptor<tasks_ccastop, 0U, 1U>;
inline constexpr std::uint32_t tasks_ccastop_trigger = 0x1U;
}
struct events_ready : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ready_fields {
using events_ready = core::field_descriptor<events_ready, 0U, 1U>;
inline constexpr std::uint32_t events_ready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ready_generated = 0x1U;
}
struct events_address : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_address_fields {
using events_address = core::field_descriptor<events_address, 0U, 1U>;
inline constexpr std::uint32_t events_address_notgenerated = 0x0U;
inline constexpr std::uint32_t events_address_generated = 0x1U;
}
struct events_payload : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_payload_fields {
using events_payload = core::field_descriptor<events_payload, 0U, 1U>;
inline constexpr std::uint32_t events_payload_notgenerated = 0x0U;
inline constexpr std::uint32_t events_payload_generated = 0x1U;
}
struct events_end : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_end_fields {
using events_end = core::field_descriptor<events_end, 0U, 1U>;
inline constexpr std::uint32_t events_end_notgenerated = 0x0U;
inline constexpr std::uint32_t events_end_generated = 0x1U;
}
struct events_disabled : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_disabled_fields {
using events_disabled = core::field_descriptor<events_disabled, 0U, 1U>;
inline constexpr std::uint32_t events_disabled_notgenerated = 0x0U;
inline constexpr std::uint32_t events_disabled_generated = 0x1U;
}
struct events_devmatch : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_devmatch_fields {
using events_devmatch = core::field_descriptor<events_devmatch, 0U, 1U>;
inline constexpr std::uint32_t events_devmatch_notgenerated = 0x0U;
inline constexpr std::uint32_t events_devmatch_generated = 0x1U;
}
struct events_devmiss : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_devmiss_fields {
using events_devmiss = core::field_descriptor<events_devmiss, 0U, 1U>;
inline constexpr std::uint32_t events_devmiss_notgenerated = 0x0U;
inline constexpr std::uint32_t events_devmiss_generated = 0x1U;
}
struct events_rssiend : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rssiend_fields {
using events_rssiend = core::field_descriptor<events_rssiend, 0U, 1U>;
inline constexpr std::uint32_t events_rssiend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rssiend_generated = 0x1U;
}
struct events_bcmatch : core::register_descriptor<std::uint32_t, 0x128U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_bcmatch_fields {
using events_bcmatch = core::field_descriptor<events_bcmatch, 0U, 1U>;
inline constexpr std::uint32_t events_bcmatch_notgenerated = 0x0U;
inline constexpr std::uint32_t events_bcmatch_generated = 0x1U;
}
struct events_crcok : core::register_descriptor<std::uint32_t, 0x130U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_crcok_fields {
using events_crcok = core::field_descriptor<events_crcok, 0U, 1U>;
inline constexpr std::uint32_t events_crcok_notgenerated = 0x0U;
inline constexpr std::uint32_t events_crcok_generated = 0x1U;
}
struct events_crcerror : core::register_descriptor<std::uint32_t, 0x134U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_crcerror_fields {
using events_crcerror = core::field_descriptor<events_crcerror, 0U, 1U>;
inline constexpr std::uint32_t events_crcerror_notgenerated = 0x0U;
inline constexpr std::uint32_t events_crcerror_generated = 0x1U;
}
struct events_framestart : core::register_descriptor<std::uint32_t, 0x138U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_framestart_fields {
using events_framestart = core::field_descriptor<events_framestart, 0U, 1U>;
inline constexpr std::uint32_t events_framestart_notgenerated = 0x0U;
inline constexpr std::uint32_t events_framestart_generated = 0x1U;
}
struct events_edend : core::register_descriptor<std::uint32_t, 0x13CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_edend_fields {
using events_edend = core::field_descriptor<events_edend, 0U, 1U>;
inline constexpr std::uint32_t events_edend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_edend_generated = 0x1U;
}
struct events_edstopped : core::register_descriptor<std::uint32_t, 0x140U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_edstopped_fields {
using events_edstopped = core::field_descriptor<events_edstopped, 0U, 1U>;
inline constexpr std::uint32_t events_edstopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_edstopped_generated = 0x1U;
}
struct events_ccaidle : core::register_descriptor<std::uint32_t, 0x144U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ccaidle_fields {
using events_ccaidle = core::field_descriptor<events_ccaidle, 0U, 1U>;
inline constexpr std::uint32_t events_ccaidle_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ccaidle_generated = 0x1U;
}
struct events_ccabusy : core::register_descriptor<std::uint32_t, 0x148U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ccabusy_fields {
using events_ccabusy = core::field_descriptor<events_ccabusy, 0U, 1U>;
inline constexpr std::uint32_t events_ccabusy_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ccabusy_generated = 0x1U;
}
struct events_ccastopped : core::register_descriptor<std::uint32_t, 0x14CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_ccastopped_fields {
using events_ccastopped = core::field_descriptor<events_ccastopped, 0U, 1U>;
inline constexpr std::uint32_t events_ccastopped_notgenerated = 0x0U;
inline constexpr std::uint32_t events_ccastopped_generated = 0x1U;
}
struct events_rateboost : core::register_descriptor<std::uint32_t, 0x150U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rateboost_fields {
using events_rateboost = core::field_descriptor<events_rateboost, 0U, 1U>;
inline constexpr std::uint32_t events_rateboost_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rateboost_generated = 0x1U;
}
struct events_txready : core::register_descriptor<std::uint32_t, 0x154U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_txready_fields {
using events_txready = core::field_descriptor<events_txready, 0U, 1U>;
inline constexpr std::uint32_t events_txready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_txready_generated = 0x1U;
}
struct events_rxready : core::register_descriptor<std::uint32_t, 0x158U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_rxready_fields {
using events_rxready = core::field_descriptor<events_rxready, 0U, 1U>;
inline constexpr std::uint32_t events_rxready_notgenerated = 0x0U;
inline constexpr std::uint32_t events_rxready_generated = 0x1U;
}
struct events_mhrmatch : core::register_descriptor<std::uint32_t, 0x15CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_mhrmatch_fields {
using events_mhrmatch = core::field_descriptor<events_mhrmatch, 0U, 1U>;
inline constexpr std::uint32_t events_mhrmatch_notgenerated = 0x0U;
inline constexpr std::uint32_t events_mhrmatch_generated = 0x1U;
}
struct events_sync : core::register_descriptor<std::uint32_t, 0x168U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_sync_fields {
using events_sync = core::field_descriptor<events_sync, 0U, 1U>;
inline constexpr std::uint32_t events_sync_notgenerated = 0x0U;
inline constexpr std::uint32_t events_sync_generated = 0x1U;
}
struct events_phyend : core::register_descriptor<std::uint32_t, 0x16CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_phyend_fields {
using events_phyend = core::field_descriptor<events_phyend, 0U, 1U>;
inline constexpr std::uint32_t events_phyend_notgenerated = 0x0U;
inline constexpr std::uint32_t events_phyend_generated = 0x1U;
}
struct shorts : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x003FF97FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace shorts_fields {
using ready_start = core::field_descriptor<shorts, 0U, 1U>;
inline constexpr std::uint32_t ready_start_disabled = 0x0U;
inline constexpr std::uint32_t ready_start_enabled = 0x1U;
using end_disable = core::field_descriptor<shorts, 1U, 1U>;
inline constexpr std::uint32_t end_disable_disabled = 0x0U;
inline constexpr std::uint32_t end_disable_enabled = 0x1U;
using disabled_txen = core::field_descriptor<shorts, 2U, 1U>;
inline constexpr std::uint32_t disabled_txen_disabled = 0x0U;
inline constexpr std::uint32_t disabled_txen_enabled = 0x1U;
using disabled_rxen = core::field_descriptor<shorts, 3U, 1U>;
inline constexpr std::uint32_t disabled_rxen_disabled = 0x0U;
inline constexpr std::uint32_t disabled_rxen_enabled = 0x1U;
using address_rssistart = core::field_descriptor<shorts, 4U, 1U>;
inline constexpr std::uint32_t address_rssistart_disabled = 0x0U;
inline constexpr std::uint32_t address_rssistart_enabled = 0x1U;
using end_start = core::field_descriptor<shorts, 5U, 1U>;
inline constexpr std::uint32_t end_start_disabled = 0x0U;
inline constexpr std::uint32_t end_start_enabled = 0x1U;
using address_bcstart = core::field_descriptor<shorts, 6U, 1U>;
inline constexpr std::uint32_t address_bcstart_disabled = 0x0U;
inline constexpr std::uint32_t address_bcstart_enabled = 0x1U;
using disabled_rssistop = core::field_descriptor<shorts, 8U, 1U>;
inline constexpr std::uint32_t disabled_rssistop_disabled = 0x0U;
inline constexpr std::uint32_t disabled_rssistop_enabled = 0x1U;
using rxready_ccastart = core::field_descriptor<shorts, 11U, 1U>;
inline constexpr std::uint32_t rxready_ccastart_disabled = 0x0U;
inline constexpr std::uint32_t rxready_ccastart_enabled = 0x1U;
using ccaidle_txen = core::field_descriptor<shorts, 12U, 1U>;
inline constexpr std::uint32_t ccaidle_txen_disabled = 0x0U;
inline constexpr std::uint32_t ccaidle_txen_enabled = 0x1U;
using ccabusy_disable = core::field_descriptor<shorts, 13U, 1U>;
inline constexpr std::uint32_t ccabusy_disable_disabled = 0x0U;
inline constexpr std::uint32_t ccabusy_disable_enabled = 0x1U;
using framestart_bcstart = core::field_descriptor<shorts, 14U, 1U>;
inline constexpr std::uint32_t framestart_bcstart_disabled = 0x0U;
inline constexpr std::uint32_t framestart_bcstart_enabled = 0x1U;
using ready_edstart = core::field_descriptor<shorts, 15U, 1U>;
inline constexpr std::uint32_t ready_edstart_disabled = 0x0U;
inline constexpr std::uint32_t ready_edstart_enabled = 0x1U;
using edend_disable = core::field_descriptor<shorts, 16U, 1U>;
inline constexpr std::uint32_t edend_disable_disabled = 0x0U;
inline constexpr std::uint32_t edend_disable_enabled = 0x1U;
using ccaidle_stop = core::field_descriptor<shorts, 17U, 1U>;
inline constexpr std::uint32_t ccaidle_stop_disabled = 0x0U;
inline constexpr std::uint32_t ccaidle_stop_enabled = 0x1U;
using txready_start = core::field_descriptor<shorts, 18U, 1U>;
inline constexpr std::uint32_t txready_start_disabled = 0x0U;
inline constexpr std::uint32_t txready_start_enabled = 0x1U;
using rxready_start = core::field_descriptor<shorts, 19U, 1U>;
inline constexpr std::uint32_t rxready_start_disabled = 0x0U;
inline constexpr std::uint32_t rxready_start_enabled = 0x1U;
using phyend_disable = core::field_descriptor<shorts, 20U, 1U>;
inline constexpr std::uint32_t phyend_disable_disabled = 0x0U;
inline constexpr std::uint32_t phyend_disable_enabled = 0x1U;
using phyend_start = core::field_descriptor<shorts, 21U, 1U>;
inline constexpr std::uint32_t phyend_start_disabled = 0x0U;
inline constexpr std::uint32_t phyend_start_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x0CFFF4FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using ready = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_set = 0x1U;
using address = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t address_disabled = 0x0U;
inline constexpr std::uint32_t address_enabled = 0x1U;
inline constexpr std::uint32_t address_set = 0x1U;
using payload = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t payload_disabled = 0x0U;
inline constexpr std::uint32_t payload_enabled = 0x1U;
inline constexpr std::uint32_t payload_set = 0x1U;
using end = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_set = 0x1U;
using disabled = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t disabled_disabled = 0x0U;
inline constexpr std::uint32_t disabled_enabled = 0x1U;
inline constexpr std::uint32_t disabled_set = 0x1U;
using devmatch = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t devmatch_disabled = 0x0U;
inline constexpr std::uint32_t devmatch_enabled = 0x1U;
inline constexpr std::uint32_t devmatch_set = 0x1U;
using devmiss = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t devmiss_disabled = 0x0U;
inline constexpr std::uint32_t devmiss_enabled = 0x1U;
inline constexpr std::uint32_t devmiss_set = 0x1U;
using rssiend = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t rssiend_disabled = 0x0U;
inline constexpr std::uint32_t rssiend_enabled = 0x1U;
inline constexpr std::uint32_t rssiend_set = 0x1U;
using bcmatch = core::field_descriptor<intenset, 10U, 1U>;
inline constexpr std::uint32_t bcmatch_disabled = 0x0U;
inline constexpr std::uint32_t bcmatch_enabled = 0x1U;
inline constexpr std::uint32_t bcmatch_set = 0x1U;
using crcok = core::field_descriptor<intenset, 12U, 1U>;
inline constexpr std::uint32_t crcok_disabled = 0x0U;
inline constexpr std::uint32_t crcok_enabled = 0x1U;
inline constexpr std::uint32_t crcok_set = 0x1U;
using crcerror = core::field_descriptor<intenset, 13U, 1U>;
inline constexpr std::uint32_t crcerror_disabled = 0x0U;
inline constexpr std::uint32_t crcerror_enabled = 0x1U;
inline constexpr std::uint32_t crcerror_set = 0x1U;
using framestart = core::field_descriptor<intenset, 14U, 1U>;
inline constexpr std::uint32_t framestart_disabled = 0x0U;
inline constexpr std::uint32_t framestart_enabled = 0x1U;
inline constexpr std::uint32_t framestart_set = 0x1U;
using edend = core::field_descriptor<intenset, 15U, 1U>;
inline constexpr std::uint32_t edend_disabled = 0x0U;
inline constexpr std::uint32_t edend_enabled = 0x1U;
inline constexpr std::uint32_t edend_set = 0x1U;
using edstopped = core::field_descriptor<intenset, 16U, 1U>;
inline constexpr std::uint32_t edstopped_disabled = 0x0U;
inline constexpr std::uint32_t edstopped_enabled = 0x1U;
inline constexpr std::uint32_t edstopped_set = 0x1U;
using ccaidle = core::field_descriptor<intenset, 17U, 1U>;
inline constexpr std::uint32_t ccaidle_disabled = 0x0U;
inline constexpr std::uint32_t ccaidle_enabled = 0x1U;
inline constexpr std::uint32_t ccaidle_set = 0x1U;
using ccabusy = core::field_descriptor<intenset, 18U, 1U>;
inline constexpr std::uint32_t ccabusy_disabled = 0x0U;
inline constexpr std::uint32_t ccabusy_enabled = 0x1U;
inline constexpr std::uint32_t ccabusy_set = 0x1U;
using ccastopped = core::field_descriptor<intenset, 19U, 1U>;
inline constexpr std::uint32_t ccastopped_disabled = 0x0U;
inline constexpr std::uint32_t ccastopped_enabled = 0x1U;
inline constexpr std::uint32_t ccastopped_set = 0x1U;
using rateboost = core::field_descriptor<intenset, 20U, 1U>;
inline constexpr std::uint32_t rateboost_disabled = 0x0U;
inline constexpr std::uint32_t rateboost_enabled = 0x1U;
inline constexpr std::uint32_t rateboost_set = 0x1U;
using txready = core::field_descriptor<intenset, 21U, 1U>;
inline constexpr std::uint32_t txready_disabled = 0x0U;
inline constexpr std::uint32_t txready_enabled = 0x1U;
inline constexpr std::uint32_t txready_set = 0x1U;
using rxready = core::field_descriptor<intenset, 22U, 1U>;
inline constexpr std::uint32_t rxready_disabled = 0x0U;
inline constexpr std::uint32_t rxready_enabled = 0x1U;
inline constexpr std::uint32_t rxready_set = 0x1U;
using mhrmatch = core::field_descriptor<intenset, 23U, 1U>;
inline constexpr std::uint32_t mhrmatch_disabled = 0x0U;
inline constexpr std::uint32_t mhrmatch_enabled = 0x1U;
inline constexpr std::uint32_t mhrmatch_set = 0x1U;
using sync = core::field_descriptor<intenset, 26U, 1U>;
inline constexpr std::uint32_t sync_disabled = 0x0U;
inline constexpr std::uint32_t sync_enabled = 0x1U;
inline constexpr std::uint32_t sync_set = 0x1U;
using phyend = core::field_descriptor<intenset, 27U, 1U>;
inline constexpr std::uint32_t phyend_disabled = 0x0U;
inline constexpr std::uint32_t phyend_enabled = 0x1U;
inline constexpr std::uint32_t phyend_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x0CFFF4FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using ready = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t ready_disabled = 0x0U;
inline constexpr std::uint32_t ready_enabled = 0x1U;
inline constexpr std::uint32_t ready_clear = 0x1U;
using address = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t address_disabled = 0x0U;
inline constexpr std::uint32_t address_enabled = 0x1U;
inline constexpr std::uint32_t address_clear = 0x1U;
using payload = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t payload_disabled = 0x0U;
inline constexpr std::uint32_t payload_enabled = 0x1U;
inline constexpr std::uint32_t payload_clear = 0x1U;
using end = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t end_disabled = 0x0U;
inline constexpr std::uint32_t end_enabled = 0x1U;
inline constexpr std::uint32_t end_clear = 0x1U;
using disabled = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t disabled_disabled = 0x0U;
inline constexpr std::uint32_t disabled_enabled = 0x1U;
inline constexpr std::uint32_t disabled_clear = 0x1U;
using devmatch = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t devmatch_disabled = 0x0U;
inline constexpr std::uint32_t devmatch_enabled = 0x1U;
inline constexpr std::uint32_t devmatch_clear = 0x1U;
using devmiss = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t devmiss_disabled = 0x0U;
inline constexpr std::uint32_t devmiss_enabled = 0x1U;
inline constexpr std::uint32_t devmiss_clear = 0x1U;
using rssiend = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t rssiend_disabled = 0x0U;
inline constexpr std::uint32_t rssiend_enabled = 0x1U;
inline constexpr std::uint32_t rssiend_clear = 0x1U;
using bcmatch = core::field_descriptor<intenclr, 10U, 1U>;
inline constexpr std::uint32_t bcmatch_disabled = 0x0U;
inline constexpr std::uint32_t bcmatch_enabled = 0x1U;
inline constexpr std::uint32_t bcmatch_clear = 0x1U;
using crcok = core::field_descriptor<intenclr, 12U, 1U>;
inline constexpr std::uint32_t crcok_disabled = 0x0U;
inline constexpr std::uint32_t crcok_enabled = 0x1U;
inline constexpr std::uint32_t crcok_clear = 0x1U;
using crcerror = core::field_descriptor<intenclr, 13U, 1U>;
inline constexpr std::uint32_t crcerror_disabled = 0x0U;
inline constexpr std::uint32_t crcerror_enabled = 0x1U;
inline constexpr std::uint32_t crcerror_clear = 0x1U;
using framestart = core::field_descriptor<intenclr, 14U, 1U>;
inline constexpr std::uint32_t framestart_disabled = 0x0U;
inline constexpr std::uint32_t framestart_enabled = 0x1U;
inline constexpr std::uint32_t framestart_clear = 0x1U;
using edend = core::field_descriptor<intenclr, 15U, 1U>;
inline constexpr std::uint32_t edend_disabled = 0x0U;
inline constexpr std::uint32_t edend_enabled = 0x1U;
inline constexpr std::uint32_t edend_clear = 0x1U;
using edstopped = core::field_descriptor<intenclr, 16U, 1U>;
inline constexpr std::uint32_t edstopped_disabled = 0x0U;
inline constexpr std::uint32_t edstopped_enabled = 0x1U;
inline constexpr std::uint32_t edstopped_clear = 0x1U;
using ccaidle = core::field_descriptor<intenclr, 17U, 1U>;
inline constexpr std::uint32_t ccaidle_disabled = 0x0U;
inline constexpr std::uint32_t ccaidle_enabled = 0x1U;
inline constexpr std::uint32_t ccaidle_clear = 0x1U;
using ccabusy = core::field_descriptor<intenclr, 18U, 1U>;
inline constexpr std::uint32_t ccabusy_disabled = 0x0U;
inline constexpr std::uint32_t ccabusy_enabled = 0x1U;
inline constexpr std::uint32_t ccabusy_clear = 0x1U;
using ccastopped = core::field_descriptor<intenclr, 19U, 1U>;
inline constexpr std::uint32_t ccastopped_disabled = 0x0U;
inline constexpr std::uint32_t ccastopped_enabled = 0x1U;
inline constexpr std::uint32_t ccastopped_clear = 0x1U;
using rateboost = core::field_descriptor<intenclr, 20U, 1U>;
inline constexpr std::uint32_t rateboost_disabled = 0x0U;
inline constexpr std::uint32_t rateboost_enabled = 0x1U;
inline constexpr std::uint32_t rateboost_clear = 0x1U;
using txready = core::field_descriptor<intenclr, 21U, 1U>;
inline constexpr std::uint32_t txready_disabled = 0x0U;
inline constexpr std::uint32_t txready_enabled = 0x1U;
inline constexpr std::uint32_t txready_clear = 0x1U;
using rxready = core::field_descriptor<intenclr, 22U, 1U>;
inline constexpr std::uint32_t rxready_disabled = 0x0U;
inline constexpr std::uint32_t rxready_enabled = 0x1U;
inline constexpr std::uint32_t rxready_clear = 0x1U;
using mhrmatch = core::field_descriptor<intenclr, 23U, 1U>;
inline constexpr std::uint32_t mhrmatch_disabled = 0x0U;
inline constexpr std::uint32_t mhrmatch_enabled = 0x1U;
inline constexpr std::uint32_t mhrmatch_clear = 0x1U;
using sync = core::field_descriptor<intenclr, 26U, 1U>;
inline constexpr std::uint32_t sync_disabled = 0x0U;
inline constexpr std::uint32_t sync_enabled = 0x1U;
inline constexpr std::uint32_t sync_clear = 0x1U;
using phyend = core::field_descriptor<intenclr, 27U, 1U>;
inline constexpr std::uint32_t phyend_disabled = 0x0U;
inline constexpr std::uint32_t phyend_enabled = 0x1U;
inline constexpr std::uint32_t phyend_clear = 0x1U;
}
struct crcstatus : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace crcstatus_fields {
using crcstatus = core::field_descriptor<crcstatus, 0U, 1U>;
inline constexpr std::uint32_t crcstatus_crcerror = 0x0U;
inline constexpr std::uint32_t crcstatus_crcok = 0x1U;
}
struct rxmatch : core::register_descriptor<std::uint32_t, 0x408U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxmatch_fields {
using rxmatch = core::field_descriptor<rxmatch, 0U, 3U>;
}
struct rxcrc : core::register_descriptor<std::uint32_t, 0x40CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxcrc_fields {
using rxcrc = core::field_descriptor<rxcrc, 0U, 24U>;
}
struct dai : core::register_descriptor<std::uint32_t, 0x410U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dai_fields {
using dai = core::field_descriptor<dai, 0U, 3U>;
}
struct pdustat : core::register_descriptor<std::uint32_t, 0x414U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pdustat_fields {
using pdustat = core::field_descriptor<pdustat, 0U, 1U>;
inline constexpr std::uint32_t pdustat_lessthan = 0x0U;
inline constexpr std::uint32_t pdustat_greaterthan = 0x1U;
using cistat = core::field_descriptor<pdustat, 1U, 2U>;
inline constexpr std::uint32_t cistat_lr125kbit = 0x0U;
inline constexpr std::uint32_t cistat_lr500kbit = 0x1U;
}
struct packetptr : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace packetptr_fields {
using packetptr = core::field_descriptor<packetptr, 0U, 32U>;
}
struct frequency : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x0000017FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000002U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace frequency_fields {
using frequency = core::field_descriptor<frequency, 0U, 7U>;
using map = core::field_descriptor<frequency, 8U, 1U>;
inline constexpr std::uint32_t map_reg_default = 0x0U;
inline constexpr std::uint32_t map_low = 0x1U;
}
struct txpower : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txpower_fields {
using txpower = core::field_descriptor<txpower, 0U, 8U>;
inline constexpr std::uint32_t txpower_pos8dbm = 0x8U;
inline constexpr std::uint32_t txpower_pos7dbm = 0x7U;
inline constexpr std::uint32_t txpower_pos6dbm = 0x6U;
inline constexpr std::uint32_t txpower_pos5dbm = 0x5U;
inline constexpr std::uint32_t txpower_pos4dbm = 0x4U;
inline constexpr std::uint32_t txpower_pos3dbm = 0x3U;
inline constexpr std::uint32_t txpower_pos2dbm = 0x2U;
inline constexpr std::uint32_t txpower_reg_0dbm = 0x0U;
inline constexpr std::uint32_t txpower_neg4dbm = 0xFCU;
inline constexpr std::uint32_t txpower_neg8dbm = 0xF8U;
inline constexpr std::uint32_t txpower_neg12dbm = 0xF4U;
inline constexpr std::uint32_t txpower_neg16dbm = 0xF0U;
inline constexpr std::uint32_t txpower_neg20dbm = 0xECU;
inline constexpr std::uint32_t txpower_neg30dbm = 0xE2U;
inline constexpr std::uint32_t txpower_neg40dbm = 0xD8U;
}
struct mode : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x0000000FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mode_fields {
using mode = core::field_descriptor<mode, 0U, 4U>;
inline constexpr std::uint32_t mode_nrf_1mbit = 0x0U;
inline constexpr std::uint32_t mode_nrf_2mbit = 0x1U;
inline constexpr std::uint32_t mode_ble_1mbit = 0x3U;
inline constexpr std::uint32_t mode_ble_2mbit = 0x4U;
inline constexpr std::uint32_t mode_ble_lr125kbit = 0x5U;
inline constexpr std::uint32_t mode_ble_lr500kbit = 0x6U;
inline constexpr std::uint32_t mode_ieee802154_250kbit = 0xFU;
}
struct pcnf0 : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x67DF010FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pcnf0_fields {
using lflen = core::field_descriptor<pcnf0, 0U, 4U>;
using s0len = core::field_descriptor<pcnf0, 8U, 1U>;
using s1len = core::field_descriptor<pcnf0, 16U, 4U>;
using s1incl = core::field_descriptor<pcnf0, 20U, 1U>;
inline constexpr std::uint32_t s1incl_automatic = 0x0U;
inline constexpr std::uint32_t s1incl_include = 0x1U;
using cilen = core::field_descriptor<pcnf0, 22U, 2U>;
using plen = core::field_descriptor<pcnf0, 24U, 2U>;
inline constexpr std::uint32_t plen_reg_8bit = 0x0U;
inline constexpr std::uint32_t plen_reg_16bit = 0x1U;
inline constexpr std::uint32_t plen_reg_32bitzero = 0x2U;
inline constexpr std::uint32_t plen_longrange = 0x3U;
using crcinc = core::field_descriptor<pcnf0, 26U, 1U>;
inline constexpr std::uint32_t crcinc_exclude = 0x0U;
inline constexpr std::uint32_t crcinc_include = 0x1U;
using termlen = core::field_descriptor<pcnf0, 29U, 2U>;
}
struct pcnf1 : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x0307FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pcnf1_fields {
using maxlen = core::field_descriptor<pcnf1, 0U, 8U>;
using statlen = core::field_descriptor<pcnf1, 8U, 8U>;
using balen = core::field_descriptor<pcnf1, 16U, 3U>;
using endian = core::field_descriptor<pcnf1, 24U, 1U>;
inline constexpr std::uint32_t endian_little = 0x0U;
inline constexpr std::uint32_t endian_big = 0x1U;
using whiteen = core::field_descriptor<pcnf1, 25U, 1U>;
inline constexpr std::uint32_t whiteen_disabled = 0x0U;
inline constexpr std::uint32_t whiteen_enabled = 0x1U;
}
struct base0 : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace base0_fields {
using base0 = core::field_descriptor<base0, 0U, 32U>;
}
struct base1 : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace base1_fields {
using base1 = core::field_descriptor<base1, 0U, 32U>;
}
struct prefix0 : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prefix0_fields {
using ap0 = core::field_descriptor<prefix0, 0U, 8U>;
using ap1 = core::field_descriptor<prefix0, 8U, 8U>;
using ap2 = core::field_descriptor<prefix0, 16U, 8U>;
using ap3 = core::field_descriptor<prefix0, 24U, 8U>;
}
struct prefix1 : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prefix1_fields {
using ap4 = core::field_descriptor<prefix1, 0U, 8U>;
using ap5 = core::field_descriptor<prefix1, 8U, 8U>;
using ap6 = core::field_descriptor<prefix1, 16U, 8U>;
using ap7 = core::field_descriptor<prefix1, 24U, 8U>;
}
struct txaddress : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace txaddress_fields {
using txaddress = core::field_descriptor<txaddress, 0U, 3U>;
}
struct rxaddresses : core::register_descriptor<std::uint32_t, 0x530U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rxaddresses_fields {
using addr0 = core::field_descriptor<rxaddresses, 0U, 1U>;
inline constexpr std::uint32_t addr0_disabled = 0x0U;
inline constexpr std::uint32_t addr0_enabled = 0x1U;
using addr1 = core::field_descriptor<rxaddresses, 1U, 1U>;
inline constexpr std::uint32_t addr1_disabled = 0x0U;
inline constexpr std::uint32_t addr1_enabled = 0x1U;
using addr2 = core::field_descriptor<rxaddresses, 2U, 1U>;
inline constexpr std::uint32_t addr2_disabled = 0x0U;
inline constexpr std::uint32_t addr2_enabled = 0x1U;
using addr3 = core::field_descriptor<rxaddresses, 3U, 1U>;
inline constexpr std::uint32_t addr3_disabled = 0x0U;
inline constexpr std::uint32_t addr3_enabled = 0x1U;
using addr4 = core::field_descriptor<rxaddresses, 4U, 1U>;
inline constexpr std::uint32_t addr4_disabled = 0x0U;
inline constexpr std::uint32_t addr4_enabled = 0x1U;
using addr5 = core::field_descriptor<rxaddresses, 5U, 1U>;
inline constexpr std::uint32_t addr5_disabled = 0x0U;
inline constexpr std::uint32_t addr5_enabled = 0x1U;
using addr6 = core::field_descriptor<rxaddresses, 6U, 1U>;
inline constexpr std::uint32_t addr6_disabled = 0x0U;
inline constexpr std::uint32_t addr6_enabled = 0x1U;
using addr7 = core::field_descriptor<rxaddresses, 7U, 1U>;
inline constexpr std::uint32_t addr7_disabled = 0x0U;
inline constexpr std::uint32_t addr7_enabled = 0x1U;
}
struct crccnf : core::register_descriptor<std::uint32_t, 0x534U,
    core::access_semantics::read_write, 0x00000303U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace crccnf_fields {
using len = core::field_descriptor<crccnf, 0U, 2U>;
inline constexpr std::uint32_t len_disabled = 0x0U;
inline constexpr std::uint32_t len_one = 0x1U;
inline constexpr std::uint32_t len_two = 0x2U;
inline constexpr std::uint32_t len_three = 0x3U;
using skipaddr = core::field_descriptor<crccnf, 8U, 2U>;
inline constexpr std::uint32_t skipaddr_include = 0x0U;
inline constexpr std::uint32_t skipaddr_skip = 0x1U;
inline constexpr std::uint32_t skipaddr_ieee802154 = 0x2U;
}
struct crcpoly : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace crcpoly_fields {
using crcpoly = core::field_descriptor<crcpoly, 0U, 24U>;
}
struct crcinit : core::register_descriptor<std::uint32_t, 0x53CU,
    core::access_semantics::read_write, 0x00FFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace crcinit_fields {
using crcinit = core::field_descriptor<crcinit, 0U, 24U>;
}
struct tifs : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0x000003FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tifs_fields {
using tifs = core::field_descriptor<tifs, 0U, 10U>;
}
struct rssisample : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rssisample_fields {
using rssisample = core::field_descriptor<rssisample, 0U, 7U>;
}
struct state : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace state_fields {
using state = core::field_descriptor<state, 0U, 4U>;
inline constexpr std::uint32_t state_disabled = 0x0U;
inline constexpr std::uint32_t state_rxru = 0x1U;
inline constexpr std::uint32_t state_rxidle = 0x2U;
inline constexpr std::uint32_t state_rx = 0x3U;
inline constexpr std::uint32_t state_rxdisable = 0x4U;
inline constexpr std::uint32_t state_txru = 0x9U;
inline constexpr std::uint32_t state_txidle = 0xAU;
inline constexpr std::uint32_t state_tx = 0xBU;
inline constexpr std::uint32_t state_txdisable = 0xCU;
}
struct datawhiteiv : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0x0000007FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000040U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace datawhiteiv_fields {
using datawhiteiv = core::field_descriptor<datawhiteiv, 0U, 7U>;
}
struct bcc : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace bcc_fields {
using bcc = core::field_descriptor<bcc, 0U, 32U>;
}
struct dab0 : core::register_descriptor<std::uint32_t, 0x600U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab0_fields {
using dab = core::field_descriptor<dab0, 0U, 32U>;
}
struct dab1 : core::register_descriptor<std::uint32_t, 0x604U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab1_fields {
using dab = core::field_descriptor<dab1, 0U, 32U>;
}
struct dab2 : core::register_descriptor<std::uint32_t, 0x608U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab2_fields {
using dab = core::field_descriptor<dab2, 0U, 32U>;
}
struct dab3 : core::register_descriptor<std::uint32_t, 0x60CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab3_fields {
using dab = core::field_descriptor<dab3, 0U, 32U>;
}
struct dab4 : core::register_descriptor<std::uint32_t, 0x610U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab4_fields {
using dab = core::field_descriptor<dab4, 0U, 32U>;
}
struct dab5 : core::register_descriptor<std::uint32_t, 0x614U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab5_fields {
using dab = core::field_descriptor<dab5, 0U, 32U>;
}
struct dab6 : core::register_descriptor<std::uint32_t, 0x618U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab6_fields {
using dab = core::field_descriptor<dab6, 0U, 32U>;
}
struct dab7 : core::register_descriptor<std::uint32_t, 0x61CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dab7_fields {
using dab = core::field_descriptor<dab7, 0U, 32U>;
}
struct dap0 : core::register_descriptor<std::uint32_t, 0x620U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap0_fields {
using dap = core::field_descriptor<dap0, 0U, 16U>;
}
struct dap1 : core::register_descriptor<std::uint32_t, 0x624U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap1_fields {
using dap = core::field_descriptor<dap1, 0U, 16U>;
}
struct dap2 : core::register_descriptor<std::uint32_t, 0x628U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap2_fields {
using dap = core::field_descriptor<dap2, 0U, 16U>;
}
struct dap3 : core::register_descriptor<std::uint32_t, 0x62CU,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap3_fields {
using dap = core::field_descriptor<dap3, 0U, 16U>;
}
struct dap4 : core::register_descriptor<std::uint32_t, 0x630U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap4_fields {
using dap = core::field_descriptor<dap4, 0U, 16U>;
}
struct dap5 : core::register_descriptor<std::uint32_t, 0x634U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap5_fields {
using dap = core::field_descriptor<dap5, 0U, 16U>;
}
struct dap6 : core::register_descriptor<std::uint32_t, 0x638U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap6_fields {
using dap = core::field_descriptor<dap6, 0U, 16U>;
}
struct dap7 : core::register_descriptor<std::uint32_t, 0x63CU,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dap7_fields {
using dap = core::field_descriptor<dap7, 0U, 16U>;
}
struct dacnf : core::register_descriptor<std::uint32_t, 0x640U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace dacnf_fields {
using ena0 = core::field_descriptor<dacnf, 0U, 1U>;
inline constexpr std::uint32_t ena0_disabled = 0x0U;
inline constexpr std::uint32_t ena0_enabled = 0x1U;
using ena1 = core::field_descriptor<dacnf, 1U, 1U>;
inline constexpr std::uint32_t ena1_disabled = 0x0U;
inline constexpr std::uint32_t ena1_enabled = 0x1U;
using ena2 = core::field_descriptor<dacnf, 2U, 1U>;
inline constexpr std::uint32_t ena2_disabled = 0x0U;
inline constexpr std::uint32_t ena2_enabled = 0x1U;
using ena3 = core::field_descriptor<dacnf, 3U, 1U>;
inline constexpr std::uint32_t ena3_disabled = 0x0U;
inline constexpr std::uint32_t ena3_enabled = 0x1U;
using ena4 = core::field_descriptor<dacnf, 4U, 1U>;
inline constexpr std::uint32_t ena4_disabled = 0x0U;
inline constexpr std::uint32_t ena4_enabled = 0x1U;
using ena5 = core::field_descriptor<dacnf, 5U, 1U>;
inline constexpr std::uint32_t ena5_disabled = 0x0U;
inline constexpr std::uint32_t ena5_enabled = 0x1U;
using ena6 = core::field_descriptor<dacnf, 6U, 1U>;
inline constexpr std::uint32_t ena6_disabled = 0x0U;
inline constexpr std::uint32_t ena6_enabled = 0x1U;
using ena7 = core::field_descriptor<dacnf, 7U, 1U>;
inline constexpr std::uint32_t ena7_disabled = 0x0U;
inline constexpr std::uint32_t ena7_enabled = 0x1U;
using txadd0 = core::field_descriptor<dacnf, 8U, 1U>;
using txadd1 = core::field_descriptor<dacnf, 9U, 1U>;
using txadd2 = core::field_descriptor<dacnf, 10U, 1U>;
using txadd3 = core::field_descriptor<dacnf, 11U, 1U>;
using txadd4 = core::field_descriptor<dacnf, 12U, 1U>;
using txadd5 = core::field_descriptor<dacnf, 13U, 1U>;
using txadd6 = core::field_descriptor<dacnf, 14U, 1U>;
using txadd7 = core::field_descriptor<dacnf, 15U, 1U>;
}
struct mhrmatchconf : core::register_descriptor<std::uint32_t, 0x644U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mhrmatchconf_fields {
using mhrmatchconf = core::field_descriptor<mhrmatchconf, 0U, 32U>;
}
struct mhrmatchmas : core::register_descriptor<std::uint32_t, 0x648U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace mhrmatchmas_fields {
using mhrmatchmas = core::field_descriptor<mhrmatchmas, 0U, 32U>;
}
struct modecnf0 : core::register_descriptor<std::uint32_t, 0x650U,
    core::access_semantics::read_write, 0x00000301U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000200U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace modecnf0_fields {
using ru = core::field_descriptor<modecnf0, 0U, 1U>;
inline constexpr std::uint32_t ru_reg_default = 0x0U;
inline constexpr std::uint32_t ru_fast = 0x1U;
using dtx = core::field_descriptor<modecnf0, 8U, 2U>;
inline constexpr std::uint32_t dtx_b1 = 0x0U;
inline constexpr std::uint32_t dtx_b0 = 0x1U;
inline constexpr std::uint32_t dtx_center = 0x2U;
}
struct sfd : core::register_descriptor<std::uint32_t, 0x660U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x000000A7U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace sfd_fields {
using sfd = core::field_descriptor<sfd, 0U, 8U>;
}
struct edcnt : core::register_descriptor<std::uint32_t, 0x664U,
    core::access_semantics::read_write, 0x001FFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace edcnt_fields {
using edcnt = core::field_descriptor<edcnt, 0U, 21U>;
}
struct edsample : core::register_descriptor<std::uint32_t, 0x668U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace edsample_fields {
using edlvl = core::field_descriptor<edsample, 0U, 8U>;
}
struct ccactrl : core::register_descriptor<std::uint32_t, 0x66CU,
    core::access_semantics::read_write, 0xFFFFFF07U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x052D0000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ccactrl_fields {
using ccamode = core::field_descriptor<ccactrl, 0U, 3U>;
inline constexpr std::uint32_t ccamode_edmode = 0x0U;
inline constexpr std::uint32_t ccamode_carriermode = 0x1U;
inline constexpr std::uint32_t ccamode_carrierandedmode = 0x2U;
inline constexpr std::uint32_t ccamode_carrieroredmode = 0x3U;
inline constexpr std::uint32_t ccamode_edmodetest1 = 0x4U;
using ccaedthres = core::field_descriptor<ccactrl, 8U, 8U>;
using ccacorrthres = core::field_descriptor<ccactrl, 16U, 8U>;
using ccacorrcnt = core::field_descriptor<ccactrl, 24U, 8U>;
}
struct power : core::register_descriptor<std::uint32_t, 0xFFCU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace power_fields {
using power = core::field_descriptor<power, 0U, 1U>;
inline constexpr std::uint32_t power_disabled = 0x0U;
inline constexpr std::uint32_t power_enabled = 0x1U;
}
}
