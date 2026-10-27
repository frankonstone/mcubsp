#pragma once
// @req-file REQ-0031 REQ-0034
#include <cstdint>
#include <mcubsp/core/metadata.hpp>

namespace mcubsp::board::waveshare {

struct esp32p4_nano_pins {
    static constexpr core::pin_id uart0_tx{0, 37};
    static constexpr core::pin_id uart0_rx{0, 38};
    static constexpr core::pin_id default_i2c_sda{0, 7};
    static constexpr core::pin_id default_i2c_scl{0, 8};
    static constexpr std::uint8_t audio_codec_i2c_address = 0x18U;
    static constexpr core::pin_id audio_i2s_mclk{0, 13};
    static constexpr core::pin_id audio_i2s_bclk{0, 12};
    static constexpr core::pin_id audio_i2s_ws{0, 10};
    static constexpr core::pin_id audio_i2s_data_out{0, 9};
    static constexpr core::pin_id audio_i2s_data_in{0, 11};
    static constexpr core::pin_id audio_amplifier_enable{0, 53};
    static constexpr core::pin_id microsd_d0{0, 39};
    static constexpr core::pin_id microsd_d1{0, 40};
    static constexpr core::pin_id microsd_d2{0, 41};
    static constexpr core::pin_id microsd_d3{0, 42};
    static constexpr core::pin_id microsd_clock{0, 43};
    static constexpr core::pin_id microsd_command{0, 44};
    // The NANO schematic switches microSD power with an active-low GPIO45.
    static constexpr core::pin_id microsd_power_enable{0, 45};
    static constexpr core::pin_id wifi_sdio_d0{0, 14};
    static constexpr core::pin_id wifi_sdio_d1{0, 15};
    static constexpr core::pin_id wifi_sdio_d2{0, 16};
    static constexpr core::pin_id wifi_sdio_d3{0, 17};
    static constexpr core::pin_id wifi_sdio_clock{0, 18};
    static constexpr core::pin_id wifi_sdio_command{0, 19};
    // ESP32-C5 CHIP_PU is active high; driving it low resets the coprocessor.
    static constexpr core::pin_id wifi_coprocessor_enable{0, 54};
    static constexpr std::uint8_t ethernet_phy_address = 1U;
    static constexpr core::pin_id ethernet_crs_dv{0, 28};
    static constexpr core::pin_id ethernet_rxd0{0, 29};
    static constexpr core::pin_id ethernet_rxd1{0, 30};
    static constexpr core::pin_id ethernet_mdc{0, 31};
    static constexpr core::pin_id ethernet_txd0{0, 34};
    static constexpr core::pin_id ethernet_txd1{0, 35};
    static constexpr core::pin_id ethernet_tx_enable{0, 49};
    static constexpr core::pin_id ethernet_reference_clock{0, 50};
    static constexpr core::pin_id ethernet_phy_reset{0, 51};
    static constexpr core::pin_id ethernet_mdio{0, 52};
    // Free expansion-header GPIOs on the NANO. Unlike the microSD and Wi-Fi
    // links, these pins are not committed to an on-board SDIO peripheral.
    static constexpr core::pin_id default_spi_miso{0, 20};
    static constexpr core::pin_id default_spi_mosi{0, 21};
    static constexpr core::pin_id default_spi_sck{0, 22};
    static constexpr core::pin_id default_spi_cs{0, 23};
};

} // namespace mcubsp::board::waveshare
