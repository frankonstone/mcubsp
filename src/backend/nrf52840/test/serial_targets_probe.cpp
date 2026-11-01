// @req-file REQ-0059 REQ-0060 REQ-0061 REQ-0062
#include <mcubsp/backend/nrf52840/spis.hpp>
#include <mcubsp/backend/nrf52840/spi.hpp>
#include <mcubsp/backend/nrf52840/twis.hpp>
#include <mcubsp/backend/nrf52840/uart.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
namespace {
using namespace mcubsp;
namespace n = backend::nrf52840;
using C = n::system_context<core::direct_mmio, execution::fake_policy>;
using B = n::dma_buffer_pool<execution::fake_policy>;
template <unsigned I>
void spi_target(C &c, n::clock_controller<C> &clocks, B &buffers) {
  using Binding = n::spis_binding<I, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
  n::spis_controller<Binding, C, B> target{c, clocks, buffers};
  if (!target.initialize())
    return;
  std::array<std::uint8_t, 8> tx{}, rx{};
  auto op = target.transfer(tx, rx);
  if (op) {
    (void)target.interrupt_delivery(true);
    (void)target.poll();
    (void)op->cancel();
    (void)op->take();
  }
  (void)target.transferred_count();
  (void)target.shutdown();
}
template <unsigned I>
void twi_target(C &c, n::clock_controller<C> &clocks, B &buffers) {
  using Binding = n::twis_binding<I, {0, 6}, {0, 8}>;
  n::twis_controller<Binding, C, B> target{c, clocks, buffers};
  if (!target.initialize({{0x42}, driver::i2c_address{0x43}}))
    return;
  std::array<std::uint8_t, 8> tx{}, rx{};
  auto op = target.listen();
  if (op) {
    (void)target.interrupt_delivery(true);
    (void)target.poll();
    auto request = target.pending_request();
    if (request) {
      if (request->direction == n::twis_direction::master_read)
        (void)target.provide_transmit(tx);
      else
        (void)target.provide_receive(rx);
      (void)target.resume();
    }
    (void)op->cancel();
    (void)op->take();
  }
  (void)target.transferred_count();
  (void)target.shutdown();
}
template <unsigned I> void spi_cpu(C &c, n::clock_controller<C> &clocks) {
  using Binding = n::spi_binding<I, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
  n::spi_controller<Binding, C> bus{c, clocks};
  if (!bus.initialize(*n::solve_spi_timing(I, {16000000, 1000000, 0})))
    return;
  std::array<std::uint8_t, 8> tx{}, rx{};
  std::array segments{driver::byte_segment(tx, rx)};
  auto op = bus.transaction(segments);
  if (op) {
    (void)bus.poll();
    (void)op->cancel();
    (void)op->take();
  }
  (void)bus.shutdown();
}
} // namespace
void nrf_serial_targets_probe(C &c, n::clock_controller<C> &clocks,
                              B &buffers) {
  spi_target<0>(c, clocks, buffers);
  spi_target<1>(c, clocks, buffers);
  spi_target<2>(c, clocks, buffers);
  twi_target<0>(c, clocks, buffers);
  twi_target<1>(c, clocks, buffers);
  spi_cpu<0>(c, clocks);
  spi_cpu<1>(c, clocks);
  spi_cpu<2>(c, clocks);
}
