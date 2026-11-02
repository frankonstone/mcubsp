#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/backend/nrf52840/comparator.hpp>
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
  comp_controller<decltype(context)> comp{context, clocks};
  lpcomp_controller<decltype(context)> lp{context, clocks};
  power_controller<decltype(context)> power{context, clocks};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x10000100, 0x52840);
  }
  ~fixture() {
    (void)comp.shutdown();
    (void)lp.shutdown();
    (void)power.shutdown();
    (void)clocks.shutdown();
  }
};
TEST(nrf_comparator, comp_configures_reference_thresholds_and_excludes_lpcomp,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.comp
                  .initialize({.positive_input = 4,
                               .external_input = 5,
                               .threshold_down = 10,
                               .threshold_up = 50,
                               .reference = comp_reference::external,
                               .speed = comp_speed::high})
                  .has_value());
  EXPECT_EQ(f.mmio.peek(0x40013530), 10U | (50U << 8));
  EXPECT_EQ(f.mmio.peek(0x40013538), 1U);
  EXPECT_FALSE(f.lp.initialize().has_value());
  EXPECT_FALSE(
      f.gpio.claim({0, 28}, driver::gpio_direction::input).has_value());
  ASSERT_TRUE(f.comp.shutdown().has_value());
  EXPECT_TRUE(f.mmio.wrote(0x40013FFC, 0));
  EXPECT_TRUE(f.mmio.wrote(0x40013FFC, 1));
  ASSERT_TRUE(f.lp.initialize().has_value());
}
TEST(nrf_comparator, start_ready_sample_and_crossings_are_acknowledged,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(
      f.comp
          .initialize(
              {.positive_input = 1, .external_input = 0, .differential = true})
          .has_value());
  EXPECT_FALSE(f.comp.sample().has_value());
  ASSERT_TRUE(f.comp.start().has_value());
  auto first = f.comp.poll();
  ASSERT_TRUE(first.has_value());
  EXPECT_TRUE(first->ready);
  f.mmio.poke(0x40013400, 1);
  EXPECT_TRUE(*f.comp.sample());
  f.mmio.poke(0x40013104, 1);
  f.mmio.poke(0x40013108, 1);
  f.mmio.poke(0x4001310C, 1);
  auto crossing = f.comp.poll();
  ASSERT_TRUE(crossing.has_value());
  EXPECT_TRUE(crossing->down && crossing->up && crossing->cross);
  EXPECT_EQ(f.mmio.peek(0x4001310C), 0U);
  ASSERT_TRUE(f.comp.stop().has_value());
  EXPECT_FALSE(f.comp.sample().has_value());
}
TEST(nrf_comparator, lpcomp_wake_requires_ready_and_keeps_no_hfclock_client,
     "REQ-0064", "REQ-0056") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.power.initialize().has_value());
  ASSERT_TRUE(f.lp.initialize({.reference = 7,
                               .external_input = 1,
                               .wake = comparator_wake::up})
                  .has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
  ASSERT_TRUE(f.lp.start().has_value());
  EXPECT_FALSE(f.lp.prepare_system_off().has_value());
  EXPECT_FALSE(f.power.system_off(true).has_value());
  ASSERT_TRUE(f.lp.poll()->ready);
  ASSERT_TRUE(f.lp.prepare_system_off().has_value());
  EXPECT_TRUE(f.lp.wake_ready());
  ASSERT_TRUE(f.power.system_off(f.lp.wake_ready()).has_value());
  EXPECT_EQ(f.mmio.peek(0x40000500), 1U);
}
TEST(nrf_comparator,
     invalid_threshold_reference_and_wake_encodings_do_not_claim_hardware,
     "REQ-0064") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  EXPECT_FALSE(f.comp.initialize({.threshold_down = 60, .threshold_up = 10})
                   .has_value());
  EXPECT_FALSE(f.lp.initialize({.external_input = 2}).has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::comp));
}
TEST(nrf_comparator, disable_fault_retains_pin_resource_and_clock_for_retry,
     "REQ-0064", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.comp.initialize().has_value());
  f.mmio.failed_address = 0x40013500;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  EXPECT_FALSE(f.comp.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  EXPECT_FALSE(f.lp.initialize().has_value());
  ASSERT_TRUE(f.comp.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
} // namespace
