#pragma once

// @req-file REQ-0031 REQ-0033

#include <array>
#include <mcubsp/core/metadata.hpp>

namespace mcubsp::board::nucleo_h753zi {

// NUCLEO-H753ZI MB1364 revision E: UM2407 Rev.6 section 7.6.1.
inline constexpr core::pin_id user_led_yellow{4, 1}; // PE1, active high
inline constexpr core::pin_id stlink_vcp_tx{3, 8};   // USART3_TX, AF7
inline constexpr core::pin_id stlink_vcp_rx{3, 9};   // USART3_RX, AF7
inline constexpr core::pin_id arduino_d13_sck{0, 5}; // SPI1_SCK, AF5
inline constexpr core::pin_id arduino_d12_miso{0, 6};
inline constexpr core::pin_id arduino_d11_mosi{1, 5};
inline constexpr core::pin_id arduino_d10_cs{3, 14};
inline constexpr core::pin_id arduino_d15_scl{1, 8};
inline constexpr core::pin_id arduino_d14_sda{1, 9};
inline constexpr core::pin_id arduino_a0{0, 3};
inline constexpr core::pin_id timer1_ch1{0, 8};

// MB1364-H753ZI-E01 LAN8742A RMII wiring. UM2407 Rev.6 requires JP6 and
// solder bridges SB27/SB29/SB30/SB31/SB36/SB57/SB64/SB72 to be closed.
inline constexpr core::pin_id ethernet_ref_clk{0, 1};
inline constexpr core::pin_id ethernet_mdio{0, 2};
inline constexpr core::pin_id ethernet_mdc{2, 1};
inline constexpr core::pin_id ethernet_crs_dv{0, 7};
inline constexpr core::pin_id ethernet_rxd0{2, 4};
inline constexpr core::pin_id ethernet_rxd1{2, 5};
inline constexpr core::pin_id ethernet_tx_en{6, 11};
inline constexpr core::pin_id ethernet_txd0{6, 13};
inline constexpr core::pin_id ethernet_txd1{1, 13};
inline constexpr std::uint8_t ethernet_phy_address = 0U;

struct ethernet_binding {
    static constexpr std::array rmii_pins{
        ethernet_ref_clk, ethernet_mdio, ethernet_mdc, ethernet_crs_dv,
        ethernet_rxd0, ethernet_rxd1, ethernet_tx_en, ethernet_txd0, ethernet_txd1};
};

} // namespace mcubsp::board::nucleo_h753zi
