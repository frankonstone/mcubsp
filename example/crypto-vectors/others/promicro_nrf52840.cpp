// @req-file REQ-0071 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/cryptocell.hpp>
#include <mcubsp/backend/nrf52840/ecb.hpp>
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

static n::cryptocell_controller<decltype(context), decltype(buffers)> cc{
    context, clocks, buffers};
static n::ecb_controller<decltype(context), decltype(buffers)> ecb{
    context, clocks, buffers};
static n::ecb_data block{};
alignas(4) static std::array<std::uint8_t, 16> result{};
int main() {
  if (!begin() || !cc.initialize() || !ecb.initialize())
    finish(1);
  n::cc_aes_configuration aes{};
  for (unsigned i = 0; i < 16; ++i) {
    block.key[i] = i;
    block.cleartext[i] = i * 17;
    aes.key[i / 4] |= unsigned(i) << (8 * (i % 4));
  }
  constexpr std::array<std::uint8_t, 16> expected{
      0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 4,    0x30,
      0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};
  if (!cc.aes(aes, block.cleartext, result) || result != expected ||
      !ecb.start(block))
    finish(2);
  for (;;) {
    auto complete = ecb.poll();
    if (!complete)
      finish(3);
    if (*complete)
      break;
  }
  if (block.ciphertext != expected)
    finish(4);
  mcubsp_demo_count = 1;
  alignas(4) static std::array<std::uint8_t, 3> abc{'a', 'b', 'c'};
  auto hash = cc.hash(n::cc_hash_mode::sha256, abc);
  constexpr std::array<std::uint8_t, 32> digest{
      0xba, 0x78, 0x16, 0xbf, 0x8f, 1,    0xcf, 0xea, 0x41, 0x41, 0x40,
      0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 3,    0x61, 0xa3, 0x96, 0x17,
      0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0,    0x15, 0xad};
  if (!hash || hash->bytes != digest)
    finish(5);
  mcubsp_demo_count = 2;
  if (!cc.pka_configure(128))
    finish(6);
  std::array<std::uint32_t, 4> a{100}, b{7}, difference{};
  if (!cc.pka_load(2, a) || !cc.pka_load(3, b) ||
      !cc.pka_execute({mcubsp::mcu::nrf52840::product::cc_pka::opcode_fields::
                           opcode_encoding::value_subdecneg}) ||
      !cc.pka_read(4, difference) || difference[0] != 93)
    finish(7);
  mcubsp_demo_count = 3;
  auto entropy = cc.entropy();
  if (!entropy)
    finish(8); // Never publish random output or keys.
  if (!ecb.shutdown() || !cc.shutdown() || !clocks.shutdown())
    finish(9);
  finish(0x600D);
}
