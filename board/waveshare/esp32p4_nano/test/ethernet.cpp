// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0003 REQ-0009 REQ-0010 REQ-0011 REQ-0013 REQ-0014 REQ-0034 REQ-0036 REQ-0043

#include <cstdint>

#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcubsp/backend/esp32p4/ethernet.hpp>
#include <mcubsp/board/esp32p4_nano.hpp>
#include <mcubsp/backend/esp32p4/gpio.hpp>
#include <mcubsp/board/esp32p4_nano.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/gpio.hpp>

extern "C" void mcubsp_esp32p4_ethernet_test_link_anchor() noexcept {}

namespace mcubsp::board_test::esp32p4_nano {

TEST(esp32p4_ethernet, initializes_emac_and_identifies_onboard_ip101,
     "REQ-0003", "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0013",
     "REQ-0014", "REQ-0034", "REQ-0036", "REQ-0043") {
    core::direct_mmio mmio;

    backend::esp32p4::gpio_backend reset_gpio{mmio};
    driver::gpio_pin reset{reset_gpio,
                           board::esp32p4_nano::ethernet_phy_reset};
    const auto reset_initialized = reset.initialize({
        .direction = driver::gpio_direction::output,
        .initial_level = driver::gpio_level::low,
        .output_type = driver::gpio_output_type::push_pull,
        .pull = driver::gpio_pull::none,
        .drive = driver::gpio_drive::middle,
    });
    ASSERT_TRUE(reset_initialized.has_value());
    if (!reset_initialized) return;
    // IP101 requires at least 10 ms reset assertion and 10 ms post-reset.
    esp_rom_delay_us(10'000U);
    ASSERT_TRUE(reset.set().has_value());
    esp_rom_delay_us(10'000U);

    backend::esp32p4::ethernet_rmii_mac<decltype(mmio), mcubsp::board::esp32p4_nano::ethernet_binding> mac{mmio};
    const backend::esp32p4::ethernet_mac_address address{
        {0x02U, 0x00U, 0x00U, 0x04U, 0x00U, 0x01U}};
    const auto initialized = mac.initialize(address);
    ASSERT_TRUE(initialized.has_value());
    if (!initialized) return;

    const auto identity = mac.identify_ip101(
        board::esp32p4_nano::ethernet_phy_address);
    ASSERT_TRUE(identity.has_value());
    if (!identity) return;
    ASSERT_TRUE(identity->valid());

    // BMSR link is latched low. The second read reports current state.
    static_cast<void>(mac.read_phy(
        board::esp32p4_nano::ethernet_phy_address, 1U));
    const auto bmsr = mac.read_phy(
        board::esp32p4_nano::ethernet_phy_address, 1U);
    ASSERT_TRUE(bmsr.has_value());
    if (!bmsr) return;
    const bool link_up = (*bmsr & (1U << 2U)) != 0U;

    esp_rom_printf(
        "MCUBSP_ETHERNET backend=esp32p4 interface=rmii phy=IP101GRI "
        "address=1 refclk_hz=50000000 id1=0x%04x id2=0x%04x "
        "oui=0x%06lx model=0x%02x bmsr=0x%04x link=%s "
        "traffic=not_tested\n",
        static_cast<unsigned>(identity->id1),
        static_cast<unsigned>(identity->id2),
        static_cast<unsigned long>(identity->oui()),
        static_cast<unsigned>(identity->model()),
        static_cast<unsigned>(*bmsr), link_up ? "up" : "down");
}

} // namespace mcubsp::board_test::esp32p4_nano
