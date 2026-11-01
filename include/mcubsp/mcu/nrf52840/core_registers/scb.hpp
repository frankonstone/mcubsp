#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::scb {
inline constexpr std::uintptr_t base = 0xE000ED00U;
struct cpuid : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cpuid_fields {
using implementer = core::field_descriptor<cpuid, 24U, 8U, core::access_semantics::read_only>;
using variant = core::field_descriptor<cpuid, 20U, 4U, core::access_semantics::read_only>;
using architecture = core::field_descriptor<cpuid, 16U, 4U, core::access_semantics::read_only>;
using partno = core::field_descriptor<cpuid, 4U, 12U, core::access_semantics::read_only>;
using revision = core::field_descriptor<cpuid, 0U, 4U, core::access_semantics::read_only>;
}
struct icsr : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x9E000000U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using icsr_status = core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, icsr>;
namespace icsr_fields {
using nmipendset = core::field_descriptor<icsr, 31U, 1U, core::access_semantics::command>;
using pendsvset = core::field_descriptor<icsr, 28U, 1U, core::access_semantics::command>;
using pendsvclr = core::field_descriptor<icsr, 27U, 1U, core::access_semantics::command>;
using pendstset = core::field_descriptor<icsr, 26U, 1U, core::access_semantics::command>;
using pendstclr = core::field_descriptor<icsr, 25U, 1U, core::access_semantics::command>;
using isrpreempt = core::field_descriptor<icsr, 23U, 1U, core::access_semantics::read_only>;
using isrpending = core::field_descriptor<icsr, 22U, 1U, core::access_semantics::read_only>;
using vectpending = core::field_descriptor<icsr, 12U, 9U, core::access_semantics::read_only>;
using rettobase = core::field_descriptor<icsr, 11U, 1U, core::access_semantics::read_only>;
using vectactive = core::field_descriptor<icsr, 0U, 9U, core::access_semantics::read_only>;
}
struct vtor : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0xFFFFFF80U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace vtor_fields {
using tbloff = core::field_descriptor<vtor, 7U, 25U, core::access_semantics::read_write>;
}
struct aircr : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x707U, false,
    core::reserved_bit_policy::write_zero, 0x5FA0000U, 0x0U, 0x0U> {};
