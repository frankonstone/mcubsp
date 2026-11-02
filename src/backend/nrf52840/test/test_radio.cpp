#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/events.hpp>
#include <mcubsp/backend/nrf52840/radio.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_radio : public nrf52840_test::fake_analog {
public:
  bool responds{true};
  unsigned starts{}, disables{};
  std::optional<std::uintptr_t> failed_write;
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto r = fake_analog::write<T>(a, v);
    if (!r)
      return r;
    if ((a == 0x40001000 || a == 0x40001004) && v && responds) {
      poke(0x40001550, a == 0x40001000 ? 10 : 2);
      poke(0x40001100, 1);
      if (peek(0x40001200) & 1) {
        ++starts;
        poke(0x40001550, a == 0x40001000 ? 11 : 3);
      }
    }
    if (a == 0x40001008 && v) {
      ++starts;
      poke(0x40001550, 3);
    }
    if (a == 0x40001010 && v) {
      ++disables;
      if (responds) {
        poke(0x40001550, 0);
        poke(0x40001110, 1);
      }
    }
    if (a == 0x40001014 && v && responds)
      poke(0x4000111C, 1);
    if (a == 0x40001024 && v && responds)
      poke(0x4000113C, 1);
    if (a == 0x4000102C && v && responds)
      poke(0x40001144, 1);
    if (failed_write == a) {
      failed_write.reset();
      return std::unexpected(core::mmio_error::invalid_value);
    }
    return {};
  }
  void complete(bool physical = true) {
    poke(0x4000110C, 1);
    if (physical)
      poke(0x4000116C, 1);
    auto shorts = peek(0x40001200);
    if ((shorts & 2) || ((shorts & (1U << 20)) && physical)) {
      poke(0x40001550, 0);
      poke(0x40001110, 1);
    }
  }
};
struct fixture {
  fake_radio mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_radio> gpio{mmio, resources};
  system_context<fake_radio, execution::fake_policy> context{mmio, execution,
                                                             gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy> buffers{resources, execution};
  radio_controller<decltype(context), decltype(buffers),
                   nrf52840_test::analog_address, 64>
      radio{context, clocks, buffers, 100};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 5);
    mmio.poke(0x40001788, 1);
    mmio.poke(0x40001548, 100);
  }
  ~fixture() {
    mmio.responds = true;
    (void)radio.shutdown();
    (void)clocks.shutdown();
  }
  bool init(radio_phy phy = radio_phy::ble1) {
    return clocks.initialize() && radio.initialize({.phy = phy});
  }
};
TEST(nrf_radio, every_phy_has_correct_preamble_crc_and_ram_layout, "REQ-0069",
     "REQ-0062") {
  for (auto phy :
       std::array{radio_phy::nordic1, radio_phy::nordic2, radio_phy::ble1,
                  radio_phy::ble2, radio_phy::coded125, radio_phy::coded500,
                  radio_phy::ieee802154}) {
    fixture f;
    ASSERT_TRUE(f.init(phy));
    EXPECT_EQ(f.mmio.peek(0x40001510), unsigned(phy));
    unsigned plen = phy == radio_phy::ble2         ? 1
                    : phy == radio_phy::ieee802154 ? 2
                    : (phy == radio_phy::coded125 || phy == radio_phy::coded500)
                        ? 3
                        : 0;
    EXPECT_EQ((f.mmio.peek(0x40001514) >> 24) & 3, plen);
    alignas(4) std::array<std::uint8_t, 258> packet{};
    if (phy == radio_phy::ieee802154)
      packet[0] = 5;
    else if (phy == radio_phy::nordic1 || phy == radio_phy::nordic2)
      packet[0] = 3;
    else
      packet[1] = 3;
    ASSERT_TRUE(f.radio.transmit(packet).has_value());
    f.mmio.complete();
    auto result = f.radio.poll();
    ASSERT_TRUE(result && result->has_value());
    EXPECT_FALSE((**result).receive);
  }
}
TEST(nrf_radio, invalid_configuration_and_short_rx_buffer_are_rejected,
     "REQ-0069") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  EXPECT_FALSE(f.radio.initialize({.power_dbm = 1}).has_value());
  EXPECT_FALSE(f.radio.initialize({.frequency_mhz = 2359}).has_value());
  ASSERT_TRUE(f.radio.initialize().has_value());
  std::array<std::uint8_t, 4> short_buffer{};
  EXPECT_FALSE(f.radio.receive(short_buffer).has_value());
  auto config = radio_configuration{};
  config.automatic_packet_format = false;
  config.crc.skip_address = 0;
  EXPECT_FALSE(f.radio.configure(config).has_value());
}
TEST(nrf_radio, receive_crc_rssi_and_actual_payload_count_follow_disabled,
     "REQ-0069", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::uint8_t, 258> packet{};
  packet[1] = 7;
  ASSERT_TRUE(f.radio.receive(packet).has_value());
  f.mmio.poke(0x40001104, 1);
  f.mmio.poke(0x4000111C, 1);
  f.mmio.poke(0x40001400, 1);
  f.mmio.poke(0x40001408, 3);
  f.mmio.poke(0x4000140C, 0x123456);
  f.mmio.complete();
  auto done = f.radio.poll();
  ASSERT_TRUE(done && done->has_value());
  EXPECT_EQ((**done).bytes, 9U);
  EXPECT_TRUE((**done).crc_valid);
  EXPECT_EQ((**done).address_index, 3U);
  EXPECT_EQ(*(**done).rssi_sample, 100U);
  EXPECT_EQ(compensated_rssi_dbm(100, -120), -103);
  EXPECT_EQ(compensated_rssi_dbm(100, 200), -99);
}
TEST(nrf_radio, coded_and_802154_end_does_not_mean_final_phy_completion,
     "REQ-0069", "REQ-0062") {
  for (auto phy : std::array{radio_phy::coded125, radio_phy::ieee802154}) {
    fixture f;
    ASSERT_TRUE(f.init(phy));
    alignas(4) std::array<std::uint8_t, 258> packet{};
    packet[0] = 5;
    packet[1] = 3;
    ASSERT_TRUE(f.radio.transmit(packet).has_value());
    f.mmio.complete(false);
    auto early = f.radio.poll();
    ASSERT_TRUE(early.has_value());
    EXPECT_FALSE(early->has_value());
    f.mmio.complete(true);
    auto done = f.radio.poll();
    ASSERT_TRUE(done && done->has_value());
  }
}
TEST(nrf_radio, coded_ci_rfu_forces_disable_and_reports_invalid_packet,
     "REQ-0069") {
  fixture f;
  ASSERT_TRUE(f.init(radio_phy::coded125));
  alignas(4) std::array<std::uint8_t, 258> packet{};
  ASSERT_TRUE(f.radio.receive(packet).has_value());
  f.mmio.poke(0x40001138, 1);
  f.mmio.poke(0x40001414, 6);
  ASSERT_TRUE(f.radio.poll().has_value());
  auto result = f.radio.poll();
  ASSERT_TRUE(result && result->has_value());
  EXPECT_TRUE((**result).invalid_coding);
  EXPECT_EQ((**result).bytes, 0U);
}
TEST(nrf_radio, anomaly172_strict_filter_rssi_counter_and_periodic_service,
     "REQ-0069") {
  fixture f;
  f.mmio.poke(0x40001788, 0);
  ASSERT_TRUE(f.init(radio_phy::coded125));
  alignas(4) std::array<std::uint8_t, 258> packet{};
  ASSERT_TRUE(f.radio.receive(packet).has_value());
  EXPECT_TRUE(f.mmio.peek(0x4000173C) & 0x80000000);
  ASSERT_TRUE(f.radio.poll().has_value());
  ASSERT_TRUE(f.radio.poll().has_value());
  EXPECT_EQ(f.mmio.peek(0x4000173C), 0x40003034U);
  f.mmio.poke(0x40001574, 16);
  for (unsigned i = 0; i < 34; ++i) {
    f.execution.advance(20000);
    ASSERT_TRUE(f.radio.poll().has_value());
  }
  ASSERT_TRUE(f.radio.poll().has_value());
  EXPECT_TRUE(f.mmio.peek(0x4000173C) & 0x80000000);
  EXPECT_TRUE(f.mmio.wrote(0x40001040, 1));
  EXPECT_TRUE(f.mmio.wrote(0x40001038, 1));
  ASSERT_TRUE(f.radio.cancel().has_value());
  EXPECT_EQ(f.mmio.peek(0x4000173C), 0x40003034U);
}
TEST(nrf_radio, coded_late_service_is_explicit_overrun_with_retained_buffer,
     "REQ-0069", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init(radio_phy::coded500));
  alignas(4) std::array<std::uint8_t, 258> packet{};
  ASSERT_TRUE(f.radio.receive(packet).has_value());
  f.execution.advance(64000);
  auto result = f.radio.poll();
  ASSERT_TRUE(result.has_value());
  if (!result->has_value())
    result = f.radio.poll();
  ASSERT_TRUE(result && result->has_value());
  EXPECT_TRUE((**result).service_overrun);
  EXPECT_EQ((**result).bytes, 0U);
}
TEST(nrf_radio, coded_turnaround_applies_all_four_errata_tifs_values,
     "REQ-0069") {
  fixture f;
  ASSERT_TRUE(f.init(radio_phy::coded125));
  alignas(4) std::array<std::uint8_t, 258> packet{};
  ASSERT_TRUE(f.radio.receive(packet).has_value());
  for (unsigned ci = 0; ci < 2; ++ci)
    for (auto next : std::array{radio_phy::coded125, radio_phy::coded500}) {
      f.mmio.poke(0x40001414, ci << 1);
      ASSERT_TRUE(f.radio.prepare_coded_turnaround(next).has_value());
      auto expected = ci ? (next == radio_phy::coded125 ? 154U : 144U)
                         : (next == radio_phy::coded125 ? 149U : 139U);
      EXPECT_EQ(f.mmio.peek(0x40001544), expected);
    }
}
TEST(nrf_radio, energy_cca_device_and_mac_matching_are_real_operations,
     "REQ-0069") {
  fixture f;
  ASSERT_TRUE(f.init(radio_phy::ieee802154));
  f.mmio.poke(0x40001668, 40);
  auto energy = f.radio.energy_detect();
  ASSERT_TRUE(energy.has_value());
  EXPECT_EQ(energy->dbm, -52);
  EXPECT_EQ(energy->ieee802154, 160U);
  auto cca = f.radio.clear_channel(2, 30, 45, 5);
  ASSERT_TRUE(cca.has_value());
  EXPECT_TRUE(*cca);
  EXPECT_EQ(f.mmio.peek(0x4000166C), 0x052D1E02U);
  std::array addresses{radio_device_address{0x12345678, 0xABCD, true}};
  ASSERT_TRUE(f.radio.device_addresses(addresses).has_value());
  EXPECT_EQ(f.mmio.peek(0x40001640), 0x101U);
  ASSERT_TRUE(f.radio.match_mac_header(0x1234, 0xFFFF).has_value());
  EXPECT_EQ(f.mmio.peek(0x40001644), 0x1234U);
}
TEST(nrf_radio, failed_disable_retains_dma_hfxo_until_explicit_recovery,
     "REQ-0069", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::uint8_t, 8> packet{};
  packet[1] = 3;
  f.mmio.failed_write = 0x40001000;
  EXPECT_FALSE(f.radio.transmit(packet).has_value());
  f.mmio.responds = false;
  EXPECT_FALSE(f.radio.cancel().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  f.mmio.responds = true;
  ASSERT_TRUE(f.radio.cancel().has_value());
  ASSERT_TRUE(f.radio.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_radio, ppi_routes_must_detach_before_memory_or_hardware_teardown,
     "REQ-0069", "REQ-0062", "REQ-0057") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::uint8_t, 8> packet{};
  packet[1] = 3;
  ASSERT_TRUE(f.radio.transmit(packet).has_value());
  ppi_channel<0, decltype(f.context)> route{f.context};
  ASSERT_TRUE(route
                  .initialize(f.radio.event_endpoint(0x10C),
                              f.radio.task_endpoint(0x10))
                  .has_value());
  f.mmio.complete();
  EXPECT_FALSE(f.radio.poll().has_value());
  auto before = f.mmio.disables;
  EXPECT_FALSE(f.radio.shutdown().has_value());
  EXPECT_EQ(f.mmio.disables, before);
  ASSERT_TRUE(route.shutdown().has_value());
  auto done = f.radio.poll();
  ASSERT_TRUE(done && done->has_value());
}
TEST(nrf_radio,
     optional_stack_reservation_excludes_direct_radio_and_shared_crypto,
     "REQ-0069", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  radio_stack_reservation<decltype(f.context)> stack{f.context};
  ASSERT_TRUE(stack.acquire(true).has_value());
  EXPECT_FALSE(f.radio.initialize().has_value());
  EXPECT_FALSE(stack.release(false).has_value());
  ASSERT_TRUE(stack.release(true).has_value());
  ASSERT_TRUE(f.radio.initialize().has_value());
}

TEST(nrf_radio, rejected_second_receive_preserves_active_length_storage,
     "REQ-0069", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::uint8_t, 258> first{}, second{};
  first[1] = 7;
  second[1] = 12;
  ASSERT_TRUE(f.radio.receive(first).has_value());
  EXPECT_FALSE(f.radio.receive(second).has_value());
  f.mmio.complete();
  auto result = f.radio.poll();
  ASSERT_TRUE(result && *result);
  EXPECT_EQ((**result).bytes, 9U);
}

TEST(nrf_radio, cached_task_endpoint_cannot_rearm_returned_storage, "REQ-0062",
     "REQ-0069") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::uint8_t, 258> packet{};
  packet[1] = 3;
  ASSERT_TRUE(f.radio.transmit(packet).has_value());
  auto endpoint = f.radio.task_endpoint(0);
  EXPECT_TRUE(f.context.valid(endpoint, endpoint_kind::task));
  f.mmio.complete();
  auto done = f.radio.poll();
  ASSERT_TRUE(done && *done);
  EXPECT_FALSE(f.context.valid(endpoint, endpoint_kind::task));
}
} // namespace
