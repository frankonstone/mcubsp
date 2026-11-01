// @req-file REQ-0064 REQ-0065 REQ-0071
#include <mcubsp/backend/nrf52840/qdec.hpp>
#include <mcubsp/backend/nrf52840/rng.hpp>
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
static n::temperature_controller temperature{context, clocks};
static n::rng_controller random_source{context};
using encoder_pins = n::qdec_binding<{0, 6}, {0, 8}>;
static n::qdec_controller<encoder_pins, decltype(context)> decoder{context,
                                                                   clocks};
extern "C" {
volatile std::uint32_t mcubsp_native_status{}, mcubsp_native_rounds{},
    mcubsp_rng_byte_count{};
volatile std::int32_t mcubsp_die_temperature_quarters{}, mcubsp_encoder_delta{};
volatile std::uint32_t mcubsp_encoder_double_transitions{},
    mcubsp_encoder_overflow{};
}
[[noreturn]] static void fail(unsigned code) {
  mcubsp_native_status = 0x80000000U | code;
  (void)gpio.set(profile::led_builtin);
  for (;;)
    __asm volatile("nop");
}
int main() {
  if (!profile::startup_ok())
    fail(1);
  mcubsp::io::detail::cortex_m_cycle_hooks<64000000>::begin();
  if (!gpio.claim(profile::led_builtin,
                  mcubsp::driver::gpio_direction::output) ||
      !gpio.configure(profile::led_builtin,
                      {.direction = mcubsp::driver::gpio_direction::output}))
    fail(2);
  if (!clocks.initialize() || !temperature.initialize() ||
      !random_source.initialize() || !decoder.initialize() || !decoder.start())
    fail(3);
  bool light{};
  for (;;) {
    auto sample = temperature.sample();
    if (!sample)
      fail(4);
    std::array<std::uint8_t, 16> bytes{};
    auto entropy = random_source.generate(bytes);
    if (!entropy) {
      (void)sample->cancel();
      (void)sample->take();
      fail(5);
    }
    const auto start = execution.now();
    while (!sample->ready() || !entropy->ready()) {
      (void)temperature.poll();
      (void)random_source.poll();
      if (std::uint32_t(execution.now() - start) > 64000000U) {
        (void)sample->cancel();
        (void)entropy->cancel();
        (void)sample->take();
        (void)entropy->take();
        fail(6);
      }
    }
    auto result = sample->take();
    auto generated = entropy->take();
    auto movement = decoder.read_clear();
    if (!result || !generated || !movement)
      fail(7);
    mcubsp_die_temperature_quarters = result->quarter_celsius;
    mcubsp_rng_byte_count = static_cast<std::uint32_t>(*generated);
    mcubsp_encoder_delta = movement->movement;
    mcubsp_encoder_double_transitions = movement->double_transitions;
    mcubsp_encoder_overflow = movement->overflow;
    mcubsp_native_status = 1;
    mcubsp_native_rounds = mcubsp_native_rounds + 1;
    light = !light;
    if (!(light ? gpio.set(profile::led_builtin)
                : gpio.clear(profile::led_builtin)))
      fail(8);
    const auto pause = execution.now();
    while (std::uint32_t(execution.now() - pause) < 16000000U)
      __asm volatile("nop");
  }
}
