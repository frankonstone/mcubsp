#pragma once
// @req-file REQ-0031 REQ-0034
#include <array>
#include <cstdint>
#include <string_view>
#include <mcubsp/board/waveshare/esp32p4_nano/pins.hpp>
#include <mcubsp/mcu/esp32p4/system.hpp>

namespace mcubsp::board::waveshare {

// Board identity, clock/memory facts and connections; no I/O facade or backend
// implementation is required to use these bindings.
struct esp32p4_nano_bindings : esp32p4_nano_pins {
    static constexpr std::string_view name = "Waveshare ESP32-P4-NANO";
    static constexpr std::string_view device = "ESP32-P4NRW32";
    static constexpr mcu::esp32p4::part_variant part =
        mcu::esp32p4::part_variant::nrw32x;
    static constexpr std::uint32_t external_ram_bytes = 32U * 1024U * 1024U;

    // The board connected for PLN-0019 identifies as revision v1.0. The accepted
    // mcubsp description is deliberately limited to production v3.x silicon, so
    // observations from this board are development evidence, not v3.x acceptance.
    static constexpr std::uint16_t observed_silicon_revision = 100U;
    static constexpr bool accepted_description_revision = false;

    // Initial execution ownership is HP CPU0 only. HP CPU1 and the LP CPU remain
    // disabled/unowned until separately synchronized and verified.
    static constexpr mcu::esp32p4::cpu_domain execution_core =
        mcu::esp32p4::cpu_domain::hp_cpu0;
    static constexpr bool concurrent_hp_cpu1 = false;
    static constexpr bool concurrent_lp_cpu = false;

    static constexpr std::string_view boot_transport = "CH343 USB-to-UART";
    // The connected development-only v1.0 silicon predates the production-v3.x
    // reset/gate layout represented by the MCU description. Keep its observed
    // I2C0 control bits in the board integration layer so they cannot qualify or
    // silently alter the accepted production description.
    static constexpr std::uint32_t development_i2c0_apb_gate_mask = 1U << 12U;
    static constexpr std::uint32_t development_i2c0_reset_mask = 1U << 22U;


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
};

} // namespace mcubsp::board::waveshare
