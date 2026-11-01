// @req-file REQ-0045 REQ-0059 REQ-0060 REQ-0061 REQ-0062
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/io/board.hpp>
#include <mcubsp/execution/fake_policy.hpp>
using profile = mcubsp::board::others::promicro_nrf52840;
namespace n = mcubsp::backend::nrf52840;
namespace io = mcubsp::io;
using selected = io::select<
    profile,
    io::serial_selection<n::uart_binding<n::peripheral::uarte1, {0, 6}, {0, 8},
                                         {0, 7}, {0, 12}>>,
    io::spi_selection<n::spim_binding<2, {0, 20}, {0, 22}, {0, 24}, {0, 17}>>,
    io::i2c_selection<n::twim_binding<1, {0, 11}, {1, 0}>>>;
void nrf_serial_facade_probe() {
  io::board<selected, {.serial = true, .i2c = true, .spi = true}, mcubsp::execution::fake_policy> hw;
  (void)hw.begin();
  (void)hw.serial.begin();
  (void)hw.spi.begin();
  (void)hw.i2c.begin();
  std::array<std::uint8_t, 8> a{}, b{};
  std::array blocks{std::span<std::uint8_t>{a}, std::span<std::uint8_t>{b}};
  auto stream = hw.serial.read_stream_async(blocks, 32);
  if (stream) {
    hw.poll();
    auto block = hw.serial.take_read_block();
    if (block)
      (void)hw.serial.queue_read_buffer(block->storage);
    (void)hw.serial.stop_read();
    (void)stream->cancel();
    (void)stream->take();
  }
  auto spi = hw.spi.device({.cs = {0, 13}});
  if (spi) {
    std::array segments{mcubsp::driver::byte_segment(a, b)};
    auto op = spi->transaction_async(segments);
    if (op) {
      hw.poll();
      (void)op->cancel();
      (void)op->take();
    }
    (void)spi->release();
  }
  auto twi = hw.i2c.device(0x42);
  if (twi) {
    auto op = twi->write_read_async(a, b);
    if (op) {
      hw.poll();
      (void)op->cancel();
      (void)op->take();
    }
    (void)twi->release();
  }
  (void)hw.shutdown();
}
