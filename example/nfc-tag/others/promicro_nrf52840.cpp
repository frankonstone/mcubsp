// @req-file REQ-0070 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/nfct.hpp>
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

static n::nfct_controller<decltype(context), decltype(buffers)> nfc{
    context, clocks, buffers};
static n::readonly_type2_tag<> tag;
alignas(4) static std::array<std::uint8_t, 64> rx{}, tx{};
int main() {
  if (!begin())
    finish(1);
  // Bench UID only. Dedicated NFC pads and a tuned antenna must be attached;
  // initialize checks the existing UICR NFC role and never programs it.
  std::array<std::uint8_t, 7> uid{4, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB};
  constexpr std::array<std::uint8_t, 12> ndef{0xD1, 1,   8,   'T', 2,   'e',
                                              'n',  'h', 'e', 'l', 'l', 'o'};
  if (!tag.initialize(uid, ndef))
    finish(2);
  n::nfct_configuration config{};
  config.dedicated_pads_and_antenna = true;
  std::copy(uid.begin(), uid.end(), config.id.begin());
  if (!nfc.initialize(config) || !nfc.sense())
    finish(3);
  bool selected{}, field_pending{}, armed{};
  for (;;) {
    auto events = nfc.poll_events();
    if (!events)
      finish(4);
    if (events->field_lost) {
      selected = field_pending = false;
      if (!armed && !nfc.sense())
        finish(9);
    }
    if (events->field_detected)
      field_pending = true;
    if (field_pending) {
      auto activation = nfc.activate();
      if (activation)
        field_pending = false;
      else if (activation.error() != n::system_error::busy)
        finish(10);
    }
    if (events->selected)
      selected = true;
    auto frame = nfc.poll_frame();
    if (!frame)
      finish(5);
    if (*frame) {
      armed = false;
      if ((**frame).field_lost || (**frame).cancelled) {
        selected = field_pending = false;
        if (!nfc.sense())
          finish(11);
      } else if ((**frame).receive && !(**frame).cancelled &&
                 !(**frame).receive_errors && !(**frame).timing_errors &&
                 !(**frame).extra_bits && (**frame).bytes >= 2) {
        auto response = tag.respond(std::span{rx}.first((**frame).bytes - 2));
        if (response.sleep) {
          if (!nfc.go_idle(true))
            finish(6);
          selected = false;
        } else if (!response.silent) {
          std::copy(response.data.begin(), response.data.end(), tx.begin());
          if (!nfc.transmit(std::span{tx}.first(response.bytes),
                            response.extra_bits,
                            {.crc = response.extra_bits == 0}))
            finish(7);
          armed = true;
        }
        mcubsp_demo_count = mcubsp_demo_count + 1;
      }
    }
    if (selected && !armed) {
      if (!nfc.receive(rx))
        finish(8);
      armed = true;
    }
  }
}
