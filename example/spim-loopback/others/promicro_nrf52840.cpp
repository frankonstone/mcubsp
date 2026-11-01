// @req-file REQ-0060 REQ-0062
#include <mcubsp/backend/nrf52840/spim.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/core/direct_mmio.hpp>
using profile = mcubsp::board::others::promicro_nrf52840;
namespace n = mcubsp::backend::nrf52840;
using binding = n::spim_binding<0, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
static mcubsp::core::direct_mmio mmio;
static profile::default_execution execution;
static n::gpio_backend gpio{mmio};
static n::system_context context{mmio, execution, gpio};
static n::clock_controller clocks{context};
static n::dma_buffer_pool<profile::default_execution> buffers{context.resources,
                                                              execution};
static n::spim_controller<binding, decltype(context), decltype(buffers)> serial{
    context, clocks, buffers};
extern "C" {
volatile std::uint32_t mcubsp_spim_status{}, mcubsp_spim_rounds{};
}
[[noreturn]] static void fail(unsigned code) {
  mcubsp_spim_status = 0x80000000U | code;
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
      n::solve_spim_timing(binding::instance, {16'000'000, 1'000'000, 0});
  if (!timing || !serial.initialize(*timing))
    fail(4);
  static std::array<std::uint8_t, 8> output{0x00, 0x55, 0xAA, 0xFF,
                                            'S',  'P',  'I',  '0'};
  std::array<std::uint8_t, 8> input{};
  bool light{};
  for (;;) {
    auto transfer = serial.transfer(output, input);
    if (!transfer)
      fail(5);
    const auto start = execution.now();
    while (!transfer->ready()) {
      (void)serial.poll();
      if (std::uint32_t(execution.now() - start) > 64'000'000U) {
        (void)transfer->cancel();
        (void)transfer->take();
        fail(7);
      }
    }
    const auto received = transfer->take();
    if (!received || received->received != output.size() || input != output)
      fail(8);
    mcubsp_spim_status = 1;
    mcubsp_spim_rounds = mcubsp_spim_rounds + 1;
    light = !light;
    if (!(light ? gpio.set(profile::led_builtin)
                : gpio.clear(profile::led_builtin)))
      fail(9);
    const auto pause = execution.now();
    while (std::uint32_t(execution.now() - pause) < 16'000'000U)
      __asm volatile("nop");
  }
}
