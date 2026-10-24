#pragma once

#include <array>

#include <mcubsp/mcu/ra4m1/model.hpp>

namespace mcubsp::mcu::ra4m1 {

constexpr core::source_unit_coverage reviewed_source_unit(
    std::string_view unit, std::string_view title, std::uint16_t first_page,
    core::driver_support_state driver = core::driver_support_state::deferred,
    std::string_view decision = "Complete chapter scope reviewed; registers, fields, and routes are accounted for by the pinned source catalogue.") noexcept {
    return {unit, title, first_page, core::description_coverage_state::verified,
            driver, true, true, true, decision, hardware_manual};
}

inline constexpr auto planned = core::driver_support_state::planned;
inline constexpr auto no_driver = core::driver_support_state::not_applicable;

// R01UH0887EJ0110 Rev.1.10 chapters 1-48.  Each row covers the whole source
// unit, including register/field/route absence where the chapter is descriptive
// rather than register-bearing.  Functional implementation remains independent.
inline constexpr std::array complete_source_inventory{
    reviewed_source_unit("1", "Overview", 50, no_driver),
    reviewed_source_unit("2", "CPU", 75, no_driver),
    reviewed_source_unit("3", "Operating Modes", 91, no_driver),
    reviewed_source_unit("4", "Address Space", 92, no_driver),
    reviewed_source_unit("5", "Resets", 93, no_driver),
    reviewed_source_unit("6", "Option-Setting Memory", 105, no_driver),
    reviewed_source_unit("7", "Low Voltage Detection (LVD)", 115),
    reviewed_source_unit("8", "Clock Generation Circuit", 129, no_driver),
    reviewed_source_unit("9", "Clock Frequency Accuracy Measurement Circuit (CAC)", 162),
    reviewed_source_unit("10", "Low Power Modes", 171, no_driver),
    reviewed_source_unit("11", "Battery Backup Function", 204, no_driver),
    reviewed_source_unit("12", "Register Write Protection", 224, no_driver),
    reviewed_source_unit("13", "Interrupt Controller Unit (ICU)", 225, no_driver),
    reviewed_source_unit("14", "Buses", 252, no_driver),
    reviewed_source_unit("15", "Memory Protection Unit (MPU)", 261, no_driver),
    reviewed_source_unit("16", "DMA Controller (DMAC)", 292, planned),
    reviewed_source_unit("17", "Data Transfer Controller (DTC)", 321),
    reviewed_source_unit("18", "Event Link Controller (ELC)", 346),
    reviewed_source_unit("19", "I/O Ports", 355, planned),
    reviewed_source_unit("20", "Key Interrupt Function (KINT)", 380),
    reviewed_source_unit("21", "Port Output Enable for GPT (POEG)", 386),
    reviewed_source_unit("22", "General PWM Timer (GPT)", 392, planned),
    reviewed_source_unit("23", "Low Power Asynchronous General Purpose Timer (AGT)", 514),
    reviewed_source_unit("24", "Realtime Clock (RTC)", 539),
    reviewed_source_unit("25", "Watchdog Timer (WDT)", 578),
    reviewed_source_unit("26", "Independent Watchdog Timer (IWDT)", 591),
    reviewed_source_unit("27", "USB 2.0 Full-Speed Module (USBFS)", 601),
    reviewed_source_unit("28", "Serial Communications Interface (SCI)", 696, planned),
    reviewed_source_unit("29", "I2C Bus Interface (IIC)", 822, planned),
    reviewed_source_unit("30", "Controller Area Network (CAN) Module", 898),
    reviewed_source_unit("31", "Serial Peripheral Interface (SPI)", 946, planned),
    reviewed_source_unit("32", "Cyclic Redundancy Check (CRC) Calculator", 1008),
    reviewed_source_unit("33", "Serial Sound Interface Enhanced (SSIE)", 1016),
    reviewed_source_unit("34", "Boundary Scan", 1078, no_driver),
    reviewed_source_unit("35", "14-Bit A/D Converter (ADC14)", 1084, planned),
    reviewed_source_unit("36", "12-Bit D/A Converter (DAC12)", 1155),
    reviewed_source_unit("37", "Temperature Sensor (TSN)", 1163),
    reviewed_source_unit("38", "Operational Amplifier (OPAMP)", 1166),
    reviewed_source_unit("39", "Low-Power Analog Comparator (ACMPLP)", 1179),
    reviewed_source_unit("40", "8-Bit D/A Converter (DAC8)", 1190),
    reviewed_source_unit("41", "Capacitive Touch Sensing Unit (CTSU)", 1193),
    reviewed_source_unit("42", "Data Operation Circuit (DOC)", 1229),
    reviewed_source_unit("43", "SRAM", 1234, no_driver),
    reviewed_source_unit("44", "Flash Memory", 1245, no_driver),
    reviewed_source_unit("45", "Segment LCD Controller (SLCDC)", 1262),
    reviewed_source_unit("46", "Secure Cryptographic Engine (SCE5)", 1305),
    reviewed_source_unit("47", "Internal Voltage Regulator", 1309, no_driver),
    reviewed_source_unit("48", "Electrical Characteristics", 1310, no_driver,
        "Electrical limits and timing constraints reviewed; register, field, and route categories are explicitly not applicable within this source unit."),
};

static_assert(core::complete_source_inventory(
    std::span{complete_source_inventory}, 48U));

} // namespace mcubsp::mcu::ra4m1
