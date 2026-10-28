#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::mwu {
struct events_region0_wa : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region0_wa_fields {
using wa = core::field_descriptor<events_region0_wa, 0U, 1U>;
inline constexpr std::uint32_t wa_notgenerated = 0x0U;
inline constexpr std::uint32_t wa_generated = 0x1U;
}
struct events_region0_ra : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region0_ra_fields {
using ra = core::field_descriptor<events_region0_ra, 0U, 1U>;
inline constexpr std::uint32_t ra_notgenerated = 0x0U;
inline constexpr std::uint32_t ra_generated = 0x1U;
}
struct events_region1_wa : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region1_wa_fields {
using wa = core::field_descriptor<events_region1_wa, 0U, 1U>;
inline constexpr std::uint32_t wa_notgenerated = 0x0U;
inline constexpr std::uint32_t wa_generated = 0x1U;
}
struct events_region1_ra : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region1_ra_fields {
using ra = core::field_descriptor<events_region1_ra, 0U, 1U>;
inline constexpr std::uint32_t ra_notgenerated = 0x0U;
inline constexpr std::uint32_t ra_generated = 0x1U;
}
struct events_region2_wa : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region2_wa_fields {
using wa = core::field_descriptor<events_region2_wa, 0U, 1U>;
inline constexpr std::uint32_t wa_notgenerated = 0x0U;
inline constexpr std::uint32_t wa_generated = 0x1U;
}
struct events_region2_ra : core::register_descriptor<std::uint32_t, 0x114U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region2_ra_fields {
using ra = core::field_descriptor<events_region2_ra, 0U, 1U>;
inline constexpr std::uint32_t ra_notgenerated = 0x0U;
inline constexpr std::uint32_t ra_generated = 0x1U;
}
struct events_region3_wa : core::register_descriptor<std::uint32_t, 0x118U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region3_wa_fields {
using wa = core::field_descriptor<events_region3_wa, 0U, 1U>;
inline constexpr std::uint32_t wa_notgenerated = 0x0U;
inline constexpr std::uint32_t wa_generated = 0x1U;
}
struct events_region3_ra : core::register_descriptor<std::uint32_t, 0x11CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_region3_ra_fields {
using ra = core::field_descriptor<events_region3_ra, 0U, 1U>;
inline constexpr std::uint32_t ra_notgenerated = 0x0U;
inline constexpr std::uint32_t ra_generated = 0x1U;
}
struct events_pregion0_wa : core::register_descriptor<std::uint32_t, 0x160U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_pregion0_wa_fields {
using wa = core::field_descriptor<events_pregion0_wa, 0U, 1U>;
inline constexpr std::uint32_t wa_notgenerated = 0x0U;
inline constexpr std::uint32_t wa_generated = 0x1U;
}
struct events_pregion0_ra : core::register_descriptor<std::uint32_t, 0x164U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_pregion0_ra_fields {
using ra = core::field_descriptor<events_pregion0_ra, 0U, 1U>;
inline constexpr std::uint32_t ra_notgenerated = 0x0U;
inline constexpr std::uint32_t ra_generated = 0x1U;
}
struct events_pregion1_wa : core::register_descriptor<std::uint32_t, 0x168U,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_pregion1_wa_fields {
using wa = core::field_descriptor<events_pregion1_wa, 0U, 1U>;
inline constexpr std::uint32_t wa_notgenerated = 0x0U;
inline constexpr std::uint32_t wa_generated = 0x1U;
}
struct events_pregion1_ra : core::register_descriptor<std::uint32_t, 0x16CU,
    core::access_semantics::write_zero_to_clear, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace events_pregion1_ra_fields {
using ra = core::field_descriptor<events_pregion1_ra, 0U, 1U>;
inline constexpr std::uint32_t ra_notgenerated = 0x0U;
inline constexpr std::uint32_t ra_generated = 0x1U;
}
struct inten : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace inten_fields {
using region0wa = core::field_descriptor<inten, 0U, 1U>;
inline constexpr std::uint32_t region0wa_disabled = 0x0U;
inline constexpr std::uint32_t region0wa_enabled = 0x1U;
using region0ra = core::field_descriptor<inten, 1U, 1U>;
inline constexpr std::uint32_t region0ra_disabled = 0x0U;
inline constexpr std::uint32_t region0ra_enabled = 0x1U;
using region1wa = core::field_descriptor<inten, 2U, 1U>;
inline constexpr std::uint32_t region1wa_disabled = 0x0U;
inline constexpr std::uint32_t region1wa_enabled = 0x1U;
using region1ra = core::field_descriptor<inten, 3U, 1U>;
inline constexpr std::uint32_t region1ra_disabled = 0x0U;
inline constexpr std::uint32_t region1ra_enabled = 0x1U;
using region2wa = core::field_descriptor<inten, 4U, 1U>;
inline constexpr std::uint32_t region2wa_disabled = 0x0U;
inline constexpr std::uint32_t region2wa_enabled = 0x1U;
using region2ra = core::field_descriptor<inten, 5U, 1U>;
inline constexpr std::uint32_t region2ra_disabled = 0x0U;
inline constexpr std::uint32_t region2ra_enabled = 0x1U;
using region3wa = core::field_descriptor<inten, 6U, 1U>;
inline constexpr std::uint32_t region3wa_disabled = 0x0U;
inline constexpr std::uint32_t region3wa_enabled = 0x1U;
using region3ra = core::field_descriptor<inten, 7U, 1U>;
inline constexpr std::uint32_t region3ra_disabled = 0x0U;
inline constexpr std::uint32_t region3ra_enabled = 0x1U;
using pregion0wa = core::field_descriptor<inten, 24U, 1U>;
inline constexpr std::uint32_t pregion0wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion0wa_enabled = 0x1U;
using pregion0ra = core::field_descriptor<inten, 25U, 1U>;
inline constexpr std::uint32_t pregion0ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion0ra_enabled = 0x1U;
using pregion1wa = core::field_descriptor<inten, 26U, 1U>;
inline constexpr std::uint32_t pregion1wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion1wa_enabled = 0x1U;
using pregion1ra = core::field_descriptor<inten, 27U, 1U>;
inline constexpr std::uint32_t pregion1ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion1ra_enabled = 0x1U;
}
struct intenset : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::set_alias, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenset_fields {
using region0wa = core::field_descriptor<intenset, 0U, 1U>;
inline constexpr std::uint32_t region0wa_disabled = 0x0U;
inline constexpr std::uint32_t region0wa_enabled = 0x1U;
inline constexpr std::uint32_t region0wa_set = 0x1U;
using region0ra = core::field_descriptor<intenset, 1U, 1U>;
inline constexpr std::uint32_t region0ra_disabled = 0x0U;
inline constexpr std::uint32_t region0ra_enabled = 0x1U;
inline constexpr std::uint32_t region0ra_set = 0x1U;
using region1wa = core::field_descriptor<intenset, 2U, 1U>;
inline constexpr std::uint32_t region1wa_disabled = 0x0U;
inline constexpr std::uint32_t region1wa_enabled = 0x1U;
inline constexpr std::uint32_t region1wa_set = 0x1U;
using region1ra = core::field_descriptor<intenset, 3U, 1U>;
inline constexpr std::uint32_t region1ra_disabled = 0x0U;
inline constexpr std::uint32_t region1ra_enabled = 0x1U;
inline constexpr std::uint32_t region1ra_set = 0x1U;
using region2wa = core::field_descriptor<intenset, 4U, 1U>;
inline constexpr std::uint32_t region2wa_disabled = 0x0U;
inline constexpr std::uint32_t region2wa_enabled = 0x1U;
inline constexpr std::uint32_t region2wa_set = 0x1U;
using region2ra = core::field_descriptor<intenset, 5U, 1U>;
inline constexpr std::uint32_t region2ra_disabled = 0x0U;
inline constexpr std::uint32_t region2ra_enabled = 0x1U;
inline constexpr std::uint32_t region2ra_set = 0x1U;
using region3wa = core::field_descriptor<intenset, 6U, 1U>;
inline constexpr std::uint32_t region3wa_disabled = 0x0U;
inline constexpr std::uint32_t region3wa_enabled = 0x1U;
inline constexpr std::uint32_t region3wa_set = 0x1U;
using region3ra = core::field_descriptor<intenset, 7U, 1U>;
inline constexpr std::uint32_t region3ra_disabled = 0x0U;
inline constexpr std::uint32_t region3ra_enabled = 0x1U;
inline constexpr std::uint32_t region3ra_set = 0x1U;
using pregion0wa = core::field_descriptor<intenset, 24U, 1U>;
inline constexpr std::uint32_t pregion0wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion0wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion0wa_set = 0x1U;
using pregion0ra = core::field_descriptor<intenset, 25U, 1U>;
inline constexpr std::uint32_t pregion0ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion0ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion0ra_set = 0x1U;
using pregion1wa = core::field_descriptor<intenset, 26U, 1U>;
inline constexpr std::uint32_t pregion1wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion1wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion1wa_set = 0x1U;
using pregion1ra = core::field_descriptor<intenset, 27U, 1U>;
inline constexpr std::uint32_t pregion1ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion1ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion1ra_set = 0x1U;
}
struct intenclr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::clear_alias, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace intenclr_fields {
using region0wa = core::field_descriptor<intenclr, 0U, 1U>;
inline constexpr std::uint32_t region0wa_disabled = 0x0U;
inline constexpr std::uint32_t region0wa_enabled = 0x1U;
inline constexpr std::uint32_t region0wa_clear = 0x1U;
using region0ra = core::field_descriptor<intenclr, 1U, 1U>;
inline constexpr std::uint32_t region0ra_disabled = 0x0U;
inline constexpr std::uint32_t region0ra_enabled = 0x1U;
inline constexpr std::uint32_t region0ra_clear = 0x1U;
using region1wa = core::field_descriptor<intenclr, 2U, 1U>;
inline constexpr std::uint32_t region1wa_disabled = 0x0U;
inline constexpr std::uint32_t region1wa_enabled = 0x1U;
inline constexpr std::uint32_t region1wa_clear = 0x1U;
using region1ra = core::field_descriptor<intenclr, 3U, 1U>;
inline constexpr std::uint32_t region1ra_disabled = 0x0U;
inline constexpr std::uint32_t region1ra_enabled = 0x1U;
inline constexpr std::uint32_t region1ra_clear = 0x1U;
using region2wa = core::field_descriptor<intenclr, 4U, 1U>;
inline constexpr std::uint32_t region2wa_disabled = 0x0U;
inline constexpr std::uint32_t region2wa_enabled = 0x1U;
inline constexpr std::uint32_t region2wa_clear = 0x1U;
using region2ra = core::field_descriptor<intenclr, 5U, 1U>;
inline constexpr std::uint32_t region2ra_disabled = 0x0U;
inline constexpr std::uint32_t region2ra_enabled = 0x1U;
inline constexpr std::uint32_t region2ra_clear = 0x1U;
using region3wa = core::field_descriptor<intenclr, 6U, 1U>;
inline constexpr std::uint32_t region3wa_disabled = 0x0U;
inline constexpr std::uint32_t region3wa_enabled = 0x1U;
inline constexpr std::uint32_t region3wa_clear = 0x1U;
using region3ra = core::field_descriptor<intenclr, 7U, 1U>;
inline constexpr std::uint32_t region3ra_disabled = 0x0U;
inline constexpr std::uint32_t region3ra_enabled = 0x1U;
inline constexpr std::uint32_t region3ra_clear = 0x1U;
using pregion0wa = core::field_descriptor<intenclr, 24U, 1U>;
inline constexpr std::uint32_t pregion0wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion0wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion0wa_clear = 0x1U;
using pregion0ra = core::field_descriptor<intenclr, 25U, 1U>;
inline constexpr std::uint32_t pregion0ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion0ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion0ra_clear = 0x1U;
using pregion1wa = core::field_descriptor<intenclr, 26U, 1U>;
inline constexpr std::uint32_t pregion1wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion1wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion1wa_clear = 0x1U;
using pregion1ra = core::field_descriptor<intenclr, 27U, 1U>;
inline constexpr std::uint32_t pregion1ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion1ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion1ra_clear = 0x1U;
}
struct nmien : core::register_descriptor<std::uint32_t, 0x320U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nmien_fields {
using region0wa = core::field_descriptor<nmien, 0U, 1U>;
inline constexpr std::uint32_t region0wa_disabled = 0x0U;
inline constexpr std::uint32_t region0wa_enabled = 0x1U;
using region0ra = core::field_descriptor<nmien, 1U, 1U>;
inline constexpr std::uint32_t region0ra_disabled = 0x0U;
inline constexpr std::uint32_t region0ra_enabled = 0x1U;
using region1wa = core::field_descriptor<nmien, 2U, 1U>;
inline constexpr std::uint32_t region1wa_disabled = 0x0U;
inline constexpr std::uint32_t region1wa_enabled = 0x1U;
using region1ra = core::field_descriptor<nmien, 3U, 1U>;
inline constexpr std::uint32_t region1ra_disabled = 0x0U;
inline constexpr std::uint32_t region1ra_enabled = 0x1U;
using region2wa = core::field_descriptor<nmien, 4U, 1U>;
inline constexpr std::uint32_t region2wa_disabled = 0x0U;
inline constexpr std::uint32_t region2wa_enabled = 0x1U;
using region2ra = core::field_descriptor<nmien, 5U, 1U>;
inline constexpr std::uint32_t region2ra_disabled = 0x0U;
inline constexpr std::uint32_t region2ra_enabled = 0x1U;
using region3wa = core::field_descriptor<nmien, 6U, 1U>;
inline constexpr std::uint32_t region3wa_disabled = 0x0U;
inline constexpr std::uint32_t region3wa_enabled = 0x1U;
using region3ra = core::field_descriptor<nmien, 7U, 1U>;
inline constexpr std::uint32_t region3ra_disabled = 0x0U;
inline constexpr std::uint32_t region3ra_enabled = 0x1U;
using pregion0wa = core::field_descriptor<nmien, 24U, 1U>;
inline constexpr std::uint32_t pregion0wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion0wa_enabled = 0x1U;
using pregion0ra = core::field_descriptor<nmien, 25U, 1U>;
inline constexpr std::uint32_t pregion0ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion0ra_enabled = 0x1U;
using pregion1wa = core::field_descriptor<nmien, 26U, 1U>;
inline constexpr std::uint32_t pregion1wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion1wa_enabled = 0x1U;
using pregion1ra = core::field_descriptor<nmien, 27U, 1U>;
inline constexpr std::uint32_t pregion1ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion1ra_enabled = 0x1U;
}
struct nmienset : core::register_descriptor<std::uint32_t, 0x324U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nmienset_fields {
using region0wa = core::field_descriptor<nmienset, 0U, 1U>;
inline constexpr std::uint32_t region0wa_disabled = 0x0U;
inline constexpr std::uint32_t region0wa_enabled = 0x1U;
inline constexpr std::uint32_t region0wa_set = 0x1U;
using region0ra = core::field_descriptor<nmienset, 1U, 1U>;
inline constexpr std::uint32_t region0ra_disabled = 0x0U;
inline constexpr std::uint32_t region0ra_enabled = 0x1U;
inline constexpr std::uint32_t region0ra_set = 0x1U;
using region1wa = core::field_descriptor<nmienset, 2U, 1U>;
inline constexpr std::uint32_t region1wa_disabled = 0x0U;
inline constexpr std::uint32_t region1wa_enabled = 0x1U;
inline constexpr std::uint32_t region1wa_set = 0x1U;
using region1ra = core::field_descriptor<nmienset, 3U, 1U>;
inline constexpr std::uint32_t region1ra_disabled = 0x0U;
inline constexpr std::uint32_t region1ra_enabled = 0x1U;
inline constexpr std::uint32_t region1ra_set = 0x1U;
using region2wa = core::field_descriptor<nmienset, 4U, 1U>;
inline constexpr std::uint32_t region2wa_disabled = 0x0U;
inline constexpr std::uint32_t region2wa_enabled = 0x1U;
inline constexpr std::uint32_t region2wa_set = 0x1U;
using region2ra = core::field_descriptor<nmienset, 5U, 1U>;
inline constexpr std::uint32_t region2ra_disabled = 0x0U;
inline constexpr std::uint32_t region2ra_enabled = 0x1U;
inline constexpr std::uint32_t region2ra_set = 0x1U;
using region3wa = core::field_descriptor<nmienset, 6U, 1U>;
inline constexpr std::uint32_t region3wa_disabled = 0x0U;
inline constexpr std::uint32_t region3wa_enabled = 0x1U;
inline constexpr std::uint32_t region3wa_set = 0x1U;
using region3ra = core::field_descriptor<nmienset, 7U, 1U>;
inline constexpr std::uint32_t region3ra_disabled = 0x0U;
inline constexpr std::uint32_t region3ra_enabled = 0x1U;
inline constexpr std::uint32_t region3ra_set = 0x1U;
using pregion0wa = core::field_descriptor<nmienset, 24U, 1U>;
inline constexpr std::uint32_t pregion0wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion0wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion0wa_set = 0x1U;
using pregion0ra = core::field_descriptor<nmienset, 25U, 1U>;
inline constexpr std::uint32_t pregion0ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion0ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion0ra_set = 0x1U;
using pregion1wa = core::field_descriptor<nmienset, 26U, 1U>;
inline constexpr std::uint32_t pregion1wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion1wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion1wa_set = 0x1U;
using pregion1ra = core::field_descriptor<nmienset, 27U, 1U>;
inline constexpr std::uint32_t pregion1ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion1ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion1ra_set = 0x1U;
}
struct nmienclr : core::register_descriptor<std::uint32_t, 0x328U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nmienclr_fields {
using region0wa = core::field_descriptor<nmienclr, 0U, 1U>;
inline constexpr std::uint32_t region0wa_disabled = 0x0U;
inline constexpr std::uint32_t region0wa_enabled = 0x1U;
inline constexpr std::uint32_t region0wa_clear = 0x1U;
using region0ra = core::field_descriptor<nmienclr, 1U, 1U>;
inline constexpr std::uint32_t region0ra_disabled = 0x0U;
inline constexpr std::uint32_t region0ra_enabled = 0x1U;
inline constexpr std::uint32_t region0ra_clear = 0x1U;
using region1wa = core::field_descriptor<nmienclr, 2U, 1U>;
inline constexpr std::uint32_t region1wa_disabled = 0x0U;
inline constexpr std::uint32_t region1wa_enabled = 0x1U;
inline constexpr std::uint32_t region1wa_clear = 0x1U;
using region1ra = core::field_descriptor<nmienclr, 3U, 1U>;
inline constexpr std::uint32_t region1ra_disabled = 0x0U;
inline constexpr std::uint32_t region1ra_enabled = 0x1U;
inline constexpr std::uint32_t region1ra_clear = 0x1U;
using region2wa = core::field_descriptor<nmienclr, 4U, 1U>;
inline constexpr std::uint32_t region2wa_disabled = 0x0U;
inline constexpr std::uint32_t region2wa_enabled = 0x1U;
inline constexpr std::uint32_t region2wa_clear = 0x1U;
using region2ra = core::field_descriptor<nmienclr, 5U, 1U>;
inline constexpr std::uint32_t region2ra_disabled = 0x0U;
inline constexpr std::uint32_t region2ra_enabled = 0x1U;
inline constexpr std::uint32_t region2ra_clear = 0x1U;
using region3wa = core::field_descriptor<nmienclr, 6U, 1U>;
inline constexpr std::uint32_t region3wa_disabled = 0x0U;
inline constexpr std::uint32_t region3wa_enabled = 0x1U;
inline constexpr std::uint32_t region3wa_clear = 0x1U;
using region3ra = core::field_descriptor<nmienclr, 7U, 1U>;
inline constexpr std::uint32_t region3ra_disabled = 0x0U;
inline constexpr std::uint32_t region3ra_enabled = 0x1U;
inline constexpr std::uint32_t region3ra_clear = 0x1U;
using pregion0wa = core::field_descriptor<nmienclr, 24U, 1U>;
inline constexpr std::uint32_t pregion0wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion0wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion0wa_clear = 0x1U;
using pregion0ra = core::field_descriptor<nmienclr, 25U, 1U>;
inline constexpr std::uint32_t pregion0ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion0ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion0ra_clear = 0x1U;
using pregion1wa = core::field_descriptor<nmienclr, 26U, 1U>;
inline constexpr std::uint32_t pregion1wa_disabled = 0x0U;
inline constexpr std::uint32_t pregion1wa_enabled = 0x1U;
inline constexpr std::uint32_t pregion1wa_clear = 0x1U;
using pregion1ra = core::field_descriptor<nmienclr, 27U, 1U>;
inline constexpr std::uint32_t pregion1ra_disabled = 0x0U;
inline constexpr std::uint32_t pregion1ra_enabled = 0x1U;
inline constexpr std::uint32_t pregion1ra_clear = 0x1U;
}
struct perregion0_substatwa : core::register_descriptor<std::uint32_t, 0x400U,
    core::access_semantics::write_one_to_clear, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace perregion0_substatwa_fields {
using sr0 = core::field_descriptor<perregion0_substatwa, 0U, 1U>;
inline constexpr std::uint32_t sr0_noaccess = 0x0U;
inline constexpr std::uint32_t sr0_access = 0x1U;
using sr1 = core::field_descriptor<perregion0_substatwa, 1U, 1U>;
inline constexpr std::uint32_t sr1_noaccess = 0x0U;
inline constexpr std::uint32_t sr1_access = 0x1U;
using sr2 = core::field_descriptor<perregion0_substatwa, 2U, 1U>;
inline constexpr std::uint32_t sr2_noaccess = 0x0U;
inline constexpr std::uint32_t sr2_access = 0x1U;
using sr3 = core::field_descriptor<perregion0_substatwa, 3U, 1U>;
inline constexpr std::uint32_t sr3_noaccess = 0x0U;
inline constexpr std::uint32_t sr3_access = 0x1U;
using sr4 = core::field_descriptor<perregion0_substatwa, 4U, 1U>;
inline constexpr std::uint32_t sr4_noaccess = 0x0U;
inline constexpr std::uint32_t sr4_access = 0x1U;
using sr5 = core::field_descriptor<perregion0_substatwa, 5U, 1U>;
inline constexpr std::uint32_t sr5_noaccess = 0x0U;
inline constexpr std::uint32_t sr5_access = 0x1U;
using sr6 = core::field_descriptor<perregion0_substatwa, 6U, 1U>;
inline constexpr std::uint32_t sr6_noaccess = 0x0U;
inline constexpr std::uint32_t sr6_access = 0x1U;
using sr7 = core::field_descriptor<perregion0_substatwa, 7U, 1U>;
inline constexpr std::uint32_t sr7_noaccess = 0x0U;
inline constexpr std::uint32_t sr7_access = 0x1U;
using sr8 = core::field_descriptor<perregion0_substatwa, 8U, 1U>;
inline constexpr std::uint32_t sr8_noaccess = 0x0U;
inline constexpr std::uint32_t sr8_access = 0x1U;
using sr9 = core::field_descriptor<perregion0_substatwa, 9U, 1U>;
inline constexpr std::uint32_t sr9_noaccess = 0x0U;
inline constexpr std::uint32_t sr9_access = 0x1U;
using sr10 = core::field_descriptor<perregion0_substatwa, 10U, 1U>;
inline constexpr std::uint32_t sr10_noaccess = 0x0U;
inline constexpr std::uint32_t sr10_access = 0x1U;
using sr11 = core::field_descriptor<perregion0_substatwa, 11U, 1U>;
inline constexpr std::uint32_t sr11_noaccess = 0x0U;
inline constexpr std::uint32_t sr11_access = 0x1U;
using sr12 = core::field_descriptor<perregion0_substatwa, 12U, 1U>;
inline constexpr std::uint32_t sr12_noaccess = 0x0U;
inline constexpr std::uint32_t sr12_access = 0x1U;
using sr13 = core::field_descriptor<perregion0_substatwa, 13U, 1U>;
inline constexpr std::uint32_t sr13_noaccess = 0x0U;
inline constexpr std::uint32_t sr13_access = 0x1U;
using sr14 = core::field_descriptor<perregion0_substatwa, 14U, 1U>;
inline constexpr std::uint32_t sr14_noaccess = 0x0U;
inline constexpr std::uint32_t sr14_access = 0x1U;
using sr15 = core::field_descriptor<perregion0_substatwa, 15U, 1U>;
inline constexpr std::uint32_t sr15_noaccess = 0x0U;
inline constexpr std::uint32_t sr15_access = 0x1U;
using sr16 = core::field_descriptor<perregion0_substatwa, 16U, 1U>;
inline constexpr std::uint32_t sr16_noaccess = 0x0U;
inline constexpr std::uint32_t sr16_access = 0x1U;
using sr17 = core::field_descriptor<perregion0_substatwa, 17U, 1U>;
inline constexpr std::uint32_t sr17_noaccess = 0x0U;
inline constexpr std::uint32_t sr17_access = 0x1U;
using sr18 = core::field_descriptor<perregion0_substatwa, 18U, 1U>;
inline constexpr std::uint32_t sr18_noaccess = 0x0U;
inline constexpr std::uint32_t sr18_access = 0x1U;
using sr19 = core::field_descriptor<perregion0_substatwa, 19U, 1U>;
inline constexpr std::uint32_t sr19_noaccess = 0x0U;
inline constexpr std::uint32_t sr19_access = 0x1U;
using sr20 = core::field_descriptor<perregion0_substatwa, 20U, 1U>;
inline constexpr std::uint32_t sr20_noaccess = 0x0U;
inline constexpr std::uint32_t sr20_access = 0x1U;
using sr21 = core::field_descriptor<perregion0_substatwa, 21U, 1U>;
inline constexpr std::uint32_t sr21_noaccess = 0x0U;
inline constexpr std::uint32_t sr21_access = 0x1U;
using sr22 = core::field_descriptor<perregion0_substatwa, 22U, 1U>;
inline constexpr std::uint32_t sr22_noaccess = 0x0U;
inline constexpr std::uint32_t sr22_access = 0x1U;
using sr23 = core::field_descriptor<perregion0_substatwa, 23U, 1U>;
inline constexpr std::uint32_t sr23_noaccess = 0x0U;
inline constexpr std::uint32_t sr23_access = 0x1U;
using sr24 = core::field_descriptor<perregion0_substatwa, 24U, 1U>;
inline constexpr std::uint32_t sr24_noaccess = 0x0U;
inline constexpr std::uint32_t sr24_access = 0x1U;
using sr25 = core::field_descriptor<perregion0_substatwa, 25U, 1U>;
inline constexpr std::uint32_t sr25_noaccess = 0x0U;
inline constexpr std::uint32_t sr25_access = 0x1U;
using sr26 = core::field_descriptor<perregion0_substatwa, 26U, 1U>;
inline constexpr std::uint32_t sr26_noaccess = 0x0U;
inline constexpr std::uint32_t sr26_access = 0x1U;
using sr27 = core::field_descriptor<perregion0_substatwa, 27U, 1U>;
inline constexpr std::uint32_t sr27_noaccess = 0x0U;
inline constexpr std::uint32_t sr27_access = 0x1U;
using sr28 = core::field_descriptor<perregion0_substatwa, 28U, 1U>;
inline constexpr std::uint32_t sr28_noaccess = 0x0U;
inline constexpr std::uint32_t sr28_access = 0x1U;
using sr29 = core::field_descriptor<perregion0_substatwa, 29U, 1U>;
inline constexpr std::uint32_t sr29_noaccess = 0x0U;
inline constexpr std::uint32_t sr29_access = 0x1U;
using sr30 = core::field_descriptor<perregion0_substatwa, 30U, 1U>;
inline constexpr std::uint32_t sr30_noaccess = 0x0U;
inline constexpr std::uint32_t sr30_access = 0x1U;
using sr31 = core::field_descriptor<perregion0_substatwa, 31U, 1U>;
inline constexpr std::uint32_t sr31_noaccess = 0x0U;
inline constexpr std::uint32_t sr31_access = 0x1U;
}
struct perregion0_substatra : core::register_descriptor<std::uint32_t, 0x404U,
    core::access_semantics::write_one_to_clear, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace perregion0_substatra_fields {
using sr0 = core::field_descriptor<perregion0_substatra, 0U, 1U>;
inline constexpr std::uint32_t sr0_noaccess = 0x0U;
inline constexpr std::uint32_t sr0_access = 0x1U;
using sr1 = core::field_descriptor<perregion0_substatra, 1U, 1U>;
inline constexpr std::uint32_t sr1_noaccess = 0x0U;
inline constexpr std::uint32_t sr1_access = 0x1U;
using sr2 = core::field_descriptor<perregion0_substatra, 2U, 1U>;
inline constexpr std::uint32_t sr2_noaccess = 0x0U;
inline constexpr std::uint32_t sr2_access = 0x1U;
using sr3 = core::field_descriptor<perregion0_substatra, 3U, 1U>;
inline constexpr std::uint32_t sr3_noaccess = 0x0U;
inline constexpr std::uint32_t sr3_access = 0x1U;
using sr4 = core::field_descriptor<perregion0_substatra, 4U, 1U>;
inline constexpr std::uint32_t sr4_noaccess = 0x0U;
inline constexpr std::uint32_t sr4_access = 0x1U;
using sr5 = core::field_descriptor<perregion0_substatra, 5U, 1U>;
inline constexpr std::uint32_t sr5_noaccess = 0x0U;
inline constexpr std::uint32_t sr5_access = 0x1U;
using sr6 = core::field_descriptor<perregion0_substatra, 6U, 1U>;
inline constexpr std::uint32_t sr6_noaccess = 0x0U;
inline constexpr std::uint32_t sr6_access = 0x1U;
using sr7 = core::field_descriptor<perregion0_substatra, 7U, 1U>;
inline constexpr std::uint32_t sr7_noaccess = 0x0U;
inline constexpr std::uint32_t sr7_access = 0x1U;
using sr8 = core::field_descriptor<perregion0_substatra, 8U, 1U>;
inline constexpr std::uint32_t sr8_noaccess = 0x0U;
inline constexpr std::uint32_t sr8_access = 0x1U;
using sr9 = core::field_descriptor<perregion0_substatra, 9U, 1U>;
inline constexpr std::uint32_t sr9_noaccess = 0x0U;
inline constexpr std::uint32_t sr9_access = 0x1U;
using sr10 = core::field_descriptor<perregion0_substatra, 10U, 1U>;
inline constexpr std::uint32_t sr10_noaccess = 0x0U;
inline constexpr std::uint32_t sr10_access = 0x1U;
using sr11 = core::field_descriptor<perregion0_substatra, 11U, 1U>;
inline constexpr std::uint32_t sr11_noaccess = 0x0U;
inline constexpr std::uint32_t sr11_access = 0x1U;
using sr12 = core::field_descriptor<perregion0_substatra, 12U, 1U>;
inline constexpr std::uint32_t sr12_noaccess = 0x0U;
inline constexpr std::uint32_t sr12_access = 0x1U;
using sr13 = core::field_descriptor<perregion0_substatra, 13U, 1U>;
inline constexpr std::uint32_t sr13_noaccess = 0x0U;
inline constexpr std::uint32_t sr13_access = 0x1U;
using sr14 = core::field_descriptor<perregion0_substatra, 14U, 1U>;
inline constexpr std::uint32_t sr14_noaccess = 0x0U;
inline constexpr std::uint32_t sr14_access = 0x1U;
using sr15 = core::field_descriptor<perregion0_substatra, 15U, 1U>;
inline constexpr std::uint32_t sr15_noaccess = 0x0U;
inline constexpr std::uint32_t sr15_access = 0x1U;
using sr16 = core::field_descriptor<perregion0_substatra, 16U, 1U>;
inline constexpr std::uint32_t sr16_noaccess = 0x0U;
inline constexpr std::uint32_t sr16_access = 0x1U;
using sr17 = core::field_descriptor<perregion0_substatra, 17U, 1U>;
inline constexpr std::uint32_t sr17_noaccess = 0x0U;
inline constexpr std::uint32_t sr17_access = 0x1U;
using sr18 = core::field_descriptor<perregion0_substatra, 18U, 1U>;
inline constexpr std::uint32_t sr18_noaccess = 0x0U;
inline constexpr std::uint32_t sr18_access = 0x1U;
using sr19 = core::field_descriptor<perregion0_substatra, 19U, 1U>;
inline constexpr std::uint32_t sr19_noaccess = 0x0U;
inline constexpr std::uint32_t sr19_access = 0x1U;
using sr20 = core::field_descriptor<perregion0_substatra, 20U, 1U>;
inline constexpr std::uint32_t sr20_noaccess = 0x0U;
inline constexpr std::uint32_t sr20_access = 0x1U;
using sr21 = core::field_descriptor<perregion0_substatra, 21U, 1U>;
inline constexpr std::uint32_t sr21_noaccess = 0x0U;
inline constexpr std::uint32_t sr21_access = 0x1U;
using sr22 = core::field_descriptor<perregion0_substatra, 22U, 1U>;
inline constexpr std::uint32_t sr22_noaccess = 0x0U;
inline constexpr std::uint32_t sr22_access = 0x1U;
using sr23 = core::field_descriptor<perregion0_substatra, 23U, 1U>;
inline constexpr std::uint32_t sr23_noaccess = 0x0U;
inline constexpr std::uint32_t sr23_access = 0x1U;
using sr24 = core::field_descriptor<perregion0_substatra, 24U, 1U>;
inline constexpr std::uint32_t sr24_noaccess = 0x0U;
inline constexpr std::uint32_t sr24_access = 0x1U;
using sr25 = core::field_descriptor<perregion0_substatra, 25U, 1U>;
inline constexpr std::uint32_t sr25_noaccess = 0x0U;
inline constexpr std::uint32_t sr25_access = 0x1U;
using sr26 = core::field_descriptor<perregion0_substatra, 26U, 1U>;
inline constexpr std::uint32_t sr26_noaccess = 0x0U;
inline constexpr std::uint32_t sr26_access = 0x1U;
using sr27 = core::field_descriptor<perregion0_substatra, 27U, 1U>;
inline constexpr std::uint32_t sr27_noaccess = 0x0U;
inline constexpr std::uint32_t sr27_access = 0x1U;
using sr28 = core::field_descriptor<perregion0_substatra, 28U, 1U>;
inline constexpr std::uint32_t sr28_noaccess = 0x0U;
inline constexpr std::uint32_t sr28_access = 0x1U;
using sr29 = core::field_descriptor<perregion0_substatra, 29U, 1U>;
inline constexpr std::uint32_t sr29_noaccess = 0x0U;
inline constexpr std::uint32_t sr29_access = 0x1U;
using sr30 = core::field_descriptor<perregion0_substatra, 30U, 1U>;
inline constexpr std::uint32_t sr30_noaccess = 0x0U;
inline constexpr std::uint32_t sr30_access = 0x1U;
using sr31 = core::field_descriptor<perregion0_substatra, 31U, 1U>;
inline constexpr std::uint32_t sr31_noaccess = 0x0U;
inline constexpr std::uint32_t sr31_access = 0x1U;
}
struct perregion1_substatwa : core::register_descriptor<std::uint32_t, 0x408U,
    core::access_semantics::write_one_to_clear, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace perregion1_substatwa_fields {
using sr0 = core::field_descriptor<perregion1_substatwa, 0U, 1U>;
inline constexpr std::uint32_t sr0_noaccess = 0x0U;
inline constexpr std::uint32_t sr0_access = 0x1U;
using sr1 = core::field_descriptor<perregion1_substatwa, 1U, 1U>;
inline constexpr std::uint32_t sr1_noaccess = 0x0U;
inline constexpr std::uint32_t sr1_access = 0x1U;
using sr2 = core::field_descriptor<perregion1_substatwa, 2U, 1U>;
inline constexpr std::uint32_t sr2_noaccess = 0x0U;
inline constexpr std::uint32_t sr2_access = 0x1U;
using sr3 = core::field_descriptor<perregion1_substatwa, 3U, 1U>;
inline constexpr std::uint32_t sr3_noaccess = 0x0U;
inline constexpr std::uint32_t sr3_access = 0x1U;
using sr4 = core::field_descriptor<perregion1_substatwa, 4U, 1U>;
inline constexpr std::uint32_t sr4_noaccess = 0x0U;
inline constexpr std::uint32_t sr4_access = 0x1U;
using sr5 = core::field_descriptor<perregion1_substatwa, 5U, 1U>;
inline constexpr std::uint32_t sr5_noaccess = 0x0U;
inline constexpr std::uint32_t sr5_access = 0x1U;
using sr6 = core::field_descriptor<perregion1_substatwa, 6U, 1U>;
inline constexpr std::uint32_t sr6_noaccess = 0x0U;
inline constexpr std::uint32_t sr6_access = 0x1U;
using sr7 = core::field_descriptor<perregion1_substatwa, 7U, 1U>;
inline constexpr std::uint32_t sr7_noaccess = 0x0U;
inline constexpr std::uint32_t sr7_access = 0x1U;
using sr8 = core::field_descriptor<perregion1_substatwa, 8U, 1U>;
inline constexpr std::uint32_t sr8_noaccess = 0x0U;
inline constexpr std::uint32_t sr8_access = 0x1U;
using sr9 = core::field_descriptor<perregion1_substatwa, 9U, 1U>;
inline constexpr std::uint32_t sr9_noaccess = 0x0U;
inline constexpr std::uint32_t sr9_access = 0x1U;
using sr10 = core::field_descriptor<perregion1_substatwa, 10U, 1U>;
inline constexpr std::uint32_t sr10_noaccess = 0x0U;
inline constexpr std::uint32_t sr10_access = 0x1U;
using sr11 = core::field_descriptor<perregion1_substatwa, 11U, 1U>;
inline constexpr std::uint32_t sr11_noaccess = 0x0U;
inline constexpr std::uint32_t sr11_access = 0x1U;
using sr12 = core::field_descriptor<perregion1_substatwa, 12U, 1U>;
inline constexpr std::uint32_t sr12_noaccess = 0x0U;
inline constexpr std::uint32_t sr12_access = 0x1U;
using sr13 = core::field_descriptor<perregion1_substatwa, 13U, 1U>;
inline constexpr std::uint32_t sr13_noaccess = 0x0U;
inline constexpr std::uint32_t sr13_access = 0x1U;
using sr14 = core::field_descriptor<perregion1_substatwa, 14U, 1U>;
inline constexpr std::uint32_t sr14_noaccess = 0x0U;
inline constexpr std::uint32_t sr14_access = 0x1U;
using sr15 = core::field_descriptor<perregion1_substatwa, 15U, 1U>;
inline constexpr std::uint32_t sr15_noaccess = 0x0U;
inline constexpr std::uint32_t sr15_access = 0x1U;
using sr16 = core::field_descriptor<perregion1_substatwa, 16U, 1U>;
inline constexpr std::uint32_t sr16_noaccess = 0x0U;
inline constexpr std::uint32_t sr16_access = 0x1U;
using sr17 = core::field_descriptor<perregion1_substatwa, 17U, 1U>;
inline constexpr std::uint32_t sr17_noaccess = 0x0U;
inline constexpr std::uint32_t sr17_access = 0x1U;
using sr18 = core::field_descriptor<perregion1_substatwa, 18U, 1U>;
inline constexpr std::uint32_t sr18_noaccess = 0x0U;
inline constexpr std::uint32_t sr18_access = 0x1U;
using sr19 = core::field_descriptor<perregion1_substatwa, 19U, 1U>;
inline constexpr std::uint32_t sr19_noaccess = 0x0U;
inline constexpr std::uint32_t sr19_access = 0x1U;
using sr20 = core::field_descriptor<perregion1_substatwa, 20U, 1U>;
inline constexpr std::uint32_t sr20_noaccess = 0x0U;
inline constexpr std::uint32_t sr20_access = 0x1U;
using sr21 = core::field_descriptor<perregion1_substatwa, 21U, 1U>;
inline constexpr std::uint32_t sr21_noaccess = 0x0U;
inline constexpr std::uint32_t sr21_access = 0x1U;
using sr22 = core::field_descriptor<perregion1_substatwa, 22U, 1U>;
inline constexpr std::uint32_t sr22_noaccess = 0x0U;
inline constexpr std::uint32_t sr22_access = 0x1U;
using sr23 = core::field_descriptor<perregion1_substatwa, 23U, 1U>;
inline constexpr std::uint32_t sr23_noaccess = 0x0U;
inline constexpr std::uint32_t sr23_access = 0x1U;
using sr24 = core::field_descriptor<perregion1_substatwa, 24U, 1U>;
inline constexpr std::uint32_t sr24_noaccess = 0x0U;
inline constexpr std::uint32_t sr24_access = 0x1U;
using sr25 = core::field_descriptor<perregion1_substatwa, 25U, 1U>;
inline constexpr std::uint32_t sr25_noaccess = 0x0U;
inline constexpr std::uint32_t sr25_access = 0x1U;
using sr26 = core::field_descriptor<perregion1_substatwa, 26U, 1U>;
inline constexpr std::uint32_t sr26_noaccess = 0x0U;
inline constexpr std::uint32_t sr26_access = 0x1U;
using sr27 = core::field_descriptor<perregion1_substatwa, 27U, 1U>;
inline constexpr std::uint32_t sr27_noaccess = 0x0U;
inline constexpr std::uint32_t sr27_access = 0x1U;
using sr28 = core::field_descriptor<perregion1_substatwa, 28U, 1U>;
inline constexpr std::uint32_t sr28_noaccess = 0x0U;
inline constexpr std::uint32_t sr28_access = 0x1U;
using sr29 = core::field_descriptor<perregion1_substatwa, 29U, 1U>;
inline constexpr std::uint32_t sr29_noaccess = 0x0U;
inline constexpr std::uint32_t sr29_access = 0x1U;
using sr30 = core::field_descriptor<perregion1_substatwa, 30U, 1U>;
inline constexpr std::uint32_t sr30_noaccess = 0x0U;
inline constexpr std::uint32_t sr30_access = 0x1U;
using sr31 = core::field_descriptor<perregion1_substatwa, 31U, 1U>;
inline constexpr std::uint32_t sr31_noaccess = 0x0U;
inline constexpr std::uint32_t sr31_access = 0x1U;
}
struct perregion1_substatra : core::register_descriptor<std::uint32_t, 0x40CU,
    core::access_semantics::write_one_to_clear, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace perregion1_substatra_fields {
using sr0 = core::field_descriptor<perregion1_substatra, 0U, 1U>;
inline constexpr std::uint32_t sr0_noaccess = 0x0U;
inline constexpr std::uint32_t sr0_access = 0x1U;
using sr1 = core::field_descriptor<perregion1_substatra, 1U, 1U>;
inline constexpr std::uint32_t sr1_noaccess = 0x0U;
inline constexpr std::uint32_t sr1_access = 0x1U;
using sr2 = core::field_descriptor<perregion1_substatra, 2U, 1U>;
inline constexpr std::uint32_t sr2_noaccess = 0x0U;
inline constexpr std::uint32_t sr2_access = 0x1U;
using sr3 = core::field_descriptor<perregion1_substatra, 3U, 1U>;
inline constexpr std::uint32_t sr3_noaccess = 0x0U;
inline constexpr std::uint32_t sr3_access = 0x1U;
using sr4 = core::field_descriptor<perregion1_substatra, 4U, 1U>;
inline constexpr std::uint32_t sr4_noaccess = 0x0U;
inline constexpr std::uint32_t sr4_access = 0x1U;
using sr5 = core::field_descriptor<perregion1_substatra, 5U, 1U>;
inline constexpr std::uint32_t sr5_noaccess = 0x0U;
inline constexpr std::uint32_t sr5_access = 0x1U;
using sr6 = core::field_descriptor<perregion1_substatra, 6U, 1U>;
inline constexpr std::uint32_t sr6_noaccess = 0x0U;
inline constexpr std::uint32_t sr6_access = 0x1U;
using sr7 = core::field_descriptor<perregion1_substatra, 7U, 1U>;
inline constexpr std::uint32_t sr7_noaccess = 0x0U;
inline constexpr std::uint32_t sr7_access = 0x1U;
using sr8 = core::field_descriptor<perregion1_substatra, 8U, 1U>;
inline constexpr std::uint32_t sr8_noaccess = 0x0U;
inline constexpr std::uint32_t sr8_access = 0x1U;
using sr9 = core::field_descriptor<perregion1_substatra, 9U, 1U>;
inline constexpr std::uint32_t sr9_noaccess = 0x0U;
inline constexpr std::uint32_t sr9_access = 0x1U;
using sr10 = core::field_descriptor<perregion1_substatra, 10U, 1U>;
inline constexpr std::uint32_t sr10_noaccess = 0x0U;
inline constexpr std::uint32_t sr10_access = 0x1U;
using sr11 = core::field_descriptor<perregion1_substatra, 11U, 1U>;
inline constexpr std::uint32_t sr11_noaccess = 0x0U;
inline constexpr std::uint32_t sr11_access = 0x1U;
using sr12 = core::field_descriptor<perregion1_substatra, 12U, 1U>;
inline constexpr std::uint32_t sr12_noaccess = 0x0U;
inline constexpr std::uint32_t sr12_access = 0x1U;
using sr13 = core::field_descriptor<perregion1_substatra, 13U, 1U>;
inline constexpr std::uint32_t sr13_noaccess = 0x0U;
inline constexpr std::uint32_t sr13_access = 0x1U;
using sr14 = core::field_descriptor<perregion1_substatra, 14U, 1U>;
inline constexpr std::uint32_t sr14_noaccess = 0x0U;
inline constexpr std::uint32_t sr14_access = 0x1U;
using sr15 = core::field_descriptor<perregion1_substatra, 15U, 1U>;
inline constexpr std::uint32_t sr15_noaccess = 0x0U;
inline constexpr std::uint32_t sr15_access = 0x1U;
using sr16 = core::field_descriptor<perregion1_substatra, 16U, 1U>;
inline constexpr std::uint32_t sr16_noaccess = 0x0U;
inline constexpr std::uint32_t sr16_access = 0x1U;
using sr17 = core::field_descriptor<perregion1_substatra, 17U, 1U>;
inline constexpr std::uint32_t sr17_noaccess = 0x0U;
inline constexpr std::uint32_t sr17_access = 0x1U;
using sr18 = core::field_descriptor<perregion1_substatra, 18U, 1U>;
inline constexpr std::uint32_t sr18_noaccess = 0x0U;
inline constexpr std::uint32_t sr18_access = 0x1U;
using sr19 = core::field_descriptor<perregion1_substatra, 19U, 1U>;
inline constexpr std::uint32_t sr19_noaccess = 0x0U;
inline constexpr std::uint32_t sr19_access = 0x1U;
using sr20 = core::field_descriptor<perregion1_substatra, 20U, 1U>;
inline constexpr std::uint32_t sr20_noaccess = 0x0U;
inline constexpr std::uint32_t sr20_access = 0x1U;
using sr21 = core::field_descriptor<perregion1_substatra, 21U, 1U>;
inline constexpr std::uint32_t sr21_noaccess = 0x0U;
inline constexpr std::uint32_t sr21_access = 0x1U;
using sr22 = core::field_descriptor<perregion1_substatra, 22U, 1U>;
inline constexpr std::uint32_t sr22_noaccess = 0x0U;
inline constexpr std::uint32_t sr22_access = 0x1U;
using sr23 = core::field_descriptor<perregion1_substatra, 23U, 1U>;
inline constexpr std::uint32_t sr23_noaccess = 0x0U;
inline constexpr std::uint32_t sr23_access = 0x1U;
using sr24 = core::field_descriptor<perregion1_substatra, 24U, 1U>;
inline constexpr std::uint32_t sr24_noaccess = 0x0U;
inline constexpr std::uint32_t sr24_access = 0x1U;
using sr25 = core::field_descriptor<perregion1_substatra, 25U, 1U>;
inline constexpr std::uint32_t sr25_noaccess = 0x0U;
inline constexpr std::uint32_t sr25_access = 0x1U;
using sr26 = core::field_descriptor<perregion1_substatra, 26U, 1U>;
inline constexpr std::uint32_t sr26_noaccess = 0x0U;
inline constexpr std::uint32_t sr26_access = 0x1U;
using sr27 = core::field_descriptor<perregion1_substatra, 27U, 1U>;
inline constexpr std::uint32_t sr27_noaccess = 0x0U;
inline constexpr std::uint32_t sr27_access = 0x1U;
using sr28 = core::field_descriptor<perregion1_substatra, 28U, 1U>;
inline constexpr std::uint32_t sr28_noaccess = 0x0U;
inline constexpr std::uint32_t sr28_access = 0x1U;
using sr29 = core::field_descriptor<perregion1_substatra, 29U, 1U>;
inline constexpr std::uint32_t sr29_noaccess = 0x0U;
inline constexpr std::uint32_t sr29_access = 0x1U;
using sr30 = core::field_descriptor<perregion1_substatra, 30U, 1U>;
inline constexpr std::uint32_t sr30_noaccess = 0x0U;
inline constexpr std::uint32_t sr30_access = 0x1U;
using sr31 = core::field_descriptor<perregion1_substatra, 31U, 1U>;
inline constexpr std::uint32_t sr31_noaccess = 0x0U;
inline constexpr std::uint32_t sr31_access = 0x1U;
}
struct regionen : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace regionen_fields {
using rgn0wa = core::field_descriptor<regionen, 0U, 1U>;
inline constexpr std::uint32_t rgn0wa_disable = 0x0U;
inline constexpr std::uint32_t rgn0wa_enable = 0x1U;
using rgn0ra = core::field_descriptor<regionen, 1U, 1U>;
inline constexpr std::uint32_t rgn0ra_disable = 0x0U;
inline constexpr std::uint32_t rgn0ra_enable = 0x1U;
using rgn1wa = core::field_descriptor<regionen, 2U, 1U>;
inline constexpr std::uint32_t rgn1wa_disable = 0x0U;
inline constexpr std::uint32_t rgn1wa_enable = 0x1U;
using rgn1ra = core::field_descriptor<regionen, 3U, 1U>;
inline constexpr std::uint32_t rgn1ra_disable = 0x0U;
inline constexpr std::uint32_t rgn1ra_enable = 0x1U;
using rgn2wa = core::field_descriptor<regionen, 4U, 1U>;
inline constexpr std::uint32_t rgn2wa_disable = 0x0U;
inline constexpr std::uint32_t rgn2wa_enable = 0x1U;
using rgn2ra = core::field_descriptor<regionen, 5U, 1U>;
inline constexpr std::uint32_t rgn2ra_disable = 0x0U;
inline constexpr std::uint32_t rgn2ra_enable = 0x1U;
using rgn3wa = core::field_descriptor<regionen, 6U, 1U>;
inline constexpr std::uint32_t rgn3wa_disable = 0x0U;
inline constexpr std::uint32_t rgn3wa_enable = 0x1U;
using rgn3ra = core::field_descriptor<regionen, 7U, 1U>;
inline constexpr std::uint32_t rgn3ra_disable = 0x0U;
inline constexpr std::uint32_t rgn3ra_enable = 0x1U;
using prgn0wa = core::field_descriptor<regionen, 24U, 1U>;
inline constexpr std::uint32_t prgn0wa_disable = 0x0U;
inline constexpr std::uint32_t prgn0wa_enable = 0x1U;
using prgn0ra = core::field_descriptor<regionen, 25U, 1U>;
inline constexpr std::uint32_t prgn0ra_disable = 0x0U;
inline constexpr std::uint32_t prgn0ra_enable = 0x1U;
using prgn1wa = core::field_descriptor<regionen, 26U, 1U>;
inline constexpr std::uint32_t prgn1wa_disable = 0x0U;
inline constexpr std::uint32_t prgn1wa_enable = 0x1U;
using prgn1ra = core::field_descriptor<regionen, 27U, 1U>;
inline constexpr std::uint32_t prgn1ra_disable = 0x0U;
inline constexpr std::uint32_t prgn1ra_enable = 0x1U;
}
struct regionenset : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace regionenset_fields {
using rgn0wa = core::field_descriptor<regionenset, 0U, 1U>;
inline constexpr std::uint32_t rgn0wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn0wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn0wa_set = 0x1U;
using rgn0ra = core::field_descriptor<regionenset, 1U, 1U>;
inline constexpr std::uint32_t rgn0ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn0ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn0ra_set = 0x1U;
using rgn1wa = core::field_descriptor<regionenset, 2U, 1U>;
inline constexpr std::uint32_t rgn1wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn1wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn1wa_set = 0x1U;
using rgn1ra = core::field_descriptor<regionenset, 3U, 1U>;
inline constexpr std::uint32_t rgn1ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn1ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn1ra_set = 0x1U;
using rgn2wa = core::field_descriptor<regionenset, 4U, 1U>;
inline constexpr std::uint32_t rgn2wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn2wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn2wa_set = 0x1U;
using rgn2ra = core::field_descriptor<regionenset, 5U, 1U>;
inline constexpr std::uint32_t rgn2ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn2ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn2ra_set = 0x1U;
using rgn3wa = core::field_descriptor<regionenset, 6U, 1U>;
inline constexpr std::uint32_t rgn3wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn3wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn3wa_set = 0x1U;
using rgn3ra = core::field_descriptor<regionenset, 7U, 1U>;
inline constexpr std::uint32_t rgn3ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn3ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn3ra_set = 0x1U;
using prgn0wa = core::field_descriptor<regionenset, 24U, 1U>;
inline constexpr std::uint32_t prgn0wa_disabled = 0x0U;
inline constexpr std::uint32_t prgn0wa_enabled = 0x1U;
inline constexpr std::uint32_t prgn0wa_set = 0x1U;
using prgn0ra = core::field_descriptor<regionenset, 25U, 1U>;
inline constexpr std::uint32_t prgn0ra_disabled = 0x0U;
inline constexpr std::uint32_t prgn0ra_enabled = 0x1U;
inline constexpr std::uint32_t prgn0ra_set = 0x1U;
using prgn1wa = core::field_descriptor<regionenset, 26U, 1U>;
inline constexpr std::uint32_t prgn1wa_disabled = 0x0U;
inline constexpr std::uint32_t prgn1wa_enabled = 0x1U;
inline constexpr std::uint32_t prgn1wa_set = 0x1U;
using prgn1ra = core::field_descriptor<regionenset, 27U, 1U>;
inline constexpr std::uint32_t prgn1ra_disabled = 0x0U;
inline constexpr std::uint32_t prgn1ra_enabled = 0x1U;
inline constexpr std::uint32_t prgn1ra_set = 0x1U;
}
struct regionenclr : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0x0F0000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace regionenclr_fields {
using rgn0wa = core::field_descriptor<regionenclr, 0U, 1U>;
inline constexpr std::uint32_t rgn0wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn0wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn0wa_clear = 0x1U;
using rgn0ra = core::field_descriptor<regionenclr, 1U, 1U>;
inline constexpr std::uint32_t rgn0ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn0ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn0ra_clear = 0x1U;
using rgn1wa = core::field_descriptor<regionenclr, 2U, 1U>;
inline constexpr std::uint32_t rgn1wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn1wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn1wa_clear = 0x1U;
using rgn1ra = core::field_descriptor<regionenclr, 3U, 1U>;
inline constexpr std::uint32_t rgn1ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn1ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn1ra_clear = 0x1U;
using rgn2wa = core::field_descriptor<regionenclr, 4U, 1U>;
inline constexpr std::uint32_t rgn2wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn2wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn2wa_clear = 0x1U;
using rgn2ra = core::field_descriptor<regionenclr, 5U, 1U>;
inline constexpr std::uint32_t rgn2ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn2ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn2ra_clear = 0x1U;
using rgn3wa = core::field_descriptor<regionenclr, 6U, 1U>;
inline constexpr std::uint32_t rgn3wa_disabled = 0x0U;
inline constexpr std::uint32_t rgn3wa_enabled = 0x1U;
inline constexpr std::uint32_t rgn3wa_clear = 0x1U;
using rgn3ra = core::field_descriptor<regionenclr, 7U, 1U>;
inline constexpr std::uint32_t rgn3ra_disabled = 0x0U;
inline constexpr std::uint32_t rgn3ra_enabled = 0x1U;
inline constexpr std::uint32_t rgn3ra_clear = 0x1U;
using prgn0wa = core::field_descriptor<regionenclr, 24U, 1U>;
inline constexpr std::uint32_t prgn0wa_disabled = 0x0U;
inline constexpr std::uint32_t prgn0wa_enabled = 0x1U;
inline constexpr std::uint32_t prgn0wa_clear = 0x1U;
using prgn0ra = core::field_descriptor<regionenclr, 25U, 1U>;
inline constexpr std::uint32_t prgn0ra_disabled = 0x0U;
inline constexpr std::uint32_t prgn0ra_enabled = 0x1U;
inline constexpr std::uint32_t prgn0ra_clear = 0x1U;
using prgn1wa = core::field_descriptor<regionenclr, 26U, 1U>;
inline constexpr std::uint32_t prgn1wa_disabled = 0x0U;
inline constexpr std::uint32_t prgn1wa_enabled = 0x1U;
inline constexpr std::uint32_t prgn1wa_clear = 0x1U;
using prgn1ra = core::field_descriptor<regionenclr, 27U, 1U>;
inline constexpr std::uint32_t prgn1ra_disabled = 0x0U;
inline constexpr std::uint32_t prgn1ra_enabled = 0x1U;
inline constexpr std::uint32_t prgn1ra_clear = 0x1U;
}
struct region0_start : core::register_descriptor<std::uint32_t, 0x600U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region0_start_fields {
using start = core::field_descriptor<region0_start, 0U, 32U>;
}
struct region0_end : core::register_descriptor<std::uint32_t, 0x604U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region0_end_fields {
using end = core::field_descriptor<region0_end, 0U, 32U>;
}
struct region1_start : core::register_descriptor<std::uint32_t, 0x610U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region1_start_fields {
using start = core::field_descriptor<region1_start, 0U, 32U>;
}
struct region1_end : core::register_descriptor<std::uint32_t, 0x614U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region1_end_fields {
using end = core::field_descriptor<region1_end, 0U, 32U>;
}
struct region2_start : core::register_descriptor<std::uint32_t, 0x620U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region2_start_fields {
using start = core::field_descriptor<region2_start, 0U, 32U>;
}
struct region2_end : core::register_descriptor<std::uint32_t, 0x624U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region2_end_fields {
using end = core::field_descriptor<region2_end, 0U, 32U>;
}
struct region3_start : core::register_descriptor<std::uint32_t, 0x630U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region3_start_fields {
using start = core::field_descriptor<region3_start, 0U, 32U>;
}
struct region3_end : core::register_descriptor<std::uint32_t, 0x634U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace region3_end_fields {
using end = core::field_descriptor<region3_end, 0U, 32U>;
}
struct pregion0_start : core::register_descriptor<std::uint32_t, 0x6C0U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pregion0_start_fields {
using start = core::field_descriptor<pregion0_start, 0U, 32U>;
}
struct pregion0_end : core::register_descriptor<std::uint32_t, 0x6C4U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pregion0_end_fields {
using end = core::field_descriptor<pregion0_end, 0U, 32U>;
}
struct pregion0_subs : core::register_descriptor<std::uint32_t, 0x6C8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pregion0_subs_fields {
using sr0 = core::field_descriptor<pregion0_subs, 0U, 1U>;
inline constexpr std::uint32_t sr0_exclude = 0x0U;
inline constexpr std::uint32_t sr0_include = 0x1U;
using sr1 = core::field_descriptor<pregion0_subs, 1U, 1U>;
inline constexpr std::uint32_t sr1_exclude = 0x0U;
inline constexpr std::uint32_t sr1_include = 0x1U;
using sr2 = core::field_descriptor<pregion0_subs, 2U, 1U>;
inline constexpr std::uint32_t sr2_exclude = 0x0U;
inline constexpr std::uint32_t sr2_include = 0x1U;
using sr3 = core::field_descriptor<pregion0_subs, 3U, 1U>;
inline constexpr std::uint32_t sr3_exclude = 0x0U;
inline constexpr std::uint32_t sr3_include = 0x1U;
using sr4 = core::field_descriptor<pregion0_subs, 4U, 1U>;
inline constexpr std::uint32_t sr4_exclude = 0x0U;
inline constexpr std::uint32_t sr4_include = 0x1U;
using sr5 = core::field_descriptor<pregion0_subs, 5U, 1U>;
inline constexpr std::uint32_t sr5_exclude = 0x0U;
inline constexpr std::uint32_t sr5_include = 0x1U;
using sr6 = core::field_descriptor<pregion0_subs, 6U, 1U>;
inline constexpr std::uint32_t sr6_exclude = 0x0U;
inline constexpr std::uint32_t sr6_include = 0x1U;
using sr7 = core::field_descriptor<pregion0_subs, 7U, 1U>;
inline constexpr std::uint32_t sr7_exclude = 0x0U;
inline constexpr std::uint32_t sr7_include = 0x1U;
using sr8 = core::field_descriptor<pregion0_subs, 8U, 1U>;
inline constexpr std::uint32_t sr8_exclude = 0x0U;
inline constexpr std::uint32_t sr8_include = 0x1U;
using sr9 = core::field_descriptor<pregion0_subs, 9U, 1U>;
inline constexpr std::uint32_t sr9_exclude = 0x0U;
inline constexpr std::uint32_t sr9_include = 0x1U;
using sr10 = core::field_descriptor<pregion0_subs, 10U, 1U>;
inline constexpr std::uint32_t sr10_exclude = 0x0U;
inline constexpr std::uint32_t sr10_include = 0x1U;
using sr11 = core::field_descriptor<pregion0_subs, 11U, 1U>;
inline constexpr std::uint32_t sr11_exclude = 0x0U;
inline constexpr std::uint32_t sr11_include = 0x1U;
using sr12 = core::field_descriptor<pregion0_subs, 12U, 1U>;
inline constexpr std::uint32_t sr12_exclude = 0x0U;
inline constexpr std::uint32_t sr12_include = 0x1U;
using sr13 = core::field_descriptor<pregion0_subs, 13U, 1U>;
inline constexpr std::uint32_t sr13_exclude = 0x0U;
inline constexpr std::uint32_t sr13_include = 0x1U;
using sr14 = core::field_descriptor<pregion0_subs, 14U, 1U>;
inline constexpr std::uint32_t sr14_exclude = 0x0U;
inline constexpr std::uint32_t sr14_include = 0x1U;
using sr15 = core::field_descriptor<pregion0_subs, 15U, 1U>;
inline constexpr std::uint32_t sr15_exclude = 0x0U;
inline constexpr std::uint32_t sr15_include = 0x1U;
using sr16 = core::field_descriptor<pregion0_subs, 16U, 1U>;
inline constexpr std::uint32_t sr16_exclude = 0x0U;
inline constexpr std::uint32_t sr16_include = 0x1U;
using sr17 = core::field_descriptor<pregion0_subs, 17U, 1U>;
inline constexpr std::uint32_t sr17_exclude = 0x0U;
inline constexpr std::uint32_t sr17_include = 0x1U;
using sr18 = core::field_descriptor<pregion0_subs, 18U, 1U>;
inline constexpr std::uint32_t sr18_exclude = 0x0U;
inline constexpr std::uint32_t sr18_include = 0x1U;
using sr19 = core::field_descriptor<pregion0_subs, 19U, 1U>;
inline constexpr std::uint32_t sr19_exclude = 0x0U;
inline constexpr std::uint32_t sr19_include = 0x1U;
using sr20 = core::field_descriptor<pregion0_subs, 20U, 1U>;
inline constexpr std::uint32_t sr20_exclude = 0x0U;
inline constexpr std::uint32_t sr20_include = 0x1U;
using sr21 = core::field_descriptor<pregion0_subs, 21U, 1U>;
inline constexpr std::uint32_t sr21_exclude = 0x0U;
inline constexpr std::uint32_t sr21_include = 0x1U;
using sr22 = core::field_descriptor<pregion0_subs, 22U, 1U>;
inline constexpr std::uint32_t sr22_exclude = 0x0U;
inline constexpr std::uint32_t sr22_include = 0x1U;
using sr23 = core::field_descriptor<pregion0_subs, 23U, 1U>;
inline constexpr std::uint32_t sr23_exclude = 0x0U;
inline constexpr std::uint32_t sr23_include = 0x1U;
using sr24 = core::field_descriptor<pregion0_subs, 24U, 1U>;
inline constexpr std::uint32_t sr24_exclude = 0x0U;
inline constexpr std::uint32_t sr24_include = 0x1U;
using sr25 = core::field_descriptor<pregion0_subs, 25U, 1U>;
inline constexpr std::uint32_t sr25_exclude = 0x0U;
inline constexpr std::uint32_t sr25_include = 0x1U;
using sr26 = core::field_descriptor<pregion0_subs, 26U, 1U>;
inline constexpr std::uint32_t sr26_exclude = 0x0U;
inline constexpr std::uint32_t sr26_include = 0x1U;
using sr27 = core::field_descriptor<pregion0_subs, 27U, 1U>;
inline constexpr std::uint32_t sr27_exclude = 0x0U;
inline constexpr std::uint32_t sr27_include = 0x1U;
using sr28 = core::field_descriptor<pregion0_subs, 28U, 1U>;
inline constexpr std::uint32_t sr28_exclude = 0x0U;
inline constexpr std::uint32_t sr28_include = 0x1U;
using sr29 = core::field_descriptor<pregion0_subs, 29U, 1U>;
inline constexpr std::uint32_t sr29_exclude = 0x0U;
inline constexpr std::uint32_t sr29_include = 0x1U;
using sr30 = core::field_descriptor<pregion0_subs, 30U, 1U>;
inline constexpr std::uint32_t sr30_exclude = 0x0U;
inline constexpr std::uint32_t sr30_include = 0x1U;
using sr31 = core::field_descriptor<pregion0_subs, 31U, 1U>;
inline constexpr std::uint32_t sr31_exclude = 0x0U;
inline constexpr std::uint32_t sr31_include = 0x1U;
}
struct pregion1_start : core::register_descriptor<std::uint32_t, 0x6D0U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pregion1_start_fields {
using start = core::field_descriptor<pregion1_start, 0U, 32U>;
}
struct pregion1_end : core::register_descriptor<std::uint32_t, 0x6D4U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pregion1_end_fields {
using end = core::field_descriptor<pregion1_end, 0U, 32U>;
}
struct pregion1_subs : core::register_descriptor<std::uint32_t, 0x6D8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pregion1_subs_fields {
using sr0 = core::field_descriptor<pregion1_subs, 0U, 1U>;
inline constexpr std::uint32_t sr0_exclude = 0x0U;
inline constexpr std::uint32_t sr0_include = 0x1U;
using sr1 = core::field_descriptor<pregion1_subs, 1U, 1U>;
inline constexpr std::uint32_t sr1_exclude = 0x0U;
inline constexpr std::uint32_t sr1_include = 0x1U;
using sr2 = core::field_descriptor<pregion1_subs, 2U, 1U>;
inline constexpr std::uint32_t sr2_exclude = 0x0U;
inline constexpr std::uint32_t sr2_include = 0x1U;
using sr3 = core::field_descriptor<pregion1_subs, 3U, 1U>;
inline constexpr std::uint32_t sr3_exclude = 0x0U;
inline constexpr std::uint32_t sr3_include = 0x1U;
using sr4 = core::field_descriptor<pregion1_subs, 4U, 1U>;
inline constexpr std::uint32_t sr4_exclude = 0x0U;
inline constexpr std::uint32_t sr4_include = 0x1U;
using sr5 = core::field_descriptor<pregion1_subs, 5U, 1U>;
inline constexpr std::uint32_t sr5_exclude = 0x0U;
inline constexpr std::uint32_t sr5_include = 0x1U;
using sr6 = core::field_descriptor<pregion1_subs, 6U, 1U>;
inline constexpr std::uint32_t sr6_exclude = 0x0U;
inline constexpr std::uint32_t sr6_include = 0x1U;
using sr7 = core::field_descriptor<pregion1_subs, 7U, 1U>;
inline constexpr std::uint32_t sr7_exclude = 0x0U;
inline constexpr std::uint32_t sr7_include = 0x1U;
using sr8 = core::field_descriptor<pregion1_subs, 8U, 1U>;
inline constexpr std::uint32_t sr8_exclude = 0x0U;
inline constexpr std::uint32_t sr8_include = 0x1U;
using sr9 = core::field_descriptor<pregion1_subs, 9U, 1U>;
inline constexpr std::uint32_t sr9_exclude = 0x0U;
inline constexpr std::uint32_t sr9_include = 0x1U;
using sr10 = core::field_descriptor<pregion1_subs, 10U, 1U>;
inline constexpr std::uint32_t sr10_exclude = 0x0U;
inline constexpr std::uint32_t sr10_include = 0x1U;
using sr11 = core::field_descriptor<pregion1_subs, 11U, 1U>;
inline constexpr std::uint32_t sr11_exclude = 0x0U;
inline constexpr std::uint32_t sr11_include = 0x1U;
using sr12 = core::field_descriptor<pregion1_subs, 12U, 1U>;
inline constexpr std::uint32_t sr12_exclude = 0x0U;
inline constexpr std::uint32_t sr12_include = 0x1U;
using sr13 = core::field_descriptor<pregion1_subs, 13U, 1U>;
inline constexpr std::uint32_t sr13_exclude = 0x0U;
inline constexpr std::uint32_t sr13_include = 0x1U;
using sr14 = core::field_descriptor<pregion1_subs, 14U, 1U>;
inline constexpr std::uint32_t sr14_exclude = 0x0U;
inline constexpr std::uint32_t sr14_include = 0x1U;
using sr15 = core::field_descriptor<pregion1_subs, 15U, 1U>;
inline constexpr std::uint32_t sr15_exclude = 0x0U;
inline constexpr std::uint32_t sr15_include = 0x1U;
using sr16 = core::field_descriptor<pregion1_subs, 16U, 1U>;
inline constexpr std::uint32_t sr16_exclude = 0x0U;
inline constexpr std::uint32_t sr16_include = 0x1U;
using sr17 = core::field_descriptor<pregion1_subs, 17U, 1U>;
inline constexpr std::uint32_t sr17_exclude = 0x0U;
inline constexpr std::uint32_t sr17_include = 0x1U;
using sr18 = core::field_descriptor<pregion1_subs, 18U, 1U>;
inline constexpr std::uint32_t sr18_exclude = 0x0U;
inline constexpr std::uint32_t sr18_include = 0x1U;
using sr19 = core::field_descriptor<pregion1_subs, 19U, 1U>;
inline constexpr std::uint32_t sr19_exclude = 0x0U;
inline constexpr std::uint32_t sr19_include = 0x1U;
using sr20 = core::field_descriptor<pregion1_subs, 20U, 1U>;
inline constexpr std::uint32_t sr20_exclude = 0x0U;
inline constexpr std::uint32_t sr20_include = 0x1U;
using sr21 = core::field_descriptor<pregion1_subs, 21U, 1U>;
inline constexpr std::uint32_t sr21_exclude = 0x0U;
inline constexpr std::uint32_t sr21_include = 0x1U;
using sr22 = core::field_descriptor<pregion1_subs, 22U, 1U>;
inline constexpr std::uint32_t sr22_exclude = 0x0U;
inline constexpr std::uint32_t sr22_include = 0x1U;
using sr23 = core::field_descriptor<pregion1_subs, 23U, 1U>;
inline constexpr std::uint32_t sr23_exclude = 0x0U;
inline constexpr std::uint32_t sr23_include = 0x1U;
using sr24 = core::field_descriptor<pregion1_subs, 24U, 1U>;
inline constexpr std::uint32_t sr24_exclude = 0x0U;
inline constexpr std::uint32_t sr24_include = 0x1U;
using sr25 = core::field_descriptor<pregion1_subs, 25U, 1U>;
inline constexpr std::uint32_t sr25_exclude = 0x0U;
inline constexpr std::uint32_t sr25_include = 0x1U;
using sr26 = core::field_descriptor<pregion1_subs, 26U, 1U>;
inline constexpr std::uint32_t sr26_exclude = 0x0U;
inline constexpr std::uint32_t sr26_include = 0x1U;
using sr27 = core::field_descriptor<pregion1_subs, 27U, 1U>;
inline constexpr std::uint32_t sr27_exclude = 0x0U;
inline constexpr std::uint32_t sr27_include = 0x1U;
using sr28 = core::field_descriptor<pregion1_subs, 28U, 1U>;
inline constexpr std::uint32_t sr28_exclude = 0x0U;
inline constexpr std::uint32_t sr28_include = 0x1U;
using sr29 = core::field_descriptor<pregion1_subs, 29U, 1U>;
inline constexpr std::uint32_t sr29_exclude = 0x0U;
inline constexpr std::uint32_t sr29_include = 0x1U;
using sr30 = core::field_descriptor<pregion1_subs, 30U, 1U>;
inline constexpr std::uint32_t sr30_exclude = 0x0U;
inline constexpr std::uint32_t sr30_include = 0x1U;
using sr31 = core::field_descriptor<pregion1_subs, 31U, 1U>;
inline constexpr std::uint32_t sr31_exclude = 0x0U;
inline constexpr std::uint32_t sr31_include = 0x1U;
}
}
