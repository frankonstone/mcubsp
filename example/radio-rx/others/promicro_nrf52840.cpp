// @req-file REQ-0069 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/radio.hpp>
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

static n::radio_controller<decltype(context), decltype(buffers)> radio{
    context, clocks, buffers};
alignas(4) static std::array<std::uint8_t, 258> packet{};
extern "C" {
volatile unsigned mcubsp_radio_crc_errors{};
}
int main() {
  if (!begin())
    finish(1);
  n::radio_configuration config{}; // Select any of the seven PHYs explicitly.
  config.phy = n::radio_phy::ble1;
  config.frequency_mhz = 2402;
  if (!radio.initialize(config))
    finish(2);
  const auto header = (config.phy == n::radio_phy::nordic1 ||
                       config.phy == n::radio_phy::nordic2 ||
                       config.phy == n::radio_phy::ieee802154)
                          ? 1U
                          : 2U;
  for (;;) {

    if (!radio.receive(packet))
      finish(3);

    bool complete{};
    auto start = execution.now();
    while (!complete) {
      auto result = radio.poll();
      if (!result)
        finish(4);
      if (*result) {
        if ((**result).service_overrun || (**result).invalid_coding)
          finish(5);

        if ((**result).crc_valid && packet[header] == 0x4D &&
            packet[header + 1] == 0x43)
          mcubsp_demo_count = mcubsp_demo_count + 1;
        else
          mcubsp_radio_crc_errors = mcubsp_radio_crc_errors + 1;

        complete = true;
      }
      if (std::uint32_t(execution.now() - start) > 64000000U) {
        if (!radio.cancel())
          finish(6);
        complete = true;
      }
    }
  }
}
