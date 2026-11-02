// @req-file REQ-0066 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/qspi.hpp>
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

using pins =
    n::qspi_binding<{0, 19}, {0, 17}, {0, 20}, {0, 21}, {0, 22}, {0, 23}>;
static n::qspi_controller<pins, decltype(context), decltype(buffers)> flash{
    context, clocks, buffers};
alignas(4) static std::array<std::uint32_t, 4> tx{0x12345678, 0xABCDEF01,
                                                  0x4D435542, 0xFFFF0000},
    rx{};
static bool wait() {
  auto until = execution.now();
  while (flash.busy()) {
    auto done = flash.poll();
    if (!done)
      return false;
    if (std::uint32_t(execution.now() - until) > 128000000U)
      return false;
  }
  return true;
}
int main() {
  if (!begin())
    finish(1);
  // Explicit bench contract: attached 1 MiB, 256-byte-page NOR, fast single
  // read/02h page program/20h 4 KiB erase/05h WIP bit0. Only its last sector is
  // writable. This is external flash; internal flash/UICR are never modified.
  n::qspi_device device{.capacity_bytes = 0x100000,
                        .maximum_clock_hz = 32000000,
                        .operation_timeout = 128000000};
  std::array<mcubsp::mcu::nrf52840::memory::region, 1> protected_ranges{
      {{0, 0xFF000}}};
  if (!flash.initialize(device, {}, {0xFF000, 4096}, protected_ranges))
    finish(2);
  if (!flash.erase(0xFF000, 4096) || !wait() || !flash.program(0xFF000, tx) ||
      !wait() || !flash.read(0xFF000, rx) || !wait() || rx != tx)
    finish(3);
  auto mapping = flash.borrow_xip(0xFF000, 16,
                                  {.reviewed_linker_and_read_accesses = true});
  if (!mapping || !flash.copy_xip_words(*mapping, 0, rx) ||
      !flash.return_xip(*mapping) || rx != tx)
    finish(4);
  if (!flash.shutdown() || !clocks.shutdown())
    finish(5);
  finish(0x600D);
}
