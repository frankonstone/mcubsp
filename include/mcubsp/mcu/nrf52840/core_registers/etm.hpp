#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::etm {
inline constexpr std::uintptr_t base = 0xE0041000U;
struct etmcr : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_write, 0x10232FF1U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x411U, 0xFFFFFFFFU> {};
namespace etmcr_fields {
using timestamp_enable = core::field_descriptor<etmcr, 28U, 1U, core::access_semantics::read_write>;
using port_size_high = core::field_descriptor<etmcr, 21U, 1U, core::access_semantics::read_write>;
using port_mode_low = core::field_descriptor<etmcr, 16U, 2U, core::access_semantics::read_write>;
using port_mode_high = core::field_descriptor<etmcr, 13U, 1U, core::access_semantics::read_write>;
using port_select = core::field_descriptor<etmcr, 11U, 1U, core::access_semantics::read_write>;
using programming = core::field_descriptor<etmcr, 10U, 1U, core::access_semantics::read_write>;
using debug_request = core::field_descriptor<etmcr, 9U, 1U, core::access_semantics::read_write>;
using branch_output = core::field_descriptor<etmcr, 8U, 1U, core::access_semantics::read_write>;
using stall = core::field_descriptor<etmcr, 7U, 1U, core::access_semantics::read_write>;
using port_size_low = core::field_descriptor<etmcr, 4U, 3U, core::access_semantics::read_write>;
using powerdown = core::field_descriptor<etmcr, 0U, 1U, core::access_semantics::read_write>;
}
struct etmccr : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x8C802000U, 0xFFF1FFFFU> {};
namespace etmccr_fields {
using id_present = core::field_descriptor<etmccr, 31U, 1U, core::access_semantics::read_only>;
using memory_access = core::field_descriptor<etmccr, 27U, 1U, core::access_semantics::read_only>;
using start_stop_present = core::field_descriptor<etmccr, 26U, 1U, core::access_semantics::read_only>;
using context_comparators = core::field_descriptor<etmccr, 24U, 2U, core::access_semantics::read_only>;
using fifo_full_present = core::field_descriptor<etmccr, 23U, 1U, core::access_semantics::read_only>;
using external_outputs = core::field_descriptor<etmccr, 20U, 3U, core::access_semantics::read_only>;
using external_inputs = core::field_descriptor<etmccr, 17U, 3U, core::access_semantics::read_only>;
using sequencer = core::field_descriptor<etmccr, 16U, 1U, core::access_semantics::read_only>;
using counters = core::field_descriptor<etmccr, 13U, 3U, core::access_semantics::read_only>;
using memory_decoders = core::field_descriptor<etmccr, 8U, 5U, core::access_semantics::read_only>;
using data_comparators = core::field_descriptor<etmccr, 4U, 4U, core::access_semantics::read_only>;
using address_pairs = core::field_descriptor<etmccr, 0U, 4U, core::access_semantics::read_only>;
}
struct etmtrigger : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::read_write, 0x1FFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmtrigger_fields {
using function = core::field_descriptor<etmtrigger, 14U, 3U, core::access_semantics::read_write>;
using resource_b = core::field_descriptor<etmtrigger, 7U, 7U, core::access_semantics::read_write>;
using resource_a = core::field_descriptor<etmtrigger, 0U, 7U, core::access_semantics::read_write>;
}
struct etmsr : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::read_write, 0xCU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmsr_fields {
using trigger = core::field_descriptor<etmsr, 3U, 1U, core::access_semantics::read_write>;
using start_stop = core::field_descriptor<etmsr, 2U, 1U, core::access_semantics::read_write>;
using programming = core::field_descriptor<etmsr, 1U, 1U, core::access_semantics::read_only>;
using overflow = core::field_descriptor<etmsr, 0U, 1U, core::access_semantics::read_only>;
}
struct etmscr : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x20D09U, 0xFFFFFFFFU> {};
namespace etmscr_fields {
using no_fetch = core::field_descriptor<etmscr, 17U, 1U, core::access_semantics::read_only>;
using processors_minus_one = core::field_descriptor<etmscr, 12U, 3U, core::access_semantics::read_only>;
using port_mode_supported = core::field_descriptor<etmscr, 11U, 1U, core::access_semantics::read_only>;
using port_size_supported = core::field_descriptor<etmscr, 10U, 1U, core::access_semantics::read_only>;
using max_port_high = core::field_descriptor<etmscr, 9U, 1U, core::access_semantics::read_only>;
using fifo_full_supported = core::field_descriptor<etmscr, 8U, 1U, core::access_semantics::read_only>;
using max_port_low = core::field_descriptor<etmscr, 0U, 3U, core::access_semantics::read_only>;
}
struct etmteevr : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::read_write, 0x1FFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmteevr_fields {
using function = core::field_descriptor<etmteevr, 14U, 3U, core::access_semantics::read_write>;
using resource_b = core::field_descriptor<etmteevr, 7U, 7U, core::access_semantics::read_write>;
using resource_a = core::field_descriptor<etmteevr, 0U, 7U, core::access_semantics::read_write>;
}
struct etmtecr1 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_write, 0x2000000U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmtecr1_fields {
using start_stop_enable = core::field_descriptor<etmtecr1, 25U, 1U, core::access_semantics::read_write>;
}
struct etmfflr : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmfflr_fields {
using level = core::field_descriptor<etmfflr, 0U, 8U, core::access_semantics::read_write>;
}
struct etmcntrldvr1 : core::register_descriptor<std::uint32_t, 0x140U,
    core::access_semantics::read_write, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmcntrldvr1_fields {
using reload = core::field_descriptor<etmcntrldvr1, 0U, 16U, core::access_semantics::read_write>;
}
struct etmsyncfr : core::register_descriptor<std::uint32_t, 0x1E0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x400U, 0xFFFFFFFFU> {};
namespace etmsyncfr_fields {
using frequency = core::field_descriptor<etmsyncfr, 0U, 12U, core::access_semantics::read_only>;
}
struct etmidr : core::register_descriptor<std::uint32_t, 0x1E4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x4114F250U, 0xFFFFFFFFU> {};
namespace etmidr_fields {
using implementer = core::field_descriptor<etmidr, 24U, 8U, core::access_semantics::read_only>;
using branch_encoding = core::field_descriptor<etmidr, 20U, 1U, core::access_semantics::read_only>;
using security_extensions = core::field_descriptor<etmidr, 19U, 1U, core::access_semantics::read_only>;
using thumb32 = core::field_descriptor<etmidr, 18U, 1U, core::access_semantics::read_only>;
using load_pc_first = core::field_descriptor<etmidr, 16U, 1U, core::access_semantics::read_only>;
using processor_family = core::field_descriptor<etmidr, 12U, 4U, core::access_semantics::read_only>;
using major = core::field_descriptor<etmidr, 8U, 4U, core::access_semantics::read_only>;
using minor = core::field_descriptor<etmidr, 4U, 4U, core::access_semantics::read_only>;
using revision = core::field_descriptor<etmidr, 0U, 4U, core::access_semantics::read_only>;
}
struct etmccer : core::register_descriptor<std::uint32_t, 0x1E8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x18541800U, 0xFFFFFFFFU> {};
namespace etmccer_fields {
using timestamp_size = core::field_descriptor<etmccer, 29U, 1U, core::access_semantics::read_only>;
using timestamp_binary = core::field_descriptor<etmccer, 28U, 1U, core::access_semantics::read_only>;
using reduced_counter = core::field_descriptor<etmccer, 27U, 1U, core::access_semantics::read_only>;
using timestamp_implemented = core::field_descriptor<etmccer, 22U, 1U, core::access_semantics::read_only>;
using embedded_ice_behavior = core::field_descriptor<etmccer, 21U, 1U, core::access_semantics::read_only>;
using start_stop_embedded_ice = core::field_descriptor<etmccer, 20U, 1U, core::access_semantics::read_only>;
using watchpoint_inputs = core::field_descriptor<etmccer, 16U, 4U, core::access_semantics::read_only>;
using instrumentation_resources = core::field_descriptor<etmccer, 13U, 3U, core::access_semantics::read_only>;
using no_data_address_compare = core::field_descriptor<etmccer, 12U, 1U, core::access_semantics::read_only>;
using readable_registers = core::field_descriptor<etmccer, 11U, 1U, core::access_semantics::read_only>;
using extended_input_bus = core::field_descriptor<etmccer, 3U, 8U, core::access_semantics::read_only>;
using extended_selectors = core::field_descriptor<etmccer, 0U, 3U, core::access_semantics::read_only>;
}
struct etmtesseicr : core::register_descriptor<std::uint32_t, 0x1F0U,
    core::access_semantics::read_write, 0xF000FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmtesseicr_fields {
using stop = core::field_descriptor<etmtesseicr, 16U, 4U, core::access_semantics::read_write>;
using start = core::field_descriptor<etmtesseicr, 0U, 4U, core::access_semantics::read_write>;
}
struct etmtsevr : core::register_descriptor<std::uint32_t, 0x1F8U,
    core::access_semantics::read_write, 0x1FFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmtsevr_fields {
using function = core::field_descriptor<etmtsevr, 14U, 3U, core::access_semantics::read_write>;
using resource_b = core::field_descriptor<etmtsevr, 7U, 7U, core::access_semantics::read_write>;
using resource_a = core::field_descriptor<etmtsevr, 0U, 7U, core::access_semantics::read_write>;
}
struct etmtraceidr : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x7FU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmtraceidr_fields {
using trace_id = core::field_descriptor<etmtraceidr, 0U, 7U, core::access_semantics::read_write>;
}
struct etmidr2 : core::register_descriptor<std::uint32_t, 0x208U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmidr2_fields {
using swp_order = core::field_descriptor<etmidr2, 1U, 1U, core::access_semantics::read_only>;
using rfe_order = core::field_descriptor<etmidr2, 0U, 1U, core::access_semantics::read_only>;
}
struct etmpdsr : core::register_descriptor<std::uint32_t, 0x314U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x1U, 0xFFFFFFFFU> {};
namespace etmpdsr_fields {
using powered_up = core::field_descriptor<etmpdsr, 0U, 1U, core::access_semantics::read_only>;
}
struct itmiscin : core::register_descriptor<std::uint32_t, 0xEE0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace itmiscin_fields {
using corehalt = core::field_descriptor<itmiscin, 4U, 1U, core::access_semantics::read_only>;
using extin = core::field_descriptor<itmiscin, 0U, 2U, core::access_semantics::read_only>;
}
struct ittrigout : core::register_descriptor<std::uint32_t, 0xEE8U,
    core::access_semantics::write_only, 0x1U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ittrigout_fields {
using trigger = core::field_descriptor<ittrigout, 0U, 1U, core::access_semantics::write_only>;
}
struct etm_itatbctr2 : core::register_descriptor<std::uint32_t, 0xEF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etm_itatbctr2_fields {
using atready = core::field_descriptor<etm_itatbctr2, 0U, 1U, core::access_semantics::read_only>;
}
struct etm_itatbctr0 : core::register_descriptor<std::uint32_t, 0xEF8U,
    core::access_semantics::write_only, 0x1U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etm_itatbctr0_fields {
using atvalid = core::field_descriptor<etm_itatbctr0, 0U, 1U, core::access_semantics::write_only>;
}
struct etmitctrl : core::register_descriptor<std::uint32_t, 0xF00U,
    core::access_semantics::read_write, 0x1U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmitctrl_fields {
using integration = core::field_descriptor<etmitctrl, 0U, 1U, core::access_semantics::read_write>;
}
struct etmclaimset : core::register_descriptor<std::uint32_t, 0xFA0U,
    core::access_semantics::set_alias, 0xFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using etmclaimset_status = core::register_descriptor<std::uint32_t, 0xFA0U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, etmclaimset>;
