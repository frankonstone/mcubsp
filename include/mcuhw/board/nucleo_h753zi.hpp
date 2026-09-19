#pragma once

// @req-file REQ-0031 REQ-0033

#include <mcuhw/core/metadata.hpp>

namespace mcuhw::board::nucleo_h753zi {

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

} // namespace mcuhw::board::nucleo_h753zi
