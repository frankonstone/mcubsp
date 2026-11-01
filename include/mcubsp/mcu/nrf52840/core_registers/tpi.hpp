#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::tpi {
inline constexpr std::uintptr_t base = 0xE0040000U;
struct sspsr : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace sspsr_fields {
using value = core::field_descriptor<sspsr, 0U, 32U, core::access_semantics::read_only>;
}
struct cspsr : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace cspsr_fields {
using value = core::field_descriptor<cspsr, 0U, 32U, core::access_semantics::read_write>;
}
struct acpr : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_write, 0x1FFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace acpr_fields {
using prescaler = core::field_descriptor<acpr, 0U, 13U, core::access_semantics::read_write>;
}
struct sppr : core::register_descriptor<std::uint32_t, 0xF0U,
    core::access_semantics::read_write, 0x3U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace sppr_fields {
using txmode = core::field_descriptor<sppr, 0U, 2U, core::access_semantics::read_write>;
}
struct ffsr : core::register_descriptor<std::uint32_t, 0x300U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ffsr_fields {
using ftnonstop = core::field_descriptor<ffsr, 3U, 1U, core::access_semantics::read_only>;
using tcpresent = core::field_descriptor<ffsr, 2U, 1U, core::access_semantics::read_only>;
using ftstopped = core::field_descriptor<ffsr, 1U, 1U, core::access_semantics::read_only>;
using flinprog = core::field_descriptor<ffsr, 0U, 1U, core::access_semantics::read_only>;
}
struct ffcr : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::read_write, 0x2U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ffcr_fields {
using trigin = core::field_descriptor<ffcr, 8U, 1U, core::access_semantics::read_only>;
using enfcont = core::field_descriptor<ffcr, 1U, 1U, core::access_semantics::read_write>;
}
struct fscr : core::register_descriptor<std::uint32_t, 0x308U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace fscr_fields {
using value = core::field_descriptor<fscr, 0U, 32U, core::access_semantics::read_only>;
}
struct trigger : core::register_descriptor<std::uint32_t, 0xEE8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace trigger_fields {
using trigger = core::field_descriptor<trigger, 0U, 1U, core::access_semantics::read_only>;
}
struct fifo0 : core::register_descriptor<std::uint32_t, 0xEECU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace fifo0_fields {
using itm_atvalid = core::field_descriptor<fifo0, 29U, 1U, core::access_semantics::read_only>;
using itm_bytecount = core::field_descriptor<fifo0, 27U, 2U, core::access_semantics::read_only>;
using etm_atvalid = core::field_descriptor<fifo0, 26U, 1U, core::access_semantics::read_only>;
using etm_bytecount = core::field_descriptor<fifo0, 24U, 2U, core::access_semantics::read_only>;
using etm2 = core::field_descriptor<fifo0, 16U, 8U, core::access_semantics::read_only>;
using etm1 = core::field_descriptor<fifo0, 8U, 8U, core::access_semantics::read_only>;
using etm0 = core::field_descriptor<fifo0, 0U, 8U, core::access_semantics::read_only>;
}
struct itatbctr2 : core::register_descriptor<std::uint32_t, 0xEF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace itatbctr2_fields {
using atready2 = core::field_descriptor<itatbctr2, 0U, 1U, core::access_semantics::read_only>;
using atready1 = core::field_descriptor<itatbctr2, 0U, 1U, core::access_semantics::read_only>;
}
struct itatbctr0 : core::register_descriptor<std::uint32_t, 0xEF8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace itatbctr0_fields {
using atready2 = core::field_descriptor<itatbctr0, 0U, 1U, core::access_semantics::read_only>;
using atready1 = core::field_descriptor<itatbctr0, 0U, 1U, core::access_semantics::read_only>;
}
struct fifo1 : core::register_descriptor<std::uint32_t, 0xEFCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace fifo1_fields {
using itm_atvalid = core::field_descriptor<fifo1, 29U, 1U, core::access_semantics::read_only>;
using itm_bytecount = core::field_descriptor<fifo1, 27U, 2U, core::access_semantics::read_only>;
using etm_atvalid = core::field_descriptor<fifo1, 26U, 1U, core::access_semantics::read_only>;
using etm_bytecount = core::field_descriptor<fifo1, 24U, 2U, core::access_semantics::read_only>;
using itm2 = core::field_descriptor<fifo1, 16U, 8U, core::access_semantics::read_only>;
using itm1 = core::field_descriptor<fifo1, 8U, 8U, core::access_semantics::read_only>;
using itm0 = core::field_descriptor<fifo1, 0U, 8U, core::access_semantics::read_only>;
}
struct itctrl : core::register_descriptor<std::uint32_t, 0xF00U,
    core::access_semantics::read_write, 0x1U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace itctrl_fields {
using ime = core::field_descriptor<itctrl, 0U, 1U, core::access_semantics::read_write>;
}
struct claimset : core::register_descriptor<std::uint32_t, 0xFA0U,
    core::access_semantics::set_alias, 0xFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using claimset_status = core::register_descriptor<std::uint32_t, 0xFA0U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, claimset>;
