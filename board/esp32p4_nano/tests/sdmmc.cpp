// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0009 REQ-0010 REQ-0011 REQ-0013 REQ-0014 REQ-0016 REQ-0019 REQ-0034 REQ-0036 REQ-0037

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>

#include "esp_cache.h"
#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcuhw/backend/esp32p4/gpio.hpp>
#include <mcuhw/backend/esp32p4/sdmmc.hpp>
#include <mcuhw/board/esp32p4_nano.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/driver/sdmmc.hpp>

extern "C" void mcuhw_esp32p4_sdmmc_test_link_anchor() noexcept {}

namespace mcuhw::board_test::esp32p4_nano {
namespace {

class delay_policy {
public:
    void delay_us(std::uint32_t duration) noexcept {
        esp_rom_delay_us(duration);
    }
};

class dma_memory {
public:
    [[nodiscard]] std::expected<std::uint32_t, driver::sdmmc_error>
    address(const void* pointer) noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        if (address > 0xFFFF'FFFFU || address < 0x4000'0000U
            || address >= 0x4FFC'0000U) {
            return std::unexpected(driver::sdmmc_error::dma);
        }
        return static_cast<std::uint32_t>(address);
    }

    [[nodiscard]] bool prepare_for_device(const void* pointer,
                                          std::size_t size) noexcept {
        const auto result = esp_cache_msync(
            const_cast<void*>(pointer), size,
            ESP_CACHE_MSYNC_FLAG_DIR_C2M
                | ESP_CACHE_MSYNC_FLAG_INVALIDATE
                | ESP_CACHE_MSYNC_FLAG_UNALIGNED);
        return result == ESP_OK || result == ESP_ERR_NOT_SUPPORTED;
    }

    [[nodiscard]] bool complete_from_device(void* pointer,
                                            std::size_t size) noexcept {
        const auto line_size = esp_cache_get_line_size_by_addr(pointer);
        if (line_size != 0U
            && (reinterpret_cast<std::uintptr_t>(pointer) % line_size != 0U
                || size % line_size != 0U)) {
            return false;
        }
        const auto result = esp_cache_msync(
            pointer, size, ESP_CACHE_MSYNC_FLAG_DIR_M2C);
        return result == ESP_OK || result == ESP_ERR_NOT_SUPPORTED;
    }
};

alignas(64) std::array<std::byte, 512> sector_zero{};
alignas(64) std::array<std::byte, 512> original_sector{};
alignas(64) std::array<std::byte, 512> test_pattern{};
alignas(64) std::array<std::byte, 512> verification_sector{};

} // namespace

