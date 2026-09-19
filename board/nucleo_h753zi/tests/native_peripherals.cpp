// @req-file REQ-0009 REQ-0013 REQ-0022 REQ-0024 REQ-0027 REQ-0028 REQ-0029 REQ-0033 REQ-0036

#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/backend/stm32h753/adc.hpp>
#include <mcuhw/backend/stm32h753/gpio.hpp>
#include <mcuhw/backend/stm32h753/i2c.hpp>
#include <mcuhw/backend/stm32h753/memory.hpp>
#include <mcuhw/backend/stm32h753/spi.hpp>
#include <mcuhw/backend/stm32h753/timer.hpp>
#include <mcuhw/backend/stm32h753/usart.hpp>
#include <mcuhw/board/nucleo_h753zi.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/adc.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/driver/pwm.hpp>
#include <mcuhw/execution/fake_policy.hpp>

#include "support.hpp"

extern "C" volatile std::uint32_t mcuhw_nucleo_clock_status;

extern "C" {
volatile std::uint32_t mcuhw_nucleo_native_status;
volatile std::uint32_t mcuhw_nucleo_pwm_arr;
volatile std::uint32_t mcuhw_nucleo_pwm_ccr;
volatile std::uint32_t mcuhw_nucleo_adc_sample;
volatile std::uint32_t mcuhw_nucleo_uart_brr;
alignas(32) volatile std::uint32_t mcuhw_nucleo_cache_buffer[16];
alignas(32) volatile std::uint32_t mcuhw_nucleo_evidence_control[8];
alignas(32) char mcuhw_nucleo_evidence_buffer[8192];
alignas(32) std::uint8_t mcuhw_nucleo_coverage_buffer[32768];
}

namespace {
namespace board = mcuhw::board::nucleo_h753zi;
namespace stm = mcuhw::mcu::stm32h753;
using mmio_type = mcuhw::core::direct_mmio;
using execution_type = mcuhw::execution::fake_policy;
inline constexpr std::uint32_t coverage_build_id = 0x4d48'7501U;

using pwm_binding = mcuhw::backend::stm32h753::timer_binding<
    1, stm::timer_output::ch1, board::timer1_ch1>;
using pwm_backend = mcuhw::backend::stm32h753::timer_pwm_backend<
    pwm_binding, mmio_type, execution_type>;
using uart_binding = mcuhw::backend::stm32h753::usart_binding<
    3, board::stlink_vcp_rx, board::stlink_vcp_tx>;
using uart_backend = mcuhw::backend::stm32h753::usart_backend<
    uart_binding, mmio_type, execution_type>;
using i2c_binding = mcuhw::backend::stm32h753::i2c_binding<
    1, board::arduino_d15_scl, board::arduino_d14_sda>;
using i2c_backend = mcuhw::backend::stm32h753::i2c_controller_backend<
    i2c_binding, mmio_type, execution_type>;
using spi_binding = mcuhw::backend::stm32h753::spi_binding<
    1, board::arduino_d12_miso, board::arduino_d11_mosi,
    board::arduino_d13_sck, board::arduino_d10_cs>;
using spi_backend = mcuhw::backend::stm32h753::spi_master_backend<
    spi_binding, mmio_type, execution_type>;
using adc_binding = mcuhw::backend::stm32h753::adc1_binding<
    mcuhw::backend::stm32h753::adc_input<15, board::arduino_a0>>;
using adc_backend = mcuhw::backend::stm32h753::adc_backend<
    adc_binding, mmio_type, execution_type>;

template<typename Transfer, typename Handler>
[[nodiscard]] bool poll_transfer(Transfer& transfer, Handler&& handler,
                                 std::uint32_t limit = 2'000'000U) noexcept {
    for (std::uint32_t poll = 0U; poll < limit; ++poll) {
        if (transfer.ready()) return transfer.take().has_value();
        (void)handler();
    }
    return false;
}

void enable_data_cache() noexcept {
    constexpr auto scb_ccr = 0xE000'ED14U;
    constexpr auto scb_dcisw = 0xE000'EF60U;
    // STM32H753 Cortex-M7: 16 KiB, 4-way, 32-byte-line D-cache.
    for (std::uint32_t way = 0U; way < 4U; ++way) {
        for (std::uint32_t set = 0U; set < 128U; ++set) {
            *reinterpret_cast<volatile std::uint32_t*>(scb_dcisw) =
                (way << 30U) | (set << 5U);
        }
    }
    mcuhw::arch::data_synchronization_barrier();
    auto control = *reinterpret_cast<volatile std::uint32_t*>(scb_ccr);
    *reinterpret_cast<volatile std::uint32_t*>(scb_ccr) = control | (1U << 16U);
    mcuhw::arch::data_synchronization_barrier();
    mcuhw::arch::instruction_barrier();
}

[[nodiscard]] bool clean_for_swd(mmio_type& mmio, std::uintptr_t address,
                                 std::size_t size) noexcept {
    const auto plan = mcuhw::backend::stm32h753::plan_dma_buffer(
        mcuhw::core::dma_engine_kind::stm32_dma, address, size,
        mcuhw::backend::stm32h753::dma_buffer_access::dma_reads);
    mcuhw::backend::stm32h753::dma_cache_maintenance maintenance{mmio};
    return plan && maintenance.prepare(*plan).has_value();
}

TEST(nucleo_startup, owns_vectors_and_hsi_clock,
     "REQ-0031", "REQ-0033", "REQ-0036") {
    EXPECT_EQ(mcuhw_nucleo_clock_status, 0U);
    EXPECT_EQ(*reinterpret_cast<volatile const std::uint32_t*>(0xE000'ED08U),
              0x0800'0000U);
    EXPECT_EQ(mcuhw_nucleo_native_status & 1U, 1U);
}

TEST(nucleo_gpio, drives_builtin_led_through_common_driver,
     "REQ-0021", "REQ-0033", "REQ-0036") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 7U), 1U << 7U);
}