namespace claimset_fields {
using tag = core::field_descriptor<claimset, 0U, 4U, core::access_semantics::set_alias>;
}
struct claimclr : core::register_descriptor<std::uint32_t, 0xFA4U,
    core::access_semantics::clear_alias, 0xFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using claimclr_status = core::register_descriptor<std::uint32_t, 0xFA4U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, claimclr>;
namespace claimclr_fields {
using tag = core::field_descriptor<claimclr, 0U, 4U, core::access_semantics::clear_alias>;
}
struct devid : core::register_descriptor<std::uint32_t, 0xFC8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace devid_fields {
using nrzvalid = core::field_descriptor<devid, 11U, 1U, core::access_semantics::read_only>;
using mancvalid = core::field_descriptor<devid, 10U, 1U, core::access_semantics::read_only>;
using ptinvalid = core::field_descriptor<devid, 9U, 1U, core::access_semantics::read_only>;
using minbufsz = core::field_descriptor<devid, 6U, 3U, core::access_semantics::read_only>;
using asynclkin = core::field_descriptor<devid, 5U, 1U, core::access_semantics::read_only>;
using nrtraceinput = core::field_descriptor<devid, 0U, 5U, core::access_semantics::read_only>;
}
struct devtype : core::register_descriptor<std::uint32_t, 0xFCCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace devtype_fields {
using subtype = core::field_descriptor<devtype, 4U, 4U, core::access_semantics::read_only>;
using majortype = core::field_descriptor<devtype, 0U, 4U, core::access_semantics::read_only>;
}
struct pid4 : core::register_descriptor<std::uint32_t, 0xFD0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x4U, 0xFFFFFFFFU> {};
namespace pid4_fields {
using id = core::field_descriptor<pid4, 0U, 8U, core::access_semantics::read_only>;
}
struct pid5 : core::register_descriptor<std::uint32_t, 0xFD4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid5_fields {
using id = core::field_descriptor<pid5, 0U, 8U, core::access_semantics::read_only>;
}
struct pid6 : core::register_descriptor<std::uint32_t, 0xFD8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid6_fields {
using id = core::field_descriptor<pid6, 0U, 8U, core::access_semantics::read_only>;
}
struct pid7 : core::register_descriptor<std::uint32_t, 0xFDCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid7_fields {
using id = core::field_descriptor<pid7, 0U, 8U, core::access_semantics::read_only>;
}
struct pid0 : core::register_descriptor<std::uint32_t, 0xFE0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xA1U, 0xFFFFFFFFU> {};
namespace pid0_fields {
using id = core::field_descriptor<pid0, 0U, 8U, core::access_semantics::read_only>;
}
struct pid1 : core::register_descriptor<std::uint32_t, 0xFE4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xB9U, 0xFFFFFFFFU> {};
namespace pid1_fields {
using id = core::field_descriptor<pid1, 0U, 8U, core::access_semantics::read_only>;
}
struct pid2 : core::register_descriptor<std::uint32_t, 0xFE8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xBU, 0xFFFFFFFFU> {};
namespace pid2_fields {
using id = core::field_descriptor<pid2, 0U, 8U, core::access_semantics::read_only>;
}
struct pid3 : core::register_descriptor<std::uint32_t, 0xFECU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace pid3_fields {
using id = core::field_descriptor<pid3, 0U, 8U, core::access_semantics::read_only>;
}
struct cid0 : core::register_descriptor<std::uint32_t, 0xFF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xDU, 0xFFFFFFFFU> {};
namespace cid0_fields {
using id = core::field_descriptor<cid0, 0U, 8U, core::access_semantics::read_only>;
}
struct cid1 : core::register_descriptor<std::uint32_t, 0xFF4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x90U, 0xFFFFFFFFU> {};
namespace cid1_fields {
using id = core::field_descriptor<cid1, 0U, 8U, core::access_semantics::read_only>;
}
struct cid2 : core::register_descriptor<std::uint32_t, 0xFF8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x5U, 0xFFFFFFFFU> {};
namespace cid2_fields {
using id = core::field_descriptor<cid2, 0U, 8U, core::access_semantics::read_only>;
}
struct cid3 : core::register_descriptor<std::uint32_t, 0xFFCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xB1U, 0xFFFFFFFFU> {};
namespace cid3_fields {
using id = core::field_descriptor<cid3, 0U, 8U, core::access_semantics::read_only>;
}
}
