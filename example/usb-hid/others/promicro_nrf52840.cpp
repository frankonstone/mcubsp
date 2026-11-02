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
static n::usb_device<decltype(usb), n::usb_profile::hid_keyboard, 0xFFFF, 1>
    device{usb};
int main() {
  if (!begin() || !device.initialize())
    finish(1);

  // Read-only keyboard: sends the empty (all keys released) report once after
  // each enumeration. A physical input application can call send_report().
  bool sent{};
  for (;;) {
    if (!device.poll())
      finish(2);
    if (!device.configured())
      sent = false;
    if (device.configured() && !sent && !usb.suspended()) {
      if (!device.send_report({}))
        finish(3);
      sent = true;
      mcubsp_demo_count = mcubsp_demo_count + 1;
    }
  }
}
