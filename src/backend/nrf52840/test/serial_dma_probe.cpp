// @req-file REQ-0059 REQ-0060 REQ-0061 REQ-0062 REQ-0064 REQ-0065 REQ-0071
// REQ-0072
#include <mcubsp/backend/nrf52840/nvmc.hpp>
#include <mcubsp/backend/nrf52840/qdec.hpp>
#include <mcubsp/backend/nrf52840/rng.hpp>
#include <mcubsp/backend/nrf52840/spim.hpp>
#include <mcubsp/backend/nrf52840/temperature.hpp>
#include <mcubsp/backend/nrf52840/twim.hpp>
#include <mcubsp/backend/nrf52840/uarte.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
namespace {
using namespace mcubsp;
namespace n = backend::nrf52840;
using context = n::system_context<core::direct_mmio, execution::fake_policy>;
using buffers = n::dma_buffer_pool<execution::fake_policy, 8>;
template <n::peripheral P>
void uart(context &c, n::clock_controller<context> &clock, buffers &pool) {
  using binding = n::uart_binding<P, {0, 6}, {0, 8}>;
  n::uarte_controller<binding, context, buffers, n::native_dma_address, 16>
      serial{c, clock, pool, 10};
  auto timing = n::solve_uart_baud(P, {16000000, 115200, 10000});
  if (!serial.initialize(*timing))
    return;
  std::array<std::uint8_t, 8> bytes{};
  auto tx = serial.transmit(bytes);
  if (tx) {
    (void)serial.poll();
    (void)tx->cancel();
    (void)tx->take();
  }
  auto rx = serial.receive(bytes);
  if (rx) {
    (void)serial.stop_receive();
    (void)serial.poll();
    (void)rx->cancel();
    (void)rx->take();
  }
  (void)serial.interrupt_delivery(false);
  (void)serial.shutdown();
}
template <unsigned I>
void spi(context &c, n::clock_controller<context> &clock, buffers &pool) {
  using binding = n::spim_binding<I, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
  n::spim_controller<binding, context, buffers, n::native_dma_address, 2, 16>
      bus{c, clock, pool, 10};
  auto timing = n::solve_spim_timing(I, {16000000, 1000000, 0});
  if (!bus.initialize(*timing, {}, {0}))
    return;
  std::array<std::uint8_t, 8> out{}, in{};
  auto t = bus.transfer(out, in);
  if (t) {
    (void)bus.poll();
    (void)t->cancel();
    (void)t->take();
  }
  (void)bus.interrupt_delivery(false);
  (void)bus.shutdown();
}
template <unsigned I>
void i2c(context &c, n::clock_controller<context> &clock, buffers &pool) {
  using binding = n::i2c_binding<I, {0, 17}, {0, 20}>;
  n::twim_controller<binding, context, buffers, n::native_dma_address, 16> bus{
      c, clock, pool, 10};
  auto timing = n::solve_twim_timing({16000000, 100000, 0, 300, 100});
  if (!bus.initialize(*timing))
    return;
  std::array<std::uint8_t, 8> out{}, in{};
  auto t = bus.write_read({0x48}, out, in);
  if (t) {
    (void)bus.poll();
    (void)t->cancel();
    (void)t->take();
  }
  auto r = bus.read_write({0x48}, in, out);
  if (r) {
    (void)r->cancel();
    (void)r->take();
  }
  (void)bus.interrupt_delivery(false);
  (void)bus.shutdown();
}
} // namespace
void nrf_serial_dma_probe(context &c, n::clock_controller<context> &clock,
                          buffers &pool) {
  uart<n::peripheral::uarte0>(c, clock, pool);
  uart<n::peripheral::uarte1>(c, clock, pool);
  spi<0>(c, clock, pool);
  spi<1>(c, clock, pool);
  spi<2>(c, clock, pool);
  spi<3>(c, clock, pool);
  i2c<0>(c, clock, pool);
  i2c<1>(c, clock, pool);
}
void nrf_local_services_probe(context &c, n::clock_controller<context> &clock,
                              n::power_controller<context> &power) {
  n::temperature_controller<context> temperature{c, clock};
  if (temperature.initialize()) {
    auto t = temperature.sample();
    if (t) {
      (void)temperature.poll();
      (void)t->cancel();
      (void)t->take();
    }
    (void)temperature.interrupt_delivery(false);
    (void)temperature.shutdown();
  }
  n::rng_controller<context> random{c};
  if (random.initialize()) {
    std::array<std::uint8_t, 8> bytes{};
    auto t = random.generate(bytes);
    if (t) {
      (void)random.poll();
      (void)t->cancel();
      (void)t->take();
    }
    (void)random.interrupt_delivery(false);
    (void)random.shutdown();
  }
  using pins = n::qdec_binding<{0, 6}, {0, 8}>;
  n::qdec_controller<pins, context, 16> decoder{c, clock, 10};
  if (decoder.initialize()) {
    (void)decoder.start();
    (void)decoder.read_clear();
    (void)decoder.last_sample();
    (void)decoder.consume(n::qdec_event::sample);
    (void)decoder.snapshot();
    (void)decoder.task(n::qdec_task::read_clear_both);
    (void)decoder.event(n::qdec_event::report);
    (void)decoder.interrupt_delivery(31, false);
    (void)decoder.stop();
    (void)decoder.shutdown();
  }
  n::nvmc_controller<context, n::power_controller<context>, 16> flash{c, power,
                                                                      10};
  const std::array protected_ranges{
      mcu::nrf52840::memory::region{0, 0x26000},
      mcu::nrf52840::memory::region{0xEC000, 0x14000}};
  if (flash.initialize({0xE0000, 4096}, protected_ranges)) {
    (void)flash.program_word(0xE0000, 0x12345678);
    (void)flash.erase_page(0xE0000);
    (void)flash.shutdown();
  }
}
