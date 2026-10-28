#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::ficr {
struct codepagesize : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace codepagesize_fields {
using codepagesize = core::field_descriptor<codepagesize, 0U, 32U>;
}
struct codesize : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace codesize_fields {
using codesize = core::field_descriptor<codesize, 0U, 32U>;
}
struct deviceid0 : core::register_descriptor<std::uint32_t, 0x60U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace deviceid0_fields {
using deviceid = core::field_descriptor<deviceid0, 0U, 32U>;
}
struct deviceid1 : core::register_descriptor<std::uint32_t, 0x64U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace deviceid1_fields {
using deviceid = core::field_descriptor<deviceid1, 0U, 32U>;
}
struct er0 : core::register_descriptor<std::uint32_t, 0x80U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace er0_fields {
using er = core::field_descriptor<er0, 0U, 32U>;
}
struct er1 : core::register_descriptor<std::uint32_t, 0x84U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace er1_fields {
using er = core::field_descriptor<er1, 0U, 32U>;
}
struct er2 : core::register_descriptor<std::uint32_t, 0x88U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace er2_fields {
using er = core::field_descriptor<er2, 0U, 32U>;
}
struct er3 : core::register_descriptor<std::uint32_t, 0x8CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace er3_fields {
using er = core::field_descriptor<er3, 0U, 32U>;
}
struct ir0 : core::register_descriptor<std::uint32_t, 0x90U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ir0_fields {
using ir = core::field_descriptor<ir0, 0U, 32U>;
}
struct ir1 : core::register_descriptor<std::uint32_t, 0x94U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ir1_fields {
using ir = core::field_descriptor<ir1, 0U, 32U>;
}
struct ir2 : core::register_descriptor<std::uint32_t, 0x98U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ir2_fields {
using ir = core::field_descriptor<ir2, 0U, 32U>;
}
struct ir3 : core::register_descriptor<std::uint32_t, 0x9CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ir3_fields {
using ir = core::field_descriptor<ir3, 0U, 32U>;
}
struct deviceaddrtype : core::register_descriptor<std::uint32_t, 0xA0U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace deviceaddrtype_fields {
using deviceaddrtype = core::field_descriptor<deviceaddrtype, 0U, 1U>;
inline constexpr std::uint32_t deviceaddrtype_reg_public = 0x0U;
inline constexpr std::uint32_t deviceaddrtype_random = 0x1U;
}
struct deviceaddr0 : core::register_descriptor<std::uint32_t, 0xA4U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace deviceaddr0_fields {
using deviceaddr = core::field_descriptor<deviceaddr0, 0U, 32U>;
}
struct deviceaddr1 : core::register_descriptor<std::uint32_t, 0xA8U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace deviceaddr1_fields {
using deviceaddr = core::field_descriptor<deviceaddr1, 0U, 32U>;
}
struct info_part : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00052840U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace info_part_fields {
using part = core::field_descriptor<info_part, 0U, 32U>;
inline constexpr std::uint32_t part_n52820 = 0x52820U;
inline constexpr std::uint32_t part_n52833 = 0x52833U;
inline constexpr std::uint32_t part_n52840 = 0x52840U;
inline constexpr std::uint32_t part_unspecified = 0xFFFFFFFFU;
}
struct info_variant : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace info_variant_fields {
using variant = core::field_descriptor<info_variant, 0U, 32U>;
inline constexpr std::uint32_t variant_aaaa = 0x41414141U;
inline constexpr std::uint32_t variant_baaa = 0x42414141U;
inline constexpr std::uint32_t variant_caaa = 0x43414141U;
inline constexpr std::uint32_t variant_aaba = 0x41414241U;
inline constexpr std::uint32_t variant_aabb = 0x41414242U;
inline constexpr std::uint32_t variant_aaca = 0x41414341U;
inline constexpr std::uint32_t variant_aaab = 0x41414142U;
inline constexpr std::uint32_t variant_aac0 = 0x41414330U;
inline constexpr std::uint32_t variant_aada = 0x41414441U;
inline constexpr std::uint32_t variant_aad0 = 0x41414430U;
inline constexpr std::uint32_t variant_aad1 = 0x41414431U;
inline constexpr std::uint32_t variant_aaea = 0x41414541U;
inline constexpr std::uint32_t variant_aaf0 = 0x41414630U;
inline constexpr std::uint32_t variant_aafa = 0x41414641U;
inline constexpr std::uint32_t variant_unspecified = 0xFFFFFFFFU;
}
struct info_package : core::register_descriptor<std::uint32_t, 0x108U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace info_package_fields {
using package = core::field_descriptor<info_package, 0U, 32U>;
inline constexpr std::uint32_t package_qi = 0x2004U;
inline constexpr std::uint32_t package_qf = 0x2000U;
inline constexpr std::uint32_t package_ck = 0x2005U;
inline constexpr std::uint32_t package_unspecified = 0xFFFFFFFFU;
}
struct info_ram : core::register_descriptor<std::uint32_t, 0x10CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace info_ram_fields {
using ram = core::field_descriptor<info_ram, 0U, 32U>;
inline constexpr std::uint32_t ram_k16 = 0x10U;
inline constexpr std::uint32_t ram_k32 = 0x20U;
inline constexpr std::uint32_t ram_k64 = 0x40U;
inline constexpr std::uint32_t ram_k128 = 0x80U;
inline constexpr std::uint32_t ram_k256 = 0x100U;
inline constexpr std::uint32_t ram_unspecified = 0xFFFFFFFFU;
}
struct info_flash : core::register_descriptor<std::uint32_t, 0x110U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace info_flash_fields {
using flash = core::field_descriptor<info_flash, 0U, 32U>;
inline constexpr std::uint32_t flash_k128 = 0x80U;
inline constexpr std::uint32_t flash_k256 = 0x100U;
inline constexpr std::uint32_t flash_k512 = 0x200U;
inline constexpr std::uint32_t flash_k1024 = 0x400U;
inline constexpr std::uint32_t flash_k2048 = 0x800U;
inline constexpr std::uint32_t flash_unspecified = 0xFFFFFFFFU;
}
struct prodtest0 : core::register_descriptor<std::uint32_t, 0x350U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prodtest0_fields {
using prodtest = core::field_descriptor<prodtest0, 0U, 32U>;
inline constexpr std::uint32_t prodtest_done = 0xBB42319FU;
inline constexpr std::uint32_t prodtest_notdone = 0xFFFFFFFFU;
}
struct prodtest1 : core::register_descriptor<std::uint32_t, 0x354U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prodtest1_fields {
using prodtest = core::field_descriptor<prodtest1, 0U, 32U>;
inline constexpr std::uint32_t prodtest_done = 0xBB42319FU;
inline constexpr std::uint32_t prodtest_notdone = 0xFFFFFFFFU;
}
struct prodtest2 : core::register_descriptor<std::uint32_t, 0x358U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace prodtest2_fields {
using prodtest = core::field_descriptor<prodtest2, 0U, 32U>;
inline constexpr std::uint32_t prodtest_done = 0xBB42319FU;
inline constexpr std::uint32_t prodtest_notdone = 0xFFFFFFFFU;
}
struct temp_a0 : core::register_descriptor<std::uint32_t, 0x404U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_a0_fields {
using a = core::field_descriptor<temp_a0, 0U, 12U>;
}
struct temp_a1 : core::register_descriptor<std::uint32_t, 0x408U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_a1_fields {
using a = core::field_descriptor<temp_a1, 0U, 12U>;
}
struct temp_a2 : core::register_descriptor<std::uint32_t, 0x40CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_a2_fields {
using a = core::field_descriptor<temp_a2, 0U, 12U>;
}
struct temp_a3 : core::register_descriptor<std::uint32_t, 0x410U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_a3_fields {
using a = core::field_descriptor<temp_a3, 0U, 12U>;
}
struct temp_a4 : core::register_descriptor<std::uint32_t, 0x414U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_a4_fields {
using a = core::field_descriptor<temp_a4, 0U, 12U>;
}
struct temp_a5 : core::register_descriptor<std::uint32_t, 0x418U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_a5_fields {
using a = core::field_descriptor<temp_a5, 0U, 12U>;
}
struct temp_b0 : core::register_descriptor<std::uint32_t, 0x41CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_b0_fields {
using b = core::field_descriptor<temp_b0, 0U, 14U>;
}
struct temp_b1 : core::register_descriptor<std::uint32_t, 0x420U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_b1_fields {
using b = core::field_descriptor<temp_b1, 0U, 14U>;
}
struct temp_b2 : core::register_descriptor<std::uint32_t, 0x424U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_b2_fields {
using b = core::field_descriptor<temp_b2, 0U, 14U>;
}
struct temp_b3 : core::register_descriptor<std::uint32_t, 0x428U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_b3_fields {
using b = core::field_descriptor<temp_b3, 0U, 14U>;
}
struct temp_b4 : core::register_descriptor<std::uint32_t, 0x42CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_b4_fields {
using b = core::field_descriptor<temp_b4, 0U, 14U>;
}
struct temp_b5 : core::register_descriptor<std::uint32_t, 0x430U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_b5_fields {
using b = core::field_descriptor<temp_b5, 0U, 14U>;
}
struct temp_t0 : core::register_descriptor<std::uint32_t, 0x434U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_t0_fields {
using t = core::field_descriptor<temp_t0, 0U, 8U>;
}
struct temp_t1 : core::register_descriptor<std::uint32_t, 0x438U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_t1_fields {
using t = core::field_descriptor<temp_t1, 0U, 8U>;
}
struct temp_t2 : core::register_descriptor<std::uint32_t, 0x43CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_t2_fields {
using t = core::field_descriptor<temp_t2, 0U, 8U>;
}
struct temp_t3 : core::register_descriptor<std::uint32_t, 0x440U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_t3_fields {
using t = core::field_descriptor<temp_t3, 0U, 8U>;
}
struct temp_t4 : core::register_descriptor<std::uint32_t, 0x444U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace temp_t4_fields {
using t = core::field_descriptor<temp_t4, 0U, 8U>;
}
struct nfc_tagheader0 : core::register_descriptor<std::uint32_t, 0x450U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFF5FU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfc_tagheader0_fields {
using mfgid = core::field_descriptor<nfc_tagheader0, 0U, 8U>;
using ud1 = core::field_descriptor<nfc_tagheader0, 8U, 8U>;
using ud2 = core::field_descriptor<nfc_tagheader0, 16U, 8U>;
using ud3 = core::field_descriptor<nfc_tagheader0, 24U, 8U>;
}
struct nfc_tagheader1 : core::register_descriptor<std::uint32_t, 0x454U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfc_tagheader1_fields {
using ud4 = core::field_descriptor<nfc_tagheader1, 0U, 8U>;
using ud5 = core::field_descriptor<nfc_tagheader1, 8U, 8U>;
using ud6 = core::field_descriptor<nfc_tagheader1, 16U, 8U>;
using ud7 = core::field_descriptor<nfc_tagheader1, 24U, 8U>;
}
struct nfc_tagheader2 : core::register_descriptor<std::uint32_t, 0x458U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfc_tagheader2_fields {
using ud8 = core::field_descriptor<nfc_tagheader2, 0U, 8U>;
using ud9 = core::field_descriptor<nfc_tagheader2, 8U, 8U>;
using ud10 = core::field_descriptor<nfc_tagheader2, 16U, 8U>;
using ud11 = core::field_descriptor<nfc_tagheader2, 24U, 8U>;
}
struct nfc_tagheader3 : core::register_descriptor<std::uint32_t, 0x45CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfc_tagheader3_fields {
using ud12 = core::field_descriptor<nfc_tagheader3, 0U, 8U>;
using ud13 = core::field_descriptor<nfc_tagheader3, 8U, 8U>;
using ud14 = core::field_descriptor<nfc_tagheader3, 16U, 8U>;
using ud15 = core::field_descriptor<nfc_tagheader3, 24U, 8U>;
}
struct trng90b_bytes : core::register_descriptor<std::uint32_t, 0xC00U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_bytes_fields {
using bytes = core::field_descriptor<trng90b_bytes, 0U, 32U>;
}
struct trng90b_rccutoff : core::register_descriptor<std::uint32_t, 0xC04U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_rccutoff_fields {
using rccutoff = core::field_descriptor<trng90b_rccutoff, 0U, 32U>;
}
struct trng90b_apcutoff : core::register_descriptor<std::uint32_t, 0xC08U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_apcutoff_fields {
using apcutoff = core::field_descriptor<trng90b_apcutoff, 0U, 32U>;
}
struct trng90b_startup : core::register_descriptor<std::uint32_t, 0xC0CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_startup_fields {
using startup = core::field_descriptor<trng90b_startup, 0U, 32U>;
}
struct trng90b_rosc1 : core::register_descriptor<std::uint32_t, 0xC10U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_rosc1_fields {
using rosc1 = core::field_descriptor<trng90b_rosc1, 0U, 32U>;
}
struct trng90b_rosc2 : core::register_descriptor<std::uint32_t, 0xC14U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_rosc2_fields {
using rosc2 = core::field_descriptor<trng90b_rosc2, 0U, 32U>;
}
struct trng90b_rosc3 : core::register_descriptor<std::uint32_t, 0xC18U,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_rosc3_fields {
using rosc3 = core::field_descriptor<trng90b_rosc3, 0U, 32U>;
}
struct trng90b_rosc4 : core::register_descriptor<std::uint32_t, 0xC1CU,
    core::access_semantics::read_only, 0x00000000U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace trng90b_rosc4_fields {
using rosc4 = core::field_descriptor<trng90b_rosc4, 0U, 32U>;
}
}
