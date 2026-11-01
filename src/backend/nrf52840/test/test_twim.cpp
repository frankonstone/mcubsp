#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/twim.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_twim : public nrf52840_test::fake_system {
public:
  std::uintptr_t base{0x40003000};
  bool stop_responds{true};
  unsigned starts{}, stops{};
  std::uint32_t interrupts{};
  bool fail_terminal_ack{};
  bool pending_irq() const noexcept {
    for (unsigned i = 0; i < 32; ++i)
      if ((interrupts & (1U << i)) && peek(base + 0x100 + 4 * i))
        return true;
    return false;
  }
  execution::fake_policy *clock{};
  template <class T>
  std::expected<T, core::mmio_error> read(std::uintptr_t a) noexcept {
    if (clock)
      clock->advance();
    return nrf52840_test::fake_system::read<T>(a);
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto before = peek(a);
    auto r = nrf52840_test::fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a == base + 0x304)
      interrupts |= v;
    if (a == base + 0x308)
      interrupts &= ~v;
    if (a == base + 0x4C4)
      poke(a, before & ~static_cast<std::uint32_t>(v));
    if ((a == base || a == base + 8) && v) {
      ++starts;
      poke(base + 0x53C, 0);
      poke(base + 0x54C, 0);
    }
    if (a == base + 0x014 && v) {
      ++stops;
      if (stop_responds)
        poke(base + 0x104, 1);
    }
    if (fail_terminal_ack && a == base + 0x104 && before && !v) {
      fail_terminal_ack = false;
      return std::unexpected(core::mmio_error::out_of_range);
    }
    return r;
  }
  void complete(unsigned tx, unsigned rx) {
    poke(base + 0x54C, tx);
    poke(base + 0x53C, rx);
    poke(base + 0x104, 1);
  }
};
template <unsigned I = 0> struct fixture {
  using binding = i2c_binding<I, {0, 17}, {0, 20}>;
  fake_twim mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_twim> gpio{mmio, resources};
  system_context<fake_twim, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 4> buffers{resources, execution};
  twim_controller<binding, decltype(context), decltype(buffers),
                  native_dma_address, 16>
      i2c{context, clocks, buffers, 100};
  fixture() {
    mmio.base = binding::base;
    mmio.clock = &execution;
  }
  ~fixture() {
    (void)i2c.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize() {
    auto timing = solve_twim_timing({16000000, 400000, 30000, 300, 100});
    return clocks.initialize().has_value() &&
           i2c.initialize(*timing).has_value();
  }
};
std::span<std::uint8_t> ram(std::uintptr_t a = 0x20001000, std::size_t n = 8) {
  return {reinterpret_cast<std::uint8_t *>(a), n};
}
constexpr driver::i2c_address sensor{0x48};
TEST(nrf_twim, erratum_219_rate_reports_390khz_and_checks_requested_tolerance,
     "REQ-0061") {
  auto r = solve_twim_timing({16000000, 400000, 30000, 300, 100});
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->frequency, 0x06200000U);
  EXPECT_EQ(r->achieved_hz, 390000U);
  EXPECT_EQ(r->error_ppm, 25000U);
  EXPECT_FALSE(
      solve_twim_timing({16000000, 400000, 20000, 300, 100}).has_value());
  EXPECT_FALSE(solve_twim_timing({16000000, 390000, 0, 301, 100}).has_value());
}
TEST(nrf_twim,
     both_instances_initialize_open_drain_pins_and_exclude_shared_personalities,
     "REQ-0061", "REQ-0062") {
  {
    fixture f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 6U);
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x524), 0x06200000U);
    EXPECT_EQ(f.mmio.peek(0x50000744), 1U | (6U << 8));
    EXPECT_FALSE(
        f.resources.claim(f.execution, peripheral::spim0, &f).has_value());
    EXPECT_FALSE(
        f.resources.claim(f.execution, peripheral::twi0, &f).has_value());
  }
  {
    fixture<1> f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.base, 0x40004000U);
  }
}
TEST(nrf_twim,
     write_read_uses_hardware_repeated_start_and_only_stopped_publishes,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  driver::i2c_controller port{f.i2c};
  auto t = port.write_read(sensor, ram(0x20001000, 2), ram(0x20002000, 8));
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x200), (1U << 7) | (1U << 12));
  f.mmio.poke(f.mmio.base + 0x160, 1);
  f.mmio.poke(f.mmio.base + 0x15C, 1);
  EXPECT_FALSE(f.i2c.poll());
  EXPECT_FALSE(t->ready());
  f.mmio.complete(2, 8);
  ASSERT_TRUE(f.i2c.poll());
  auto r = t->take();
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->bytes_written, 2U);
  EXPECT_EQ(r->bytes_read, 8U);
}
TEST(nrf_twim,
     individual_write_read_and_reverse_repeated_start_use_distinct_shortcuts,
     "REQ-0061") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto write = f.i2c.write(sensor, ram());
  ASSERT_TRUE(write.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x200), 1U << 9);
  f.mmio.complete(8, 0);
  ASSERT_TRUE(f.i2c.poll());
  ASSERT_TRUE(write->take().has_value());
  auto read = f.i2c.read(sensor, ram());
  ASSERT_TRUE(read.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x200), 1U << 12);
  f.mmio.complete(0, 8);
  ASSERT_TRUE(f.i2c.poll());
  ASSERT_TRUE(read->take().has_value());
  auto reverse = f.i2c.read_write(sensor, ram(), ram(0x20002000));
  ASSERT_TRUE(reverse.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x200), (1U << 10) | (1U << 9));
  f.mmio.complete(8, 8);
  ASSERT_TRUE(f.i2c.poll());
  ASSERT_TRUE(reverse->take().has_value());
}
TEST(nrf_twim, nack_causes_stop_and_preserves_counts_without_releasing_early,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.stop_responds = false;
  auto t = f.i2c.write(sensor, ram());
  ASSERT_TRUE(t.has_value());
  f.mmio.poke(f.mmio.base + 0x54C, 3);
  f.mmio.poke(f.mmio.base + 0x4C4, 4);
  f.mmio.poke(f.mmio.base + 0x124, 1);
  EXPECT_FALSE(f.i2c.poll());
  EXPECT_FALSE(t->ready());
  EXPECT_TRUE(f.mmio.wrote(f.mmio.base + 0x014, 1));
  f.mmio.poke(f.mmio.base + 0x104, 1);
  ASSERT_TRUE(f.i2c.poll());
  auto r = t->take();
  ASSERT_FALSE(r.has_value());
  EXPECT_TRUE(r.error().cause == driver::i2c_error::data_nack);
  EXPECT_EQ(f.i2c.transferred_count().bytes_written, 3U);
  EXPECT_EQ(f.i2c.error_source(), 4U);
}
TEST(nrf_twim, address_nack_and_overrun_remain_observable, "REQ-0061") {
  for (unsigned bits : {2U, 1U}) {
    fixture f;
    ASSERT_TRUE(f.initialize());
    auto t = f.i2c.read(sensor, ram());
    ASSERT_TRUE(t.has_value());
    f.mmio.poke(f.mmio.base + 0x4C4, bits);
    f.mmio.poke(f.mmio.base + 0x124, 1);
    ASSERT_TRUE(f.i2c.poll());
    auto r = t->take();
    ASSERT_FALSE(r.has_value());
    EXPECT_TRUE(r.error().cause == (bits == 2 ? driver::i2c_error::address_nack
                                              : driver::i2c_error::transport));
    EXPECT_EQ(f.i2c.error_source(), bits);
  }
}
TEST(nrf_twim,
     invalid_addresses_lengths_memory_and_overlap_do_not_start_hardware,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  EXPECT_FALSE(f.i2c.write({0xFF}, ram()).has_value());
  EXPECT_FALSE(f.i2c.write(sensor, {}).has_value());
  EXPECT_FALSE(f.i2c.write(sensor, ram(0, 8)).has_value());
  EXPECT_FALSE(f.i2c.read(sensor, ram(0x2003FFFE, 8)).has_value());
  EXPECT_FALSE(f.i2c.read(sensor, ram(0x20000000, 65536)).has_value());
  EXPECT_FALSE(f.i2c.write_read(sensor, ram(), ram()).has_value());
  EXPECT_EQ(f.mmio.starts, 0U);
}
TEST(nrf_twim,
     cancellation_retains_faulted_buffers_and_recovers_original_stop_ack,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto t = f.i2c.read(sensor, ram());
  ASSERT_TRUE(t.has_value());
  f.mmio.stop_responds = false;
  f.execution.set_time(0xFFFFFFFC);
  EXPECT_FALSE(t->cancel().has_value());
  EXPECT_TRUE(t->take().error().kind ==
              core::transfer_error_kind::quiesce_failed);
  EXPECT_TRUE(f.i2c.faulted());
  EXPECT_FALSE(f.i2c.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  f.mmio.poke(f.mmio.base + 0x104, 1);
  ASSERT_TRUE(f.i2c.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_twim,
     failed_start_after_commit_is_cleaned_up_before_dma_memory_is_returned,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.mmio.base + 8;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  auto t = f.i2c.write(sensor, ram());
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(t->ready());
  ASSERT_TRUE(f.i2c.poll());
  auto r = t->take();
  ASSERT_FALSE(r.has_value());
  EXPECT_TRUE(r.error().cause == driver::i2c_error::mmio);
  ASSERT_TRUE(f.i2c.shutdown().has_value());
}
TEST(nrf_twim,
     prestart_failure_cleans_leases_and_natural_short_count_is_not_success,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.mmio.base + 0x544;
  f.mmio.fail_once = true;
  auto t = f.i2c.write(sensor, ram());
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(t->take().has_value());
  EXPECT_EQ(f.mmio.starts, 0U);
  auto next = f.i2c.write(sensor, ram());
  ASSERT_TRUE(next.has_value());
  f.mmio.complete(1, 0);
  ASSERT_TRUE(f.i2c.poll());
  EXPECT_FALSE(next->take().has_value());
}
TEST(nrf_twim, low_frequency_only_pads_reject_fast_bus_configuration,
     "REQ-0061") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  using pins = i2c_binding<0, {0, 2}, {0, 3}>;
  twim_controller<pins, decltype(f.context), decltype(f.buffers)> bus{
      f.context, f.clocks, f.buffers};
  auto timing = solve_twim_timing({16000000, 100000, 0, 300, 100});
  auto result = bus.initialize(*timing);
  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(result.error() == driver::i2c_error::invalid_rate);
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::twim0));
}
TEST(nrf_twim,
     stop_interrupt_is_acknowledged_and_failed_cancel_masks_late_events,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.i2c.interrupt_delivery(true).has_value());
  auto t = f.i2c.write(sensor, ram());
  ASSERT_TRUE(t.has_value());
  f.mmio.complete(8, 0);
  EXPECT_TRUE(f.mmio.pending_irq());
  ASSERT_TRUE(f.i2c.poll());
  ASSERT_TRUE(t->take().has_value());
  EXPECT_FALSE(f.mmio.pending_irq());
  auto next = f.i2c.read(sensor, ram());
  ASSERT_TRUE(next.has_value());
  f.mmio.stop_responds = false;
  EXPECT_FALSE(next->cancel().has_value());
  (void)next->take();
  f.mmio.poke(f.mmio.base + 0x104, 1);
  EXPECT_FALSE(f.mmio.pending_irq());
  ASSERT_TRUE(f.i2c.shutdown().has_value());
}
TEST(nrf_twim,
     terminal_ack_error_preserves_quiescent_recovery_and_partial_counts,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto t = f.i2c.write(sensor, ram());
  ASSERT_TRUE(t.has_value());
  f.mmio.complete(8, 0);
  f.mmio.fail_terminal_ack = true;
  EXPECT_FALSE(f.i2c.poll());
  EXPECT_FALSE(t->ready());
  f.mmio.stop_responds = false;
  ASSERT_TRUE(f.i2c.poll());
  EXPECT_FALSE(t->take().has_value());
  EXPECT_EQ(f.i2c.transferred_count().bytes_written, 8U);
  ASSERT_TRUE(f.i2c.shutdown().has_value());
}
} // namespace
