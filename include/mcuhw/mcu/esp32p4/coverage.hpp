#pragma once

#include <array>

#include <mcuhw/mcu/esp32p4/model.hpp>

namespace mcuhw::mcu::esp32p4 {

constexpr core::source_unit_coverage reviewed_source_unit(
    std::string_view unit, std::string_view title, std::uint16_t first_page,
    core::driver_support_state driver = core::driver_support_state::deferred,
    core::source_ref source = technical_reference_manual,
    std::string_view decision = "Complete published source-unit scope reviewed; registers, fields, and routes are accounted for while native encodings remain vendor-specific.") noexcept {
    return {unit, title, first_page, core::description_coverage_state::verified,
            driver, true, true, true, decision, source};
}

constexpr core::source_unit_coverage unavailable_source_unit(
    std::string_view unit, std::string_view title,
    std::string_view decision) noexcept {
    return {unit, title, 0U,
            core::description_coverage_state::unavailable_in_source,
            core::driver_support_state::deferred,
            false, false, false, decision, technical_reference_manual};
}

inline constexpr auto planned = core::driver_support_state::planned;
inline constexpr auto no_driver = core::driver_support_state::not_applicable;

// Series-datasheet identity/package unit, all 66 published TRM v0.7 chapters,
// and the announced but unpublished I3C unit.  The missing unit stays in the
// denominator and is never converted into placeholder metadata.
inline constexpr std::array complete_source_inventory{
    reviewed_source_unit("DS", "ESP32-P4 identity, QFN104 pins and electrical limits", 1, no_driver, series_datasheet),
    reviewed_source_unit("1", "Bus Architecture", 65, no_driver),
    reviewed_source_unit("2", "High-Performance CPU", 68, no_driver),
    reviewed_source_unit("3", "RISC-V Trace Encoder (TRACE)", 180, no_driver),
    reviewed_source_unit("4", "Processor Instruction Extensions (PIE)", 210, no_driver),
    reviewed_source_unit("5", "Low-Power CPU", 633, no_driver),
    reviewed_source_unit("6", "GDMA Controller (GDMA-AHB, GDMA-AXI)", 659, planned),
    reviewed_source_unit("7", "VDMA Controller (VDMA)", 774, planned),
    reviewed_source_unit("8", "2D-DMA Controller (2D-DMA)", 828, planned),
    reviewed_source_unit("9", "System and Memory", 901, no_driver),
    reviewed_source_unit("10", "GPIO Matrix and IO MUX", 981, planned),
    reviewed_source_unit("11", "Reset and Clock", 1081, no_driver),
    reviewed_source_unit("12", "Chip Boot Control", 1219, no_driver),
    reviewed_source_unit("13", "Interrupt Matrix", 1224, no_driver),
    reviewed_source_unit("14", "Event Task Matrix (ETM)", 1268),
    reviewed_source_unit("15", "Low-Power Management", 1425, no_driver),
    reviewed_source_unit("16", "System Timer", 1507, planned),
    reviewed_source_unit("17", "Timer Group (TIMG)", 1532, planned),
    reviewed_source_unit("18", "Watchdog Timers (WDT)", 1557),
    reviewed_source_unit("19", "RTC Timer", 1572),
    reviewed_source_unit("20", "Permission Control (PMS)", 1586, no_driver),
    reviewed_source_unit("21", "System Registers (SYSREG)", 1687, no_driver),
    reviewed_source_unit("22", "Debug Assistant", 1793, no_driver),
    reviewed_source_unit("23", "AXI Performance Monitor", 1853),
    reviewed_source_unit("24", "LP Mailbox", 1894),
    reviewed_source_unit("25", "Brown-out Detector", 1911),
    reviewed_source_unit("26", "Power Supply Detector (PSDET)", 1926),
    reviewed_source_unit("27", "AES Accelerator (AES)", 1928),
    reviewed_source_unit("28", "ECC Accelerator (ECC)", 1950),
    reviewed_source_unit("29", "HMAC Accelerator (HMAC)", 1964),
    reviewed_source_unit("30", "RSA Accelerator (RSA)", 1981),
    reviewed_source_unit("31", "SHA Accelerator (SHA)", 1992),
    reviewed_source_unit("32", "RSA Digital Signature Peripheral (RSA_DS)", 2008),
    reviewed_source_unit("33", "ECDSA Digital Signature Peripheral (ECDSA_DS)", 2019),
    reviewed_source_unit("34", "External Memory Encryption and Decryption (XTS_AES)", 2038),
    reviewed_source_unit("35", "Random Number Generator (RNG)", 2052),
    reviewed_source_unit("36", "Key Manager", 2056),
    reviewed_source_unit("37", "JPEG Codec", 2095),
    reviewed_source_unit("38", "Image Signal Processor (ISP)", 2140),
    reviewed_source_unit("39", "Pixel-Processing Accelerator (PPA)", 2255),
    reviewed_source_unit("40", "LCD and Camera Controller (LCD_CAM)", 2300),
    reviewed_source_unit("41", "H264 Encoder", 2336),
    reviewed_source_unit("42", "MIPI CSI", 2592),
    reviewed_source_unit("43", "MIPI DSI", 2633),
    reviewed_source_unit("44", "Voice Activity Detection (VAD)", 2723),
    reviewed_source_unit("45", "UART Controller (UART)", 2733, planned),
    reviewed_source_unit("46", "SPI Controller (SPI)", 2819, planned),
    reviewed_source_unit("47", "I2C Controller (I2C)", 2979, planned),
    reviewed_source_unit("48", "Analog I2C Controller", 3062),
    reviewed_source_unit("49", "I2S Controller (I2S)", 3070, planned),
    reviewed_source_unit("50", "LP I2S Controller", 3116),
    reviewed_source_unit("51", "Pulse Count Controller (PCNT)", 3142),
    reviewed_source_unit("52", "USB 2.0 High-Speed OTG", 3158),
    reviewed_source_unit("53", "USB 2.0 Full-Speed OTG", 3170),
    reviewed_source_unit("54", "USB Serial/JTAG Controller (USB_SERIAL_JTAG)", 3187),
    reviewed_source_unit("55", "Ethernet Media Access Controller (EMAC)", 3223, planned),
    reviewed_source_unit("56", "Two-Wire Automotive Interface (TWAI)", 3304),
    reviewed_source_unit("57", "SD/MMC Host Controller (SDHOST)", 3352),
    reviewed_source_unit("58", "LED PWM Controller (LEDC)", 3397, planned),
    reviewed_source_unit("59", "Motor Control PWM (MCPWM)", 3423, planned),
    reviewed_source_unit("60", "Remote Control Peripheral (RMT)", 3510),
    reviewed_source_unit("61", "Parallel IO Controller (PARLIO)", 3538),
    reviewed_source_unit("62", "BitScrambler", 3566),
    reviewed_source_unit("63", "Touch Sensor (TOUCH)", 3593),
    reviewed_source_unit("64", "Temperature Sensor (TSENS)", 3635),
    reviewed_source_unit("65", "ADC Controller (ADC)", 3646, planned),
    reviewed_source_unit("66", "Analog Voltage Comparator", 3688),
    unavailable_source_unit("I3C", "I3C Controller",
        "TRM v0.7 announces an I3C chapter but publishes no authoritative register, field, or route content; I3C remains excluded from the accepted model."),
};

static_assert(core::complete_source_inventory(
    std::span{complete_source_inventory}, 68U, 1U));

} // namespace mcuhw::mcu::esp32p4
