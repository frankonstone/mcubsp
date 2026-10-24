// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0009 REQ-0010 REQ-0011 REQ-0013 REQ-0014 REQ-0017 REQ-0019 REQ-0024 REQ-0034 REQ-0036

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>

#include "esp_cache.h"
#include "esp_rom_serial_output.h"
#include "esp_rom_sys.h"

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcubsp/backend/esp32p4/gpio.hpp>
#include <mcubsp/backend/esp32p4/i2c.hpp>
#include <mcubsp/backend/esp32p4/i2s.hpp>
#include <mcubsp/board/esp32p4_nano.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/device/audio/es8311.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

extern "C" void mcubsp_esp32p4_audio_test_link_anchor() noexcept {}

namespace mcubsp::board_test::esp32p4_nano {
namespace {

using i2c_binding = backend::esp32p4::i2c_binding<
    0, board::esp32p4_nano::default_i2c_scl,
    board::esp32p4_nano::default_i2c_sda,
    board::esp32p4_nano::development_i2c0_apb_gate_mask,
    board::esp32p4_nano::development_i2c0_reset_mask>;
using i2c_backend_type = backend::esp32p4::i2c_backend<
    i2c_binding, core::direct_mmio, execution::fake_policy>;
using i2s_binding = backend::esp32p4::i2s_binding<
    0, board::esp32p4_nano::audio_i2s_mclk,
    board::esp32p4_nano::audio_i2s_bclk,
    board::esp32p4_nano::audio_i2s_ws,
    board::esp32p4_nano::audio_i2s_data_out,
    board::esp32p4_nano::audio_i2s_data_in>;

class dma_memory {
public:
    [[nodiscard]] std::expected<std::uint32_t, driver::i2s_error>
    address(const void* pointer) noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        if (address > 0xFFFF'FFFFU || address < 0x4000'0000U
            || address >= 0x4FFC'0000U) {
            return std::unexpected(driver::i2s_error::invalid_buffer);
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

using i2s_backend_type = backend::esp32p4::i2s_backend<
    i2s_binding, core::direct_mmio, execution::fake_policy, dma_memory>;

constexpr auto i2c_timing = backend::esp32p4::solve_i2c_timing({
    .clock_hz = 40'000'000U,
    .target_hz = 100'000U,
    .maximum_error_ppm = 1'000U,
    .rise_time_ns = 300U,
    .fall_time_ns = 300U,
});
constexpr auto i2s_timing = backend::esp32p4::solve_i2s_timing({
    .clock_hz = 40'000'000U,
    .sample_rate_hz = 48'000U,
    .mclk_multiple = 256U,
    .slot_bits = 16U,
    .slot_count = 2U,
    .maximum_error_ppm = 1U,
});
constexpr auto codec_address = driver::make_i2c_address(
    board::esp32p4_nano::audio_codec_i2c_address);
static_assert(i2c_timing.has_value());
static_assert(i2s_timing.has_value());
static_assert(codec_address.has_value());

constexpr std::size_t completion_timeout_us = 100'000U;
constexpr std::uint32_t tone_frequency_hz = 1'000U;
constexpr std::int16_t tone_peak = 6'000;
constexpr std::size_t tone_repetitions = 48U;
constexpr std::size_t amplifier_settle_us = 20'000U;
constexpr std::size_t codec_settle_us = 5'000U;
// The RX buffer occupies complete L1 cache lines. ESP-IDF deliberately
// rejects unaligned memory-to-cache invalidation because it could discard
// unrelated dirty data sharing the first or last line.
alignas(64) std::array<std::int16_t, 2'016> microphone_samples{};
alignas(64) std::array<std::int16_t, 2'016> tone_samples{};
constexpr std::size_t tone_block_bytes =
    tone_samples.size() * sizeof(std::int16_t);
constexpr std::size_t tone_playback_bytes =
    tone_block_bytes * tone_repetitions;
constexpr std::size_t tone_duration_ms =
    (tone_samples.size() / 2U) * tone_repetitions * 1'000U / 48'000U;
static_assert(tone_duration_ms == 1'008U);

[[nodiscard]] bool write_codec(i2c_backend_type& i2c,
                               device::es8311::register_write value) noexcept {
    const std::array<std::uint8_t, 2> bytes{value.address, value.value};
    auto started = i2c.write(*codec_address, bytes);
    if (!started) return false;
    auto transfer = std::move(*started);
    for (std::size_t elapsed = 0U;
         elapsed < completion_timeout_us && !transfer.ready(); ++elapsed) {
        static_cast<void>(i2c.handle_interrupt());
        if (!transfer.ready()) esp_rom_delay_us(1U);
    }
    if (!transfer.ready()) {
        static_cast<void>(transfer.cancel());
        return false;
    }
    const auto result = transfer.take();
    return result.has_value() && result->bytes_written == bytes.size()
        && result->bytes_read == 0U;
}

template<typename Transfer, typename Backend>
[[nodiscard]] bool await_transfer(Transfer& transfer,
                                  Backend& backend) noexcept {
    for (std::size_t elapsed = 0U;
         elapsed < completion_timeout_us && !transfer.ready(); ++elapsed) {
        static_cast<void>(backend.handle_interrupt());
        if (!transfer.ready()) esp_rom_delay_us(1U);
    }
    return transfer.ready();
}

void make_audible_tone() noexcept {
    // 1 kHz triangle at 48 kHz. Duplicate each sample into both I2S slots.
    for (std::size_t frame = 0U; frame < tone_samples.size() / 2U; ++frame) {
        const auto phase = static_cast<std::int32_t>(frame % 48U);
        const auto rising = phase < 24 ? phase : 48 - phase;
        const auto sample = static_cast<std::int16_t>(
            (rising - 12) * (tone_peak / 12));
        tone_samples[frame * 2U] = sample;
        tone_samples[frame * 2U + 1U] = sample;
    }
}

struct sample_observation {
    std::int16_t minimum{};
    std::int16_t maximum{};
    std::uint32_t nonzero{};
};

[[nodiscard]] sample_observation observe_samples() noexcept {
    sample_observation result{microphone_samples[0], microphone_samples[0], 0U};
    for (const auto sample : microphone_samples) {
        if (sample < result.minimum) result.minimum = sample;
        if (sample > result.maximum) result.maximum = sample;
        if (sample != 0) ++result.nonzero;
    }
    return result;
}

} // namespace

TEST(esp32p4_audio, captures_onboard_microphone_and_plays_audible_tone,
     "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0013", "REQ-0014",
     "REQ-0017", "REQ-0019", "REQ-0024", "REQ-0034", "REQ-0036") {
    core::direct_mmio mmio;
    execution::fake_policy i2c_execution;
    execution::fake_policy i2s_execution;
    dma_memory memory;
    i2c_backend_type i2c{mmio, i2c_execution};
    i2s_backend_type i2s{mmio, i2s_execution, memory};
    backend::esp32p4::gpio_backend amplifier_gpio{mmio};
    driver::gpio_pin amplifier{amplifier_gpio,
                               board::esp32p4_nano::audio_amplifier_enable};

    ASSERT_TRUE(amplifier.initialize({
        .direction = driver::gpio_direction::output,
        .initial_level = driver::gpio_level::low,
        .output_type = driver::gpio_output_type::push_pull,
        .pull = driver::gpio_pull::none,
        .drive = driver::gpio_drive::middle,
    }).has_value());
    ASSERT_TRUE(i2s.initialize(*i2s_timing).has_value());
    ASSERT_TRUE(i2c.initialize(*i2c_timing).has_value());

    bool codec_configured = true;
    for (const auto value : device::es8311::startup_sequence_48khz_16bit()) {
        codec_configured = codec_configured && write_codec(i2c, value);
    }
    codec_configured = codec_configured
        && write_codec(i2c, device::es8311::unity_output_volume)
        && write_codec(i2c, device::es8311::mute_dac);
    ASSERT_TRUE(codec_configured);
    const auto cleanup = [&]() noexcept {
        static_cast<void>(amplifier.clear());
        static_cast<void>(write_codec(i2c, device::es8311::mute_dac));
        static_cast<void>(i2c.shutdown());
        static_cast<void>(i2s.shutdown());
        static_cast<void>(amplifier.release());
    };
    if (!codec_configured) {
        cleanup();
        return;
    }

    microphone_samples.fill(0);
    auto receive_started = i2s.receive(microphone_samples);
    ASSERT_TRUE(receive_started.has_value());
    if (!receive_started) {
        cleanup();
        return;
    }
    auto receive = std::move(*receive_started);
    ASSERT_TRUE(await_transfer(receive, i2s));
    if (!receive.ready()) {
        static_cast<void>(receive.cancel());
        cleanup();
        return;
    }
    const auto received = receive.take();
    ASSERT_TRUE(received.has_value());
    if (!received) {
        cleanup();
        return;
    }
    ASSERT_EQ(received->bytes, microphone_samples.size() * sizeof(std::int16_t));
    if (received->bytes != microphone_samples.size() * sizeof(std::int16_t)) {
        cleanup();
        return;
    }
    const auto observation = observe_samples();
    ASSERT_TRUE(observation.nonzero != 0U);
    if (observation.nonzero == 0U) {
        cleanup();
        return;
    }

    make_audible_tone();
    const bool amplifier_enabled = amplifier.set().has_value();
    ASSERT_TRUE(amplifier_enabled);
    if (!amplifier_enabled) {
        cleanup();
        return;
    }
    esp_rom_delay_us(amplifier_settle_us);
    const bool codec_unmuted =
        write_codec(i2c, device::es8311::unmute_dac);
    ASSERT_TRUE(codec_unmuted);
    if (!codec_unmuted) {
        cleanup();
        return;
    }
    esp_rom_delay_us(codec_settle_us);

    std::size_t playback_bytes{};
    bool playback_complete = true;
    for (std::size_t repetition = 0U; repetition < tone_repetitions;
         ++repetition) {
        auto transmit_started = i2s.transmit(tone_samples);
        if (!transmit_started) {
            playback_complete = false;
            break;
        }
        auto transmit = std::move(*transmit_started);
        if (!await_transfer(transmit, i2s)) {
            static_cast<void>(transmit.cancel());
            playback_complete = false;
            break;
        }
        const auto transmitted = transmit.take();
        if (!transmitted || transmitted->bytes != tone_block_bytes) {
            playback_complete = false;
            break;
        }
        playback_bytes += transmitted->bytes;
    }

    const bool codec_muted = write_codec(i2c, device::es8311::mute_dac);
    esp_rom_delay_us(codec_settle_us);
    const bool amplifier_muted = amplifier.clear().has_value();
    ASSERT_TRUE(playback_complete);
    ASSERT_EQ(playback_bytes, tone_playback_bytes);
    ASSERT_TRUE(codec_muted);
    ASSERT_TRUE(amplifier_muted);
    if (!playback_complete || playback_bytes != tone_playback_bytes
        || !codec_muted || !amplifier_muted) {
        cleanup();
        return;
    }

    esp_rom_printf(
        "MCUBSP_AUDIO backend=esp32p4 i2s=0 codec=ES8311 rate=48000 "
        "format=philips16 capture_bytes=%u mic_nonzero=%u mic_min=%d "
        "mic_max=%d playback_bytes=%u tone_hz=%u tone_peak=%d "
        "duration_ms=%u amp_gpio=53 "
        "verification=dma_codec_readback_no_acoustic_oracle\n",
        static_cast<unsigned>(received->bytes),
        static_cast<unsigned>(observation.nonzero),
        static_cast<int>(observation.minimum),
        static_cast<int>(observation.maximum),
        static_cast<unsigned>(playback_bytes),
        static_cast<unsigned>(tone_frequency_hz),
        static_cast<int>(tone_peak),
        static_cast<unsigned>(tone_duration_ms));
    ASSERT_TRUE(i2c.shutdown().has_value());
    ASSERT_TRUE(i2s.shutdown().has_value());
    ASSERT_TRUE(amplifier.release().has_value());
}

} // namespace mcubsp::board_test::esp32p4_nano
