// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0010 REQ-0011 REQ-0014 REQ-0017 REQ-0019 REQ-0028 REQ-0034 REQ-0036

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcubsp/backend/esp32p4/spi.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

extern "C" void mcubsp_esp32p4_spi_test_link_anchor() noexcept {}

namespace mcubsp::board_test::esp32p4_nano {
namespace {

using binding = backend::esp32p4::spi_binding<
    2, board::waveshare::esp32p4_nano_bindings::default_spi_miso,
    board::waveshare::esp32p4_nano_bindings::default_spi_mosi,
    board::waveshare::esp32p4_nano_bindings::default_spi_sck,
    board::waveshare::esp32p4_nano_bindings::default_spi_cs>;
using backend_type = backend::esp32p4::spi_master_backend<
    binding, core::direct_mmio, execution::fake_policy>;

constexpr auto timing_1m = backend::esp32p4::solve_spi_timing({
    .clock_hz = 40'000'000U,
    .target_hz = 1'000'000U,
    .maximum_error_ppm = 0U,
});
static_assert(timing_1m.has_value());
constexpr std::size_t completion_timeout_us = 20'000U;

[[nodiscard]] volatile const std::uint32_t&
register_at(std::uintptr_t address) noexcept {
    return *reinterpret_cast<volatile const std::uint32_t*>(address);
}

} // namespace

TEST(esp32p4_spi, gpspi2_executes_one_native_multisegment_command,
     "REQ-0010", "REQ-0011", "REQ-0014", "REQ-0017", "REQ-0019",
     "REQ-0028", "REQ-0034", "REQ-0036") {
    core::direct_mmio mmio;
    execution::fake_policy execution;
    backend_type spi{mmio, execution};
    ASSERT_TRUE(spi.initialize(*timing_1m).has_value());

    ASSERT_TRUE((register_at(binding::clocks.system_gate_register)
                 & binding::clocks.system_gate_mask) != 0U);
    ASSERT_TRUE((register_at(binding::clocks.apb_gate_register)
                 & binding::clocks.apb_gate_mask) != 0U);
    ASSERT_EQ(register_at(binding::base + 0x0CU),
              timing_1m->clock_register);
    ASSERT_EQ(register_at(mcu::esp32p4::gpio_matrix_base + 0x558U
                           + binding::clock_pin.index * 4U) & 0x1FFU,
              binding::clock_matrix_signal);
    ASSERT_EQ(register_at(mcu::esp32p4::gpio_matrix_base + 0x558U
                           + binding::select_pin.index * 4U) & 0x1FFU,
              binding::select_matrix_signal);
    ASSERT_EQ(register_at(mcu::esp32p4::gpio_matrix_base + 0x158U
                           + binding::miso_matrix_signal * 4U) & 0x3FU,
              binding::miso_pin.index);

    constexpr std::array<std::uint16_t, 2> output{0x9FU, 0x00U};
    std::array<std::uint16_t, 2> input{};
    const std::array segments{
        driver::spi_segment{output, {}, 0xFFU},
        driver::spi_segment{{}, input, 0xFFU},
    };
    auto started = spi.transaction(segments);
    ASSERT_TRUE(started.has_value());
    if (!started) return;
    auto transfer = std::move(*started);
    for (std::size_t elapsed = 0U;
         elapsed < completion_timeout_us && !transfer.ready(); ++elapsed) {
        static_cast<void>(spi.handle_interrupt());
        if (!transfer.ready()) esp_rom_delay_us(1U);
    }
    ASSERT_TRUE(transfer.ready());
    if (!transfer.ready()) {
        static_cast<void>(transfer.cancel());
        return;
    }
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    if (!result) return;
    ASSERT_EQ(result->frames, 4U);
    ASSERT_EQ(register_at(binding::base + 0x00U) & (1U << 24U), 0U);
    esp_rom_printf(
        "MCUBSP_SPI backend=esp32p4 spi=2 rate=1000000 frames=4 "
        "segments=2 completion=trans_done target=none\n");
    ASSERT_TRUE(spi.shutdown().has_value());
}

} // namespace mcubsp::board_test::esp32p4_nano
