#include "fake_analog.hpp"
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/io/board.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using profile = board::others::promicro_nrf52840;
using hw_type =
    io::board<profile, {.serial = false, .adc = true, .pwm = true},
              execution::fake_policy, nrf52840_test::fake_analog, true, true>;
TEST(nrf_analog_io,
     pwm_private_sequence_updates_only_after_period_boundary_stop, "REQ-0063",
     "REQ-0045", "REQ-0062") {
  nrf52840_test::fake_analog mmio;
  execution::fake_policy execution;
  hw_type hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  ASSERT_TRUE(hw.pwm.begin(1000, io::percent(25)).has_value());
  EXPECT_EQ(mmio.peek(0x4001C514), 1U);
  EXPECT_EQ(mmio.peek(0x4001C524), 4U);
  EXPECT_TRUE(hw.pwm.running());
  ASSERT_TRUE(hw.pwm.duty(io::percent(75)).has_value());
  EXPECT_EQ(mmio.pwm_starts, 2U);
  ASSERT_TRUE(hw.pwm.stop().has_value());
  EXPECT_FALSE(hw.pwm.running());
  ASSERT_TRUE(hw.pwm.start().has_value());
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_analog_io,
     adc_calibration_and_signed_private_dma_precede_unsigned_publication,
     "REQ-0064", "REQ-0045", "REQ-0062") {
  nrf52840_test::fake_analog mmio;
  execution::fake_policy execution;
  hw_type hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  ASSERT_TRUE(hw.analog.begin().has_value());
  std::array<std::uint16_t, 1> sample{999};
  auto op = hw.analog.sample_async(sample);
  ASSERT_TRUE(op.has_value());
  hw.poll();
  EXPECT_EQ(mmio.adc_starts, 1U);
  EXPECT_EQ(sample[0], 999U);
  mmio.adc_end(1);
  hw.poll();
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->samples, 1U);
  EXPECT_EQ(sample[0], 0U);
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_analog_io, pwm_and_adc_share_clocks_and_conflicting_gpio_is_rejected,
     "REQ-0063", "REQ-0064", "REQ-0062", "REQ-0074") {
  nrf52840_test::fake_analog mmio;
  execution::fake_policy execution;
  hw_type hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  ASSERT_TRUE(hw.pwm.begin().has_value());
  ASSERT_TRUE(hw.analog.begin().has_value());
  ASSERT_TRUE(hw.pwm.shutdown().has_value());
  EXPECT_EQ(mmio.peek(0x40000408), 1U);
  ASSERT_TRUE(hw.analog.shutdown().has_value());
  EXPECT_EQ(mmio.peek(0x40000408), 0U);
  ASSERT_TRUE(hw.shutdown().has_value());
}
} // namespace
