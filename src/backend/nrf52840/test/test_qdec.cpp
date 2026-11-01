#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/events.hpp>
#include <mcubsp/backend/nrf52840/qdec.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_qdec : public nrf52840_test::fake_system {
public:
  bool stop_responds{true}, sample_after_snapshot{};
  execution::fake_policy *clock{};
  template <class T>
  std::expected<T, core::mmio_error> read(std::uintptr_t a) noexcept {
    if (clock)
      clock->advance();
    return nrf52840_test::fake_system::read<T>(a);
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto r = nrf52840_test::fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (v && a == 0x40012004 && stop_responds)
      poke(0x40012110, 1);
    if (v && a == 0x40012008) {
      poke(0x40012518, peek(0x40012514));
      poke(0x40012548, peek(0x40012544));
      poke(0x40012514, 0);
      poke(0x40012544, 0);
      if (sample_after_snapshot) {
        sample_after_snapshot = false;
        inject(1);
      }
    }
    return r;
  }
  void inject(int motion) {
    poke(0x4001250C, static_cast<std::uint32_t>(motion));
    poke(0x40012100, 1);
    if (motion == 2) {
      auto count = peek(0x40012544);
      if (count == 15)
        poke(0x40012108, 1);
      else
        poke(0x40012544, count + 1);
      poke(0x4001210C, 1);
    } else {
      auto count = std::bit_cast<std::int32_t>(peek(0x40012514));
      if (count + motion > 1023 || count + motion < -1024)
        poke(0x40012108, 1);
      else
        poke(0x40012514, static_cast<std::uint32_t>(count + motion));
      if (motion)
        poke(0x40012104, 1);
    }
  }
};
struct fixture {
  using binding = qdec_binding<{0, 6}, {0, 8}, {0, 7}>;
  fake_qdec mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_qdec> gpio{mmio, resources};
  system_context<fake_qdec, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  qdec_controller<binding, decltype(context), 16> decoder{context, clocks, 100};
  fixture() { mmio.clock = &execution; }
  ~fixture() {
    (void)decoder.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(qdec_configuration config = {}) {
    return clocks.initialize().has_value() &&
           decoder.initialize(config).has_value();
  }
};
TEST(nrf_qdec, validates_configuration_and_maps_input_led_and_clock_ownership,
     "REQ-0065") {
  fixture f;
  ASSERT_TRUE(f.initialize({.sample_period = 10,
                            .report_period = 8,
                            .led_pre_microseconds = 511,
                            .debounce = false,
                            .led_active_high = false}));
  EXPECT_EQ(f.mmio.peek(0x40012508), 10U);
  EXPECT_EQ(f.mmio.peek(0x40012510), 8U);
  EXPECT_EQ(f.mmio.peek(0x40012540), 511U);
  EXPECT_EQ(f.mmio.peek(0x40012528), 0U);
  EXPECT_EQ(f.mmio.peek(0x40012504), 0U);
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  EXPECT_FALSE(
      f.resources.claim(f.execution, peripheral::qdec, &f).has_value());
  ASSERT_TRUE(f.decoder.start().has_value());
  ASSERT_TRUE(f.decoder.stop().has_value());
}
TEST(nrf_qdec, both_directions_and_double_transitions_use_atomic_read_clear,
     "REQ-0065") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.decoder.start().has_value());
  f.mmio.inject(-1);
  f.mmio.inject(-1);
  f.mmio.inject(1);
  f.mmio.inject(2);
  auto r = f.decoder.read_clear();
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->movement, -1);
  EXPECT_EQ(r->double_transitions, 1U);
  EXPECT_TRUE(r->report_ready);
  EXPECT_TRUE(r->double_ready);
  EXPECT_FALSE(r->overflow);
  EXPECT_EQ(*f.decoder.last_sample(), 2);
  EXPECT_TRUE(*f.decoder.consume(qdec_event::sample));
  EXPECT_FALSE(*f.decoder.consume(qdec_event::sample));
  auto next = f.decoder.read_clear();
  ASSERT_TRUE(next.has_value());
  EXPECT_EQ(next->movement, 0);
}
TEST(nrf_qdec,
     accumulator_and_double_transition_overflows_are_reported_without_wrapping,
     "REQ-0065") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0x40012514, 1023);
  f.mmio.inject(1);
  for (unsigned i = 0; i < 16; ++i)
    f.mmio.inject(2);
  auto r = f.decoder.read_clear();
  ASSERT_TRUE(r.has_value());
  EXPECT_TRUE(r->overflow);
  EXPECT_EQ(r->movement, 1023);
  EXPECT_EQ(r->double_transitions, 15U);
  f.mmio.poke(0x40012514, static_cast<std::uint32_t>(-1024));
  f.mmio.inject(-1);
  r = f.decoder.read_clear();
  ASSERT_TRUE(r.has_value());
  EXPECT_TRUE(r->overflow);
  EXPECT_EQ(r->movement, -1024);
}
TEST(nrf_qdec,
     events_arriving_during_snapshot_are_preserved_for_the_next_report,
     "REQ-0065") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.inject(1);
  f.mmio.sample_after_snapshot = true;
  auto first = f.decoder.read_clear();
  ASSERT_TRUE(first.has_value());
  EXPECT_EQ(first->movement, 1);
  EXPECT_EQ(f.mmio.peek(0x40012104), 1U);
  auto next = f.decoder.read_clear();
  ASSERT_TRUE(next.has_value());
  EXPECT_EQ(next->movement, 1);
  EXPECT_TRUE(next->report_ready);
}
TEST(nrf_qdec,
     ppi_endpoint_leases_block_cpu_snapshots_and_release_until_route_teardown,
     "REQ-0065", "REQ-0057") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto event = f.decoder.event(qdec_event::report);
  ppi_channel<2, decltype(f.context)> route{f.context};
  ASSERT_TRUE(
      route.initialize(event, f.decoder.task(qdec_task::read_clear_both))
          .has_value());
  EXPECT_FALSE(f.decoder.read_clear().has_value());
  EXPECT_FALSE(f.decoder.shutdown().has_value());
  ASSERT_TRUE(route.shutdown().has_value());
  ASSERT_TRUE(f.decoder.shutdown().has_value());
  EXPECT_FALSE(f.context.valid(event, endpoint_kind::event));
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::qdec));
}
TEST(nrf_qdec, bounded_stop_retains_instance_pins_and_clock_until_original_ack,
     "REQ-0065") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.decoder.start().has_value());
  f.mmio.stop_responds = false;
  EXPECT_FALSE(f.decoder.stop().has_value());
  EXPECT_FALSE(f.decoder.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  EXPECT_FALSE(f.gpio.claim({0, 6}, driver::gpio_direction::input).has_value());
  f.mmio.poke(0x40012110, 1);
  ASSERT_TRUE(f.decoder.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_qdec, invalid_periods_are_rejected_without_hardware_claims,
     "REQ-0065") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  EXPECT_FALSE(f.decoder.initialize({.sample_period = 11}).has_value());
  EXPECT_FALSE(f.decoder.initialize({.report_period = 9}).has_value());
  EXPECT_FALSE(f.decoder.initialize({.led_pre_microseconds = 512}).has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::qdec));
}
} // namespace
