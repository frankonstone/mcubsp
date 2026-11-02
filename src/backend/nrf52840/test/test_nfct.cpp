#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/nfct.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_nfct : public nrf52840_test::fake_analog {
public:
  bool responds{true};
  unsigned activations{};
  std::optional<std::uintptr_t> failed_write;
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto before = peek(a);
    auto r = fake_analog::write<T>(a, v);
    if (!r)
      return r;
    if (a == 0x40005000 && v) {
      ++activations;
      poke(0x40005410, 3);
      poke(0x40005100, 1);
    }
    if ((a == 0x40005004 || a == 0x40005008) && v && responds)
      poke(0x40005410, 0);
    if (a == 0x4000501C && v)
      poke(0x40005410, 4);
    if (a == 0x4000500C && v)
      poke(0x40005410, 6);
    if (a == 0x40005404 || a == 0x4000540C)
      poke(a, before & ~v);
    if (failed_write == a) {
      failed_write.reset();
      return std::unexpected(core::mmio_error::invalid_value);
    }
    return {};
  }
};
struct fixture {
  fake_nfct mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_nfct> gpio{mmio, resources};
  system_context<fake_nfct, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy> buffers{resources, execution};
  nfct_controller<decltype(context), decltype(buffers),
                  nrf52840_test::analog_address, 64>
      nfct{context, clocks, buffers, 100};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 5);
    mmio.poke(0x1000120C, 1);
  }
  ~fixture() {
    mmio.responds = true;
    (void)nfct.shutdown();
    (void)clocks.shutdown();
  }
  bool init() {
    return clocks.initialize() &&
           nfct.initialize({.dedicated_pads_and_antenna = true});
  }
  bool activate() {
    mmio.poke(0x40005104, 1);
    if (!nfct.poll_events())
      return false;
    execution.advance(64000);
    if (!nfct.activate())
      return false;
    mmio.poke(0x4000514C, 1);
    return nfct.poll_events().has_value();
  }
  std::span<std::uint8_t> buffer() {
    return {reinterpret_cast<std::uint8_t *>(0x20001000), 32};
  }
};
TEST(nrf_nfct, explicit_antenna_uicr_and_reserved_pad_contract, "REQ-0070",
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  EXPECT_FALSE(f.nfct.initialize({}).has_value());
  f.mmio.poke(0x1000120C, 0);
  EXPECT_FALSE(
      f.nfct.initialize({.dedicated_pads_and_antenna = true}).has_value());
  EXPECT_FALSE(f.gpio.claim({0, 9}, driver::gpio_direction::input).has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::nfct));
}
TEST(nrf_nfct, field_activation_waits_full_millisecond_and_sense_releases_hfxo,
     "REQ-0070", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
  f.mmio.poke(0x40005104, 1);
  ASSERT_TRUE(f.nfct.poll_events().has_value());
  EXPECT_FALSE(f.nfct.activate().has_value());
  EXPECT_EQ(f.mmio.activations, 0U);
  f.execution.advance(64000);
  ASSERT_TRUE(f.nfct.activate().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  ASSERT_TRUE(f.nfct.poll_events().has_value());
  ASSERT_TRUE(f.nfct.sense().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
  ASSERT_TRUE(f.nfct.prepare_system_off().has_value());
  EXPECT_TRUE(f.nfct.wake_ready());
}
TEST(nrf_nfct, receive_count_crc_parity_and_terminal_event_latching, "REQ-0070",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  ASSERT_TRUE(f.activate());
  ASSERT_TRUE(f.nfct.receive(f.buffer()).has_value());
  f.mmio.poke(0x4000512C, 1);
  auto pending = f.nfct.poll_frame();
  ASSERT_TRUE(pending.has_value());
  EXPECT_FALSE(pending->has_value());
  f.mmio.poke(0x40005524, 4U << 3);
  f.mmio.poke(0x40005118, 1);
  f.mmio.failed_write = 0x40005118;
  EXPECT_FALSE(f.nfct.poll_frame().has_value());
  auto done = f.nfct.poll_frame();
  ASSERT_TRUE(done && done->has_value());
  EXPECT_EQ((**done).bytes, 4U);
  EXPECT_EQ((**done).receive_errors, 0U);
}
TEST(nrf_nfct, tx_waits_for_last_air_symbol_and_restores_idle_delay,
     "REQ-0070") {
  fixture f;
  ASSERT_TRUE(f.init());
  ASSERT_TRUE(f.activate());
  ASSERT_TRUE(
      f.nfct.transmit(f.buffer(), 0, {}, {.minimum = 0x480, .maximum = 0x5000})
          .has_value());
  f.mmio.poke(0x40005130, 1);
  auto pending = f.nfct.poll_frame();
  ASSERT_TRUE(pending.has_value());
  EXPECT_FALSE(pending->has_value());
  f.mmio.poke(0x40005110, 1);
  auto result = f.nfct.poll_frame();
  ASSERT_TRUE(result && result->has_value());
  EXPECT_EQ(f.mmio.peek(0x40005508), 0x1000U);
  ASSERT_TRUE(f.nfct.go_idle(true).has_value());
}
TEST(nrf_nfct, field_loss_and_failed_disable_keep_memory_until_disabled_state,
     "REQ-0070", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  ASSERT_TRUE(f.activate());
  ASSERT_TRUE(f.nfct.receive(f.buffer()).has_value());
  f.mmio.responds = false;
  f.mmio.poke(0x40005108, 1);
  EXPECT_FALSE(f.nfct.poll_frame().has_value());
  EXPECT_FALSE(f.nfct.cancel().has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::nfct));
  f.mmio.responds = true;
  auto result = f.nfct.poll_frame();
  ASSERT_TRUE(result && result->has_value());
  EXPECT_TRUE((**result).field_lost);
  EXPECT_EQ((**result).bytes, 0U);
}
TEST(nrf_nfct, readonly_type2_read_wrap_ndef_and_write_nak, "REQ-0070") {
  readonly_type2_tag<16> tag;
  std::array<std::uint8_t, 7> uid{4, 1, 2, 3, 4, 5, 6};
  std::array<std::uint8_t, 5> ndef{0xD1, 1, 1, 'T', 0};
  ASSERT_TRUE(tag.initialize(uid, ndef).has_value());
  auto read = tag.respond(std::array<std::uint8_t, 2>{0x30, 3});
  EXPECT_EQ(read.bytes, 16U);
  EXPECT_EQ(read.data[0], 0xE1U);
  EXPECT_EQ(read.data[3], 0x0FU);
  EXPECT_EQ(read.data[4], 3U);
  EXPECT_EQ(read.data[5], 5U);
  auto wrap = tag.respond(std::array<std::uint8_t, 2>{0x30, 15});
  EXPECT_EQ(wrap.data[4], 4U);
  EXPECT_EQ(wrap.data[7], 0x88 ^ 4U ^ 1U ^ 2U);
  auto write = tag.respond(std::array<std::uint8_t, 6>{0xA2, 4, 1, 2, 3, 4});
  EXPECT_EQ(write.extra_bits, 4U);
  EXPECT_EQ(write.data[0], 0U);
  auto sleep = tag.respond(std::array<std::uint8_t, 2>{0x50, 0});
  EXPECT_TRUE(sleep.sleep && sleep.silent);
}
} // namespace
