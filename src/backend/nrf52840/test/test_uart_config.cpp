#include <mcutest/mcutest.hpp>
#include <mcubsp/backend/nrf52840/uart_config.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
TEST(nrf_uart_config, legacy_uart_and_uarte_keep_different_actual_baud_and_encodings,"REQ-0055","REQ-0059") {
    const auto legacy=solve_uart_baud(peripheral::uart0,{16'000'000,115200,10000});
    const auto dma=solve_uart_baud(peripheral::uarte0,{16'000'000,115200,10000});
    ASSERT_TRUE(legacy.has_value());ASSERT_TRUE(dma.has_value());
    EXPECT_EQ(legacy->encoding,0x01D7E000U);EXPECT_EQ(legacy->achieved_baud,115942U);
    EXPECT_EQ(dma->encoding,0x01D60000U);EXPECT_EQ(dma->achieved_baud,115108U);
    EXPECT_FALSE(solve_uart_baud(peripheral::uart0,{16'000'000,115200,1000}).has_value());
    EXPECT_TRUE(solve_uart_baud(peripheral::uarte1,{16'000'000,115200,1000}).has_value());
    EXPECT_EQ(solve_uart_baud(peripheral::uarte1,{16'000'000,250000,0})->achieved_baud,250000U);
    EXPECT_FALSE(solve_uart_baud(peripheral::uarte0,{16'000'000,921600,10000}).has_value());
}
TEST(nrf_uart_config, baud_solver_rejects_invalid_clocks_rates_and_instances,"REQ-0059") {
    EXPECT_FALSE(solve_uart_baud(peripheral::uarte0,{64'000'000,115200,20000}).has_value());
    EXPECT_FALSE(solve_uart_baud(peripheral::uarte0,{16'000'000,0,20000}).has_value());
    EXPECT_FALSE(solve_uart_baud(peripheral::uarte0,{16'000'000,1000001,20000}).has_value());
    EXPECT_FALSE(solve_uart_baud(peripheral::spi0,{16'000'000,115200,20000}).has_value());
}
TEST(nrf_uart_config, framing_and_low_frequency_pad_limits_follow_the_selected_personality,"REQ-0059") {
    EXPECT_TRUE(uart_format_supported(peripheral::uart0,{}));
    EXPECT_FALSE(uart_format_supported(peripheral::uart0,{.stop_bits=driver::uart_stop_bits::two}));
    EXPECT_TRUE(uart_format_supported(peripheral::uarte0,{.parity=driver::uart_parity::even,.stop_bits=driver::uart_stop_bits::two}));
    EXPECT_FALSE(uart_format_supported(peripheral::uarte0,{.parity=driver::uart_parity::odd}));
    EXPECT_FALSE(uart_format_supported(peripheral::uarte0,{.data_bits=driver::uart_data_bits::seven}));
    using fast=uart_binding<peripheral::uarte1,{0,6},{0,8}>;
    static_assert(fast::base==0x40028000);
    EXPECT_TRUE(fast::supports_rate(1000000));
    using restricted=uart_binding<peripheral::uart0,{0,2},{0,3}>;
    EXPECT_TRUE(restricted::supports_rate(1205));
    EXPECT_FALSE(restricted::supports_rate(115942));
}
} // namespace