TEST(nucleo_cache, cleans_aligned_axi_sram_for_dma,
     "REQ-0009", "REQ-0013", "REQ-0033") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 6U), 1U << 6U);
    EXPECT_EQ(mcuhw_nucleo_cache_buffer[0], 0xCACE'0000U);
    EXPECT_EQ(mcuhw_nucleo_cache_buffer[15], 0xCACE'000FU);
}

TEST(nucleo_pwm, configures_tim1_20khz_25_percent,
     "REQ-0013", "REQ-0022", "REQ-0033") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 1U), 1U << 1U);
    EXPECT_EQ(mcuhw_nucleo_pwm_arr, 3199U);
    EXPECT_EQ(mcuhw_nucleo_pwm_ccr, 800U);
}

TEST(nucleo_usart, configures_usart3_vcp_115200,
     "REQ-0013", "REQ-0027", "REQ-0033") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 2U), 1U << 2U);
    EXPECT_EQ(mcuhw_nucleo_uart_brr, 556U);
}

TEST(nucleo_i2c, initializes_arduino_i2c_binding,
     "REQ-0013", "REQ-0024", "REQ-0025", "REQ-0033") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 3U), 1U << 3U);
}

TEST(nucleo_spi, initializes_arduino_spi_binding,
     "REQ-0013", "REQ-0028", "REQ-0033") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 4U), 1U << 4U);
}

TEST(nucleo_adc, calibrates_and_converts_a0,
     "REQ-0013", "REQ-0029", "REQ-0033") {
    EXPECT_EQ(mcuhw_nucleo_native_status & (1U << 5U), 1U << 5U);
    EXPECT_TRUE(mcuhw_nucleo_adc_sample <= 16'383U);
}

constexpr mcuhw::board_test::nucleo_h753zi::evidence_metadata evidence{
    .suite = "native_peripherals",
    .firmware = "pln-0018",
    .fixture = "on-board resources and direct ST-LINK observation",
    .coverage_build_id = coverage_build_id,
};

} // namespace

