#pragma once
// @req-file REQ-0055 REQ-0073
#include <array>
#include <string_view>
#include <mcubsp/mcu/nrf52840/core_registers/nvic.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/scb.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/scnscb.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/systick.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/itm.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/dwt.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/tpi.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/mpu.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/fpu.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/debug.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/fpb.hpp>
#include <mcubsp/mcu/nrf52840/core_registers/etm.hpp>
namespace mcubsp::mcu::nrf52840::core_registers {
struct block { std::string_view name; std::uintptr_t base; unsigned registers; };
inline constexpr std::array inventory{
    block{"NVIC", 0xE000E100U, 54U},
    block{"SCB", 0xE000ED00U, 34U},
    block{"SCnSCB", 0xE000E000U, 2U},
    block{"SysTick", 0xE000E010U, 4U},
    block{"ITM", 0xE0000000U, 145U},
    block{"DWT", 0xE0001000U, 32U},
    block{"TPI", 0xE0040000U, 29U},
    block{"MPU", 0xE000ED90U, 11U},
    block{"FPU", 0xE000EF30U, 5U},
    block{"CoreDebug", 0xE000EDF0U, 4U},
    block{"FPB", 0xE0002000U, 22U},
    block{"ETM", 0xE0041000U, 40U},
};
inline constexpr unsigned priority_bits = 3, external_irq_slots = 48, mpu_regions = 8;
inline constexpr unsigned fpb_instruction_comparators = 6, fpb_literal_comparators = 2, dwt_comparators = 4;
[[nodiscard]] constexpr bool core_register_selector(unsigned value) noexcept {
    return value <= 18 || value == 20 || value == 33 || (value >= 64 && value <= 95);
}
}
