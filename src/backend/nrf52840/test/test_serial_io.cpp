#include "fake_uart.hpp"
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/io/board.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_io : public nrf52840_test::fake_uart {
public:
  static constexpr std::uintptr_t spi_base = 0x40023000, twi_base = 0x40004000;
  bool dma_stop_responds{true};
  template <class T>
  std::expected<T, core::mmio_error> read(std::uintptr_t a) noexcept {
    if (clock && !dma_stop_responds && a == spi_base + 0x104)
      clock->advance(1'000'000);
    return nrf52840_test::fake_uart::read<T>(a);
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto r = nrf52840_test::fake_uart::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a == spi_base + 0x51C) {
      poke(spi_base + 0x518, v ^ 0xFFU);
      poke(spi_base + 0x108, 1);
    }
    if ((a == spi_base + 0x14 || a == twi_base + 0x14) && v &&
        dma_stop_responds)
      poke(a + 0xF0, 1);
    return r;
  }
  void spi_complete(unsigned tx, unsigned rx) {
    poke(spi_base + 0x54C, tx);
    poke(spi_base + 0x53C, rx);
    poke(spi_base + 0x118, 1);
  }
};
using profile = board::others::promicro_nrf52840;
using cpu_serial = uart_binding<peripheral::uart0, {0, 6}, {0, 8}>;
using cpu_spi = spi_binding<2, {0, 20}, {0, 22}, {0, 24}, {0, 17}>;
using dma_spi = spim_binding<2, {0, 20}, {0, 22}, {0, 24}, {0, 17}>;
using dma_twi = twim_binding<1, {0, 11}, {1, 0}>;
template <class Spi>
using hardware =
    io::board<profile, {.serial = false}, execution::fake_policy, fake_io, true,
              true, io::serial_selection<cpu_serial>, io::spi_selection<Spi>,
              io::i2c_selection<dma_twi>>;
