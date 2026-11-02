#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/saadc.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
struct fixture {
  nrf52840_test::fake_analog mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<decltype(mmio)> gpio{mmio, resources};
  system_context<decltype(mmio), execution::fake_policy> context{
      mmio, execution, gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy> buffers{resources, execution};
  saadc_controller<decltype(context), decltype(buffers),
                   nrf52840_test::analog_address, 4, 16>
      adc{context, clocks, buffers, 100};
  fixture() {
    mmio.execution = &execution;
    for (unsigned i = 0; i < 3; ++i)
      mmio.poke(0x40007640 + 4 * i, 10 + i);
  }
  ~fixture() {
    (void)adc.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(saadc_configuration config = {}) {
    std::array channels{saadc_channel_configuration{}};
    return clocks.initialize().has_value() &&
           adc.initialize(channels, config).has_value();
  }
  bool calibrate() {
    auto op = adc.calibrate();
    if (!op)
      return false;
    (void)adc.poll();
    return op->take().has_value();
  }
};
TEST(
    nrf_saadc,
    initialization_preserves_trims_configures_signed_channels_and_claims_analog_pins,
    "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  EXPECT_EQ(f.mmio.peek(0x40007640), 10U);
  EXPECT_EQ(f.mmio.peek(0x40007510), 1U);
  EXPECT_EQ(f.mmio.peek(0x400075F0), 2U);
  EXPECT_EQ(f.mmio.peek(mcu::nrf52840::pin_configuration({0, 2})), 2U);
  EXPECT_FALSE(f.gpio.claim({0, 2}, driver::gpio_direction::input).has_value());
  std::array<std::int16_t, 4> samples{};
  EXPECT_FALSE(f.adc.sample(samples).has_value());
  ASSERT_TRUE(f.calibrate());
  EXPECT_TRUE(f.adc.calibrated());
}
TEST(nrf_saadc, calibration_is_excluded_from_sampling_and_uses_at_least_ten_us,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  std::array channels{
      saadc_channel_configuration{.acquisition = saadc_acquisition::us3}};
  ASSERT_TRUE(f.adc.initialize(channels).has_value());
  auto cal = f.adc.calibrate();
  ASSERT_TRUE(cal.has_value());
  EXPECT_EQ((f.mmio.peek(0x40007518) >> 16) & 7U, 2U);
  ASSERT_TRUE(f.adc.poll());
  ASSERT_TRUE(cal->take().has_value());
  EXPECT_EQ((f.mmio.peek(0x40007518) >> 16) & 7U, 0U);
  std::array<std::int16_t, 4> samples{};
  auto op = f.adc.sample(samples);
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(f.adc.calibrate().has_value());
  ASSERT_TRUE(op->cancel().has_value());
  (void)op->take();
  ASSERT_TRUE(f.adc.take_buffer().has_value());
}
TEST(nrf_saadc,
     bounded_stream_requeues_completed_storage_and_reports_actual_total,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.calibrate());
  std::array<std::int16_t, 4> a{}, b{};
  std::array blocks{std::span<std::int16_t>{a}, std::span<std::int16_t>{b}};
  auto op = f.adc.sample_stream(blocks, 10);
  ASSERT_TRUE(op.has_value());
  f.mmio.adc_end(4);
  ASSERT_TRUE(f.adc.poll());
  auto first = f.adc.take_buffer();
  ASSERT_TRUE(first.has_value());
  EXPECT_EQ(first->count, 4U);
  ASSERT_TRUE(f.adc.queue_buffer(a).has_value());
  f.mmio.adc_end(4);
  ASSERT_TRUE(f.adc.poll());
  f.mmio.adc_end(2);
  ASSERT_TRUE(f.adc.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->samples, 10U);
  EXPECT_EQ(f.adc.transferred_count(), 10U);
  auto second = f.adc.take_buffer(), last = f.adc.take_buffer();
  ASSERT_TRUE(second.has_value());
  ASSERT_TRUE(last.has_value());
  EXPECT_EQ(last->count, 2U);
  EXPECT_TRUE(second->sequence < last->sequence);
  EXPECT_EQ(f.mmio.adc_starts, 3U);
}
TEST(nrf_saadc, queue_exhaustion_overlap_and_limit_events_are_observable,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.calibrate());
  std::array<std::int16_t, 4> samples{};
  std::array blocks{std::span<std::int16_t>{samples}};
  auto op = f.adc.sample_stream(blocks, 8);
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(f.adc.queue_buffer(samples).has_value());
  f.mmio.poke(0x40007118, 1);
  f.mmio.poke(0x4000711C, 1);
  f.mmio.adc_end(4);
  ASSERT_TRUE(f.adc.poll());
  auto result = op->take();
  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(result.error().cause == driver::adc_error::conversion_overrun);
  auto limits = f.adc.take_limits();
  EXPECT_EQ(limits.high, 1U);
  EXPECT_EQ(limits.low, 1U);
  EXPECT_EQ(f.adc.take_buffer()->count, 4U);
}
TEST(nrf_saadc, scan_diff_reference_and_timer_limits_are_explicit, "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  std::array channels{
      saadc_channel_configuration{.channel = 3,
                                  .positive = saadc_input::ain4,
                                  .negative = saadc_input::ain5,
                                  .gain = saadc_gain::two,
                                  .differential = true},
      saadc_channel_configuration{.channel = 1, .positive = saadc_input::vdd}};
  EXPECT_FALSE(f.adc.initialize(channels, {.timer_cc = 1600}).has_value());
  ASSERT_TRUE(f.adc.initialize(channels).has_value());
  EXPECT_EQ(f.mmio.peek(0x40007540), 5U);
  EXPECT_EQ(f.mmio.peek(0x40007544), 6U);
  ASSERT_TRUE(f.calibrate());
  std::array<std::int16_t, 3> odd{};
  EXPECT_FALSE(f.adc.sample(odd).has_value());
  std::array<std::int16_t, 4> results{};
  auto op = f.adc.sample(results);
  ASSERT_TRUE(op.has_value());
  f.mmio.adc_end(4);
  ASSERT_TRUE(f.adc.poll());
  ASSERT_TRUE(op->take().has_value());
  (void)f.adc.take_buffer();
}
TEST(nrf_saadc, unsafe_vdd_reference_gain_and_oversize_buffers_are_rejected,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  std::array channels{saadc_channel_configuration{
      .gain = saadc_gain::one, .reference = saadc_reference::vdd_quarter}};
  EXPECT_FALSE(f.adc.initialize(channels).has_value());
  ASSERT_TRUE(
      f.adc.initialize(channels, {.minimum_supply_mv = 3000}).has_value());
  ASSERT_TRUE(f.calibrate());
  std::span<std::int16_t> huge{reinterpret_cast<std::int16_t *>(0x20000000),
                               32768};
  EXPECT_FALSE(f.adc.sample(huge).has_value());
  EXPECT_EQ(f.mmio.adc_starts, 0U);
}
TEST(nrf_saadc, failed_stop_holds_the_partial_buffer_and_clock_until_ack,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.calibrate());
  std::array<std::int16_t, 8> samples{};
  auto op = f.adc.sample(samples);
  ASSERT_TRUE(op.has_value());
  f.mmio.poke(0x40007634, 3);
  f.mmio.stop_responds = false;
  EXPECT_FALSE(op->cancel().has_value());
  (void)op->take();
  EXPECT_FALSE(f.adc.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  f.mmio.poke(0x40007114, 1);
  ASSERT_TRUE(f.adc.shutdown().has_value());
  auto partial = f.adc.take_buffer();
  ASSERT_TRUE(partial.has_value());
  EXPECT_EQ(partial->count, 3U);
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
  EXPECT_EQ(f.mmio.peek(0x40007640), 10U);
}
TEST(nrf_saadc,
     calibration_cancel_waits_for_calibratedone_and_retains_analog_owner,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.calibration_responds = false;
  auto cal = f.adc.calibrate();
  ASSERT_TRUE(cal.has_value());
  EXPECT_FALSE(cal->cancel().has_value());
  (void)cal->take();
  EXPECT_FALSE(f.adc.shutdown().has_value());
  f.mmio.poke(0x40007110, 1);
  ASSERT_TRUE(f.adc.shutdown().has_value());
}
TEST(nrf_saadc, stop_ack_failure_does_not_wait_for_another_hardware_stop,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.calibrate());
  std::array<std::int16_t, 4> samples{};
  auto op = f.adc.sample(samples);
  ASSERT_TRUE(op.has_value());
  f.mmio.adc_end(4);
  f.mmio.poke(0x40007114, 1);
  f.mmio.fail_event_ack = 0x40007114;
  EXPECT_FALSE(f.adc.poll());
  f.mmio.stop_responds = false;
  ASSERT_TRUE(f.adc.poll());
  EXPECT_FALSE(op->take().has_value());
  EXPECT_EQ(f.adc.take_buffer()->count, 4U);
}
} // namespace
