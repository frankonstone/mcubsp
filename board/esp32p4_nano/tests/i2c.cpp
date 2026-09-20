// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0011 REQ-0014 REQ-0017 REQ-0019 REQ-0024 REQ-0034 REQ-0036

#include <array>
#include <cstdint>
#include <utility>

#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcuhw/backend/esp32p4/i2c.hpp>
#include <mcuhw/board/esp32p4_nano.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/execution/fake_policy.hpp>

extern "C" void mcuhw_esp32p4_i2c_test_link_anchor() noexcept {}

namespace mcuhw::board_test::esp32p4_nano {
namespace {

using binding = backend::esp32p4::i2c_binding<
    0, board::esp32p4_nano::default_i2c_scl,
    board::esp32p4_nano::default_i2c_sda,
    board::esp32p4_nano::development_i2c0_apb_gate_mask,
    board::esp32p4_nano::development_i2c0_reset_mask>;
using backend_type = backend::esp32p4::i2c_backend<
    binding, core::direct_mmio, execution::fake_policy>;

constexpr auto timing_100k = backend::esp32p4::solve_i2c_timing({
    .clock_hz = 40'000'000U,
    .target_hz = 100'000U,
    .maximum_error_ppm = 1'000U,
    .rise_time_ns = 300U,
    .fall_time_ns = 300U,
});
static_assert(timing_100k.has_value());

constexpr auto codec_address = driver::make_i2c_address(
    board::esp32p4_nano::audio_codec_i2c_address);
static_assert(codec_address.has_value());

constexpr std::uint8_t es8311_chip_id1_register = 0xFDU;
constexpr std::uint8_t es8311_chip_id2_register = 0xFEU;
constexpr std::uint8_t es8311_version_register = 0xFFU;
constexpr std::uint8_t es8311_chip_id1 = 0x83U;
constexpr std::uint8_t es8311_chip_id2 = 0x11U;
constexpr std::size_t completion_timeout_us = 20'000U;

[[nodiscard]] volatile const std::uint32_t&
register_at(std::uintptr_t address) noexcept {
    return *reinterpret_cast<volatile const std::uint32_t*>(address);
}

struct register_read {
    bool started{};
    bool completed{};
    bool succeeded{};
    std::uint8_t value{};
};

[[nodiscard]] register_read read_es8311_register(
    backend_type& i2c, std::uint8_t selected_register) noexcept {
    const std::array<std::uint8_t, 1> selection{selected_register};
    std::array<std::uint8_t, 1> response{};
    auto started = i2c.write_read(*codec_address, selection, response);
    if (!started) return {};

    register_read observation{.started = true};
    auto transfer = std::move(*started);
    for (std::size_t elapsed = 0U;
         elapsed < completion_timeout_us && !transfer.ready(); ++elapsed) {
        static_cast<void>(i2c.handle_interrupt());
        if (!transfer.ready()) esp_rom_delay_us(1U);
    }
    observation.completed = transfer.ready();
    if (!observation.completed) {
        static_cast<void>(transfer.cancel());
        return observation;
    }

    const auto result = transfer.take();
    observation.succeeded = result.has_value()
        && result->bytes_written == selection.size()
        && result->bytes_read == response.size();
    if (observation.succeeded) observation.value = response[0];
    return observation;
}

} // namespace

TEST(esp32p4_i2c, i2c0_reads_onboard_es8311_identity_with_repeated_start,
     "REQ-0011", "REQ-0014", "REQ-0017", "REQ-0019", "REQ-0024",
     "REQ-0034", "REQ-0036") {
    core::direct_mmio mmio;
    execution::fake_policy execution;
    backend_type i2c{mmio, execution};
    const auto initialized = i2c.initialize(*timing_100k);
    ASSERT_TRUE(initialized.has_value());
    if (!initialized) return;

    const auto scl_pad = mcu::esp32p4::io_mux_base + 0x04U
        + binding::scl_pin.index * 4U;
    const auto scl_out = mcu::esp32p4::gpio_matrix_base + 0x558U
        + binding::scl_pin.index * 4U;
    const auto scl_in = mcu::esp32p4::gpio_matrix_base + 0x158U
        + binding::scl_matrix_signal * 4U;
    ASSERT_TRUE((register_at(binding::clocks.apb_gate_register)
                 & binding::clocks.apb_gate_mask) != 0U);
    ASSERT_TRUE((register_at(binding::clocks.source_register) & (1U << 1U))
                != 0U);
    ASSERT_EQ((register_at(binding::clocks.divider_register) >> 2U) & 0xFFU,
              timing_100k->backend_encoding - 1U);
    ASSERT_EQ(register_at(binding::base + 0x00U) & 0x1FFU,
              timing_100k->brl - 1U);
    ASSERT_TRUE((register_at(binding::base + 0x04U) & (1U << 4U)) != 0U);
    ASSERT_FALSE((register_at(binding::base + 0x08U) & (1U << 4U)) != 0U);
    ASSERT_EQ((register_at(scl_pad) >> 12U) & 0x7U, 1U);
    ASSERT_EQ(register_at(scl_out) & 0x1FFU, binding::scl_matrix_signal);
    ASSERT_EQ(register_at(scl_in) & 0x3FU, binding::scl_pin.index);

    const auto chip_id1 = read_es8311_register(
        i2c, es8311_chip_id1_register);
    const auto chip_id2 = read_es8311_register(
        i2c, es8311_chip_id2_register);
    const auto version = read_es8311_register(
        i2c, es8311_version_register);
    ASSERT_TRUE(chip_id1.started && chip_id1.completed && chip_id1.succeeded);
    ASSERT_TRUE(chip_id2.started && chip_id2.completed && chip_id2.succeeded);
    ASSERT_TRUE(version.started && version.completed && version.succeeded);
    ASSERT_EQ(chip_id1.value, es8311_chip_id1);
    ASSERT_EQ(chip_id2.value, es8311_chip_id2);
    esp_rom_printf(
        "MCUHW_I2C backend=esp32p4 i2c=0 address=0x18 device=ES8311 "
        "chip_id=0x%02x%02x version=0x%02x "
        "completion=repeated_start_read\n",
        static_cast<unsigned>(chip_id1.value),
        static_cast<unsigned>(chip_id2.value),
        static_cast<unsigned>(version.value));
    ASSERT_TRUE(i2c.shutdown().has_value());
}

} // namespace mcuhw::board_test::esp32p4_nano
