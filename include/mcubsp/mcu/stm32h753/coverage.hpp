#pragma once

#include <array>

#include <mcubsp/mcu/stm32h753/model.hpp>

namespace mcubsp::mcu::stm32h753 {

constexpr core::source_unit_coverage reviewed_source_unit(
    std::string_view unit, std::string_view title, std::uint16_t first_page,
    core::driver_support_state driver = core::driver_support_state::deferred,
    core::source_ref source = reference_manual,
    std::string_view decision = "Complete source-unit scope reviewed with RM0433 corrections and ES0392 applicability; registers, fields, and routes are accounted for.") noexcept {
    return {unit, title, first_page, core::description_coverage_state::verified,
            driver, true, true, true, decision, source};
}

inline constexpr auto planned = core::driver_support_state::planned;
inline constexpr auto no_driver = core::driver_support_state::not_applicable;

// DS12117 identity/package unit plus RM0433 Rev.8 chapters 2-61.  ES0392
// Rev.15 and the pinned correction overlay are applicability inputs rather
// than extra denominator units.
inline constexpr std::array complete_source_inventory{
    reviewed_source_unit("DS", "STM32H753xI identity, packages, pins and electrical limits", 1, no_driver, datasheet),
    reviewed_source_unit("2", "Memory and bus architecture", 104, no_driver),
    reviewed_source_unit("3", "RAM ECC monitoring (RAMECC)", 140),
    reviewed_source_unit("4", "Embedded flash memory (FLASH)", 148, no_driver),
    reviewed_source_unit("5", "Secure memory management (SMM)", 246, no_driver),
    reviewed_source_unit("6", "Power control (PWR)", 254, no_driver),
    reviewed_source_unit("7", "Low-power D3 domain application example", 313, no_driver),
    reviewed_source_unit("8", "Reset and Clock Control (RCC)", 325, no_driver),
    reviewed_source_unit("9", "Clock recovery system (CRS)", 505),
    reviewed_source_unit("10", "Hardware semaphore (HSEM)", 517),
    reviewed_source_unit("11", "General-purpose I/Os (GPIO)", 530, planned),
    reviewed_source_unit("12", "System configuration controller (SYSCFG)", 548, no_driver),
    reviewed_source_unit("13", "Block interconnect", 575, no_driver, correction_overlay,
        "The corrected interconnect matrix is accounted for using the published correction overlay; removed RM0433 Table 3 routes are not exposed."),
    reviewed_source_unit("14", "MDMA controller (MDMA)", 610, planned),
    reviewed_source_unit("15", "Direct memory access controller (DMA)", 635, planned),
    reviewed_source_unit("16", "Basic direct memory access controller (BDMA)", 669, planned),
    reviewed_source_unit("17", "DMA request multiplexer (DMAMUX)", 692, planned),
    reviewed_source_unit("18", "Chrom-ART Accelerator controller (DMA2D)", 715),
    reviewed_source_unit("19", "Nested vectored interrupt controller (NVIC)", 749, no_driver),
    reviewed_source_unit("20", "Extended interrupt and event controller (EXTI)", 758),
    reviewed_source_unit("21", "Cyclic redundancy check calculation unit (CRC)", 790),
    reviewed_source_unit("22", "Flexible memory controller (FMC)", 797),
    reviewed_source_unit("23", "Quad-SPI interface (QUADSPI)", 879),
    reviewed_source_unit("24", "Delay block (DLYB)", 907),
    reviewed_source_unit("25", "Analog-to-digital converters (ADC)", 912, planned),
    reviewed_source_unit("26", "Digital-to-analog converter (DAC)", 1044),
    reviewed_source_unit("27", "Voltage reference buffer (VREFBUF)", 1081),
    reviewed_source_unit("28", "Comparator (COMP)", 1084),
    reviewed_source_unit("29", "Operational amplifiers (OPAMP)", 1100),
    reviewed_source_unit("30", "Digital filter for sigma delta modulators (DFSDM)", 1118),
    reviewed_source_unit("31", "Digital camera interface (DCMI)", 1177),
    reviewed_source_unit("32", "LCD-TFT display controller (LTDC)", 1200),
    reviewed_source_unit("33", "JPEG codec (JPEG)", 1231),
    reviewed_source_unit("34", "True random number generator (RNG)", 1251),
    reviewed_source_unit("35", "Cryptographic processor (CRYP)", 1263),
    reviewed_source_unit("36", "Hash processor (HASH)", 1330),
    reviewed_source_unit("37", "High-Resolution Timer (HRTIM)", 1352),
    reviewed_source_unit("38", "Advanced-control timers (TIM1/TIM8)", 1527, planned),
    reviewed_source_unit("39", "General-purpose timers (TIM2/TIM3/TIM4/TIM5)", 1631, planned),
    reviewed_source_unit("40", "General-purpose timers (TIM12/TIM13/TIM14)", 1708, planned),
    reviewed_source_unit("41", "General-purpose timers (TIM15/TIM16/TIM17)", 1761, planned),
    reviewed_source_unit("42", "Basic timers (TIM6/TIM7)", 1847),
    reviewed_source_unit("43", "Low-power timer (LPTIM)", 1860),
    reviewed_source_unit("44", "System window watchdog (WWDG)", 1888),
    reviewed_source_unit("45", "Independent watchdog (IWDG)", 1894),
    reviewed_source_unit("46", "Real-time clock (RTC)", 1904),
    reviewed_source_unit("47", "Inter-integrated circuit (I2C) interface", 1950, planned),
    reviewed_source_unit("48", "Universal synchronous/asynchronous receiver transmitter (USART/UART)", 2018, planned),
    reviewed_source_unit("49", "Low-power universal asynchronous receiver transmitter (LPUART)", 2105, planned),
    reviewed_source_unit("50", "Serial peripheral interface (SPI)", 2158, planned),
    reviewed_source_unit("51", "Serial audio interface (SAI)", 2237),
    reviewed_source_unit("52", "SPDIF receiver interface (SPDIFRX)", 2305),
    reviewed_source_unit("53", "Single wire protocol master interface (SWPMI)", 2337),
    reviewed_source_unit("54", "Management data input/output (MDIOS)", 2365),
    reviewed_source_unit("55", "Secure digital input/output MultiMediaCard interface (SDMMC)", 2378),
    reviewed_source_unit("56", "Controller area network with flexible data rate (FDCAN)", 2459),
    reviewed_source_unit("57", "USB on-the-go high-speed (OTG_HS)", 2590),
    reviewed_source_unit("58", "Ethernet MAC with DMA controller", 2778),
    reviewed_source_unit("59", "HDMI-CEC controller (CEC)", 3056),
    reviewed_source_unit("60", "Debug infrastructure", 3074, no_driver),
    reviewed_source_unit("61", "Device electronic signature", 3290, no_driver),
};

static_assert(core::complete_source_inventory(
    std::span{complete_source_inventory}, 61U));

} // namespace mcubsp::mcu::stm32h753
