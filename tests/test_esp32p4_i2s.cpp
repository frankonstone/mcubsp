#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/esp32p4/i2s.hpp>
#include <mcuhw/core/mmio.hpp>
#include <mcuhw/device/audio/es8311.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {

class sparse_mmio {
public:
    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        }
        if (fail_) return std::unexpected(mcuhw::core::mmio_error::invalid_value);
        return static_cast<Value>(value(address));
    }

    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t address, Value next) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        }
        if (fail_) return std::unexpected(mcuhw::core::mmio_error::invalid_value);
        auto word = static_cast<std::uint32_t>(next);
        if ((address == mcuhw::mcu::esp32p4::i2s0_base + 0x20U
             || address == mcuhw::mcu::esp32p4::i2s0_base + 0x24U)
            && (word & (1U << 8U)) != 0U) {
            word &= ~(1U << 8U);
        }
        const auto dma = mcuhw::mcu::esp32p4::gdma_ahb_base;
        if (address == dma + 0x0CU) {
            set(dma, value(dma) & ~word);
            return {};
        }
        if (address == dma + 0x3CU) {
            set(dma + 0x30U, value(dma + 0x30U) & ~word);
            return {};
        }
        set(address, word);
        return {};
    }

    void set(std::uintptr_t address, std::uint32_t next) noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) {
                cells_[index].value = next;
                return;
            }
        }
        if (count_ < cells_.size()) cells_[count_++] = {address, next};
    }

    [[nodiscard]] std::uint32_t value(std::uintptr_t address) const noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) return cells_[index].value;
        }
        return 0U;
    }

    void fail(bool enabled = true) noexcept { fail_ = enabled; }

private:
    struct cell { std::uintptr_t address{}; std::uint32_t value{}; };
    std::array<cell, 160> cells_{};
    std::size_t count_{};
    bool fail_{};
};

class fake_dma_memory {
public:
    [[nodiscard]] std::expected<std::uint32_t, mcuhw::driver::i2s_error>
    address(const void*) noexcept {
        const auto result = next_;
        next_ += 0x100U;
        return result;
    }
    [[nodiscard]] bool prepare_for_device(const void*, std::size_t) noexcept {
        ++prepared_;
        return !fail_;
    }
    [[nodiscard]] bool complete_from_device(void*, std::size_t) noexcept {
        ++completed_;
        return !fail_;
    }
    void fail(bool enabled = true) noexcept { fail_ = enabled; }
    [[nodiscard]] std::size_t prepared() const noexcept { return prepared_; }
    [[nodiscard]] std::size_t completed() const noexcept { return completed_; }

private:
    std::uint32_t next_{0x4FF0'1000U};
    std::size_t prepared_{};
    std::size_t completed_{};
    bool fail_{};
};

using binding = mcuhw::backend::esp32p4::i2s_binding<
    0, mcuhw::core::pin_id{0, 13}, mcuhw::core::pin_id{0, 12},
    mcuhw::core::pin_id{0, 10}, mcuhw::core::pin_id{0, 9},
    mcuhw::core::pin_id{0, 11}>;
using backend_type = mcuhw::backend::esp32p4::i2s_backend<
    binding, sparse_mmio, mcuhw::execution::fake_policy, fake_dma_memory>;

constexpr auto timing_48k = mcuhw::backend::esp32p4::solve_i2s_timing({
    .clock_hz = 40'000'000U,
    .sample_rate_hz = 48'000U,
    .mclk_multiple = 256U,
    .slot_bits = 16U,
    .slot_count = 2U,
    .maximum_error_ppm = 1U,
});
static_assert(timing_48k.has_value());

TEST(esp32p4_i2s, solves_exact_48khz_codec_clock,
     "REQ-0011", "REQ-0034") {
    ASSERT_TRUE(timing_48k.has_value());
    EXPECT_EQ(timing_48k->achieved_sample_rate_hz, 48'000U);
    EXPECT_EQ(timing_48k->mclk_hz, 12'288'000U);
    EXPECT_EQ(timing_48k->bclk_hz, 1'536'000U);
    EXPECT_EQ(timing_48k->divider_integer, 3U);
    EXPECT_EQ(timing_48k->divider_numerator, 49U);
    EXPECT_EQ(timing_48k->divider_denominator, 192U);
    EXPECT_EQ(timing_48k->bclk_divider, 8U);
    EXPECT_FALSE(mcuhw::backend::esp32p4::solve_i2s_timing(
        {0U, 48'000U, 256U, 16U, 2U, 0U}).has_value());
    EXPECT_FALSE(mcuhw::backend::esp32p4::solve_i2s_timing(
        {40'000'000U, 48'000U, 256U, 24U, 2U, 0U}).has_value());
}

TEST(esp32p4_i2s, configures_philips_clocks_slots_and_gpio_matrix,
     "REQ-0010", "REQ-0011", "REQ-0013", "REQ-0034") {
    sparse_mmio mmio;
    fake_dma_memory memory;
    mcuhw::execution::fake_policy execution;
    backend_type i2s{mmio, execution, memory};
    ASSERT_TRUE(i2s.initialize(*timing_48k).has_value());
    EXPECT_TRUE((mmio.value(mcuhw::mcu::esp32p4::hp_soc_clock_ctrl1)
                 & (1U << 3U)) != 0U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::hp_rst_en1)
                  & (1U << 1U),
              0U);
    EXPECT_TRUE((mmio.value(mcuhw::mcu::esp32p4::hp_soc_clock_ctrl2)
                 & (1U << 14U)) != 0U);
    EXPECT_TRUE((mmio.value(mcuhw::mcu::esp32p4::i2s0_base + 0x20U)
                 & (1U << 19U)) != 0U);
    EXPECT_TRUE((mmio.value(mcuhw::mcu::esp32p4::i2s0_base + 0x24U)
                 & (1U << 13U)) != 0U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::i2s0_base + 0x28U)
                  & 0x1FFU, 15U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + binding::mclk_pin.index * 4U) & 0x1FFU,
              binding::mclk_matrix_signal);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gpio_matrix_base + 0x158U
                         + binding::data_matrix_signal * 4U) & 0x3FU,
              binding::data_in_pin.index);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gdma_ahb_base + 0xA0U), 3U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gdma_ahb_base + 0x100U), 3U);
    EXPECT_TRUE(i2s.shutdown().has_value());
}

