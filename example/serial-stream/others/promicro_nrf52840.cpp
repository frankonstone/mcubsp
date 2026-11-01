// @req-file REQ-0045 REQ-0059 REQ-0062
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/io/board.hpp>
namespace io = mcubsp::io;
using profile = mcubsp::board::others::promicro_nrf52840;
using stream_profile=io::select<profile,io::serial_selection<mcubsp::backend::nrf52840::uart_binding<mcubsp::backend::nrf52840::peripheral::uarte0,{0,6},{0,8}>>>;
static io::board<stream_profile> hw;
extern "C" {
volatile std::uint32_t mcubsp_stream_status{}, mcubsp_stream_bytes{},
    mcubsp_stream_blocks{};
}
[[noreturn]] static void fail(unsigned code) {
  mcubsp_stream_status = 0x80000000U | code;
  for (;;)
    __asm volatile("nop");
}
int main() {
  if (!profile::startup_ok() || !hw.begin() || !hw.serial.begin())
    fail(1);
  // Join native P0.06 TX to P0.08 RX. No CTS/RTS jumper is implicit.
  static std::array<std::uint8_t, 48> output{};
  for (unsigned i = 0; i < output.size(); ++i)
    output[i] = static_cast<std::uint8_t>(i ^ 0xA5);
  std::array<std::uint8_t, 16> a{}, b{};
  const std::array blocks{std::span<std::uint8_t>{a},
                          std::span<std::uint8_t>{b}};
  auto rx = hw.serial.read_stream_async(blocks, output.size());
  if (!rx)
    fail(2);
  auto tx = hw.serial.write_async(output);
  if (!tx) {
    (void)rx->cancel();
    (void)rx->take();
    fail(3);
  }
  const auto until = hw.deadline_after(std::chrono::seconds{2});
  if (!until)
    fail(4);
  std::size_t offset{};
  for (;;) {
    hw.poll();
    while (auto block = hw.serial.take_read_block()) {
      if (block->count > output.size() - offset)
        fail(5);
      for (unsigned i = 0; i < block->count; ++i)
        if (block->storage[i] != output[offset + i])
          fail(6);
      offset += block->count;
      mcubsp_stream_bytes = offset;
      mcubsp_stream_blocks = mcubsp_stream_blocks + 1;
      if (offset < output.size() &&
          !hw.serial.queue_read_buffer(block->storage))
        fail(7);
    }
    if (rx->ready() && tx->ready())
      break;
    if (mcubsp::execution::reached(hw.now(), *until)) {
      (void)rx->cancel();
      (void)tx->cancel();
      (void)rx->take();
      (void)tx->take();
      fail(8);
    }
  }
  auto received = rx->take();
  auto sent = tx->take();
  if (!received || !sent || *received != output.size() ||
      offset != output.size())
    fail(9);
  if (!hw.shutdown())
    fail(10);
  mcubsp_stream_status = 1;
  for (;;)
    __asm volatile("nop");
}
