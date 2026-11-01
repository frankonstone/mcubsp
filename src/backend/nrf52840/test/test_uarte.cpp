#include "fake_uarte.hpp"
#include <mcubsp/backend/nrf52840/uarte.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
using mmio_type = nrf52840_test::fake_uarte;
using context_type = system_context<mmio_type, execution::fake_policy>;
using buffers_type = dma_buffer_pool<execution::fake_policy, 4>;
template <peripheral P = peripheral::uarte0> struct fixture {
  using binding = uart_binding<P, {0, 6}, {0, 8}>;
  mmio_type mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<mmio_type> gpio{mmio, resources};
  context_type context{mmio, execution, gpio};
  clock_controller<context_type> clocks{context};
  buffers_type buffers{resources, execution};
  uarte_controller<binding, context_type, buffers_type,
                   nrf52840_test::fake_dma_address, 32>
      serial{context, clocks, buffers, 100};
  fixture() {
    mmio.base = binding::base;
    mmio.clock = &execution;
  }
  ~fixture() {
    (void)serial.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize() {
    auto timing = solve_uart_baud(P, {16'000'000, 115200, 10000});
    return clocks.initialize().has_value() &&
           serial.initialize(*timing).has_value();
  }
  std::uintptr_t base() const { return binding::base; }
  void tx_done(unsigned n) {
    mmio.poke(base() + 0x54C, n);
    mmio.poke(base() + 0x120, 1);
  }
  void rx_done(unsigned n) {
    mmio.poke(base() + 0x53C, n);
    mmio.poke(base() + 0x110, 1);
  }
};
std::span<std::uint8_t> ram(std::uintptr_t a = 0x20001000, std::size_t n = 8) {
  return {reinterpret_cast<std::uint8_t *>(a), n};
}
TEST(nrf_uarte, both_instances_configure_native_addresses_and_exclude_uart0,
     "REQ-0059", "REQ-0062") {
  {
    fixture f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.peek(f.base() + 0x500), 8U);
    EXPECT_EQ(f.mmio.peek(f.base() + 0x524), 0x01D60000U);
    EXPECT_FALSE(
        f.resources.claim(f.execution, peripheral::uart0, &f).has_value());
  }
  {
    fixture<peripheral::uarte1> f;
    ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.peek(0x40028500), 8U);
  }
}
TEST(nrf_uarte, tx_waits_for_stop_and_preserves_dma_ownership_until_ack,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.tx_stop_responds = false;
  auto tx = f.serial.transmit(ram());
  ASSERT_TRUE(tx.has_value());
  f.tx_done(8);
  (void)f.serial.poll();
  EXPECT_FALSE(tx->ready());
  EXPECT_FALSE(f.serial.shutdown().has_value());
  EXPECT_FALSE(
      f.resources.claim(f.execution, peripheral::uart0, &f).has_value());
  f.mmio.poke(f.base() + 0x158, 1);
  ASSERT_TRUE(f.serial.poll());
  ASSERT_TRUE(tx->take().has_value());
  EXPECT_EQ(f.serial.transmitted_count(), 8U);
  ASSERT_TRUE(f.serial.shutdown().has_value());
}
TEST(nrf_uarte,
     simultaneous_rx_tx_have_separate_leases_and_common_transfer_facade,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  driver::uart port{f.serial};
  auto rx = port.receive(ram());
  auto tx = port.transmit(ram(0x20002000));
  ASSERT_TRUE(rx.has_value());
  ASSERT_TRUE(tx.has_value());
  f.rx_done(8);
  f.tx_done(8);
  ASSERT_TRUE(f.serial.poll());
  ASSERT_TRUE(tx->take().has_value());
  EXPECT_EQ(*rx->take(), 8U);
  EXPECT_EQ(f.mmio.flushes, 1U);
}
TEST(nrf_uarte, rejects_flash_code_alias_bounds_counts_and_overlapping_rx_tx,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  for (auto bytes : {ram(0, 8), ram(0x00800000, 8), ram(0x2003FFFE, 8),
                     ram(0x20001000, 256)})
    EXPECT_FALSE(f.serial.transmit(bytes).has_value());
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  EXPECT_FALSE(f.serial.transmit(ram()).has_value());
  ASSERT_TRUE(rx->cancel().has_value());
  (void)rx->take();
}
TEST(nrf_uarte, rx_requires_rxto_then_private_fifo_flush_before_publishing,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.rx_stop_responds = false;
  f.mmio.flush_responds = false;
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  f.rx_done(8);
  (void)f.serial.poll();
  EXPECT_FALSE(rx->ready());
  EXPECT_EQ(f.mmio.flushes, 0U);
  f.mmio.poke(f.base() + 0x144, 1);
  (void)f.serial.poll();
  EXPECT_FALSE(rx->ready());
  EXPECT_EQ(f.mmio.peek(f.base() + 0x534), 0x2003FFF0U);
  EXPECT_EQ(f.mmio.peek(f.base() + 0x538), 5U);
  EXPECT_EQ(f.mmio.flushes, 1U);
  (void)f.serial.poll();
  EXPECT_EQ(f.mmio.flushes, 1U);
  f.mmio.poke(f.base() + 0x53C, 0);
  f.mmio.poke(f.base() + 0x110, 1);
  ASSERT_TRUE(f.serial.poll());
  EXPECT_EQ(*rx->take(), 8U);
}
TEST(nrf_uarte,
     explicit_stop_reports_partial_count_and_residual_fifo_is_observable,
     "REQ-0059") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  f.mmio.poke(f.base() + 0x53C, 3);
  ASSERT_TRUE(f.serial.stop_receive().has_value());
  ASSERT_TRUE(f.serial.poll());
  EXPECT_EQ(*rx->take(), 3U);
  auto next = f.serial.receive(ram());
  ASSERT_TRUE(next.has_value());
  f.mmio.fifo_bytes = 2;
  f.rx_done(8);
  ASSERT_TRUE(f.serial.poll());
  auto result = next->take();
  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(result.error().cause == driver::uart_error::overrun);
  EXPECT_EQ(f.serial.received_count(), 8U);
}
TEST(nrf_uarte, receive_error_stops_dma_and_retains_actual_count_and_source,
     "REQ-0059") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  f.mmio.poke(f.base() + 0x53C, 2);
  f.mmio.poke(f.base() + 0x480, 2);
  f.mmio.poke(f.base() + 0x124, 1);
  ASSERT_TRUE(f.serial.poll());
  auto r = rx->take();
  ASSERT_FALSE(r.has_value());
  EXPECT_TRUE(r.error().cause == driver::uart_error::parity);
  EXPECT_EQ(f.serial.received_count(), 2U);
  EXPECT_EQ(f.serial.error_source(), 2U);
}
TEST(nrf_uarte,
     cancelled_rx_retains_memory_until_late_ack_and_shutdown_recovery,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.rx_stop_responds = false;
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  f.execution.set_time(0xFFFFFFFC);
  EXPECT_FALSE(rx->cancel().has_value());
  EXPECT_TRUE(rx->take().error().kind ==
              core::transfer_error_kind::quiesce_failed);
  EXPECT_TRUE(f.serial.faulted());
  EXPECT_FALSE(f.serial.shutdown().has_value());
  EXPECT_FALSE(
      f.resources.claim(f.execution, peripheral::uart0, &f).has_value());
  f.mmio.poke(f.base() + 0x144, 1);
  ASSERT_TRUE(f.serial.shutdown().has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::uarte0));
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_uarte,
     start_write_failure_after_commit_is_quiesced_before_returning_memory,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.base();
  f.mmio.fail_after_commit = f.mmio.fail_once = true;
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  EXPECT_FALSE(rx->ready());
  EXPECT_TRUE(f.serial.faulted());
  ASSERT_TRUE(f.serial.poll());
  EXPECT_FALSE(rx->take().has_value());
  ASSERT_TRUE(f.serial.shutdown().has_value());
}
TEST(nrf_uarte, prestart_failure_discards_prepared_leases_and_allows_a_retry,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.base() + 0x548;
  f.mmio.fail_once = true;
  auto tx = f.serial.transmit(ram());
  ASSERT_TRUE(tx.has_value());
  EXPECT_FALSE(tx->take().has_value());
  EXPECT_EQ(f.mmio.starts, 0U);
  auto next = f.serial.transmit(ram());
  ASSERT_TRUE(next.has_value());
  f.tx_done(8);
  ASSERT_TRUE(f.serial.poll());
  ASSERT_TRUE(next->take().has_value());
}
TEST(nrf_uarte,
     short_tx_count_is_failure_and_late_end_cannot_finish_a_new_transfer,
     "REQ-0059") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto tx = f.serial.transmit(ram());
  ASSERT_TRUE(tx.has_value());
  f.tx_done(2);
  ASSERT_TRUE(f.serial.poll());
  EXPECT_FALSE(tx->take().has_value());
  f.tx_done(8);
  auto next = f.serial.transmit(ram());
  ASSERT_TRUE(next.has_value());
  (void)f.serial.poll();
  EXPECT_FALSE(next->ready());
  f.tx_done(8);
  ASSERT_TRUE(f.serial.poll());
  ASSERT_TRUE(next->take().has_value());
}
TEST(nrf_uarte, interrupt_cancel_is_bounded_and_preserves_faulted_dma_leases,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto tx = f.serial.transmit(ram());
  ASSERT_TRUE(tx.has_value());
  f.mmio.tx_stop_responds = false;
  f.execution.set_isr(true);
  auto reads = f.mmio.reads;
  EXPECT_FALSE(tx->cancel().has_value());
  EXPECT_TRUE(f.mmio.reads - reads < 8U);
  f.execution.set_isr(false);
  (void)tx->take();
  f.mmio.poke(f.base() + 0x158, 1);
  ASSERT_TRUE(f.serial.shutdown().has_value());
}
TEST(nrf_uarte, reviewed_silicon_supports_full_count_and_two_stop_bits,
     "REQ-0059") {
  fixture f;
  f.mmio.poke(0x10000130, 8);
  f.mmio.poke(0x10000134, 5);
  ASSERT_TRUE(f.clocks.initialize().has_value());
  auto timing = solve_uart_baud(peripheral::uarte0, {16000000, 115200, 10000});
  ASSERT_TRUE(
      f.serial.initialize(*timing, {.stop_bits = driver::uart_stop_bits::two})
          .has_value());
  EXPECT_EQ(f.mmio.peek(f.base() + 0x56C), 16U);
  auto tx = f.serial.transmit(ram(0x20000000, 65535));
  ASSERT_TRUE(tx.has_value());
  f.tx_done(65535);
  ASSERT_TRUE(f.serial.poll());
  ASSERT_TRUE(tx->take().has_value());
  EXPECT_FALSE(f.serial.transmit(ram(0x20000000, 65536)).has_value());
}
TEST(nrf_uarte, irq_events_are_acknowledged_across_stop_and_fifo_flush,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.serial.interrupt_delivery(true).has_value());
  auto tx = f.serial.transmit(ram());
  ASSERT_TRUE(tx.has_value());
  f.mmio.tx_stop_responds = false;
  f.tx_done(8);
  EXPECT_TRUE(f.mmio.pending_irq());
  (void)f.serial.poll();
  EXPECT_FALSE(f.mmio.pending_irq());
  f.mmio.poke(f.base() + 0x158, 1);
  ASSERT_TRUE(f.serial.poll());
  ASSERT_TRUE(tx->take().has_value());
  EXPECT_FALSE(f.mmio.pending_irq());
  f.mmio.rx_stop_responds = false;
  f.mmio.flush_responds = false;
  auto rx = f.serial.receive(ram());
  ASSERT_TRUE(rx.has_value());
  f.rx_done(8);
  (void)f.serial.poll();
  EXPECT_FALSE(f.mmio.pending_irq());
  f.mmio.poke(f.base() + 0x144, 1);
  (void)f.serial.poll();
  EXPECT_FALSE(f.mmio.pending_irq());
  f.mmio.poke(f.base() + 0x53C, 0);
  f.mmio.poke(f.base() + 0x110, 1);
  ASSERT_TRUE(f.serial.poll());
  EXPECT_EQ(*rx->take(), 8U);
  EXPECT_FALSE(f.mmio.pending_irq());
}
TEST(nrf_uarte, stop_proof_survives_a_failed_event_clear_after_commit,
     "REQ-0059", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto tx = f.serial.transmit(ram());
  ASSERT_TRUE(tx.has_value());
  f.tx_done(8);
  f.mmio.fail_terminal_ack = true;
  (void)f.serial.poll();
  EXPECT_FALSE(tx->ready());
  f.mmio.tx_stop_responds = false;
  ASSERT_TRUE(f.serial.poll());
  EXPECT_FALSE(tx->take().has_value());
  EXPECT_EQ(f.serial.transmitted_count(), 8U);
  ASSERT_TRUE(f.serial.shutdown().has_value());
}
} // namespace