TEST(esp32p4_sdmmc, identifies_reads_writes_and_restores_onboard_card,
     "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0013", "REQ-0014",
     "REQ-0016", "REQ-0019", "REQ-0034", "REQ-0036", "REQ-0037") {
    core::direct_mmio mmio;
    dma_memory memory;
    backend::esp32p4::sdmmc_host_backend host{mmio, memory};
    ASSERT_TRUE(host.enable_io_power());
    backend::esp32p4::gpio_backend power_gpio{mmio};
    driver::gpio_pin power{power_gpio,
                           board::esp32p4_nano::microsd_power_enable};
    const auto power_initialized = power.initialize({
        .direction = driver::gpio_direction::output,
        .initial_level = driver::gpio_level::high,
        .output_type = driver::gpio_output_type::push_pull,
        .pull = driver::gpio_pull::none,
        .drive = driver::gpio_drive::middle,
    });
    ASSERT_TRUE(power_initialized.has_value());
    if (!power_initialized) return;
    // Waveshare's board example requires a 100 ms high pulse to remove card
    // power before the active-low enable is asserted again.
    esp_rom_delay_us(100'000U);
    const auto powered = power.clear();
    ASSERT_TRUE(powered.has_value());
    if (!powered) {
        static_cast<void>(power.release());
        return;
    }

    delay_policy delay;
    driver::sd_card card{host, delay};
    const auto initialized = card.initialize();
    if (!initialized) {
        const auto ctrl = mmio.read<std::uint32_t>(0x5008'3000U);
        const auto verid = mmio.read<std::uint32_t>(0x5008'306CU);
        const auto hcon = mmio.read<std::uint32_t>(0x5008'3070U);
        const auto clk1 = mmio.read<std::uint32_t>(0x500E'6034U);
        const auto clk2 = mmio.read<std::uint32_t>(0x500E'6038U);
        esp_rom_printf("MCUHW_SDMMC_ERROR stage=initialize code=%u "
                       "host_stage=%u ctrl=0x%08lx verid=0x%08lx "
                       "hcon=0x%08lx clk1=0x%08lx clk2=0x%08lx "
                       "command=%u rintsts=0x%08lx idsts=0x%08lx\n",
                       static_cast<unsigned>(initialized.error()),
                       static_cast<unsigned>(host.last_initialization_stage()),
                       static_cast<unsigned long>(ctrl.value_or(0U)),
                       static_cast<unsigned long>(verid.value_or(0U)),
                       static_cast<unsigned long>(hcon.value_or(0U)),
                       static_cast<unsigned long>(clk1.value_or(0U)),
                       static_cast<unsigned long>(clk2.value_or(0U)),
                       static_cast<unsigned>(host.last_command_index()),
                       static_cast<unsigned long>(
                           host.last_interrupt_status()),
                       static_cast<unsigned long>(host.last_dma_status()));
    }
    ASSERT_TRUE(initialized.has_value());
    if (!initialized) {
        static_cast<void>(power.set());
        static_cast<void>(power.release());
        return;
    }
    const auto read = card.read_sector(0U, sector_zero);
    if (!read) {
        esp_rom_printf(
            "MCUHW_SDMMC_ERROR stage=read_sector code=%u command=%u "
            "rintsts=0x%08lx idsts=0x%08lx\n",
            static_cast<unsigned>(read.error()),
            static_cast<unsigned>(host.last_command_index()),
            static_cast<unsigned long>(host.last_interrupt_status()),
            static_cast<unsigned long>(host.last_dma_status()));
    }
    ASSERT_TRUE(read.has_value());
    if (!read) {
        static_cast<void>(card.shutdown());
        static_cast<void>(power.set());
        static_cast<void>(power.release());
        return;
    }

    const auto& info = card.info();
    const auto test_sector = static_cast<std::uint32_t>(
        info.sector_count - 1U);
    const auto original_read = card.read_sector(test_sector, original_sector);
    bool pattern_written{};
    bool pattern_verified{};
    if (original_read) {
        for (std::size_t index = 0U; index < test_pattern.size(); ++index) {
            test_pattern[index] = static_cast<std::byte>(
                (index * 37U + 0x5AU) & 0xFFU);
        }
        pattern_written = card.write_sector(test_sector, test_pattern)
            .has_value();
        if (pattern_written
            && card.read_sector(test_sector, verification_sector)) {
            pattern_verified = verification_sector == test_pattern;
        }
    }

    // Restore the original sector even after a failed pattern verification.
    // This keeps the test repeatable and preserves any filesystem metadata.
    const auto restored = original_read
        && card.write_sector(test_sector, original_sector).has_value();
    bool restore_verified{};
    if (restored && card.read_sector(test_sector, verification_sector)) {
        restore_verified = verification_sector == original_sector;
    }
    if (!original_read || !pattern_written || !pattern_verified
        || !restored || !restore_verified) {
        esp_rom_printf(
            "MCUHW_SDMMC_ERROR stage=write_read_restore original=%u "
            "written=%u verified=%u restored=%u restore_verified=%u\n",
            original_read ? 1U : 0U, pattern_written ? 1U : 0U,
            pattern_verified ? 1U : 0U, restored ? 1U : 0U,
            restore_verified ? 1U : 0U);
    }
    ASSERT_TRUE(original_read.has_value());
    ASSERT_TRUE(pattern_written);
    ASSERT_TRUE(pattern_verified);
    ASSERT_TRUE(restored);
    ASSERT_TRUE(restore_verified);

    esp_rom_printf(
        "MCUHW_SDMMC backend=esp32p4 slot=0 width=4 d0=39 d1=40 d2=41 "
        "d3=42 clk=43 cmd=44 power_gpio=45 read=sector0 "
        "write_read_restore=last_sector "
        "capacity_sectors=%llu high_capacity=%u mid=0x%02x product=%s "
        "mbr_signature=%02x%02x\n",
        static_cast<unsigned long long>(info.sector_count),
        info.high_capacity ? 1U : 0U,
        static_cast<unsigned>(info.manufacturer_id),
        info.product_name.data(),
        static_cast<unsigned>(std::to_integer<std::uint8_t>(sector_zero[511])),
        static_cast<unsigned>(std::to_integer<std::uint8_t>(sector_zero[510])));

    ASSERT_TRUE(info.four_bit);
    ASSERT_TRUE(info.sector_count != 0U);
    ASSERT_TRUE(card.shutdown().has_value());
    ASSERT_TRUE(power.set().has_value());
    ASSERT_TRUE(power.release().has_value());
}

} // namespace mcuhw::board_test::esp32p4_nano
