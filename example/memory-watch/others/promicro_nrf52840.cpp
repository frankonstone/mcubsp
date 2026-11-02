// @req-file REQ-0072 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/factory.hpp>
#include <mcubsp/backend/nrf52840/mwu.hpp>
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

static n::factory_information info{context};
static n::memory_watch_controller watch{context};
alignas(4) static volatile std::uint32_t watched{};
int main() {
  if (!begin() || !info.geometry() || !info.device_address() ||
      !watch.initialize())
    finish(1);
  // No IRQ/NMI is enabled. Poll CPU access events; DMA accesses do not count.
  if (!watch.watch(
          0, {reinterpret_cast<std::uintptr_t>(&watched), sizeof(watched)},
          true, true))
    finish(2);
  watched = 123;
  auto value = watched;
  __asm volatile("" ::: "memory");
  auto events = watch.poll();
  if (!events || !(events->reads & 1) || !(events->writes & 1) || value != 123)
    finish(3);
  if (!watch.shutdown() || !clocks.shutdown())
    finish(4);
  finish(0x600D);
}
