#pragma once

// @req-file REQ-0031 REQ-0034

#include <array>
#include <cstdint>
#include <string_view>

#include <mcubsp/core/metadata.hpp>
#include <mcubsp/mcu/esp32p4/system.hpp>

namespace mcubsp::board::esp32p4_nano {

inline constexpr std::string_view name = "Waveshare ESP32-P4-NANO";
inline constexpr std::string_view device = "ESP32-P4NRW32";
inline constexpr mcu::esp32p4::part_variant part =
    mcu::esp32p4::part_variant::nrw32x;
inline constexpr std::uint32_t external_ram_bytes = 32U * 1024U * 1024U;

// The board connected for PLN-0019 identifies as revision v1.0. The accepted
// mcubsp description is deliberately limited to production v3.x silicon, so
// observations from this board are development evidence, not v3.x acceptance.
inline constexpr std::uint16_t observed_silicon_revision = 100U;
inline constexpr bool accepted_description_revision = false;

// Initial execution ownership is HP CPU0 only. HP CPU1 and the LP CPU remain
// disabled/unowned until separately synchronized and verified.
inline constexpr mcu::esp32p4::cpu_domain execution_core =
    mcu::esp32p4::cpu_domain::hp_cpu0;
inline constexpr bool concurrent_hp_cpu1 = false;
inline constexpr bool concurrent_lp_cpu = false;

inline constexpr std::string_view boot_transport = "CH343 USB-to-UART";
inline constexpr core::pin_id uart0_tx{0, 37};
inline constexpr core::pin_id uart0_rx{0, 38};
inline constexpr core::pin_id default_i2c_sda{0, 7};
inline constexpr core::pin_id default_i2c_scl{0, 8};
inline constexpr std::uint8_t audio_codec_i2c_address = 0x18U;
inline constexpr core::pin_id audio_i2s_mclk{0, 13};
inline constexpr core::pin_id audio_i2s_bclk{0, 12};
inline constexpr core::pin_id audio_i2s_ws{0, 10};
inline constexpr core::pin_id audio_i2s_data_out{0, 9};
inline constexpr core::pin_id audio_i2s_data_in{0, 11};
inline constexpr core::pin_id audio_amplifier_enable{0, 53};
inline constexpr core::pin_id microsd_d0{0, 39};
inline constexpr core::pin_id microsd_d1{0, 40};
inline constexpr core::pin_id microsd_d2{0, 41};
inline constexpr core::pin_id microsd_d3{0, 42};
inline constexpr core::pin_id microsd_clock{0, 43};
inline constexpr core::pin_id microsd_command{0, 44};
// The NANO schematic switches microSD power with an active-low GPIO45.
inline constexpr core::pin_id microsd_power_enable{0, 45};
inline constexpr core::pin_id wifi_sdio_d0{0, 14};
inline constexpr core::pin_id wifi_sdio_d1{0, 15};
inline constexpr core::pin_id wifi_sdio_d2{0, 16};
inline constexpr core::pin_id wifi_sdio_d3{0, 17};
inline constexpr core::pin_id wifi_sdio_clock{0, 18};
inline constexpr core::pin_id wifi_sdio_command{0, 19};
// ESP32-C5 CHIP_PU is active high; driving it low resets the coprocessor.
inline constexpr core::pin_id wifi_coprocessor_enable{0, 54};
inline constexpr std::uint8_t ethernet_phy_address = 1U;
inline constexpr core::pin_id ethernet_crs_dv{0, 28};
inline constexpr core::pin_id ethernet_rxd0{0, 29};
inline constexpr core::pin_id ethernet_rxd1{0, 30};
inline constexpr core::pin_id ethernet_mdc{0, 31};
inline constexpr core::pin_id ethernet_txd0{0, 34};
inline constexpr core::pin_id ethernet_txd1{0, 35};
inline constexpr core::pin_id ethernet_tx_enable{0, 49};
inline constexpr core::pin_id ethernet_reference_clock{0, 50};
inline constexpr core::pin_id ethernet_phy_reset{0, 51};
inline constexpr core::pin_id ethernet_mdio{0, 52};
// Free expansion-header GPIOs on the NANO. Unlike the microSD and Wi-Fi
// links, these pins are not committed to an on-board SDIO peripheral.
inline constexpr core::pin_id default_spi_miso{0, 20};
inline constexpr core::pin_id default_spi_mosi{0, 21};
inline constexpr core::pin_id default_spi_sck{0, 22};
inline constexpr core::pin_id default_spi_cs{0, 23};

// The connected development-only v1.0 silicon predates the production-v3.x
// reset/gate layout represented by the MCU description. Keep its observed
// I2C0 control bits in the board integration layer so they cannot qualify or
// silently alter the accepted production description.
inline constexpr std::uint32_t development_i2c0_apb_gate_mask = 1U << 12U;
inline constexpr std::uint32_t development_i2c0_reset_mask = 1U << 22U;


struct ethernet_binding {
    static constexpr std::array rmii_inputs{ethernet_crs_dv.index, ethernet_rxd0.index, ethernet_rxd1.index};
    static constexpr std::array rmii_outputs{ethernet_txd0.index, ethernet_txd1.index, ethernet_tx_enable.index};
    static constexpr auto reference_clock = ethernet_reference_clock.index;
    static constexpr auto mdc = ethernet_mdc.index;
    static constexpr auto mdio = ethernet_mdio.index;
    static constexpr auto phy_address = ethernet_phy_address;
};

struct sdmmc_slot0_binding : mcu::esp32p4::sdmmc_instance {
    static constexpr auto d0 = microsd_d0.index;
    static constexpr auto d1 = microsd_d1.index;
    static constexpr auto d2 = microsd_d2.index;
    static constexpr auto d3 = microsd_d3.index;
    static constexpr auto clock = microsd_clock.index;
    static constexpr auto command = microsd_command.index;
    static constexpr std::uint8_t slot = 0U;
    static constexpr bool uses_gpio_matrix = false;
    static constexpr bool requires_io_power = true;
};

struct sdmmc_slot1_wifi_binding : mcu::esp32p4::sdmmc_instance {
    static constexpr auto d0 = wifi_sdio_d0.index;
    static constexpr auto d1 = wifi_sdio_d1.index;
    static constexpr auto d2 = wifi_sdio_d2.index;
    static constexpr auto d3 = wifi_sdio_d3.index;
    static constexpr auto clock = wifi_sdio_clock.index;
    static constexpr auto command = wifi_sdio_command.index;
    static constexpr std::uint8_t slot = 1U;
    static constexpr bool uses_gpio_matrix = true;
    static constexpr bool requires_io_power = false;
    static constexpr std::uint16_t clock_signal = 0U;
    static constexpr std::uint16_t command_signal = 1U;
    static constexpr std::uint16_t d0_signal = 2U;
    static constexpr std::uint16_t d1_signal = 3U;
    static constexpr std::uint16_t d2_signal = 4U;
    static constexpr std::uint16_t d3_signal = 5U;
};

} // namespace mcubsp::board::esp32p4_nano
