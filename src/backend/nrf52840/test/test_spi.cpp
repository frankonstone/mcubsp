#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/spi.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_spi : public nrf52840_test::fake_system {
public:
  std::uintptr_t base{0x40003000};
  unsigned starts{};
  std::uint32_t interrupts{};
  bool auto_ready{}, fail_ack{};
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    bool ack = fail_ack && a == base + 0x108 && !v && peek(a);
    if (ack)
      fail_ack = false;
    auto r = nrf52840_test::fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a == base + 0x304)
      interrupts |= v;
    if (a == base + 0x308)
      interrupts &= ~v;
    if (a == base + 0x51C) {
      ++starts;
      if (auto_ready)
        complete(v);
    }
    if (ack)
      return std::unexpected(core::mmio_error::out_of_range);
    return r;
  }
  void complete(std::uint32_t v) {
    poke(base + 0x518, v);
    poke(base + 0x108, 1);
  }
};
template <unsigned I = 0> struct fixture {
  using binding = spi_binding<I, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
  fake_spi mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_spi> gpio{mmio, resources};
  system_context<fake_spi, execution::fake_policy> context{mmio, execution,
                                                           gpio};
  clock_controller<decltype(context)> clocks{context};
  spi_controller<binding, decltype(context), 4, 4> spi{context, clocks, 100};
  fixture() { mmio.base = binding::base; }
  ~fixture() {
    (void)spi.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(driver::spi_configuration c = {}) {
    return clocks.initialize().has_value() &&
           spi.initialize(*solve_spi_timing(I, {16000000, 1000000, 0}), c)
               .has_value();
  }
};
TEST(nrf_spi, all_instances_modes_and_orders_configure_the_legacy_personality,
     "REQ-0060") {
  for (unsigned mode = 0; mode < 4; ++mode) {
    fixture f;
    ASSERT_TRUE(f.initialize(
        {.mode = static_cast<driver::spi_mode>(mode),
         .bit_order = driver::spi_bit_order::least_significant_first}));
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 1U);
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x554),
              1U | ((mode & 1) << 1) | ((mode & 2) << 1));
  }
  fixture<1> one;
  ASSERT_TRUE(one.initialize());
  fixture<2> two;
  ASSERT_TRUE(two.initialize());
}
TEST(nrf_spi, ready_drives_cpu_bytes_and_cs_spans_all_segments, "REQ-0060") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint8_t, 2> tx{4, 5}, rx{};
  std::array segments{driver::byte_segment(tx, rx),
                      driver::byte_segment({}, std::span(rx).first(1), 0xA5)};
  auto op = f.spi.transaction(segments);
  ASSERT_TRUE(op.has_value());
  for (unsigned i = 0; i < 3; ++i) {
    f.mmio.complete(10 + i);
    ASSERT_TRUE(f.spi.poll());
    EXPECT_FALSE(f.mmio.peek(f.mmio.base + 0x108));
  }
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->frames, 3U);
  EXPECT_EQ(rx[0], 12U);
  EXPECT_EQ(rx[1], 11U);
  EXPECT_EQ(f.mmio.starts, 3U);
}
TEST(nrf_spi, invalid_segments_and_aliases_fail_before_transmitting, "REQ-0060",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint8_t, 2> tx{}, rx{};
  std::array bad{driver::byte_segment(tx, std::span(rx).first(1))};
  EXPECT_FALSE(f.spi.transaction(bad).has_value());
  EXPECT_FALSE(
      f.resources.claim(f.execution, peripheral::spis0, &f).has_value());
  EXPECT_EQ(f.mmio.starts, 0U);
}
TEST(nrf_spi,
     cancel_waits_for_final_ready_and_recovers_late_without_touching_rx,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint8_t, 1> rx{99};
  std::array s{driver::byte_segment({}, rx)};
  auto op = f.spi.transaction(s);
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(op->cancel().has_value());
  (void)op->take();
  EXPECT_FALSE(f.spi.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  f.mmio.complete(42);
  ASSERT_TRUE(f.spi.shutdown().has_value());
  EXPECT_EQ(rx[0], 99U);
}
TEST(nrf_spi, failed_start_after_commit_retains_clock_until_ready, "REQ-0060",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint8_t, 1> tx{3};
  std::array s{driver::byte_segment(tx, {})};
  f.mmio.failed_address = f.mmio.base + 0x51C;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  auto op = f.spi.transaction(s);
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(op->take().has_value());
  EXPECT_FALSE(f.spi.shutdown().has_value());
  f.mmio.complete(0);
  ASSERT_TRUE(f.spi.shutdown().has_value());
}
} // namespace
