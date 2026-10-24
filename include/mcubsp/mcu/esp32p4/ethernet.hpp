#pragma once

// @req-file REQ-0005 REQ-0007 REQ-0010 REQ-0011 REQ-0012 REQ-0013 REQ-0034 REQ-0043

#include <array>
#include <cstdint>

#include <mcubsp/mcu/esp32p4/interrupts_dma.hpp>

namespace mcubsp::mcu::esp32p4 {

inline constexpr core::source_ref ethernet_source{
    "ESP32-P4 TRM", "pre-release v0.7",
    "Chapter 55, PDF pages 3223-3303; RMII, SMI and channel-0 DMA"};

inline constexpr std::uintptr_t ethernet_base = 0x5009'8000U;
inline constexpr std::uintptr_t hp_system_base = 0x500E'5000U;

inline constexpr std::array ethernet_registers{
    register_record{"EMAC_MACCONFIGURATION_REG", ethernet_base + 0x0000U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"EMAC_MACFRAMEFILTER_REG", ethernet_base + 0x0004U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"EMAC_GMIIADDRESS_REG", ethernet_base + 0x0010U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_GMIIDATA_REG", ethernet_base + 0x0014U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_VERSION_REG", ethernet_base + 0x0020U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_MACADDRESS0HIGH_REG", ethernet_base + 0x0040U, 4, core::access_semantics::read_write, 0x8000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_MACADDRESS0LOW_REG", ethernet_base + 0x0044U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_BUSMODE_REG", ethernet_base + 0x1000U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 1U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"EMAC_TRANSMITPOLLDEMAND_REG", ethernet_base + 0x1004U, 4, core::access_semantics::command, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_RECEIVEPOLLDEMAND_REG", ethernet_base + 0x1008U, 4, core::access_semantics::command, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_RECEIVEDESCRIPTORLISTADDRESS_REG", ethernet_base + 0x100CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_TRANSMITDESCRIPTORLISTADDRESS_REG", ethernet_base + 0x1010U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_STATUS_REG", ethernet_base + 0x1014U, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
    register_record{"EMAC_OPERATIONMODE_REG", ethernet_base + 0x1018U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ethernet_source},
    register_record{"EMAC_INTERRUPTENABLE_REG", ethernet_base + 0x101CU, 4, core::access_semantics::read_write, 0x0001'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ethernet_source},
};

inline constexpr core::dma_constraints ethernet_dma_constraints{
    core::dma_engine_kind::esp_ethernet,
    static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral)
        | static_cast<std::uint8_t>(core::dma_direction::peripheral_to_memory),
    4U, 64U, 0x1FFFU};

} // namespace mcubsp::mcu::esp32p4