TEST(nrf_serial_io, selected_buses_share_one_clock_service_and_shutdown_cleanly,
     "REQ-0045", "REQ-0056", "REQ-0059", "REQ-0060", "REQ-0061", "REQ-0062") {
  fake_io mmio;
  execution::fake_policy execution;
  hardware<cpu_spi> hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  auto &serial = hw.peripheral<0>();
  auto &spi = hw.peripheral<1>();
  auto &twi = hw.peripheral<2>();
  ASSERT_TRUE(serial.begin().has_value());
  ASSERT_TRUE(spi.begin().has_value());
  ASSERT_TRUE(twi.begin().has_value());
  EXPECT_EQ(mmio.peek(0x40000408), 1U);
  ASSERT_TRUE(serial.shutdown().has_value());
  EXPECT_EQ(mmio.peek(0x40000408), 1U);
  ASSERT_TRUE(spi.shutdown().has_value());
  EXPECT_EQ(mmio.peek(0x40000408), 1U);
  ASSERT_TRUE(twi.shutdown().has_value());
  EXPECT_EQ(mmio.peek(0x40000408), 0U);
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_serial_io, common_serial_stream_returns_blocks_and_keeps_board_alive,
     "REQ-0045", "REQ-0059") {
  fake_io mmio;
  execution::fake_policy execution;
  hardware<cpu_spi> hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  auto &serial = hw.peripheral<0>();
  ASSERT_TRUE(serial.begin().has_value());
  std::array<std::uint8_t, 2> bytes{};
  std::array blocks{std::span<std::uint8_t>{bytes}};
  auto op = serial.read_stream_async(blocks, 2);
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(hw.shutdown().has_value());
  mmio.inject(std::uint8_t{4});
  mmio.inject(std::uint8_t{5});
  hw.poll();
  hw.poll();
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 2U);
  auto block = serial.take_read_block();
  ASSERT_TRUE(block.has_value());
  EXPECT_EQ(block->count, 2U);
  EXPECT_EQ(bytes[1], 5U);
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_serial_io, cpu_spi_device_lease_selects_and_returns_common_result,
     "REQ-0045", "REQ-0046", "REQ-0060") {
  fake_io mmio;
  execution::fake_policy execution;
  hardware<cpu_spi> hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  auto &spi = hw.peripheral<1>();
  ASSERT_TRUE(spi.begin().has_value());
  auto device = spi.device({.cs = {0, 12}, .frequency = 2000000});
  ASSERT_TRUE(device.has_value());
  std::array<std::uint8_t, 2> tx{0xA5, 0x55}, rx{};
  std::array segments{driver::byte_segment(tx, rx)};
  auto op = device->transaction_async(segments);
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(device->release().has_value());
  hw.poll();
  hw.poll();
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->frames, 2U);
  EXPECT_EQ(rx[0], 0x5AU);
  ASSERT_TRUE(device->release().has_value());
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_serial_io, spim_device_lease_waits_for_stop_before_releasing_cs,
     "REQ-0045", "REQ-0046", "REQ-0060", "REQ-0062") {
  fake_io mmio;
  mmio.poke(0x10000130, 8);
  mmio.poke(0x10000134, 3);
  execution::fake_policy execution;
  hardware<dma_spi> hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  auto &spi = hw.peripheral<1>();
  ASSERT_TRUE(spi.begin().has_value());
  auto device = spi.device({.cs = {0, 12}});
  ASSERT_TRUE(device.has_value());
  std::span<const std::uint8_t> tx{
      reinterpret_cast<const std::uint8_t *>(0x20001000), 8};
  std::span<std::uint8_t> rx{reinterpret_cast<std::uint8_t *>(0x20002000), 8};
  std::array segments{driver::byte_segment(tx, rx)};
  auto op = device->transaction_async(segments);
  ASSERT_TRUE(op.has_value());
  mmio.dma_stop_responds = false;
  mmio.spi_complete(8, 8);
  hw.poll();
  EXPECT_FALSE(op->ready());
  EXPECT_FALSE(device->release().has_value());
  mmio.poke(fake_io::spi_base + 0x104, 1);
  hw.poll();
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->frames, 8U);
  ASSERT_TRUE(device->release().has_value());
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_serial_io, alias_conflicts_are_detected_across_cpu_and_dma_selections,
     "REQ-0045", "REQ-0060", "REQ-0061", "REQ-0062") {
  using Alias = io::board<
      profile, {.serial = false}, execution::fake_policy, fake_io, true, true,
      io::spi_selection<spim_binding<1, {0, 20}, {0, 22}, {0, 24}, {0, 17}>>,
      io::i2c_selection<dma_twi>>;
  fake_io mmio;
  execution::fake_policy execution;
  Alias hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  ASSERT_TRUE(hw.peripheral<0>().begin().has_value());
  auto before = mmio.trace_count;
  EXPECT_FALSE(hw.peripheral<1>().begin().has_value());
  EXPECT_EQ(mmio.trace_count, before);
  ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf_serial_io,
     failed_spim_cancel_can_recover_select_and_release_device_after_late_stop,
     "REQ-0045", "REQ-0046", "REQ-0060", "REQ-0062") {
  fake_io mmio;
  execution::fake_policy execution;
  mmio.clock = &execution;
  hardware<dma_spi> hw{execution, mmio};
  ASSERT_TRUE(hw.begin().has_value());
  auto &spi = hw.peripheral<1>();
  ASSERT_TRUE(spi.begin().has_value());
  auto device = spi.device({.cs = {0, 12}});
  ASSERT_TRUE(device.has_value());
  std::span<const std::uint8_t> tx{
      reinterpret_cast<const std::uint8_t *>(0x20001000), 8};
  std::array segments{driver::byte_segment(tx, {})};
  auto op = device->transaction_async(segments);
  ASSERT_TRUE(op.has_value());
  mmio.dma_stop_responds = false;
  EXPECT_FALSE(op->cancel().has_value());
  EXPECT_FALSE(op->take().has_value());
  EXPECT_FALSE(spi.recover_select().has_value());
  EXPECT_FALSE(device->release().has_value());
  mmio.poke(fake_io::spi_base + 0x104, 1);
  ASSERT_TRUE(spi.recover_select().has_value());
  ASSERT_TRUE(device->release().has_value());
  ASSERT_TRUE(hw.shutdown().has_value());
}
} // namespace
