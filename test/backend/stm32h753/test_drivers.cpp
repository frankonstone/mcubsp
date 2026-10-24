#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <span>
#include <type_traits>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/stm32h753/adc.hpp>
#include <mcubsp/backend/stm32h753/i2c.hpp>
#include <mcubsp/backend/stm32h753/memory.hpp>
#include <mcubsp/backend/stm32h753/spi.hpp>
#include <mcubsp/backend/stm32h753/timer.hpp>
#include <mcubsp/backend/stm32h753/usart.hpp>
#include <mcubsp/board/nucleo_h753zi.hpp>
#include <mcubsp/driver/adc.hpp>
#include <mcubsp/driver/i2c.hpp>
#include <mcubsp/driver/pwm.hpp>
#include <mcubsp/driver/spi.hpp>
#include <mcubsp/driver/uart.hpp>
#include <mcubsp/execution/fake_policy.hpp>

namespace {

class sparse_mmio {
    struct cell { std::uintptr_t address{}; std::uint32_t value{}; bool used{}; };
public:
    template<typename Value>
        requires(std::is_unsigned_v<Value>)
    [[nodiscard]] std::expected<Value, mcubsp::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        return static_cast<Value>(value(address));
    }

    template<typename Value>
        requires(std::is_unsigned_v<Value>)
    [[nodiscard]] std::expected<void, mcubsp::core::mmio_error>
    write(std::uintptr_t address, Value next) noexcept {
        auto stored = static_cast<std::uint32_t>(next);
        constexpr std::uintptr_t adc_isr = 0x4002'2000U;
        constexpr std::uintptr_t adc_cr = 0x4002'2008U;
        if (address == adc_isr) {
            set(address, value(address) & ~stored);
            return {};
        }
        if (address == adc_cr) {
            if ((stored & (1U << 31U)) != 0U) stored &= ~(1U << 31U);
            set(address, stored);
            if ((stored & (1U << 28U)) != 0U) {
                set(adc_isr, value(adc_isr) | (1U << 12U));
            }
            if ((stored & 1U) != 0U) set(adc_isr, value(adc_isr) | 1U);
            return {};
        }
        set(address, stored);
        return {};
    }

    void set(std::uintptr_t address, std::uint32_t next) noexcept {
        for (auto& current : cells_) {
            if (current.used && current.address == address) {
                current.value = next; return;
            }
        }
        for (auto& current : cells_) {
            if (!current.used) {
                current = {address, next, true}; return;
            }
        }
    }

    [[nodiscard]] std::uint32_t value(std::uintptr_t address) const noexcept {
        for (const auto& current : cells_) {
            if (current.used && current.address == address) return current.value;
        }
        return 0U;
    }
private:
    std::array<cell, 256> cells_{};
};

namespace stm = mcubsp::mcu::stm32h753;
namespace board = mcubsp::board::nucleo_h753zi;
using execution = mcubsp::execution::fake_policy;

using timer_binding = mcubsp::backend::stm32h753::timer_binding<
    1, stm::timer_output::ch1, board::timer1_ch1>;
using timer_backend = mcubsp::backend::stm32h753::timer_pwm_backend<
    timer_binding, sparse_mmio, execution>;
using usart_binding = mcubsp::backend::stm32h753::usart_binding<
    3, board::stlink_vcp_rx, board::stlink_vcp_tx>;
using usart_backend = mcubsp::backend::stm32h753::usart_backend<
    usart_binding, sparse_mmio, execution>;
using i2c_binding = mcubsp::backend::stm32h753::i2c_binding<
    1, board::arduino_d15_scl, board::arduino_d14_sda>;
using i2c_backend = mcubsp::backend::stm32h753::i2c_controller_backend<
    i2c_binding, sparse_mmio, execution>;
using spi_binding = mcubsp::backend::stm32h753::spi_binding<
    1, board::arduino_d12_miso, board::arduino_d11_mosi,
    board::arduino_d13_sck, board::arduino_d10_cs>;
using spi_backend = mcubsp::backend::stm32h753::spi_master_backend<
    spi_binding, sparse_mmio, execution>;
using adc_binding = mcubsp::backend::stm32h753::adc1_binding<
    mcubsp::backend::stm32h753::adc_input<15, board::arduino_a0>>;
