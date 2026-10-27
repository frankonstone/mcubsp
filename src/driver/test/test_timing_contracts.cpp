#include <concepts>
#include <type_traits>

#include <mcutest/mcutest.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
#include <mcubsp/board/st/nucleo_h753zi.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {
using mmio = mcubsp::testing::recording_mmio<64, 16>;
using execution = mcubsp::execution::fake_policy;
using ra_uart = mcubsp::board::arduino::uno_r4_minima::serial_backend<mmio, execution>;
using st_uart = mcubsp::board::st::nucleo_h753zi::serial_backend<mmio, execution>;
using esp_uart = mcubsp::board::waveshare::esp32p4_nano::serial_backend<mmio, execution>;
using ra_spi = mcubsp::board::arduino::uno_r4_minima::spi_backend<mmio, execution>;
using st_spi = mcubsp::board::st::nucleo_h753zi::spi_backend<mmio, execution>;
using esp_spi = mcubsp::board::waveshare::esp32p4_nano::spi_backend<mmio, execution>;

template<class Backend, class Timing>
concept uart_configuration = requires(Backend& backend, Timing timing) {
    backend.initialize(timing, mcubsp::driver::uart_format{});
};
template<class Backend, class Timing>
concept spi_configuration = requires(Backend& backend, Timing timing) {
    backend.initialize(timing, mcubsp::driver::spi_configuration{});
};
template<class T> concept has_native_encoding = requires(T timing) {
    timing.backend_encoding;
};

using ra_baud = mcubsp::backend::ra4m1::uart_baud_timing;
using st_baud = mcubsp::backend::stm32h753::uart_baud_timing;
using esp_baud = mcubsp::backend::esp32p4::uart_baud_timing;
static_assert(uart_configuration<ra_uart, ra_baud>);
static_assert(uart_configuration<st_uart, st_baud>);
static_assert(uart_configuration<esp_uart, esp_baud>);
static_assert(!uart_configuration<ra_uart, st_baud>);
static_assert(!uart_configuration<st_uart, esp_baud>);
static_assert(!uart_configuration<esp_uart, ra_baud>);
static_assert(!uart_configuration<st_uart, mcubsp::driver::uart_baud_timing>);
static_assert(!has_native_encoding<mcubsp::driver::uart_baud_timing>);
static_assert(!has_native_encoding<mcubsp::driver::i2c_timing>);
static_assert(!has_native_encoding<mcubsp::driver::spi_timing>);
static_assert(!spi_configuration<ra_spi, mcubsp::backend::stm32h753::spi_timing>);
static_assert(!spi_configuration<st_spi, mcubsp::backend::esp32p4::spi_timing>);
static_assert(!spi_configuration<esp_spi, mcubsp::backend::ra4m1::spi_timing>);
static_assert(!uart_configuration<mcubsp::driver::uart<st_uart>, ra_baud>);

static_assert(!std::convertible_to<mcubsp::backend::ra4m1::i2c_timing,
                                   mcubsp::backend::stm32h753::i2c_timing>);
static_assert(!std::convertible_to<mcubsp::backend::stm32h753::i2c_timing,
                                   mcubsp::backend::esp32p4::i2c_timing>);
static_assert(!std::convertible_to<mcubsp::backend::ra4m1::pwm_timing,
                                   mcubsp::backend::stm32h753::pwm_timing>);

TEST(timing_contracts, native_solutions_expose_vendor_independent_results,
     "REQ-0003", "REQ-0011") {
    const auto ra = mcubsp::backend::ra4m1::solve_uart_baud({48'000'000U, 115'200U, 20'000U});
    const auto st = mcubsp::backend::stm32h753::solve_usart_baud({64'000'000U, 115'200U, 20'000U});
    const auto esp = mcubsp::backend::esp32p4::solve_uart_baud({40'000'000U, 115'200U, 20'000U});
    ASSERT_TRUE(ra.has_value());
    ASSERT_TRUE(st.has_value());
    ASSERT_TRUE(esp.has_value());
    const mcubsp::driver::uart_baud_timing summaries[]{*ra, *st, *esp};
    for (const auto summary : summaries) {
        EXPECT_EQ(summary.target_baud, 115'200U);
        EXPECT_TRUE(summary.achieved_baud > 113'000U);
        EXPECT_TRUE(summary.achieved_baud < 117'000U);
    }
}

TEST(timing_contracts, uart_tolerances_are_relative_to_requested_baud,
     "REQ-0011", "REQ-0027") {
    // RA: 48 MHz / 42 versus 1.1 Mbaud gives 1.8 / 46.2 = 38961 ppm.
    const auto ra = mcubsp::backend::ra4m1::solve_uart_baud({48'000'000U, 1'100'000U, 38'961U});
    ASSERT_TRUE(ra.has_value());
    EXPECT_EQ(ra->error_ppm, 38'961U);
    EXPECT_FALSE(mcubsp::backend::ra4m1::solve_uart_baud({48'000'000U, 1'100'000U, 38'000U}).has_value());

    // STM: 64 MHz / 58 gives 0.2 / 63.8 = 3135 ppm.
    const auto st = mcubsp::backend::stm32h753::solve_usart_baud({64'000'000U, 1'100'000U, 3'135U});
    ASSERT_TRUE(st.has_value());
    EXPECT_EQ(st->error_ppm, 3'135U);
    EXPECT_FALSE(mcubsp::backend::stm32h753::solve_usart_baud({64'000'000U, 1'100'000U, 3'130U}).has_value());

    // ESP: 640 MHz / 125 versus 5.1 Mbaud gives 2.5 / 637.5 = 3922 ppm.
    const auto esp = mcubsp::backend::esp32p4::solve_uart_baud({40'000'000U, 5'100'000U, 3'922U});
    ASSERT_TRUE(esp.has_value());
    EXPECT_EQ(esp->error_ppm, 3'922U);
    EXPECT_FALSE(mcubsp::backend::esp32p4::solve_uart_baud({40'000'000U, 5'100'000U, 3'910U}).has_value());
}

TEST(timing_contracts, pwm_tolerances_are_relative_to_requested_frequency,
     "REQ-0011", "REQ-0022") {
    // RA: 48 MHz / 7 versus 7 MHz gives 1 / 49 = 20408 ppm.
    const auto ra = mcubsp::backend::ra4m1::solve_pwm_timing({48'000'000U, 7'000'000U, 20'408U, 32U});
    ASSERT_TRUE(ra.has_value());
    EXPECT_EQ(ra->error_ppm, 20'408U);
    EXPECT_FALSE(mcubsp::backend::ra4m1::solve_pwm_timing({48'000'000U, 7'000'000U, 20'407U, 32U}).has_value());

    // STM: 64 MHz / 9 versus 7 MHz gives 1 / 63 = 15873 ppm.
    const auto st = mcubsp::backend::stm32h753::solve_timer_pwm({64'000'000U, 7'000'000U, 15'873U, 32U});
    ASSERT_TRUE(st.has_value());
    EXPECT_EQ(st->error_ppm, 15'873U);
    EXPECT_FALSE(mcubsp::backend::stm32h753::solve_timer_pwm({64'000'000U, 7'000'000U, 15'700U, 32U}).has_value());
}
} // namespace