int main() {
    mmio_type mmio;
    execution_type execution;
    std::uint32_t status = mcuhw_nucleo_clock_status == 0U ? 1U : 0U;

    enable_data_cache();
    for (std::uint32_t index = 0U; index < 16U; ++index) {
        mcuhw_nucleo_cache_buffer[index] = 0xCACE'0000U | index;
    }
    const auto cache_plan = mcuhw::backend::stm32h753::plan_dma_buffer(
        mcuhw::core::dma_engine_kind::stm32_dma,
        reinterpret_cast<std::uintptr_t>(mcuhw_nucleo_cache_buffer),
        sizeof(mcuhw_nucleo_cache_buffer),
        mcuhw::backend::stm32h753::dma_buffer_access::dma_reads);
    mcuhw::backend::stm32h753::dma_cache_maintenance cache{mmio};
    if (cache_plan && cache.prepare(*cache_plan)) status |= 1U << 6U;

    mcuhw::backend::stm32h753::gpio_backend gpio_hardware{mmio};
    mcuhw::driver::gpio_pin led{gpio_hardware, board::user_led_yellow};
    const mcuhw::driver::gpio_configuration led_configuration{
        .direction = mcuhw::driver::gpio_direction::output,
        .initial_level = mcuhw::driver::gpio_level::low,
        .output_type = mcuhw::driver::gpio_output_type::push_pull,
        .pull = mcuhw::driver::gpio_pull::none,
        .drive = mcuhw::driver::gpio_drive::low,
    };
    if (led.initialize(led_configuration) && led.set()) {
        const auto level = led.read();
        if (level && *level == mcuhw::driver::gpio_level::high && led.clear()) {
            status |= 1U << 7U;
        }
    }

    constexpr auto pwm_timing = mcuhw::backend::stm32h753::solve_timer_pwm(
        {64'000'000U, 20'000U, 0U, 16U});
    pwm_backend pwm_hardware{mmio, execution};
    mcuhw::driver::pwm_channel pwm{pwm_hardware};
    if (pwm_timing
        && pwm.initialize(*pwm_timing, mcuhw::driver::pwm_duty::percent(25U))
        && pwm.start()) {
        mcuhw_nucleo_pwm_arr =
            *reinterpret_cast<volatile const std::uint32_t*>(0x4001'002CU);
        mcuhw_nucleo_pwm_ccr =
            *reinterpret_cast<volatile const std::uint32_t*>(0x4001'0034U);
        if (mcuhw_nucleo_pwm_arr == 3199U
            && mcuhw_nucleo_pwm_ccr == 800U) status |= 1U << 1U;
        (void)pwm.stop();
        (void)pwm.release();
    }

    constexpr auto uart_timing = mcuhw::backend::stm32h753::solve_usart_baud(
        {64'000'000U, 115'200U, 1'000U});
    uart_backend uart{mmio, execution};
    if (uart_timing && uart.initialize(*uart_timing, {})) {
        mcuhw_nucleo_uart_brr =
            *reinterpret_cast<volatile const std::uint32_t*>(0x4000'480CU);
        if (mcuhw_nucleo_uart_brr == 556U) status |= 1U << 2U;
    }

    constexpr auto i2c_timing = mcuhw::backend::stm32h753::solve_stm32_i2c_timing(
        {64'000'000U, 100'000U, 20'000U, 400U, 100U});
    i2c_backend i2c{mmio, execution};
    if (i2c_timing && i2c.initialize(*i2c_timing) && i2c.shutdown()) {
        status |= 1U << 3U;
    }

    constexpr auto spi_timing = mcuhw::backend::stm32h753::solve_stm32_spi_timing(
        {64'000'000U, 8'000'000U, 0U});
    spi_backend spi{mmio, execution};
    if (spi_timing && spi.initialize(*spi_timing) && spi.shutdown()) {
        status |= 1U << 4U;
    }

    adc_backend adc{mmio, execution};
    if (adc.initialize({})) {
        const std::array<mcuhw::driver::adc_channel, 1> channels{{{15U}}};
        std::array<std::uint16_t, 1> samples{};
        auto started = adc.convert(channels, samples);
        if (started) {
            auto transfer = std::move(*started);
            if (poll_transfer(transfer, [&adc] {
                    return adc.handle_conversion_interrupt();
                })) {
                mcuhw_nucleo_adc_sample = samples[0];
                status |= 1U << 5U;
            }
        }
        (void)adc.shutdown();
    }

    mcuhw_nucleo_native_status = status;
    const auto captured = mcuhw::board_test::nucleo_h753zi::capture(
        std::span<char>{mcuhw_nucleo_evidence_buffer},
        std::span<std::uint8_t>{mcuhw_nucleo_coverage_buffer},
        evidence, [&](auto& observations) {
            observations("native_status", mcuhw_nucleo_native_status);
            observations("pwm_arr", mcuhw_nucleo_pwm_arr);
            observations("pwm_ccr", mcuhw_nucleo_pwm_ccr);
            observations("adc_sample", mcuhw_nucleo_adc_sample);
            observations("usart3_brr", mcuhw_nucleo_uart_brr);
            observations("board_changes", "none");
        });

    const bool evidence_clean = clean_for_swd(
        mmio, reinterpret_cast<std::uintptr_t>(mcuhw_nucleo_evidence_buffer),
        sizeof(mcuhw_nucleo_evidence_buffer));
    const bool coverage_clean = clean_for_swd(
        mmio, reinterpret_cast<std::uintptr_t>(mcuhw_nucleo_coverage_buffer),
        sizeof(mcuhw_nucleo_coverage_buffer));
    mcuhw_nucleo_evidence_control[0] = 0x4D43'5543U; // MCUC
    mcuhw_nucleo_evidence_control[1] =
        captured.complete && evidence_clean && coverage_clean ? 1U : 0U;
    mcuhw_nucleo_evidence_control[2] =
        static_cast<std::uint32_t>(captured.evidence_size);
    mcuhw_nucleo_evidence_control[3] =
        static_cast<std::uint32_t>(captured.coverage_size);
    mcuhw_nucleo_evidence_control[4] =
        static_cast<std::uint32_t>(captured.test_count);
    mcuhw_nucleo_evidence_control[5] =
        static_cast<std::uint32_t>(captured.failures);
    mcuhw_nucleo_evidence_control[6] = coverage_build_id;
    mcuhw_nucleo_evidence_control[7] = status;
    (void)clean_for_swd(
        mmio,
        reinterpret_cast<std::uintptr_t>(mcuhw_nucleo_evidence_control),
        sizeof(mcuhw_nucleo_evidence_control));

    constexpr std::array<std::uint8_t, 25> message{
        'M','C','U','H','W',' ','H','7','5','3',' ','U','S','A','R','T','3',' ','O','K','\r','\n','\0','\0','\0'};
    const auto bytes = std::span<const std::uint8_t>{message}.first(22U);
    for (;;) {
        if ((status & (1U << 2U)) != 0U) {
            auto started = uart.transmit(bytes);
            if (started) {
                auto transfer = std::move(*started);
                (void)poll_transfer(transfer, [&uart] {
                    return uart.handle_interrupt();
                });
            }
        }
        for (std::uint32_t delay = 0U; delay < 4'000'000U; ++delay) {
            mcuhw::arch::instruction_barrier();
        }
    }
}
