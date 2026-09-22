#pragma once

#include <array>
#include <cstdint>

#include <mcuhw/mcu/ra4m1/model.hpp>
#include <mcuhw/mcu/ra4m1/system.hpp>

namespace mcuhw::mcu::ra4m1 {

inline constexpr core::source_ref icu_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "13.2.6-13.4, Tables 13.3-13.4, PDF pages 233-247"};

// Opaque Renesas IELS/DELS encodings, deliberately distinct from NVIC lines.
enum class event : std::uint16_t {
    dmac0_int = 0x011,
    dmac1_int = 0x012,
    dmac2_int = 0x013,
    dmac3_int = 0x014,
    dtc_complete = 0x015,
    adc_scan_end = 0x029,
    adc_group_b = 0x02A,
    iic0_rxi = 0x035,
    iic0_txi = 0x036,
    iic0_tei = 0x037,
    iic0_eei = 0x038,
    iic1_rxi = 0x03A,
    iic1_txi = 0x03B,
    iic1_tei = 0x03C,
    iic1_eei = 0x03D,
    gpt0_compare_a = 0x057,
    gpt0_compare_b = 0x058,
    gpt0_overflow = 0x05D,
    gpt0_underflow = 0x05E,
    gpt1_compare_a = 0x05F,
    gpt1_compare_b = 0x060,
    gpt1_overflow = 0x065,
    gpt1_underflow = 0x066,
    sci0_rxi = 0x098,
    sci0_txi = 0x099,
    sci0_tei = 0x09A,
    sci0_eri = 0x09B,
    sci1_rxi = 0x09E,
    sci1_txi = 0x09F,
    sci1_tei = 0x0A0,
    sci1_eri = 0x0A1,
    sci2_rxi = 0x0A3,
    sci2_txi = 0x0A4,
    sci2_tei = 0x0A5,
    sci2_eri = 0x0A6,
    sci9_rxi = 0x0A8,
    sci9_txi = 0x0A9,
    sci9_tei = 0x0AA,
    sci9_eri = 0x0AB,
    spi0_rxi = 0x0AD,
    spi0_txi = 0x0AE,
    spi0_idle = 0x0AF,
    spi0_error = 0x0B0,
    spi1_rxi = 0x0B2,
    spi1_txi = 0x0B3,
    spi1_idle = 0x0B4,
    spi1_error = 0x0B5,
};

constexpr std::uint16_t event_number(event value) noexcept {
    return static_cast<std::uint16_t>(value);
}

inline constexpr std::array interrupt_sources{
    event::dmac0_int, event::dmac1_int, event::dmac2_int, event::dmac3_int,
    event::dtc_complete, event::adc_scan_end, event::adc_group_b,
    event::iic0_rxi, event::iic0_txi, event::iic0_tei, event::iic0_eei,
    event::iic1_rxi, event::iic1_txi, event::iic1_tei, event::iic1_eei,
    event::gpt0_compare_a, event::gpt0_compare_b, event::gpt0_overflow,
    event::gpt0_underflow, event::gpt1_compare_a, event::gpt1_compare_b,
    event::gpt1_overflow, event::gpt1_underflow,
    event::sci0_rxi, event::sci0_txi, event::sci0_tei, event::sci0_eri,
    event::sci1_rxi, event::sci1_txi, event::sci1_tei, event::sci1_eri,
    event::sci2_rxi, event::sci2_txi, event::sci2_tei, event::sci2_eri,
    event::sci9_rxi, event::sci9_txi, event::sci9_tei, event::sci9_eri,
    event::spi0_rxi, event::spi0_txi, event::spi0_idle, event::spi0_error,
    event::spi1_rxi, event::spi1_txi, event::spi1_idle, event::spi1_error,
};

