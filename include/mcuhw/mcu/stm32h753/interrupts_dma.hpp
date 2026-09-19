#pragma once

#include <array>
#include <cstdint>

#include <mcuhw/mcu/stm32h753/system.hpp>

namespace mcuhw::mcu::stm32h753 {

inline constexpr core::source_ref nvic_source{
    "RM0433", "Rev.8", "Chapter 19, Table 145, PDF pages 822-829"};

// Peripheral event identities are deliberately not NVIC line numbers. Several
// timer events share one fixed NVIC line on this part.
enum class event : std::uint16_t {
    dma1_stream0 = 0x100, dma1_stream1, dma1_stream2, dma1_stream3,
    dma1_stream4, dma1_stream5, dma1_stream6,
    adc12,
    tim1_break, tim1_update, tim1_trigger_commutation, tim1_capture_compare,
    tim2, tim3, tim4,
    i2c1_event, i2c1_error, spi1, usart1,
    tim8_break, tim12, tim8_update, tim13, tim8_trigger_commutation, tim14,
    tim8_capture_compare,
    tim5,
    dma2_stream0, dma2_stream1, dma2_stream2, dma2_stream3,
    dma2_stream4, dma2_stream5, dma2_stream6, dma2_stream7,
};

[[nodiscard]] constexpr std::uint16_t event_number(event value) noexcept {
    return static_cast<std::uint16_t>(value);
}

constexpr core::interrupt_route fixed_irq(event source, std::uint16_t line,
                                          bool exclusive = true) noexcept {
    return {event_number(source), line, core::interrupt_route_kind::fixed_line,
            line, exclusive, nvic_source};
}

inline constexpr std::array interrupt_routes{
    fixed_irq(event::dma1_stream0, 11), fixed_irq(event::dma1_stream1, 12),
    fixed_irq(event::dma1_stream2, 13), fixed_irq(event::dma1_stream3, 14),
    fixed_irq(event::dma1_stream4, 15), fixed_irq(event::dma1_stream5, 16),
    fixed_irq(event::dma1_stream6, 17), fixed_irq(event::adc12, 18),
    fixed_irq(event::tim1_break, 24), fixed_irq(event::tim1_update, 25),
    fixed_irq(event::tim1_trigger_commutation, 26),
    fixed_irq(event::tim1_capture_compare, 27), fixed_irq(event::tim2, 28),
    fixed_irq(event::tim3, 29), fixed_irq(event::tim4, 30),
    fixed_irq(event::i2c1_event, 31), fixed_irq(event::i2c1_error, 32),
    fixed_irq(event::spi1, 35), fixed_irq(event::usart1, 37),
    fixed_irq(event::tim8_break, 43, false), fixed_irq(event::tim12, 43, false),
    fixed_irq(event::tim8_update, 44, false), fixed_irq(event::tim13, 44, false),
    fixed_irq(event::tim8_trigger_commutation, 45, false),
    fixed_irq(event::tim14, 45, false),
    fixed_irq(event::tim8_capture_compare, 46), fixed_irq(event::tim5, 50),
    fixed_irq(event::dma2_stream0, 56), fixed_irq(event::dma2_stream1, 57),
    fixed_irq(event::dma2_stream2, 58), fixed_irq(event::dma2_stream3, 59),
    fixed_irq(event::dma2_stream4, 60), fixed_irq(event::dma2_stream5, 68),
    fixed_irq(event::dma2_stream6, 69), fixed_irq(event::dma2_stream7, 70),
};

struct interrupt_database {
    [[nodiscard]] static constexpr bool valid_cpu_route(
        event source, std::uint16_t line) noexcept {
        return core::interrupt_route_exists(interrupt_routes,
                                             event_number(source), line);
    }
};

inline constexpr core::source_ref dma_source{
    "RM0433", "Rev.8",
    "Chapters 14-18; Tables 122-123, PDF pages 634-786"};
inline constexpr core::source_ref dma_errata_source{
    "ES0392", "Rev.15", "Sections 2.4 and 2.5, PDF pages 19-22"};

inline constexpr auto all_dma_directions = static_cast<std::uint8_t>(
    static_cast<std::uint8_t>(core::dma_direction::memory_to_memory)
    | static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral)
    | static_cast<std::uint8_t>(core::dma_direction::peripheral_to_memory));