TEST(esp32p4_i2s, completes_finite_receive_and_transmit_dma,
     "REQ-0009", "REQ-0013", "REQ-0014", "REQ-0017", "REQ-0019") {
    sparse_mmio mmio;
    fake_dma_memory memory;
    mcuhw::execution::fake_policy execution;
    backend_type i2s{mmio, execution, memory};
    ASSERT_TRUE(i2s.initialize(*timing_48k).has_value());
    std::array<std::int16_t, 8> samples{};

    auto receiving_start = i2s.receive(samples);
    ASSERT_TRUE(receiving_start.has_value());
    auto receiving = std::move(*receiving_start);
    EXPECT_FALSE(receiving.ready());
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + binding::bclk_pin.index * 4U) & 0x1FFU,
              binding::rx_bclk_matrix_signal);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + binding::ws_pin.index * 4U) & 0x1FFU,
              binding::rx_ws_matrix_signal);
    mmio.set(mcuhw::mcu::esp32p4::gdma_ahb_base, 1U << 1U);
    EXPECT_TRUE(i2s.handle_interrupt());
    const auto received = receiving.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_TRUE(received->direction == mcuhw::driver::i2s_direction::receive);
    EXPECT_EQ(received->bytes, samples.size() * sizeof(samples[0]));
    EXPECT_EQ(memory.completed(), 1U);

    auto transmitting_start = i2s.transmit(samples);
    ASSERT_TRUE(transmitting_start.has_value());
    auto transmitting = std::move(*transmitting_start);
    EXPECT_TRUE(memory.prepared() >= 3U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + binding::bclk_pin.index * 4U) & 0x1FFU,
              binding::tx_bclk_matrix_signal);
    EXPECT_EQ(mmio.value(mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + binding::ws_pin.index * 4U) & 0x1FFU,
              binding::tx_ws_matrix_signal);
    mmio.set(mcuhw::mcu::esp32p4::gdma_ahb_base + 0x30U, 1U << 3U);
    EXPECT_TRUE(i2s.handle_interrupt());
    const auto transmitted = transmitting.take();
    ASSERT_TRUE(transmitted.has_value());
    EXPECT_TRUE(transmitted->direction
                == mcuhw::driver::i2s_direction::transmit);
    EXPECT_EQ(transmitted->bytes, samples.size() * sizeof(samples[0]));

    auto cancelled_start = i2s.receive(samples);
    ASSERT_TRUE(cancelled_start.has_value());
    auto cancelled = std::move(*cancelled_start);
    ASSERT_TRUE(cancelled.cancel().has_value());
    EXPECT_FALSE(cancelled.take().has_value());
    EXPECT_TRUE(i2s.shutdown().has_value());
}

TEST(es8311, exposes_bounded_48khz_analog_mic_and_dac_sequence,
     "REQ-0011", "REQ-0024", "REQ-0034") {
    constexpr auto sequence = mcuhw::device::es8311::startup_sequence_48khz_16bit();
    static_assert(sequence.size() == 38U);
    EXPECT_TRUE((sequence.front()
                 == mcuhw::device::es8311::register_write{0x0DU, 0xFAU}));
    EXPECT_TRUE((sequence.back()
                 == mcuhw::device::es8311::register_write{0x45U, 0x00U}));
    EXPECT_EQ(mcuhw::device::es8311::conservative_output_volume.address,
              0x32U);
    EXPECT_TRUE(mcuhw::device::es8311::conservative_output_volume.value
                < 0xBFU);
    EXPECT_TRUE((mcuhw::device::es8311::unity_output_volume
                 == mcuhw::device::es8311::register_write{0x32U, 0xBFU}));
}

} // namespace
