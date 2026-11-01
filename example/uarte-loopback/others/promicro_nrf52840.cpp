// @req-file REQ-0059 REQ-0062
#include <mcubsp/backend/nrf52840/uarte.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/core/direct_mmio.hpp>
using profile = mcubsp::board::others::promicro_nrf52840;
namespace n = mcubsp::backend::nrf52840;
using binding = n::uart_binding<n::peripheral::uarte0, {0, 6}, {0, 8}>;
static mcubsp::core::direct_mmio mmio;
static profile::default_execution execution;
static n::gpio_backend gpio{mmio};
static n::system_context context{mmio, execution, gpio};
static n::clock_controller clocks{context};
static n::dma_buffer_pool<profile::default_execution> buffers{context.resources,
                                                              execution};
static n::uarte_controller<binding, decltype(context), decltype(buffers)>
    serial{context, clocks, buffers};
extern "C" {
volatile std::uint32_t mcubsp_uart_status{}, mcubsp_uart_rounds{};
}
[[noreturn]] static void fail(unsigned code) {
  mcubsp_uart_status = 0x80000000U | code;
  (void)gpio.set(profile::led_builtin);
  for (;;)
    __asm volatile("nop");
}
int main() {
  if (!profile::startup_ok())
    fail(1);
  mcubsp::io::detail::cortex_m_cycle_hooks<64'000'000>::begin();
  if (!gpio.claim(profile::led_builtin,
                  mcubsp::driver::gpio_direction::output) ||
      !gpio.configure(profile::led_builtin,
                      {.direction = mcubsp::driver::gpio_direction::output}))
    fail(2);
  if (!clocks.initialize())
    fail(3);
  const auto timing =
      n::solve_uart_baud(binding::instance, {16'000'000, 115200, 10000});
  if (!timing || !serial.initialize(*timing))
    fail(4);
  static std::array<std::uint8_t, 8> output{0x00, 0x55, 0xAA, 0xFF,
                                            'D',  'M',  'A',  '0'};
  std::array<std::uint8_t, 8> input{};
  bool light{};
  for (;;) {
    auto rx = serial.receive(input);
    if (!rx)
      fail(5);
    auto tx = serial.transmit(output);
    if (!tx) {
      (void)rx->cancel();
      (void)rx->take();
      fail(6);
    }
    const auto start = execution.now();
    while (!rx->ready() || !tx->ready()) {
      (void)serial.poll();
      if (std::uint32_t(execution.now() - start) > 64'000'000U) {
        (void)rx->cancel();
        (void)tx->cancel();
        (void)rx->take();
        (void)tx->take();
        fail(7);
      }
    }
    const auto received = rx->take();
    const auto sent = tx->take();
    if (!received || !sent || *received != output.size() || input != output)
      fail(8);
    mcubsp_uart_status = 1;
    mcubsp_uart_rounds = mcubsp_uart_rounds + 1;
    light = !light;
    if (!(light ? gpio.set(profile::led_builtin)
                : gpio.clear(profile::led_builtin)))
      fail(9);
    const auto pause = execution.now();
    while (std::uint32_t(execution.now() - pause) < 16'000'000U)
      __asm volatile("nop");
  }
}
