// @req-file REQ-0063 REQ-0064 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/pwm.hpp>
#include <mcubsp/backend/nrf52840/saadc.hpp>
#include <mcubsp/backend/nrf52840/temperature.hpp>
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

using pins = n::pwm_binding<0, {0, 15}>;
static n::pwm_controller<pins, decltype(context), decltype(buffers)> pwm{
    context, clocks, buffers};
static n::saadc_controller<decltype(context), decltype(buffers)> adc{
    context, clocks, buffers};
static n::temperature_controller temperature{context, clocks};
alignas(4) static std::array<std::uint16_t, 4> duty{8000, 8000, 8000, 8000};
alignas(4) static std::array<std::int16_t, 32> samples{};
extern "C" {
volatile std::int16_t mcubsp_adc_sample{};
}
int main() {
  if (!begin())
    finish(1);
  auto timing = n::solve_pwm_timing({16000000, 1000, 0, 15});
  std::array<n::saadc_channel_configuration, 1> channel{};
  if (!timing || !pwm.initialize(*timing) || !adc.initialize(channel) ||
      !temperature.initialize())
    finish(2);
  auto calibration = adc.calibrate();
  if (!calibration)
    finish(3);
  while (!calibration->ready())
    if (!adc.poll())
      finish(4);
  if (!calibration->take())
    finish(5);
  auto light = pwm.play({duty}, {duty}, {.loops = 1, .repeat = true});
  if (!light)
    finish(6);
  for (unsigned i = 0; i < 100; ++i) {
    auto scan = adc.sample(samples);
    auto temp = temperature.sample();
    if (!scan || !temp)
      finish(7);
    while (!scan->ready() || !temp->ready())
      if (!adc.poll() || !temperature.poll() || !pwm.poll())
        finish(8);
    if (!scan->take() || !temp->take())
      finish(9);
    mcubsp_adc_sample = samples[0];
    mcubsp_demo_count = i + 1;
  }
  if (!light->cancel() || !light->take() || !adc.shutdown() ||
      !pwm.shutdown() || !temperature.shutdown() || !clocks.shutdown())
    finish(10);
  finish(0x600D);
}
