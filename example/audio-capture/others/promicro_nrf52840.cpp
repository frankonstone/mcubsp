// @req-file REQ-0067 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/i2s.hpp>
#include <mcubsp/backend/nrf52840/pdm.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/core/direct_mmio.hpp>
using profile = mcubsp::board::others::promicro_nrf52840;
namespace n = mcubsp::backend::nrf52840;
static mcubsp::core::direct_mmio mmio;
static profile::default_execution execution;
static n::gpio_backend gpio{mmio};
static n::system_context context{mmio, execution, gpio};
static n::clock_controller clocks{context};
static n::dma_buffer_pool<profile::default_execution, 24> buffers{
    gpio.resources(), execution};
extern "C" {
volatile unsigned mcubsp_demo_status{}, mcubsp_demo_count{};
}
[[noreturn]] static void finish(unsigned code) {
  mcubsp_demo_status = code;
  for (;;)
    __asm volatile("nop");
}
static bool begin() {
  if (!profile::startup_ok())
    return false;
  mcubsp::io::detail::cortex_m_cycle_hooks<64000000>::begin();
  return clocks.initialize().has_value();
}

using ipins = n::i2s_binding<{0, 20}, {0, 21}, {0, 22}, {0, 23}, {0, 24}>;
using ppins = n::pdm_binding<{0, 17}, {0, 19}>;
static n::i2s_controller<ipins, decltype(context), decltype(buffers)> i2s{
    context, clocks, buffers};
static n::pdm_controller<ppins, decltype(context), decltype(buffers)> pdm{
    context, clocks, buffers};
alignas(4) static std::array<std::array<std::uint32_t, 256>, 3> tx{}, rx{};
alignas(4) static std::array<std::array<std::int16_t, 512>, 3> microphone{};
int main() {
  if (!begin() || !i2s.initialize() || !pdm.initialize())
    finish(1);
  if (!i2s.start({tx[0], rx[0], 0}, {tx[2], rx[2], 2}, 2) ||
      !i2s.enqueue({tx[1], rx[1], 1}) ||
      !pdm.start(microphone[0], microphone[2], 2) ||
      !pdm.enqueue(microphone[1]))
    finish(2);
  while (i2s.running() || pdm.running()) {
    if (!i2s.poll() || !pdm.poll())
      finish(3);
    if (i2s.take_completed())
      mcubsp_demo_count = mcubsp_demo_count + 1;
    if (pdm.take_completed())
      mcubsp_demo_count = mcubsp_demo_count + 1;
  }
  if (i2s.result().service_overrun || pdm.result().service_overrun ||
      i2s.result().completed_blocks != 2 || pdm.result().completed_blocks != 2)
    finish(4);
  if (!i2s.shutdown() || !pdm.shutdown() || !clocks.shutdown())
    finish(5);
  finish(0x600D);
}
