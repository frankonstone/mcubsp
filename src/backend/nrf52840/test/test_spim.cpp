#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/spim.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_spim : public nrf52840_test::fake_system {
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
    if (a == base + 0x010 && v) {
      ++starts;
      poke(base + 0x14C, 1);
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
    poke(base + 0x118, 1);
  }
};
template <unsigned I = 0> struct fixture {
  using binding = spim_binding<I, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
  fake_spim mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_spim> gpio{mmio, resources};
  system_context<fake_spim, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 4> buffers{resources, execution};
  spim_controller<binding, decltype(context), decltype(buffers),
                  native_dma_address, 4, 16>
      spi{context, clocks, buffers, 100};
  fixture() {
    mmio.base = binding::base;
    mmio.clock = &execution;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 3);
  }
  ~fixture() {
    (void)spi.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(driver::spi_configuration config = {}) {
    auto timing =
        solve_spim_timing(I, {16000000, I == 3 ? 32000000U : 1000000U, 0});
    return clocks.initialize().has_value() &&
           spi.initialize(*timing, config, {0}).has_value();
  }
};
std::span<std::uint8_t> ram(std::uintptr_t a = 0x20001000, std::size_t n = 8) {
  return {reinterpret_cast<std::uint8_t *>(a), n};
}
TEST(nrf_spim, rate_solver_enforces_native_instance_limits_and_encodings,
     "REQ-0060") {
  EXPECT_FALSE(solve_spim_timing(0, {16000000, 32000000, 0}).has_value());
  EXPECT_EQ(solve_spim_timing(3, {16000000, 32000000, 0})->encoding,
            0x14000000U);
  EXPECT_EQ(solve_spim_timing(3, {16000000, 16000000, 0})->encoding,
            0x0A000000U);
  EXPECT_FALSE(solve_spim_timing(4, {16000000, 1000000, 0}).has_value());
  EXPECT_FALSE(solve_spim_timing(0, {16000000, 900000, 100}).has_value());
}
TEST(nrf_spim, each_instance_initializes_with_its_native_configuration,
     "REQ-0060", "REQ-0062") {
  {
    fixture<0> f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 7U);
  }
  {
    fixture<1> f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.base, 0x40004000U);
  }
  {
    fixture<2> f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.base, 0x40023000U);
  }
  {
    fixture<3> f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x524), 0x14000000U);
    ASSERT_TRUE(f.spi.shutdown().has_value());
    EXPECT_TRUE(f.mmio.wrote(f.mmio.base + 4, 1));
  }
}
TEST(nrf_spim,
     alias_and_gpio_conflicts_prevent_foreign_register_reconfiguration,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto before = f.mmio.trace_count;
  EXPECT_FALSE(
      f.resources.claim(f.execution, peripheral::twim0, &f).has_value());
  EXPECT_EQ(before, f.mmio.trace_count);
  EXPECT_FALSE(
      f.gpio.claim({0, 6}, driver::gpio_direction::output).has_value());
}
TEST(nrf_spim, all_spi_modes_bit_orders_and_cs_polarities_are_encoded,
     "REQ-0060") {
  for (unsigned mode = 0; mode < 4; ++mode) {
    fixture f;
    ASSERT_TRUE(f.initialize(
        {.mode = static_cast<driver::spi_mode>(mode),
         .bit_order = driver::spi_bit_order::least_significant_first,
         .chip_select = driver::spi_chip_select_polarity::active_high}));
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x554),
              1U | ((mode & 1U) << 1) | ((mode & 2U) << 1));
    auto t = f.spi.transfer(ram(), ram(0x20002000));
    ASSERT_TRUE(t.has_value());
    EXPECT_TRUE(f.mmio.wrote(0x50000508, 1U << 12));
    f.mmio.complete(8, 8);
    ASSERT_TRUE(f.spi.poll());
    ASSERT_TRUE(t->take().has_value());
    EXPECT_TRUE(f.mmio.wrote(0x5000050C, 1U << 12));
  }
}
TEST(nrf_spim, unequal_buffers_use_orc_and_report_actual_counts_after_stopped,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.stop_responds = false;
  auto t = f.spi.transfer(ram(0x20001000, 2), ram(0x20002000, 8), 0xA5);
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x5C0), 0xA5U);
  f.mmio.complete(2, 8);
  EXPECT_FALSE(f.spi.poll());
  EXPECT_FALSE(t->ready());
  f.mmio.poke(f.mmio.base + 0x104, 1);
  ASSERT_TRUE(f.spi.poll());
  auto r = t->take();
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->frames, 8U);
  EXPECT_EQ(r->transmitted, 2U);
  EXPECT_EQ(r->received, 8U);
}
TEST(nrf_spim,
     chip_select_spans_multiple_segments_and_releases_only_at_final_stop,
     "REQ-0060") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array s{spim_segment{ram(0x20001000, 2), {}, 0xFF},
               spim_segment{{}, ram(0x20002000, 4), 0x55}};
  auto t = f.spi.transaction(s);
  ASSERT_TRUE(t.has_value());
  auto before = f.mmio.trace_count;
  f.mmio.complete(2, 0);
  ASSERT_TRUE(f.spi.poll());
  EXPECT_FALSE(t->ready());
  EXPECT_EQ(f.mmio.starts, 2U);
  for (unsigned i = before; i < f.mmio.trace_count; ++i)
    EXPECT_FALSE(f.mmio.trace[i].address == 0x50000508 &&
                 f.mmio.trace[i].value == (1U << 12));
  f.mmio.complete(0, 4);
  ASSERT_TRUE(f.spi.poll());
  auto r = t->take();
  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->frames, 6U);
}
TEST(nrf_spim, half_direction_transfers_still_program_valid_zero_count_pointers,
     "REQ-0060") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto tx = f.spi.transfer(ram(), {});
  ASSERT_TRUE(tx.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x538), 0U);
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x534), 0x20001000U);
  f.mmio.complete(8, 0);
  ASSERT_TRUE(f.spi.poll());
  ASSERT_TRUE(tx->take().has_value());
  auto rx = f.spi.transfer({}, ram());
  ASSERT_TRUE(rx.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x548), 0U);
  f.mmio.complete(0, 8);
  ASSERT_TRUE(f.spi.poll());
  ASSERT_TRUE(rx->take().has_value());
}
TEST(nrf_spim,
     invalid_bounds_flash_and_overlap_are_rejected_without_starting_dma,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  EXPECT_FALSE(f.spi.transfer(ram(0, 8), {}).has_value());
  EXPECT_FALSE(f.spi.transfer(ram(0x2003FFFC, 8), {}).has_value());
  EXPECT_FALSE(f.spi.transfer(ram(0x20000000, 65536), {}).has_value());
  auto t = f.spi.transfer(ram(), ram());
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(t->take().has_value());
  EXPECT_EQ(f.mmio.starts, 0U);
  auto next = f.spi.transfer(ram(), {});
  ASSERT_TRUE(next.has_value());
  ASSERT_TRUE(next->cancel().has_value());
  (void)next->take();
}
TEST(nrf_spim, spim3_dedicated_ahb_slave_is_held_against_other_dma_clients,
     "REQ-0060", "REQ-0062") {
  fixture<3> f;
  ASSERT_TRUE(f.initialize());
  auto t = f.spi.transfer(ram(0x20001000), ram(0x20002000));
  ASSERT_TRUE(t.has_value());
  auto other = f.resources.claim(f.execution, peripheral::uarte1, &f);
  ASSERT_TRUE(other.has_value());
  EXPECT_FALSE(f.buffers
                   .prepare(*other, mcu::nrf52840::dma_constraints{16, 1, 1},
                            0x20000000, 8, 8, dma_access::read)
                   .has_value());
  ASSERT_TRUE(f.resources.release(f.execution, *other).has_value());
  f.mmio.complete(8, 8);
  ASSERT_TRUE(f.spi.poll());
  ASSERT_TRUE(t->take().has_value());
  EXPECT_FALSE(f.spi.transfer(ram(0x20002000), {}).has_value());
}
TEST(nrf_spim, unknown_stepping_excludes_spim3_and_single_byte_anomaly_58,
     "REQ-0060") {
  {
    fixture<3> f;
    f.mmio.poke(0x10000134, 0);
    EXPECT_FALSE(f.initialize());
  }
  {
    fixture f;
    f.mmio.poke(0x10000134, 0);
    ASSERT_TRUE(f.initialize());
    EXPECT_FALSE(
        f.spi.transfer(ram(0x20001000, 1), ram(0x20002000, 1)).has_value());
  }
}
TEST(nrf_spim, failed_start_after_commit_waits_for_hardware_stop_before_failure,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.mmio.base + 0x010;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  auto t = f.spi.transfer(ram(), {});
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(t->ready());
  ASSERT_TRUE(f.spi.poll());
  auto r = t->take();
  ASSERT_FALSE(r.has_value());
  EXPECT_TRUE(r.error().cause == driver::spi_error::mmio);
  ASSERT_TRUE(f.spi.shutdown().has_value());
}
TEST(nrf_spim, cancellation_fault_keeps_cs_clock_and_buffers_until_late_ack,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto t = f.spi.transfer(ram(), ram(0x20002000));
  ASSERT_TRUE(t.has_value());
  f.mmio.stop_responds = false;
  EXPECT_FALSE(t->cancel().has_value());
  EXPECT_TRUE(t->take().error().kind ==
              core::transfer_error_kind::quiesce_failed);
  EXPECT_FALSE(f.spi.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  EXPECT_TRUE(f.spi.faulted());
  f.mmio.poke(f.mmio.base + 0x104, 1);
  ASSERT_TRUE(f.spi.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_spim, prestart_register_fault_cleans_leases_and_cs_for_reuse,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.mmio.base + 0x544;
  f.mmio.fail_once = true;
  auto t = f.spi.transfer(ram(), {});
  ASSERT_TRUE(t.has_value());
  EXPECT_FALSE(t->take().has_value());
  EXPECT_EQ(f.mmio.starts, 0U);
  auto next = f.spi.transfer(ram(), {});
  ASSERT_TRUE(next.has_value());
  f.mmio.complete(8, 0);
  ASSERT_TRUE(f.spi.poll());
  ASSERT_TRUE(next->take().has_value());
}
TEST(nrf_spim, end_and_stop_interrupts_do_not_remain_asserted_after_service,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.spi.interrupt_delivery(true).has_value());
  auto t = f.spi.transfer(ram(), ram(0x20002000));
  ASSERT_TRUE(t.has_value());
  f.mmio.stop_responds = false;
  f.mmio.complete(8, 8);
  EXPECT_TRUE(f.mmio.pending_irq());
  EXPECT_FALSE(f.spi.poll());
  EXPECT_FALSE(f.mmio.pending_irq());
  f.mmio.poke(f.mmio.base + 0x104, 1);
  EXPECT_TRUE(f.mmio.pending_irq());
  ASSERT_TRUE(f.spi.poll());
  ASSERT_TRUE(t->take().has_value());
  EXPECT_FALSE(f.mmio.pending_irq());
}
TEST(nrf_spim, stop_proof_survives_failed_terminal_acknowledgment, "REQ-0060",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto t = f.spi.transfer(ram(), {});
  ASSERT_TRUE(t.has_value());
  f.mmio.complete(8, 0);
  f.mmio.fail_terminal_ack = true;
  EXPECT_FALSE(f.spi.poll());
  EXPECT_FALSE(t->ready());
  f.mmio.stop_responds = false;
  ASSERT_TRUE(f.spi.poll());
  EXPECT_FALSE(t->take().has_value());
  ASSERT_TRUE(f.spi.shutdown().has_value());
}
} // namespace