namespace etmclaimset_fields {
using tag = core::field_descriptor<etmclaimset, 0U, 4U, core::access_semantics::set_alias>;
}
struct etmclaimclr : core::register_descriptor<std::uint32_t, 0xFA4U,
    core::access_semantics::clear_alias, 0xFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using etmclaimclr_status = core::register_descriptor<std::uint32_t, 0xFA4U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, etmclaimclr>;
namespace etmclaimclr_fields {
using tag = core::field_descriptor<etmclaimclr, 0U, 4U, core::access_semantics::clear_alias>;
}
struct etmlar : core::register_descriptor<std::uint32_t, 0xFB0U,
    core::access_semantics::command, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmlar_fields {
using key = core::field_descriptor<etmlar, 0U, 32U, core::access_semantics::command>;
}
struct etmlsr : core::register_descriptor<std::uint32_t, 0xFB4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmlsr_fields {
using access_8bit = core::field_descriptor<etmlsr, 2U, 1U, core::access_semantics::read_only>;
using locked = core::field_descriptor<etmlsr, 1U, 1U, core::access_semantics::read_only>;
using present = core::field_descriptor<etmlsr, 0U, 1U, core::access_semantics::read_only>;
}
struct etmauthstatus : core::register_descriptor<std::uint32_t, 0xFB8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace etmauthstatus_fields {
using secure_noninvasive = core::field_descriptor<etmauthstatus, 6U, 2U, core::access_semantics::read_only>;
using nonsecure_noninvasive = core::field_descriptor<etmauthstatus, 2U, 2U, core::access_semantics::read_only>;
}
struct etmdevtype : core::register_descriptor<std::uint32_t, 0xFCCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x13U, 0xFFFFFFFFU> {};
namespace etmdevtype_fields {
using subtype = core::field_descriptor<etmdevtype, 4U, 4U, core::access_semantics::read_only>;
using major = core::field_descriptor<etmdevtype, 0U, 4U, core::access_semantics::read_only>;
}
struct etmpidr4 : core::register_descriptor<std::uint32_t, 0xFD0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x4U, 0xFFFFFFFFU> {};
namespace etmpidr4_fields {
using id = core::field_descriptor<etmpidr4, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr5 : core::register_descriptor<std::uint32_t, 0xFD4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmpidr5_fields {
using id = core::field_descriptor<etmpidr5, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr6 : core::register_descriptor<std::uint32_t, 0xFD8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmpidr6_fields {
using id = core::field_descriptor<etmpidr6, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr7 : core::register_descriptor<std::uint32_t, 0xFDCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmpidr7_fields {
using id = core::field_descriptor<etmpidr7, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr0 : core::register_descriptor<std::uint32_t, 0xFE0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x25U, 0xFFFFFFFFU> {};
namespace etmpidr0_fields {
using id = core::field_descriptor<etmpidr0, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr1 : core::register_descriptor<std::uint32_t, 0xFE4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xB9U, 0xFFFFFFFFU> {};
namespace etmpidr1_fields {
using id = core::field_descriptor<etmpidr1, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr2 : core::register_descriptor<std::uint32_t, 0xFE8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xBU, 0xFFFFFFFFU> {};
namespace etmpidr2_fields {
using id = core::field_descriptor<etmpidr2, 0U, 8U, core::access_semantics::read_only>;
}
struct etmpidr3 : core::register_descriptor<std::uint32_t, 0xFECU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0xFFFFFFFFU> {};
namespace etmpidr3_fields {
using id = core::field_descriptor<etmpidr3, 0U, 8U, core::access_semantics::read_only>;
}
struct etmcidr0 : core::register_descriptor<std::uint32_t, 0xFF0U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xDU, 0xFFFFFFFFU> {};
namespace etmcidr0_fields {
using id = core::field_descriptor<etmcidr0, 0U, 8U, core::access_semantics::read_only>;
}
struct etmcidr1 : core::register_descriptor<std::uint32_t, 0xFF4U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x90U, 0xFFFFFFFFU> {};
namespace etmcidr1_fields {
using id = core::field_descriptor<etmcidr1, 0U, 8U, core::access_semantics::read_only>;
}
struct etmcidr2 : core::register_descriptor<std::uint32_t, 0xFF8U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x5U, 0xFFFFFFFFU> {};
namespace etmcidr2_fields {
using id = core::field_descriptor<etmcidr2, 0U, 8U, core::access_semantics::read_only>;
}
struct etmcidr3 : core::register_descriptor<std::uint32_t, 0xFFCU,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0xB1U, 0xFFFFFFFFU> {};
namespace etmcidr3_fields {
using id = core::field_descriptor<etmcidr3, 0U, 8U, core::access_semantics::read_only>;
}
}