using aircr_status = core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, aircr>;
namespace aircr_fields {
using vectkey = core::field_descriptor<aircr, 16U, 16U, core::access_semantics::command>;
using vectkeystat = core::field_descriptor<aircr, 16U, 16U, core::access_semantics::read_only>;
using endianess = core::field_descriptor<aircr, 15U, 1U, core::access_semantics::read_only>;
using prigroup = core::field_descriptor<aircr, 8U, 3U, core::access_semantics::command>;
using sysresetreq = core::field_descriptor<aircr, 2U, 1U, core::access_semantics::command>;
using vectclractive = core::field_descriptor<aircr, 1U, 1U, core::access_semantics::command>;
using vectreset = core::field_descriptor<aircr, 0U, 1U, core::access_semantics::command>;
}
struct scr : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_write, 0x16U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace scr_fields {
using sevonpend = core::field_descriptor<scr, 4U, 1U, core::access_semantics::read_write>;
using sleepdeep = core::field_descriptor<scr, 2U, 1U, core::access_semantics::read_write>;
using sleeponexit = core::field_descriptor<scr, 1U, 1U, core::access_semantics::read_write>;
}
struct ccr : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_write, 0x31BU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ccr_fields {
using stkalign = core::field_descriptor<ccr, 9U, 1U, core::access_semantics::read_write>;
using bfhfnmign = core::field_descriptor<ccr, 8U, 1U, core::access_semantics::read_write>;
using div_0_trp = core::field_descriptor<ccr, 4U, 1U, core::access_semantics::read_write>;
using unalign_trp = core::field_descriptor<ccr, 3U, 1U, core::access_semantics::read_write>;
using usersetmpend = core::field_descriptor<ccr, 1U, 1U, core::access_semantics::read_write>;
using nonbasethrdena = core::field_descriptor<ccr, 0U, 1U, core::access_semantics::read_write>;
}
struct shp0 : core::register_descriptor<std::uint8_t, 0x18U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp0_fields {
using priority = core::field_descriptor<shp0, 5U, 3U, core::access_semantics::read_write>;
}
struct shp1 : core::register_descriptor<std::uint8_t, 0x19U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp1_fields {
using priority = core::field_descriptor<shp1, 5U, 3U, core::access_semantics::read_write>;
}
struct shp2 : core::register_descriptor<std::uint8_t, 0x1AU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp2_fields {
using priority = core::field_descriptor<shp2, 5U, 3U, core::access_semantics::read_write>;
}
struct shp7 : core::register_descriptor<std::uint8_t, 0x1FU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp7_fields {
using priority = core::field_descriptor<shp7, 5U, 3U, core::access_semantics::read_write>;
}
struct shp8 : core::register_descriptor<std::uint8_t, 0x20U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp8_fields {
using priority = core::field_descriptor<shp8, 5U, 3U, core::access_semantics::read_write>;
}
struct shp10 : core::register_descriptor<std::uint8_t, 0x22U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp10_fields {
using priority = core::field_descriptor<shp10, 5U, 3U, core::access_semantics::read_write>;
}
struct shp11 : core::register_descriptor<std::uint8_t, 0x23U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shp11_fields {
using priority = core::field_descriptor<shp11, 5U, 3U, core::access_semantics::read_write>;
}
struct shcsr : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_write, 0x7FD8BU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace shcsr_fields {
using usgfaultena = core::field_descriptor<shcsr, 18U, 1U, core::access_semantics::read_write>;
using busfaultena = core::field_descriptor<shcsr, 17U, 1U, core::access_semantics::read_write>;
using memfaultena = core::field_descriptor<shcsr, 16U, 1U, core::access_semantics::read_write>;
using svcallpended = core::field_descriptor<shcsr, 15U, 1U, core::access_semantics::read_write>;
using busfaultpended = core::field_descriptor<shcsr, 14U, 1U, core::access_semantics::read_write>;
using memfaultpended = core::field_descriptor<shcsr, 13U, 1U, core::access_semantics::read_write>;
using usgfaultpended = core::field_descriptor<shcsr, 12U, 1U, core::access_semantics::read_write>;
using systickact = core::field_descriptor<shcsr, 11U, 1U, core::access_semantics::read_write>;
using pendsvact = core::field_descriptor<shcsr, 10U, 1U, core::access_semantics::read_write>;
using monitoract = core::field_descriptor<shcsr, 8U, 1U, core::access_semantics::read_write>;
using svcallact = core::field_descriptor<shcsr, 7U, 1U, core::access_semantics::read_write>;
using usgfaultact = core::field_descriptor<shcsr, 3U, 1U, core::access_semantics::read_write>;
using busfaultact = core::field_descriptor<shcsr, 1U, 1U, core::access_semantics::read_write>;
using memfaultact = core::field_descriptor<shcsr, 0U, 1U, core::access_semantics::read_write>;
}
struct cfsr : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::write_one_to_clear, 0x30FBFBBU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cfsr_fields {
using usgfaultsr = core::field_descriptor<cfsr, 16U, 16U, core::access_semantics::write_one_to_clear>;
using busfaultsr = core::field_descriptor<cfsr, 8U, 8U, core::access_semantics::write_one_to_clear>;
using memfaultsr = core::field_descriptor<cfsr, 0U, 8U, core::access_semantics::write_one_to_clear>;
using mmarvalid = core::field_descriptor<cfsr, 7U, 1U, core::access_semantics::write_one_to_clear>;
using mlsperr = core::field_descriptor<cfsr, 5U, 1U, core::access_semantics::write_one_to_clear>;
using mstkerr = core::field_descriptor<cfsr, 4U, 1U, core::access_semantics::write_one_to_clear>;
using munstkerr = core::field_descriptor<cfsr, 3U, 1U, core::access_semantics::write_one_to_clear>;
using daccviol = core::field_descriptor<cfsr, 1U, 1U, core::access_semantics::write_one_to_clear>;
using iaccviol = core::field_descriptor<cfsr, 0U, 1U, core::access_semantics::write_one_to_clear>;
using bfarvalid = core::field_descriptor<cfsr, 15U, 1U, core::access_semantics::write_one_to_clear>;
using lsperr = core::field_descriptor<cfsr, 13U, 1U, core::access_semantics::write_one_to_clear>;
using stkerr = core::field_descriptor<cfsr, 12U, 1U, core::access_semantics::write_one_to_clear>;
using unstkerr = core::field_descriptor<cfsr, 11U, 1U, core::access_semantics::write_one_to_clear>;
using impreciserr = core::field_descriptor<cfsr, 10U, 1U, core::access_semantics::write_one_to_clear>;
using preciserr = core::field_descriptor<cfsr, 9U, 1U, core::access_semantics::write_one_to_clear>;
using ibuserr = core::field_descriptor<cfsr, 8U, 1U, core::access_semantics::write_one_to_clear>;
using divbyzero = core::field_descriptor<cfsr, 25U, 1U, core::access_semantics::write_one_to_clear>;
using unaligned = core::field_descriptor<cfsr, 24U, 1U, core::access_semantics::write_one_to_clear>;
using nocp = core::field_descriptor<cfsr, 19U, 1U, core::access_semantics::write_one_to_clear>;
using invpc = core::field_descriptor<cfsr, 18U, 1U, core::access_semantics::write_one_to_clear>;
using invstate = core::field_descriptor<cfsr, 17U, 1U, core::access_semantics::write_one_to_clear>;
using undefinstr = core::field_descriptor<cfsr, 16U, 1U, core::access_semantics::write_one_to_clear>;
}
struct hfsr : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::write_one_to_clear, 0xC0000002U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace hfsr_fields {
using debugevt = core::field_descriptor<hfsr, 31U, 1U, core::access_semantics::write_one_to_clear>;
using forced = core::field_descriptor<hfsr, 30U, 1U, core::access_semantics::write_one_to_clear>;
using vecttbl = core::field_descriptor<hfsr, 1U, 1U, core::access_semantics::write_one_to_clear>;
}
struct dfsr : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::write_one_to_clear, 0x1FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace dfsr_fields {
using external = core::field_descriptor<dfsr, 4U, 1U, core::access_semantics::write_one_to_clear>;
using vcatch = core::field_descriptor<dfsr, 3U, 1U, core::access_semantics::write_one_to_clear>;
using dwttrap = core::field_descriptor<dfsr, 2U, 1U, core::access_semantics::write_one_to_clear>;
using bkpt = core::field_descriptor<dfsr, 1U, 1U, core::access_semantics::write_one_to_clear>;
using halted = core::field_descriptor<dfsr, 0U, 1U, core::access_semantics::write_one_to_clear>;
}
struct mmfar : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mmfar_fields {
using value = core::field_descriptor<mmfar, 0U, 32U, core::access_semantics::read_write>;
}
struct bfar : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace bfar_fields {
using value = core::field_descriptor<bfar, 0U, 32U, core::access_semantics::read_write>;
}
struct afsr : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace afsr_fields {
using value = core::field_descriptor<afsr, 0U, 32U, core::access_semantics::read_write>;
}
struct pfr0 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pfr0_fields {
using state0 = core::field_descriptor<pfr0, 0U, 4U, core::access_semantics::read_only>;
using state1 = core::field_descriptor<pfr0, 4U, 4U, core::access_semantics::read_only>;
}
struct pfr1 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace pfr1_fields {
using m_profile = core::field_descriptor<pfr1, 8U, 4U, core::access_semantics::read_only>;
}
struct dfr : core::register_descriptor<std::uint32_t, 0x48U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace dfr_fields {
using m_profile_debug = core::field_descriptor<dfr, 20U, 4U, core::access_semantics::read_only>;
}
struct adr : core::register_descriptor<std::uint32_t, 0x4CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace adr_fields {
using implementation_0 = core::field_descriptor<adr, 0U, 4U, core::access_semantics::read_only>;
using implementation_1 = core::field_descriptor<adr, 4U, 4U, core::access_semantics::read_only>;
using implementation_2 = core::field_descriptor<adr, 8U, 4U, core::access_semantics::read_only>;
using implementation_3 = core::field_descriptor<adr, 12U, 4U, core::access_semantics::read_only>;
}
struct mmfr0 : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mmfr0_fields {
using pmsa = core::field_descriptor<mmfr0, 4U, 4U, core::access_semantics::read_only>;
using outermost_shareability = core::field_descriptor<mmfr0, 8U, 4U, core::access_semantics::read_only>;
using shareability_levels = core::field_descriptor<mmfr0, 12U, 4U, core::access_semantics::read_only>;
using tcm = core::field_descriptor<mmfr0, 16U, 4U, core::access_semantics::read_only>;
using auxiliary_registers = core::field_descriptor<mmfr0, 20U, 4U, core::access_semantics::read_only>;
}
struct mmfr1 : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mmfr1_fields {
}
struct mmfr2 : core::register_descriptor<std::uint32_t, 0x58U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mmfr2_fields {
using wfi_stall = core::field_descriptor<mmfr2, 24U, 4U, core::access_semantics::read_only>;
}
struct mmfr3 : core::register_descriptor<std::uint32_t, 0x5CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace mmfr3_fields {
}
struct isar0 : core::register_descriptor<std::uint32_t, 0x60U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace isar0_fields {
using bit_count = core::field_descriptor<isar0, 4U, 4U, core::access_semantics::read_only>;
using bitfield = core::field_descriptor<isar0, 8U, 4U, core::access_semantics::read_only>;
using compare_branch = core::field_descriptor<isar0, 12U, 4U, core::access_semantics::read_only>;
using coprocessor = core::field_descriptor<isar0, 16U, 4U, core::access_semantics::read_only>;
using debug = core::field_descriptor<isar0, 20U, 4U, core::access_semantics::read_only>;
using divide = core::field_descriptor<isar0, 24U, 4U, core::access_semantics::read_only>;
}
struct isar1 : core::register_descriptor<std::uint32_t, 0x64U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace isar1_fields {
using extend = core::field_descriptor<isar1, 12U, 4U, core::access_semantics::read_only>;
using if_then = core::field_descriptor<isar1, 16U, 4U, core::access_semantics::read_only>;
using immediate = core::field_descriptor<isar1, 20U, 4U, core::access_semantics::read_only>;
using interwork = core::field_descriptor<isar1, 24U, 4U, core::access_semantics::read_only>;
}
struct isar2 : core::register_descriptor<std::uint32_t, 0x68U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace isar2_fields {
using load_store = core::field_descriptor<isar2, 0U, 4U, core::access_semantics::read_only>;
using memory_hint = core::field_descriptor<isar2, 4U, 4U, core::access_semantics::read_only>;
using multi_access_interrupt = core::field_descriptor<isar2, 8U, 4U, core::access_semantics::read_only>;
using multiply = core::field_descriptor<isar2, 12U, 4U, core::access_semantics::read_only>;
using multiply_signed = core::field_descriptor<isar2, 16U, 4U, core::access_semantics::read_only>;
using multiply_unsigned = core::field_descriptor<isar2, 20U, 4U, core::access_semantics::read_only>;
using reversal = core::field_descriptor<isar2, 28U, 4U, core::access_semantics::read_only>;
}
struct isar3 : core::register_descriptor<std::uint32_t, 0x6CU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace isar3_fields {
using saturate = core::field_descriptor<isar3, 0U, 4U, core::access_semantics::read_only>;
using simd = core::field_descriptor<isar3, 4U, 4U, core::access_semantics::read_only>;
using svc = core::field_descriptor<isar3, 8U, 4U, core::access_semantics::read_only>;
using synchronization = core::field_descriptor<isar3, 12U, 4U, core::access_semantics::read_only>;
using table_branch = core::field_descriptor<isar3, 16U, 4U, core::access_semantics::read_only>;
using thumb_copy = core::field_descriptor<isar3, 20U, 4U, core::access_semantics::read_only>;
using true_nop = core::field_descriptor<isar3, 24U, 4U, core::access_semantics::read_only>;
}
struct isar4 : core::register_descriptor<std::uint32_t, 0x70U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace isar4_fields {
using unprivileged = core::field_descriptor<isar4, 0U, 4U, core::access_semantics::read_only>;
using with_shifts = core::field_descriptor<isar4, 4U, 4U, core::access_semantics::read_only>;
using writeback = core::field_descriptor<isar4, 8U, 4U, core::access_semantics::read_only>;
using barrier = core::field_descriptor<isar4, 16U, 4U, core::access_semantics::read_only>;
using synchronization_fraction = core::field_descriptor<isar4, 20U, 4U, core::access_semantics::read_only>;
using psr_m = core::field_descriptor<isar4, 24U, 4U, core::access_semantics::read_only>;
}
struct cpacr : core::register_descriptor<std::uint32_t, 0x88U,
    core::access_semantics::read_write, 0xF00000U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cpacr_fields {
using cp10 = core::field_descriptor<cpacr, 20U, 2U, core::access_semantics::read_write>;
using cp11 = core::field_descriptor<cpacr, 22U, 2U, core::access_semantics::read_write>;
}
}
