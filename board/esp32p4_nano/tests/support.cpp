// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "support.hpp"

#include <cstddef>
#include <cstdint>

#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/runner.hpp>

#include <mcucov/mcucov.hpp>
#include <mcucov/sinks/buffered.hpp>

// mcutest's polymorphic runner emits a deleting-destructor entry even though
// this stack-owned runner is never deleted. Keep the board-test path
// allocation-free and avoid pulling the C++ allocator into the application
// for that unreachable path.
void operator delete(void*) noexcept {}
void operator delete(void*, std::size_t) noexcept {}

extern "C" void mcuhw_esp32p4_gpio_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_i2c_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_audio_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_ethernet_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_sdmmc_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_wifi_sdio_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_spi_test_link_anchor() noexcept;
extern "C" void mcuhw_esp32p4_uart_test_link_anchor() noexcept;

namespace mcuhw::board_test::esp32p4_nano {
void run_uart_preflight() noexcept;
}

namespace mcuhw::board_test::esp32p4_nano {
namespace {

struct uart_output {
    void write(mcutest::StringView text) noexcept {
        for (std::size_t index = 0; index < text.size(); ++index) {
            esp_rom_output_putc(text[index]);
        }
    }
};

struct coverage_hex_sink {
    bool MCUCOV_NO_INSTRUMENT write(const std::uint8_t* bytes,
                                    std::size_t size) noexcept {
        constexpr char hexadecimal[] = "0123456789abcdef";
        char encoded[128];
        std::size_t offset{};
        while (offset < size) {
            const auto remaining = size - offset;
            const auto chunk = remaining < sizeof(encoded) / 2U
                ? remaining
                : sizeof(encoded) / 2U;
            for (std::size_t index = 0; index < chunk; ++index) {
                const auto byte = bytes[offset + index];
                encoded[index * 2U] = hexadecimal[byte >> 4U];
                encoded[index * 2U + 1U] = hexadecimal[byte & 0x0fU];
            }
            for (std::size_t index = 0; index < chunk * 2U; ++index) {
                esp_rom_output_putc(encoded[index]);
            }
            offset += chunk;
        }
        return true;
    }
};

} // namespace

run_result MCUCOV_NO_INSTRUMENT run_tests_and_dump_coverage() noexcept {
    mcuhw_esp32p4_gpio_test_link_anchor();
    mcuhw_esp32p4_i2c_test_link_anchor();
    mcuhw_esp32p4_audio_test_link_anchor();
    mcuhw_esp32p4_ethernet_test_link_anchor();
    mcuhw_esp32p4_sdmmc_test_link_anchor();
    mcuhw_esp32p4_wifi_sdio_test_link_anchor();
    mcuhw_esp32p4_spi_test_link_anchor();
    mcuhw_esp32p4_uart_test_link_anchor();
    run_uart_preflight();
    mcutest::Runner<uart_output> runner;
    runner.run_all_registered();
    const auto failures = runner.summary();
    const auto tests = mcutest::registry().count;
    const auto registry_overflow = mcutest::registry().overflow_count;

    esp_rom_printf("MCUHW_COVERAGE_HEX:");
    coverage_hex_sink backend;
    mcucov::BufferedSink<coverage_hex_sink, 64> sink{backend};
    const bool written = mcucov::dump(sink);
    esp_rom_printf("\n");
    return {
        .tests = tests,
        .failures = failures,
        .registry_overflow = registry_overflow,
        .coverage_written = written,
    };
}

} // namespace mcuhw::board_test::esp32p4_nano