using adc_backend = mcubsp::backend::stm32h753::adc_backend<
    adc_binding, sparse_mmio, execution>;

TEST(stm32h753_solvers, produce_native_timer_uart_i2c_and_spi_encodings,
     "REQ-0011", "REQ-0022", "REQ-0024", "REQ-0027", "REQ-0028") {
    const auto timer = mcubsp::backend::stm32h753::solve_timer_pwm(
        {64'000'000U, 20'000U, 0U, 16U});
    const auto uart = mcubsp::backend::stm32h753::solve_usart_baud(
        {64'000'000U, 115'200U, 1'000U});
    const auto i2c = mcubsp::backend::stm32h753::solve_stm32_i2c_timing(
        {64'000'000U, 100'000U, 20'000U, 400U, 100U});
    const auto spi = mcubsp::backend::stm32h753::solve_stm32_spi_timing(
        {64'000'000U, 8'000'000U, 0U});
    ASSERT_TRUE(timer.has_value());
    EXPECT_EQ(timer->period_counts, 3200U);
    ASSERT_TRUE(uart.has_value());
    EXPECT_EQ(uart->brr, 556U);
    ASSERT_TRUE(i2c.has_value());
    EXPECT_TRUE(i2c->timingr != 0U);
    ASSERT_TRUE(spi.has_value());
    EXPECT_EQ(spi->prescaler_encoding, 2U);
}

TEST(stm32h753_timer, drives_pwm_and_publishes_update_event,
     "REQ-0014", "REQ-0022", "REQ-0033") {
    sparse_mmio mmio;
    execution policy;
    timer_backend backend{mmio, policy};
    mcubsp::driver::pwm_channel pwm{backend};
    const auto timing = mcubsp::backend::stm32h753::solve_timer_pwm(
        {64'000'000U, 20'000U, 0U, 16U});
    ASSERT_TRUE(timing.has_value());
    ASSERT_TRUE(pwm.initialize(*timing, mcubsp::driver::pwm_duty::percent(25U)));
    EXPECT_EQ(mmio.value(0x4001'0034U), 800U);
    ASSERT_TRUE(pwm.start());
    auto pending = pwm.next_period();
    ASSERT_TRUE(pending.has_value());
    auto event = std::move(*pending);
    mmio.set(0x4001'0010U, 1U);
    EXPECT_TRUE(backend.handle_update_interrupt());
    EXPECT_TRUE(event.take().has_value());
    ASSERT_TRUE(pwm.stop());
    ASSERT_TRUE(pwm.release());
}

TEST(stm32h753_usart, completes_vcp_transmit_only_after_tc,
     "REQ-0014", "REQ-0027", "REQ-0033") {
    sparse_mmio mmio;
    execution policy;
    usart_backend backend{mmio, policy};
    const auto timing = mcubsp::backend::stm32h753::solve_usart_baud(
        {64'000'000U, 115'200U, 1'000U});
    ASSERT_TRUE(timing.has_value());
    ASSERT_TRUE(backend.initialize(*timing, {}));
    const std::array<std::uint8_t, 2> bytes{0x4DU, 0x37U};
    auto started = backend.transmit(bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    mmio.set(0x4000'481CU, 1U << 7U);
    EXPECT_TRUE(backend.handle_interrupt());
    EXPECT_TRUE(backend.handle_interrupt());
    EXPECT_FALSE(transfer.ready());
    mmio.set(0x4000'481CU, 1U << 6U);
    EXPECT_TRUE(backend.handle_interrupt());
    EXPECT_TRUE(transfer.take().has_value());
    ASSERT_TRUE(backend.shutdown());
}

TEST(stm32h753_i2c, performs_write_and_stops_at_autoend,
     "REQ-0014", "REQ-0024", "REQ-0025", "REQ-0033") {
    sparse_mmio mmio;
    execution policy;
    i2c_backend backend{mmio, policy};
    const auto timing = mcubsp::backend::stm32h753::solve_stm32_i2c_timing(
        {64'000'000U, 100'000U, 20'000U, 400U, 100U});
    ASSERT_TRUE(timing.has_value());
    ASSERT_TRUE(backend.initialize(*timing));
    const std::array<std::uint8_t, 1> byte{0xA5U};
    auto started = backend.write({0x50U}, byte);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    mmio.set(0x4000'5418U, 1U << 1U);
    EXPECT_TRUE(backend.handle_interrupt());
    EXPECT_EQ(mmio.value(0x4000'5428U), 0xA5U);
    mmio.set(0x4000'5418U, 1U << 5U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, 1U);
    ASSERT_TRUE(backend.shutdown());
}

TEST(stm32h753_spi, holds_software_cs_until_eot,
     "REQ-0014", "REQ-0028", "REQ-0033") {
    sparse_mmio mmio;
    execution policy;
    spi_backend backend{mmio, policy};
    const auto timing = mcubsp::backend::stm32h753::solve_stm32_spi_timing(
        {64'000'000U, 8'000'000U, 0U});
    ASSERT_TRUE(timing.has_value());
    ASSERT_TRUE(backend.initialize(*timing));
    const std::array<std::uint16_t, 1> output{0x5AU};
    std::array<std::uint16_t, 1> input{};
    const std::array<mcubsp::driver::spi_segment, 1> segments{{{output, input, 0U}}};
    auto started = backend.transaction(segments);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    mmio.set(0x4001'3014U, 1U << 1U);
    EXPECT_TRUE(backend.handle_interrupt());
    mmio.set(0x4001'3030U, 0xA5U);
    mmio.set(0x4001'3014U, 1U);
    EXPECT_TRUE(backend.handle_interrupt());
    mmio.set(0x4001'3014U, 1U << 3U);
    EXPECT_TRUE(backend.handle_interrupt());
    ASSERT_TRUE(transfer.take().has_value());
    EXPECT_EQ(input[0], 0xA5U);
    ASSERT_TRUE(backend.shutdown());
}

TEST(stm32h753_adc, calibrates_and_converts_bound_a0_input,
     "REQ-0014", "REQ-0029", "REQ-0033") {
    sparse_mmio mmio;
    execution policy;
    adc_backend backend{mmio, policy};
    ASSERT_TRUE(backend.initialize({}));
    const std::array<mcubsp::driver::adc_channel, 1> channels{{{15U}}};
    std::array<std::uint16_t, 1> results{};
    auto started = backend.convert(channels, results);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    mmio.set(0x4002'2040U, 0x2A5U);
    mmio.set(0x4002'2000U, (1U << 2U) | (1U << 3U));
    EXPECT_TRUE(backend.handle_conversion_interrupt());
    ASSERT_TRUE(transfer.take().has_value());
    EXPECT_EQ(results[0], 0x2A5U);
    ASSERT_TRUE(backend.shutdown());
}

TEST(stm32h753_dma_memory, rejects_tcm_and_plans_cache_maintenance,
     "REQ-0009", "REQ-0013", "REQ-0033") {
    using mcubsp::backend::stm32h753::dma_buffer_access;
    const auto dtcm = mcubsp::backend::stm32h753::plan_dma_buffer(
        mcubsp::core::dma_engine_kind::stm32_dma, 0x2000'0000U, 32U,
        dma_buffer_access::dma_reads);
    EXPECT_FALSE(dtcm.has_value());
    const auto unaligned = mcubsp::backend::stm32h753::plan_dma_buffer(
        mcubsp::core::dma_engine_kind::stm32_dma, 0x2400'0001U, 32U,
        dma_buffer_access::dma_reads);
    EXPECT_FALSE(unaligned.has_value());
    const auto axi = mcubsp::backend::stm32h753::plan_dma_buffer(
        mcubsp::core::dma_engine_kind::stm32_dma, 0x2400'0000U, 64U,
        dma_buffer_access::bidirectional);
    ASSERT_TRUE(axi.has_value());
    EXPECT_TRUE(axi->clean_before);
    EXPECT_TRUE(axi->invalidate_before);
    EXPECT_TRUE(axi->invalidate_after);
    sparse_mmio mmio;
    mcubsp::backend::stm32h753::dma_cache_maintenance maintenance{mmio};
    ASSERT_TRUE(maintenance.prepare(*axi));
    ASSERT_TRUE(maintenance.complete(*axi));
}

} // namespace
