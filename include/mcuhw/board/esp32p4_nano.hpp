#pragma once

// @req-file REQ-0031 REQ-0034

#include <cstdint>
#include <string_view>

#include <mcuhw/core/metadata.hpp>
#include <mcuhw/mcu/esp32p4/system.hpp>

namespace mcuhw::board::esp32p4_nano {

inline constexpr std::string_view name = "Waveshare ESP32-P4-NANO";
inline constexpr std::string_view device = "ESP32-P4NRW32";
inline constexpr mcu::esp32p4::part_variant part =
    mcu::esp32p4::part_variant::nrw32x;
inline constexpr std::uint32_t external_ram_bytes = 32U * 1024U * 1024U;

// The board connected for PLN-0019 identifies as revision v1.0. The accepted
// mcuhw description is deliberately limited to production v3.x silicon, so
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

} // namespace mcuhw::board::esp32p4_nano
