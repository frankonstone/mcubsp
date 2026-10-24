#pragma once

// @req-file REQ-0005 REQ-0007 REQ-0010 REQ-0011 REQ-0012 REQ-0013 REQ-0033 REQ-0043

#include <array>
#include <cstdint>

#include <mcubsp/mcu/stm32h753/pins.hpp>
#include <mcubsp/mcu/stm32h753/system.hpp>

namespace mcubsp::mcu::stm32h753 {

inline constexpr core::source_ref ethernet_source{
    "RM0433 / DS12117", "Rev.8 / Rev.10",
    "RM0433 Chapter 57, PDF pages 2774-2991; DS12117 Table 9 AF11"};

inline constexpr std::uintptr_t ethernet_base = 0x4002'8000U;
inline constexpr std::uintptr_t syscfg_base = 0x5800'0400U;

// Offsets used by the allocation-free RMII/MDIO and channel-0 DMA slice.
inline constexpr std::array ethernet_registers{
    register_record{"ETH_MACCR", ethernet_base + 0x0000U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_MACPFR", ethernet_base + 0x0008U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_MACMDIOAR", ethernet_base + 0x0200U, 4, core::access_semantics::read_write, 0x03FF'FF1DU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_MACMDIODR", ethernet_base + 0x0204U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_MACA0HR", ethernet_base + 0x0300U, 4, core::access_semantics::read_write, 0x8000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_MACA0LR", ethernet_base + 0x0304U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_MTLTQOMR", ethernet_base + 0x0D00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_MTLRQOMR", ethernet_base + 0x0D30U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_DMAMR", ethernet_base + 0x1000U, 4, core::access_semantics::read_write, 0x0003'0001U, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_DMACCR", ethernet_base + 0x1100U, 4, core::access_semantics::read_write, 0x001D'3FFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_DMACTCR", ethernet_base + 0x1104U, 4, core::access_semantics::read_write, 0x003F'1011U, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_DMACRCR", ethernet_base + 0x1108U, 4, core::access_semantics::read_write, 0x803F'7FFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"ETH_DMACTDLAR", ethernet_base + 0x1114U, 4, core::access_semantics::read_write, 0xFFFF'FFFCU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_DMACRDLAR", ethernet_base + 0x111CU, 4, core::access_semantics::read_write, 0xFFFF'FFFCU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_DMACTDTPR", ethernet_base + 0x1120U, 4, core::access_semantics::read_write, 0xFFFF'FFFCU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_DMACRDTPR", ethernet_base + 0x1128U, 4, core::access_semantics::read_write, 0xFFFF'FFFCU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_DMACTDRLR", ethernet_base + 0x112CU, 4, core::access_semantics::read_write, 0x0000'03FFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_DMACRDRLR", ethernet_base + 0x1130U, 4, core::access_semantics::read_write, 0x0000'03FFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"ETH_DMACSR", ethernet_base + 0x1160U, 4, core::access_semantics::write_one_to_clear, 0x003F'DFC7U, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
};

inline constexpr core::dma_constraints ethernet_dma_constraints{
    core::dma_engine_kind::stm32_ethernet,
    static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral)
        | static_cast<std::uint8_t>(core::dma_direction::peripheral_to_memory),
    4U, 4U, 0x3FFFU};

} // namespace mcubsp::mcu::stm32h753