struct interrupt_database {
    [[nodiscard]] static constexpr bool event_exists(event requested) noexcept {
        for (const auto available : interrupt_sources) {
            if (available == requested) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] static constexpr bool valid_cpu_route(event requested,
                                                         std::uint16_t line) noexcept {
        return line < 32U && event_exists(requested);
    }
};

// One row demonstrates the configurable relation; valid_cpu_route expresses
// the complete 32-line domain without manufacturing 32 copies per event.
inline constexpr std::array interrupt_routes{
    core::interrupt_route{event_number(event::gpt0_compare_a), 0,
        core::interrupt_route_kind::selectable_line,
        event_number(event::gpt0_compare_a), true, icu_source},
    core::interrupt_route{event_number(event::iic0_rxi), 1,
        core::interrupt_route_kind::selectable_line,
        event_number(event::iic0_rxi), true, icu_source},
};

using ielsr0 = core::register_descriptor<std::uint32_t, 0x300,
    core::access_semantics::read_write, 0x0101'01FFU, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFFF'FFFFU>;
using delsr0 = core::register_descriptor<std::uint32_t, 0x280,
    core::access_semantics::read_write, 0x0000'00FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using iels = core::field_descriptor<ielsr0, 0, 9>;
using ir = core::field_descriptor<ielsr0, 16, 1>;
using dtce = core::field_descriptor<ielsr0, 24, 1>;
using dels = core::field_descriptor<delsr0, 0, 8>;

inline constexpr std::uintptr_t icu_base = 0x4000'6000U;

inline constexpr std::array icu_registers{
    register_record{"IELSR0", 0x4000'6300U, 4, core::access_semantics::read_write, 0x0101'01FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, icu_source},
    register_record{"IELSR31", 0x4000'637CU, 4, core::access_semantics::read_write, 0x0101'01FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, icu_source},
    register_record{"DELSR0", 0x4000'6280U, 4, core::access_semantics::read_write, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, icu_source},
    register_record{"DELSR3", 0x4000'628CU, 4, core::access_semantics::read_write, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, icu_source},
};

inline constexpr core::source_ref dmac_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "16.1-16.2 and 16.8, PDF pages 292-302 and 320"};
inline constexpr core::source_ref dtc_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "17.1-17.3 and 17.11, PDF pages 321-329 and 345"};

inline constexpr core::dma_constraints dmac_constraints{
    core::dma_engine_kind::ra_dmac,
    static_cast<std::uint8_t>(core::dma_direction::memory_to_memory)
        | static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral)
        | static_cast<std::uint8_t>(core::dma_direction::peripheral_to_memory),
    1U | 2U | 4U, 1U, 65'536U};
inline constexpr core::dma_constraints dtc_constraints{
    core::dma_engine_kind::ra_dtc,
    static_cast<std::uint8_t>(core::dma_direction::memory_to_memory)
        | static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral)
        | static_cast<std::uint8_t>(core::dma_direction::peripheral_to_memory),
    1U | 2U | 4U, 1U, 65'535U};

inline constexpr std::array dma_requests{
    core::dma_request{event_number(event::adc_scan_end), dmac_constraints, icu_source},
    core::dma_request{event_number(event::iic0_rxi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::iic0_txi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::gpt0_compare_a), dmac_constraints, icu_source},
    core::dma_request{event_number(event::sci0_rxi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::sci0_txi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::sci2_rxi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::sci2_txi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::spi0_rxi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::spi0_txi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::spi1_rxi), dmac_constraints, icu_source},
    core::dma_request{event_number(event::spi1_txi), dmac_constraints, icu_source},
};

inline constexpr std::array dma_reachability{
    core::dma_reachability{core::dma_engine_kind::ra_dmac, static_cast<std::uint16_t>(memory_id::code_flash), true, false},
    core::dma_reachability{core::dma_engine_kind::ra_dmac, static_cast<std::uint16_t>(memory_id::sram0), true, true},
    core::dma_reachability{core::dma_engine_kind::ra_dtc, static_cast<std::uint16_t>(memory_id::code_flash), true, false},
    core::dma_reachability{core::dma_engine_kind::ra_dtc, static_cast<std::uint16_t>(memory_id::sram0), true, true},
};

using dmsar = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using dmdar = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using dmtmd = core::register_descriptor<std::uint16_t, 0x10,
    core::access_semantics::read_write, 0xF303U, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFFU>;
using dmcrb = core::register_descriptor<std::uint16_t, 0x0C,
    core::access_semantics::read_write, 0xFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFFU>;
using dmcnt = core::register_descriptor<std::uint8_t, 0x1C,
    core::access_semantics::read_write, 0x01U, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFU>;
using dmreq = core::register_descriptor<std::uint8_t, 0x1D,
    core::access_semantics::read_write, 0x11U, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFU>;
using dmsts = core::register_descriptor<std::uint8_t, 0x1E,
    core::access_semantics::read_write, 0x10U, false,
    core::reserved_bit_policy::preserve, 0U, 0U, 0xFFU>;
using transfer_size = core::field_descriptor<dmtmd, 8, 2>;
using transfer_mode = core::field_descriptor<dmtmd, 14, 2>;

inline constexpr std::array dmac_registers{
    register_record{"DMSAR", 0x4000'5000U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMDAR", 0x4000'5004U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMCRA", 0x4000'5008U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMCRB", 0x4000'500CU, 2, core::access_semantics::read_write, 0xFFFFU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMTMD", 0x4000'5010U, 2, core::access_semantics::read_write, 0xF303U, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMINT", 0x4000'5013U, 1, core::access_semantics::read_write, 0x1FU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMAMD", 0x4000'5014U, 2, core::access_semantics::read_write, 0xDFDFU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMOFR", 0x4000'5018U, 4, core::access_semantics::read_write, 0x01FF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, dmac_source},
    register_record{"DMCNT", 0x4000'501CU, 1, core::access_semantics::read_write, 0x01U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMREQ", 0x4000'501DU, 1, core::access_semantics::read_write, 0x11U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, dmac_source},
    register_record{"DMSTS", 0x4000'501EU, 1, core::access_semantics::read_write, 0x10U, 0U, 0xFFU, core::reserved_bit_policy::preserve, dmac_source},
    register_record{"DMAST", 0x4000'5200U, 1, core::access_semantics::read_write, 0x01U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, dmac_source},
};

// DTC transfer data are six packed SRAM fields, not memory-mapped peripheral
// registers. This representation intentionally cannot be used as an MMIO block.
// @req REQ-0005 REQ-0007 REQ-0013
struct dtc_transfer_data {
    std::uint8_t mode_a;
    std::uint8_t mode_b;
    std::uint32_t source;
    std::uint32_t destination;
    std::uint16_t count_b;
    std::uint16_t count_a;
};

static_assert(sizeof(dtc_transfer_data) == 16U);

} // namespace mcuhw::mcu::ra4m1