inline constexpr core::dma_constraints dma12_constraints{
    core::dma_engine_kind::stm32_dma, all_dma_directions, 1U | 2U | 4U,
    1U, 65'535U};
inline constexpr core::dma_constraints bdma_constraints{
    core::dma_engine_kind::stm32_bdma, all_dma_directions, 1U | 2U | 4U,
    1U, 65'535U};
inline constexpr core::dma_constraints mdma_constraints{
    core::dma_engine_kind::stm32_mdma, all_dma_directions, 1U | 2U | 4U,
    1U, 65'535U};

// DMAMUX request encodings are engine-local vendor values, not interrupt IDs.
inline constexpr std::array dma_requests{
    core::dma_request{9, dma12_constraints, dma_source},
    core::dma_request{10, dma12_constraints, dma_source},
    core::dma_request{11, dma12_constraints, dma_source},
    core::dma_request{12, dma12_constraints, dma_source},
    core::dma_request{13, dma12_constraints, dma_source},
    core::dma_request{14, dma12_constraints, dma_source},
    core::dma_request{15, dma12_constraints, dma_source},
    core::dma_request{16, dma12_constraints, dma_source},
    core::dma_request{17, dma12_constraints, dma_source},
    core::dma_request{18, dma12_constraints, dma_source},
    core::dma_request{19, dma12_constraints, dma_source},
    core::dma_request{20, dma12_constraints, dma_source},
    core::dma_request{21, dma12_constraints, dma_source},
    core::dma_request{22, dma12_constraints, dma_source},
    core::dma_request{33, dma12_constraints, dma_source},
    core::dma_request{34, dma12_constraints, dma_source},
    core::dma_request{37, dma12_constraints, dma_source},
    core::dma_request{38, dma12_constraints, dma_source},
    core::dma_request{41, dma12_constraints, dma_source},
    core::dma_request{42, dma12_constraints, dma_source},
    core::dma_request{115, dma12_constraints, dma_source},
};

// RM0433 narrative reachability is used instead of the known-bad Table 3
// interconnect matrix. DMA1/2 cannot access either TCM; BDMA is confined to D3;
// MDMA can reach all listed internal memories through its AHB slave port.
inline constexpr std::array dma_reachability{
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 0, true, false},
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 1, true, false},
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 4, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 5, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 6, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 7, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_dma, 8, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_bdma, 8, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_bdma, 9, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 0, true, false},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 1, true, false},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 2, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 3, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 4, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 5, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 6, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 7, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 8, true, true},
    core::dma_reachability{core::dma_engine_kind::stm32_mdma, 9, true, true},
};

inline constexpr std::uintptr_t dma1_base = 0x4002'0000U;
inline constexpr std::uintptr_t dma2_base = 0x4002'0400U;
inline constexpr std::uintptr_t dmamux1_base = 0x4002'0800U;

using dma_lisr = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using dma_lifcr = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::write_one_to_clear, 0x0F7D'0F7DU, false,
    core::reserved_bit_policy::write_zero>;
using dma_s0cr = core::register_descriptor<std::uint32_t, 0x10,
    core::access_semantics::read_write, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using dma_s0ndtr = core::register_descriptor<std::uint32_t, 0x14,
    core::access_semantics::read_write, 0x0000'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using dmamux_c0cr = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x07FF'037FU, false,
    core::reserved_bit_policy::write_zero>;
using dmamux_rgcfr = core::register_descriptor<std::uint32_t, 0x144,
    core::access_semantics::write_only, 0x0000'00FFU, false,
    core::reserved_bit_policy::write_zero>;
using dma_stream_enable = core::field_descriptor<dma_s0cr, 0, 1>;
using dmamux_request_id = core::field_descriptor<dmamux_c0cr, 0, 7>;

inline constexpr std::array dma_registers{
    register_record{"DMA1_LISR", dma1_base + 0x00U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_HISR", dma1_base + 0x04U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_LIFCR", dma1_base + 0x08U, 4, core::access_semantics::write_one_to_clear, 0x0F7D'0F7DU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_HIFCR", dma1_base + 0x0CU, 4, core::access_semantics::write_one_to_clear, 0x0F7D'0F7DU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_S0CR", dma1_base + 0x10U, 4, core::access_semantics::read_write, 0x01FF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, dma_source},
    register_record{"DMA1_S0NDTR", dma1_base + 0x14U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_S0PAR", dma1_base + 0x18U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_S0M0AR", dma1_base + 0x1CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_S0M1AR", dma1_base + 0x20U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMA1_S0FCR", dma1_base + 0x24U, 4, core::access_semantics::read_write, 0x0000'0087U, 0x21U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, dma_source},
    register_record{"DMAMUX1_C0CR", dmamux1_base + 0x00U, 4, core::access_semantics::read_write, 0x07FF'037FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMAMUX1_CSR", dmamux1_base + 0x80U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMAMUX1_CFR", dmamux1_base + 0x84U, 4, core::access_semantics::write_one_to_clear, 0x0000'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMAMUX1_RG0CR", dmamux1_base + 0x100U, 4, core::access_semantics::read_write, 0x00FF'0107U, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMAMUX1_RGSR", dmamux1_base + 0x140U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, dma_source},
    register_record{"DMAMUX1_RGCFR", dmamux1_base + 0x144U, 4, core::access_semantics::write_only, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, dma_errata_source},
};

} // namespace mcuhw::mcu::stm32h753
