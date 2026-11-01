// @req-file REQ-0061 REQ-0062
#include <mcubsp/backend/nrf52840/twis.hpp>
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
using binding = n::twis_binding<0, {0, 11}, {1, 0}>;
static n::twis_controller<binding, decltype(context), decltype(buffers)> target{
    context, clocks, buffers};
extern "C" {
volatile std::uint32_t mcubsp_twis_status{}, mcubsp_twis_tx{}, mcubsp_twis_rx{},
    mcubsp_twis_errors{}, mcubsp_twis_slot{};
}
[[noreturn]] static void fail(unsigned code) {
  mcubsp_twis_status = 0x80000000U | code;
  for (;;)
    __asm volatile("nop");
}
int main() {
  if (!profile::startup_ok())
    fail(1);
  mcubsp::io::detail::cortex_m_cycle_hooks<64'000'000>::begin();
  if (!clocks.initialize() ||
      !target.initialize({{0x42}, mcubsp::driver::i2c_address{0x43}}))
    fail(2);
  static std::array<std::uint8_t, 8> tx{0,   0x55, 0xAA, 0xFF,
                                        'T', 'W',  'I',  'S'};
  static std::array<std::uint8_t, 8> rx{};
  auto op = target.listen(tx, rx);
  if (!op)
    fail(3);
  mcubsp_twis_status = 2;
  const auto begin = execution.now();
  while (!op->ready()) {
    (void)target.poll();
    if (auto request = target.pending_request()) {
      mcubsp_twis_slot = request->address_slot;
      if (!target.resume()) {
        (void)op->cancel();
        (void)op->take();
        fail(4);
      }
    }
    if (std::uint32_t(execution.now() - begin) > 640'000'000U) {
      (void)op->cancel();
      (void)op->take();
      fail(5);
    }
  }
  auto result = op->take();
  if (!result)
    fail(6);
  mcubsp_twis_tx = result->transmitted;
  mcubsp_twis_rx = result->received;
  mcubsp_twis_errors = result->error_source;
  if (!target.shutdown() || !clocks.shutdown())
    fail(7);
  mcubsp_twis_status = 1;
  for (;;)
    __asm volatile("nop");
}
