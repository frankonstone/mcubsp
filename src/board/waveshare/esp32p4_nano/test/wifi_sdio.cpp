// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0003 REQ-0009 REQ-0010 REQ-0011 REQ-0014 REQ-0016 REQ-0019 REQ-0034 REQ-0036 REQ-0037

#include <cstdint>

#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcubsp/backend/esp32p4/gpio.hpp>
#include <mcubsp/backend/esp32p4/sdmmc.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/driver/sdio.hpp>

extern "C" void mcubsp_esp32p4_wifi_sdio_test_link_anchor() noexcept {}

namespace mcubsp::board_test::esp32p4_nano {
namespace {

class delay_policy {
public:
    void delay_us(std::uint32_t duration) noexcept {
        esp_rom_delay_us(duration);
    }
};

// The identity probe uses CMD52 only, so no data DMA mapping is requested.
class unused_dma_memory {};

} // namespace

TEST(esp32p4_wifi_sdio, enumerates_onboard_c5_and_enables_function_one,
     "REQ-0003", "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0014",
     "REQ-0016", "REQ-0019", "REQ-0034", "REQ-0036", "REQ-0037") {
    core::direct_mmio mmio;
    backend::esp32p4::gpio_backend enable_gpio{mmio};
    driver::gpio_pin enable{enable_gpio,
                            board::waveshare::esp32p4_nano_bindings::wifi_coprocessor_enable};
    const auto enable_initialized = enable.initialize({
        .direction = driver::gpio_direction::output,
        .initial_level = driver::gpio_level::low,
        .output_type = driver::gpio_output_type::push_pull,
        .pull = driver::gpio_pull::none,
        .drive = driver::gpio_drive::middle,
    });
    ASSERT_TRUE(enable_initialized.has_value());
    if (!enable_initialized) return;
    esp_rom_delay_us(10'000U);
    ASSERT_TRUE(enable.set().has_value());
    // ESP-Hosted's documented default allows the C5 firmware 1.5 s after EN.
    esp_rom_delay_us(1'500'000U);

    unused_dma_memory memory;
    using binding = board::waveshare::esp32p4_nano_bindings::sdmmc_slot1_wifi_binding;
    backend::esp32p4::sdmmc_host_backend<core::direct_mmio,
                                         unused_dma_memory, binding>
        host{mmio, memory};
    delay_policy delay;
    driver::sdio_device device{host, delay};
    const auto initialized = device.initialize();
    if (!initialized) {
        esp_rom_printf(
            "MCUBSP_WIFI_SDIO_ERROR code=%u host_stage=%u command=%u "
            "rintsts=0x%08lx idsts=0x%08lx\n",
            static_cast<unsigned>(initialized.error()),
            static_cast<unsigned>(host.last_initialization_stage()),
            static_cast<unsigned>(host.last_command_index()),
            static_cast<unsigned long>(host.last_interrupt_status()),
            static_cast<unsigned long>(host.last_dma_status()));
    }
    ASSERT_TRUE(initialized.has_value());
    if (!initialized) {
        static_cast<void>(enable.release());
        return;
    }

    esp_rom_printf(
        "MCUBSP_WIFI_SDIO backend=esp32p4 slot=1 width=4 d0=14 d1=15 "
        "d2=16 d3=17 clk=18 cmd=19 enable_gpio=54 functions=%u "
        "rca=0x%04x cccr=0x%02x cis=0x%05lx manf=0x%04x "
        "card=0x%04x function1=ready transport=identity_only\n",
        static_cast<unsigned>(initialized->function_count),
        static_cast<unsigned>(initialized->relative_address),
        static_cast<unsigned>(initialized->cccr_revision),
        static_cast<unsigned long>(initialized->cis_pointer),
        static_cast<unsigned>(initialized->manufacturer_id),
        static_cast<unsigned>(initialized->card_id));

    ASSERT_TRUE(initialized->function_count != 0U);
    ASSERT_EQ(initialized->manufacturer_id, 0x0092U);
    ASSERT_EQ(initialized->card_id, 0x6666U);
    ASSERT_TRUE(initialized->function1_ready);
    ASSERT_TRUE(initialized->four_bit);
    ASSERT_TRUE(device.shutdown().has_value());
    ASSERT_TRUE(enable.release().has_value());
}

} // namespace mcubsp::board_test::esp32p4_nano
