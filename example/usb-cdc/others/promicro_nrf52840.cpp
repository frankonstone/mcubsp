// @req-file REQ-0068 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/usb_device.hpp>
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

static n::usbd_controller<decltype(context), decltype(buffers)> usb{
    context, clocks, buffers};
// Experimental bench IDs, replace with your assigned VID/PID before release.
static n::usb_device<decltype(usb), n::usb_profile::cdc_acm, 0xFFFF, 1> device{
    usb};
int main() {
  if (!begin() || !device.initialize())
    finish(1);

  std::array<std::uint8_t, 64> echo{};
  std::size_t pending{};
  for (;;) {
    if (!device.poll())
      finish(2);
    if (!pending)
      pending = device.read(echo);
    if (pending && device.configured() && !device.transmitting() &&
        !usb.suspended()) {
      if (!device.write(std::span{echo}.first(pending)))
        finish(3);
      mcubsp_demo_count = mcubsp_demo_count + pending;
      pending = 0;
    }
  }
}
