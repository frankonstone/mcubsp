#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::wdt {
struct tasks_start : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_start_fields {
using tasks_start = core::field_descriptor<tasks_start, 0U, 1U>;
inline constexpr std::uint32_t tasks_start_trigger = 0x1U;
}
struct events_timeout : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_timeout_fields {
using events_timeout = core::field_descriptor<events_timeout, 0U, 1U>;
inline constexpr std::uint32_t events_timeout_notgenerated = 0x0U;
inline constexpr std::uint32_t events_timeout_generated = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using timeout = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t timeout_disabled = 0x0U;
inline constexpr std::uint32_t timeout_enabled = 0x1U;
inline constexpr std::uint32_t timeout_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using timeout = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t timeout_disabled = 0x0U;
inline constexpr std::uint32_t timeout_enabled = 0x1U;
inline constexpr std::uint32_t timeout_clear = 0x1U;
}
struct runstatus : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace runstatus_fields {
using runstatus = core::field_descriptor<runstatus, 0U, 1U>;
inline constexpr std::uint32_t runstatus_notrunning = 0x0U;
inline constexpr std::uint32_t runstatus_running = 0x1U;
}
struct reqstatus : core::register_descriptor<std::uint32_t, 0x404U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace reqstatus_fields {
using rr0 = core::field_descriptor<reqstatus, 0U, 1U>;
inline constexpr std::uint32_t rr0_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr0_enabledandunrequested = 0x1U;
using rr1 = core::field_descriptor<reqstatus, 1U, 1U>;
inline constexpr std::uint32_t rr1_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr1_enabledandunrequested = 0x1U;
using rr2 = core::field_descriptor<reqstatus, 2U, 1U>;
inline constexpr std::uint32_t rr2_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr2_enabledandunrequested = 0x1U;
using rr3 = core::field_descriptor<reqstatus, 3U, 1U>;
inline constexpr std::uint32_t rr3_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr3_enabledandunrequested = 0x1U;
using rr4 = core::field_descriptor<reqstatus, 4U, 1U>;
inline constexpr std::uint32_t rr4_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr4_enabledandunrequested = 0x1U;
using rr5 = core::field_descriptor<reqstatus, 5U, 1U>;
inline constexpr std::uint32_t rr5_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr5_enabledandunrequested = 0x1U;
using rr6 = core::field_descriptor<reqstatus, 6U, 1U>;
inline constexpr std::uint32_t rr6_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr6_enabledandunrequested = 0x1U;
using rr7 = core::field_descriptor<reqstatus, 7U, 1U>;
inline constexpr std::uint32_t rr7_disabledorrequested = 0x0U;
inline constexpr std::uint32_t rr7_enabledandunrequested = 0x1U;
}
struct crv : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace crv_fields {
using crv = core::field_descriptor<crv, 0U, 32U>;
}
struct rren : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rren_fields {
using rr0 = core::field_descriptor<rren, 0U, 1U>;
inline constexpr std::uint32_t rr0_disabled = 0x0U;
inline constexpr std::uint32_t rr0_enabled = 0x1U;
using rr1 = core::field_descriptor<rren, 1U, 1U>;
inline constexpr std::uint32_t rr1_disabled = 0x0U;
inline constexpr std::uint32_t rr1_enabled = 0x1U;
using rr2 = core::field_descriptor<rren, 2U, 1U>;
inline constexpr std::uint32_t rr2_disabled = 0x0U;
inline constexpr std::uint32_t rr2_enabled = 0x1U;
using rr3 = core::field_descriptor<rren, 3U, 1U>;
inline constexpr std::uint32_t rr3_disabled = 0x0U;
inline constexpr std::uint32_t rr3_enabled = 0x1U;
using rr4 = core::field_descriptor<rren, 4U, 1U>;
inline constexpr std::uint32_t rr4_disabled = 0x0U;
inline constexpr std::uint32_t rr4_enabled = 0x1U;
using rr5 = core::field_descriptor<rren, 5U, 1U>;
inline constexpr std::uint32_t rr5_disabled = 0x0U;
inline constexpr std::uint32_t rr5_enabled = 0x1U;
using rr6 = core::field_descriptor<rren, 6U, 1U>;
inline constexpr std::uint32_t rr6_disabled = 0x0U;
inline constexpr std::uint32_t rr6_enabled = 0x1U;
using rr7 = core::field_descriptor<rren, 7U, 1U>;
inline constexpr std::uint32_t rr7_disabled = 0x0U;
inline constexpr std::uint32_t rr7_enabled = 0x1U;
}
struct config : core::register_descriptor<std::uint32_t, 0x50CU,
    core::access_semantics::read_write, 0x00000009U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000001U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace config_fields {
using sleep = core::field_descriptor<config, 0U, 1U>;
inline constexpr std::uint32_t sleep_pause = 0x0U;
inline constexpr std::uint32_t sleep_run = 0x1U;
using halt = core::field_descriptor<config, 3U, 1U>;
inline constexpr std::uint32_t halt_pause = 0x0U;
inline constexpr std::uint32_t halt_run = 0x1U;
}
struct rr0 : core::register_descriptor<std::uint32_t, 0x600U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr0_fields {
using rr = core::field_descriptor<rr0, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr1 : core::register_descriptor<std::uint32_t, 0x604U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr1_fields {
using rr = core::field_descriptor<rr1, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr2 : core::register_descriptor<std::uint32_t, 0x608U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr2_fields {
using rr = core::field_descriptor<rr2, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr3 : core::register_descriptor<std::uint32_t, 0x60CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr3_fields {
using rr = core::field_descriptor<rr3, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr4 : core::register_descriptor<std::uint32_t, 0x610U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr4_fields {
using rr = core::field_descriptor<rr4, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr5 : core::register_descriptor<std::uint32_t, 0x614U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr5_fields {
using rr = core::field_descriptor<rr5, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr6 : core::register_descriptor<std::uint32_t, 0x618U,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr6_fields {
using rr = core::field_descriptor<rr6, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
struct rr7 : core::register_descriptor<std::uint32_t, 0x61CU,
    core::access_semantics::write_only, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace rr7_fields {
using rr = core::field_descriptor<rr7, 0U, 32U>;
inline constexpr std::uint32_t rr_reload = 0x6E524635U;
}
}
