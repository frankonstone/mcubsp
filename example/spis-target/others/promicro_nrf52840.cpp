// @req-file REQ-0060 REQ-0062
#include <mcubsp/backend/nrf52840/spis.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/core/direct_mmio.hpp>
namespace n = mcubsp::backend::nrf52840;
using profile = mcubsp::board::others::promicro_nrf52840;
static mcubsp::core::direct_mmio mmio;
static profile::default_execution execution;
static n::gpio_backend gpio{mmio};
static n::system_context context{mmio, execution, gpio};
static n::clock_controller clocks{context};
static n::dma_buffer_pool<profile::default_execution> buffers{context.resources,
                                                              execution};
using binding = n::spis_binding<0, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
static n::spis_controller<binding, decltype(context), decltype(buffers)> target{
    context, clocks, buffers};
extern "C" {
volatile std::uint32_t mcubsp_spis_status{}, mcubsp_spis_tx{}, mcubsp_spis_rx{},
    mcubsp_spis_flags{};
}
[[noreturn]] static void fail(unsigned code) {
  mcubsp_spis_status = 0x80000000U | code;
  for (;;)
    __asm volatile("nop");
}
int main() {
  if (!profile::startup_ok())
    fail(1);
  mcubsp::io::detail::cortex_m_cycle_hooks<64'000'000>::begin();
  if (!clocks.initialize() || !target.initialize())
    fail(2);
  static std::array<std::uint8_t, 8> tx{0,   0x55, 0xAA, 0xFF,
                                        'S', 'P',  'I',  'S'};
  static std::array<std::uint8_t, 8> rx{};
  // One explicit submission. External master uses mode 0, MSB first, <=8 MHz.
  auto op = target.transfer(tx, rx);
  if (!op)
    fail(3);
  mcubsp_spis_status = 2;
  const auto begin = execution.now();
  while (!op->ready()) {
    (void)target.poll();
    if (std::uint32_t(execution.now() - begin) > 640'000'000U) {
      (void)op->cancel();
      (void)op->take();
      fail(4);
    }
  }
  auto result = op->take();
  if (!result)
    fail(5);
  mcubsp_spis_tx = result->transmitted;
  mcubsp_spis_rx = result->received;
  mcubsp_spis_flags = result->overread | (result->overflow << 1);
  if (!target.shutdown() || !clocks.shutdown())
    fail(6);
  mcubsp_spis_status = 1;
  for (;;)
    __asm volatile("nop");
}
