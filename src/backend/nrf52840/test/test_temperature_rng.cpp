#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/rng.hpp>
#include <mcubsp/backend/nrf52840/temperature.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
struct fixture {
  nrf52840_test::fake_system mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<decltype(mmio)> gpio{mmio, resources};
  system_context<decltype(mmio), decltype(execution)> context{mmio, execution,
                                                              gpio};
  clock_controller<decltype(context)> clocks{context};
  temperature_controller<decltype(context)> temperature{context, clocks};
  rng_controller<decltype(context)> rng{context};
  ~fixture() {
    (void)rng.shutdown();
    (void)temperature.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize_temperature() {
    return clocks.initialize().has_value() &&
           temperature.initialize().has_value();
  }
};
TEST(nrf_temperature,
     factory_linearity_coefficients_are_copied_with_reserved_bits_masked,
     "REQ-0064") {
  fixture f;
  for (unsigned i = 0; i < 17; ++i)
    f.mmio.poke(0x10000404 + i * 4, 0xABCD0100U + i);
  ASSERT_TRUE(f.initialize_temperature());
  for (unsigned i = 0; i < 17; ++i) {
    auto offset = i < 6    ? 0x520U + i * 4
                  : i < 12 ? 0x540U + (i - 6) * 4
                           : 0x560U + (i - 12) * 4;
    auto mask = i < 6 ? 0xFFFU : i < 12 ? 0x3FFFU : 0xFFU;
    EXPECT_EQ(f.mmio.peek(0x4000C000 + offset), (0xABCD0100U + i) & mask);
  }
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  EXPECT_TRUE(f.context.active_work());
}
TEST(nrf_temperature, one_shot_reports_signed_quarter_degree_units,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.initialize_temperature());
  auto t = f.temperature.sample();
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(f.temperature.poll());
  f.mmio.poke(0x4000C508, 0xFFFFFFFC);
  f.mmio.poke(0x4000C100, 1);
  ASSERT_TRUE(f.temperature.poll());
  auto value = t->take();
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value->quarter_celsius, -4);
  EXPECT_EQ(value->milli_celsius(), -1000);
  EXPECT_TRUE(f.mmio.wrote(0x4000C004, 1));
}
TEST(nrf_temperature,
     unprogrammed_calibration_and_mmio_fault_release_only_after_cleanup,
     "REQ-0064") {
  fixture f;
  f.mmio.poke(0x10000404, 0xFFFFFFFFU);
  EXPECT_FALSE(f.initialize_temperature());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::temp));
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_temperature,
     cancellation_clears_late_sample_and_new_start_waits_for_new_data,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.initialize_temperature());
  auto t = f.temperature.sample();
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(f.temperature.shutdown().has_value());
  ASSERT_TRUE(t->cancel().has_value());
  (void)t->take();
  f.mmio.poke(0x4000C100, 1);
  auto next = f.temperature.sample();
  ASSERT_TRUE(next.has_value());
  EXPECT_FALSE(f.temperature.poll());
  f.mmio.poke(0x4000C508, 100);
  f.mmio.poke(0x4000C100, 1);
  ASSERT_TRUE(f.temperature.poll());
  EXPECT_EQ(next->take()->quarter_celsius, 100);
}
TEST(nrf_temperature, failed_measurement_retains_instance_until_shutdown,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.initialize_temperature());
  auto t = f.temperature.sample();
  ASSERT_TRUE(t.has_value());
  f.mmio.failed_address = 0x4000C100;
  ASSERT_TRUE(f.temperature.poll());
  EXPECT_FALSE(t->take().has_value());
  EXPECT_FALSE(f.temperature.shutdown().has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::temp));
  f.mmio.failed_address.reset();
  ASSERT_TRUE(f.temperature.shutdown().has_value());
}
TEST(nrf_rng,
     bias_correction_and_valrdy_stop_are_explicit_and_exclusively_owned,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.rng.initialize().has_value());
  EXPECT_EQ(f.mmio.peek(0x4000D504), 1U);
  EXPECT_EQ(f.mmio.peek(0x4000D200), 1U);
  EXPECT_FALSE(f.resources.claim(f.execution, peripheral::rng, &f).has_value());
  ASSERT_TRUE(f.rng.shutdown().has_value());
  EXPECT_FALSE(f.context.active_work());
}
TEST(nrf_rng,
     bounded_cpu_buffer_handoff_returns_exact_byte_count_without_overwrite,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.rng.initialize().has_value());
  std::array<std::uint8_t, 4> bytes{0xEE, 0xEE, 0xEE, 0xEE};
  auto t = f.rng.generate(std::span{bytes}.first<3>());
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(f.rng.poll());
  for (unsigned i = 0; i < 3; ++i) {
    f.mmio.poke(0x4000D508, 0x40 + i);
    f.mmio.poke(0x4000D100, 1);
    ASSERT_TRUE(f.rng.poll());
  }
  EXPECT_EQ(*t->take(), 3U);
  EXPECT_EQ(bytes[0], 0x40U);
  EXPECT_EQ(bytes[2], 0x42U);
  EXPECT_EQ(bytes[3], 0xEEU);
  EXPECT_EQ(f.rng.generated_count(), 3U);
  EXPECT_TRUE(f.mmio.wrote(0x4000D004, 1));
}
TEST(nrf_rng, cancellation_detaches_cpu_buffer_and_preserves_partial_count,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.rng.initialize().has_value());
  std::array<std::uint8_t, 2> bytes{};
  auto t = f.rng.generate(bytes);
  ASSERT_TRUE(t.has_value());
  f.mmio.poke(0x4000D508, 7);
  f.mmio.poke(0x4000D100, 1);
  ASSERT_TRUE(f.rng.poll());
  ASSERT_TRUE(t->cancel().has_value());
  (void)t->take();
  f.mmio.poke(0x4000D508, 9);
  f.mmio.poke(0x4000D100, 1);
  EXPECT_FALSE(f.rng.poll());
  EXPECT_EQ(bytes[1], 0U);
  EXPECT_EQ(f.rng.generated_count(), 1U);
}
TEST(nrf_rng,
     empty_and_concurrent_operations_are_rejected_and_isr_service_is_bounded,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.rng.initialize(false).has_value());
  EXPECT_EQ(f.mmio.peek(0x4000D504), 0U);
  std::array<std::uint8_t, 1> bytes{};
  EXPECT_FALSE(f.rng.generate({}).has_value());
  auto t = f.rng.generate(bytes);
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(f.rng.generate(bytes).has_value());
  f.execution.set_isr(true);
  f.mmio.poke(0x4000D508, 3);
  f.mmio.poke(0x4000D100, 1);
  ASSERT_TRUE(f.rng.poll());
  f.execution.set_isr(false);
  EXPECT_EQ(*t->take(), 1U);
  EXPECT_EQ(bytes[0], 3U);
}
TEST(nrf_rng, failed_stop_detaches_output_but_retains_resource_until_recovered,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.rng.initialize().has_value());
  std::array<std::uint8_t, 1> bytes{};
  auto t = f.rng.generate(bytes);
  ASSERT_TRUE(t.has_value());
  f.mmio.failed_address = 0x4000D004;
  EXPECT_FALSE(t->cancel().has_value());
  (void)t->take();
  EXPECT_FALSE(f.rng.shutdown().has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::rng));
  f.mmio.failed_address.reset();
  ASSERT_TRUE(f.rng.shutdown().has_value());
}
} // namespace
