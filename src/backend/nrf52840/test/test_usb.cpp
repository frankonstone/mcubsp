#include "fake_analog.hpp"
#include <cstring>
#include <mcubsp/backend/nrf52840/radio.hpp>
#include <mcubsp/backend/nrf52840/usb_device.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_usb;
struct usb_address {
  fake_usb *model;
  std::uintptr_t operator()(const void *, std::size_t) const;
};
class fake_usb : public nrf52840_test::fake_analog {
public:
  static constexpr std::uintptr_t base = 0x40027000;
  bool responds{true}, dma_responds{true}, disable_responds{true};
  unsigned dma_starts{}, statuses{}, stalls{}, resets{};
  std::optional<unsigned> active;
  std::array<const void *, 64> mappings{};
  unsigned mapped{};
  std::array<std::array<std::uint8_t, 1024>, 9> incoming{}, sent{};
  std::array<unsigned, 9> sent_size{};
  std::uintptr_t address(const void *p) {
    for (unsigned i = 0; i < mapped; ++i)
      if (mappings[i] == p)
        return 0x20000000 + 4096 * i;
    if (mapped == mappings.size())
      return 0;
    mappings[mapped] = p;
    return 0x20000000 + 4096 * mapped++;
  }
  void *pointer(std::uintptr_t a) {
    auto i = (a - 0x20000000) / 4096;
    return i < mapped ? const_cast<void *>(mappings[i]) : nullptr;
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto old = peek(a);
    auto r = fake_analog::write<T>(a, v);
    if (!r)
      return r;
    if (a == base + 0x400 || a == base + 0x46C)
      poke(a, old & ~v);
    if (a == base + 0x500 && v && responds)
      poke(base + 0x400, peek(base + 0x400) | 2048);
    if (a == base + 0x500 && !v && !disable_responds)
      poke(a, 1);
    if (a == base + 0x52C && !v && responds)
      poke(base + 0x400, peek(base + 0x400) | 1024);
    if (a == base + 0x50 && v)
      ++statuses;
    if (a == base + 0x54 && v)
      ++stalls;
    if (a == 0xE000ED0C && (v & 4))
      ++resets;
    if (a >= base + 4 && a <= base + 0x48 && v) {
      unsigned i =
          a < base + 0x28 ? (a - base - 4) / 4 : 9 + (a - base - 0x28) / 4;
      // A second START before END violates the shared DMA arbiter.
      if (active)
        return std::unexpected(core::mmio_error::invalid_value);
      active = i;
      ++dma_starts;
      if (dma_responds)
        complete();
    }
    return {};
  }
  void complete() {
    if (!active)
      return;
    auto i = *active, ep = i % 9;
    bool in = i < 9;
    auto reg = base + (in ? 0x600 : 0x700) + 20 * ep;
    unsigned n = peek(reg + 4);
    if (auto p = pointer(peek(reg))) {
      if (in) {
        std::memcpy(sent[ep].data(), p, n);
        sent_size[ep] = n;
      } else
        std::memcpy(p, incoming[ep].data(), n);
    }
    poke(reg + 8, n);
    poke(base + (in ? (ep == 8 ? 0x12C : 0x108 + 4 * ep)
                    : (ep == 8 ? 0x150 : 0x130 + 4 * ep)),
         1);
    active.reset();
  }
  void ack(unsigned ep) {
    if (!ep)
      poke(base + 0x128, 1);
    else {
      poke(base + 0x46C, peek(base + 0x46C) | (1U << ep));
      poke(base + 0x160, 1);
    }
  }
  void out(unsigned ep, std::span<const std::uint8_t> data) {
    std::copy(data.begin(), data.end(), incoming[ep].begin());
    poke(base + (ep == 8 ? 0x4C0 : 0x4A0 + 4 * ep),
         data.empty() && ep == 8 ? 65536 : data.size());
    if (!ep)
      poke(base + 0x128, 1);
    else if (ep < 8) {
      poke(base + 0x46C, peek(base + 0x46C) | (1U << (16 + ep)));
      poke(base + 0x160, 1);
    }
  }
  void setup(usb_setup q) {
    std::array<unsigned, 8> b{
        q.request_type,           q.request,
        unsigned(q.value & 255),  unsigned(q.value >> 8),
        unsigned(q.index & 255),  unsigned(q.index >> 8),
        unsigned(q.length & 255), unsigned(q.length >> 8)};
    for (unsigned i = 0; i < 8; ++i)
      poke(base + 0x480 + 4 * i, b[i]);
    poke(base + 0x15C, 1);
  }
  void cause(unsigned value) {
    poke(base + 0x400, peek(base + 0x400) | value);
    poke(base + 0x158, 1);
  }
};
std::uintptr_t usb_address::operator()(const void *p, std::size_t) const {
  return model->address(p);
}
struct fixture {
  fake_usb mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_usb> gpio{mmio, resources};
  system_context<fake_usb, execution::fake_policy> context{mmio, execution,
                                                           gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 24> buffers{resources, execution};
  using controller =
      usbd_controller<decltype(context), decltype(buffers), usb_address, 64>;
  controller usb{context, clocks, buffers, 100, {&mmio}};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 5);
    mmio.poke(0x40000438, 3);
  }
  ~fixture() {
    mmio.disable_responds = true;
    (void)usb.shutdown();
    (void)clocks.shutdown();
  }
  bool init(usb_configuration c = {}) {
    return clocks.initialize() && usb.initialize(c);
  }
  template <class D> bool request(D &device, usb_setup q) {
    mmio.setup(q);
    return device.poll() && device.poll();
  }
};
TEST(nrf_usb, enable_errata_and_vbus_contract, "REQ-0068") {
  fixture f;
  f.mmio.poke(0x40000438, 0);
  ASSERT_TRUE(f.clocks.initialize().has_value());
  EXPECT_FALSE(f.usb.initialize().has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::usbd));
  f.mmio.poke(0x40000438, 3);
  ASSERT_TRUE(f.usb.initialize().has_value());
  EXPECT_EQ(f.mmio.peek(fake_usb::base + 0x800), 0x7E3U);
  EXPECT_EQ(f.mmio.peek(0x4006EC14), 0U);
  EXPECT_EQ(f.mmio.peek(0x4006ED14), 0U);
}
TEST(nrf_usb, all_non_iso_endpoints_and_single_dma_arbiter, "REQ-0068",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::array<std::uint8_t, 4>, 8> in{}, out{};
  for (unsigned ep = 1; ep < 8; ++ep) {
    ASSERT_TRUE(f.usb.configure_endpoint(ep, true, usb_endpoint_type::bulk, 64)
                    .has_value());
    ASSERT_TRUE(
        f.usb.configure_endpoint(ep, false, usb_endpoint_type::interrupt, 4)
            .has_value());
    ASSERT_TRUE(f.usb.transmit(ep, in[ep], 3).has_value());
    ASSERT_TRUE(f.usb.receive(ep, out[ep], 3).has_value());
    f.mmio.out(ep, std::array<std::uint8_t, 3>{1, 2, 3});
    f.mmio.ack(ep);
  }
  for (unsigned i = 0; i < 30; ++i)
    ASSERT_TRUE(f.usb.poll().has_value());
  for (unsigned ep = 1; ep < 8; ++ep) {
    auto a = f.usb.take_completed(ep, true),
         b = f.usb.take_completed(ep, false);
    ASSERT_TRUE(a && b);
    EXPECT_TRUE(a->acknowledged);
    EXPECT_EQ(b->bytes, 3U);
    EXPECT_EQ(out[ep][2], 3U);
  }
  EXPECT_EQ(f.mmio.dma_starts, 14U);
}
TEST(nrf_usb, in_end_is_not_ack_and_word_padding_is_required, "REQ-0068",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  ASSERT_TRUE(f.usb.configure_endpoint(1, true, usb_endpoint_type::bulk, 64)
                  .has_value());
  alignas(4) std::array<std::uint8_t, 8> bytes{1, 2, 3, 4, 5};
  EXPECT_FALSE(f.usb.transmit(1, std::span{bytes}.first(5), 5).has_value());
  ASSERT_TRUE(f.usb.transmit(1, bytes, 5).has_value());
  ASSERT_TRUE(f.usb.poll().has_value());
  ASSERT_TRUE(f.usb.poll().has_value());
  EXPECT_FALSE(f.usb.take_completed(1, true).has_value());
  f.mmio.ack(1);
  ASSERT_TRUE(f.usb.poll().has_value());
  auto done = f.usb.take_completed(1, true);
  ASSERT_TRUE(done);
  EXPECT_EQ(done->bytes, 5U);
  ASSERT_TRUE(f.usb.transmit(1, {}, 0).has_value());
  ASSERT_TRUE(f.usb.poll().has_value());
  f.mmio.ack(1);
  ASSERT_TRUE(f.usb.poll().has_value());
  EXPECT_EQ(f.usb.take_completed(1, true)->bytes, 0U);
}
TEST(nrf_usb, iso_split_sof_and_missed_frame_metadata, "REQ-0068") {
  fixture f;
  ASSERT_TRUE(f.init({.split_iso_buffer = true}));
  ASSERT_TRUE(
      f.usb.configure_endpoint(8, true, usb_endpoint_type::isochronous, 512)
          .has_value());
  ASSERT_TRUE(
      f.usb.configure_endpoint(8, false, usb_endpoint_type::isochronous, 512)
          .has_value());
  alignas(4) std::array<std::uint8_t, 8> in{1, 2, 3}, out{};
  ASSERT_TRUE(f.usb.transmit(8, in, 3).has_value());
  ASSERT_TRUE(f.usb.receive(8, out, 3).has_value());
  f.mmio.out(8, std::array<std::uint8_t, 3>{4, 5, 6});
  f.mmio.poke(fake_usb::base + 0x520, 1);
  f.mmio.poke(fake_usb::base + 0x154, 1);
  ASSERT_TRUE(f.usb.poll().has_value());
  f.mmio.poke(fake_usb::base + 0x520, 2);
  ASSERT_TRUE(f.usb.poll().has_value());
  auto tx = f.usb.take_completed(8, true);
  ASSERT_TRUE(tx);
  EXPECT_TRUE(tx->overrun);
  EXPECT_FALSE(tx->acknowledged);
  ASSERT_TRUE(f.usb.poll().has_value());
  auto rx = f.usb.take_completed(8, false);
  ASSERT_TRUE(rx);
  EXPECT_EQ(out[0], 4U);
  fixture single;
  ASSERT_TRUE(single.init());
  ASSERT_TRUE(
      single.usb
          .configure_endpoint(8, true, usb_endpoint_type::isochronous, 1023)
          .has_value());
  EXPECT_FALSE(
      single.usb.configure_endpoint(8, false, usb_endpoint_type::isochronous, 1)
          .has_value());
}
TEST(nrf_usb, reset_retains_active_dma_until_end, "REQ-0068", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  ASSERT_TRUE(f.usb.configure_endpoint(1, true, usb_endpoint_type::bulk, 64)
                  .has_value());
  alignas(4) std::array<std::uint8_t, 4> data{};
  f.mmio.dma_responds = false;
  ASSERT_TRUE(f.usb.transmit(1, data, 4).has_value());
  ASSERT_TRUE(f.usb.poll().has_value());
  f.mmio.poke(fake_usb::base + 0x100, 1);
  ASSERT_TRUE(f.usb.poll().has_value());
  EXPECT_FALSE(f.usb.take_completed(1, true).has_value());
  f.mmio.complete();
  ASSERT_TRUE(f.usb.poll().has_value());
  auto done = f.usb.take_completed(1, true);
  ASSERT_TRUE(done);
  EXPECT_TRUE(done->bus_reset);
  EXPECT_TRUE(done->cancelled);
}
TEST(nrf_usb, cancellation_requires_enable_readback_and_retries, "REQ-0068",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  alignas(4) std::array<std::uint8_t, 4> data{};
  f.mmio.dma_responds = false;
  ASSERT_TRUE(f.usb.transmit(0, data, 4).has_value());
  ASSERT_TRUE(f.usb.poll().has_value());
  f.mmio.disable_responds = false;
  EXPECT_FALSE(f.usb.cancel_all().has_value());
  EXPECT_FALSE(f.usb.take_completed(0, true).has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::usbd));
  f.mmio.disable_responds = true;
  ASSERT_TRUE(f.usb.cancel_all().has_value());
  auto done = f.usb.take_completed(0, true);
  ASSERT_TRUE(done);
  EXPECT_TRUE(done->cancelled);
}
TEST(nrf_usb, suspend_remote_wake_and_clock_ownership, "REQ-0068") {
  fixture f;
  ASSERT_TRUE(f.init({.ticks_per_second = 1000}));
  f.mmio.cause(256);
  ASSERT_TRUE(f.usb.poll().has_value());
  ASSERT_TRUE(f.usb.enter_suspend().has_value());
  EXPECT_TRUE(f.usb.suspended());
  EXPECT_TRUE(f.context.system_off_blocked());
  EXPECT_FALSE(f.usb.remote_wakeup().has_value());
  f.usb.remote_wakeup_enabled(true);
  f.execution.advance(10);
  ASSERT_TRUE(f.usb.remote_wakeup().has_value());
  EXPECT_FALSE(f.usb.suspended());
  EXPECT_EQ(f.mmio.peek(fake_usb::base + 0x508), 1U);
}
TEST(nrf_usb, cdc_descriptors_address_and_configuration, "REQ-0068") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  usb_device<fixture::controller, usb_profile::cdc_acm, 0xFFFF, 1> d{f.usb};
  ASSERT_TRUE(d.initialize().has_value());
  ASSERT_TRUE(f.request(d, {0x80, 6, 0x100, 0, 18}));
  EXPECT_EQ(f.mmio.sent_size[0], 18U);
  EXPECT_EQ(f.mmio.sent[0][4], 2U);
  f.mmio.ack(0);
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(f.mmio.statuses, 1U);
  ASSERT_TRUE(f.request(d, {0, 5, 7, 0, 0}));
  EXPECT_EQ(f.mmio.statuses, 1U);
  ASSERT_TRUE(f.request(d, {0x80, 6, 0x200, 0, 255}));
  EXPECT_EQ(f.mmio.sent_size[0], 64U);
  f.mmio.ack(0);
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(f.mmio.sent_size[0], 3U);
  f.mmio.ack(0);
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(f.request(d, {0, 9, 1, 0, 0}));
  EXPECT_TRUE(d.configured());
  EXPECT_EQ(f.mmio.peek(fake_usb::base + 0x510), 7U);
}
TEST(nrf_usb, cdc_line_coding_data_backpressure_and_no_baud_reset, "REQ-0068") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  usb_device<fixture::controller, usb_profile::cdc_acm, 0xFFFF, 1> d{f.usb};
  ASSERT_TRUE(d.initialize().has_value());
  ASSERT_TRUE(f.request(d, {0, 9, 1, 0, 0}));
  ASSERT_TRUE(f.request(d, {0x21, 0x20, 0, 0, 7}));
  f.mmio.out(0, std::array<std::uint8_t, 7>{0xB0, 4, 0, 0, 0, 0, 8});
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(d.line_coding()[0], 0xB0U);
  EXPECT_EQ(f.mmio.resets, 0U);
  f.mmio.out(1, std::array<std::uint8_t, 3>{7, 8, 9});
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.poll().has_value());
  std::array<std::uint8_t, 2> data{};
  EXPECT_EQ(d.read(data), 2U);
  EXPECT_EQ(data[1], 8U);
  EXPECT_EQ(d.read(data), 1U);
  EXPECT_EQ(data[0], 9U);
  ASSERT_TRUE(d.write(std::array<std::uint8_t, 3>{1, 2, 3}).has_value());
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_TRUE(d.transmitting());
  f.mmio.ack(1);
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_FALSE(d.transmitting());
}
TEST(nrf_usb, new_setup_does_not_overwrite_old_dma_ram, "REQ-0068",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  usb_device<fixture::controller, usb_profile::cdc_acm, 0xFFFF, 1> d{f.usb};
  ASSERT_TRUE(d.initialize().has_value());
  f.mmio.dma_responds = false;
  ASSERT_TRUE(f.request(d, {0x80, 6, 0x100, 0, 18}));
  ASSERT_TRUE(f.mmio.active.has_value());
  auto *old = static_cast<std::uint8_t *>(
      f.mmio.pointer(f.mmio.peek(fake_usb::base + 0x600)));
  ASSERT_TRUE(old);
  EXPECT_EQ(old[0], 18U);
  f.mmio.setup({0x80, 6, 0x200, 0, 67});
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(old[0], 18U);
  f.mmio.complete();
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(old[0], 9U);
}
TEST(nrf_usb, hid_report_protocol_led_idle_and_invalid_requests, "REQ-0068") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  usb_device<fixture::controller, usb_profile::hid_keyboard, 0xFFFF, 2> d{
      f.usb};
  ASSERT_TRUE(d.initialize().has_value());
  ASSERT_TRUE(f.request(d, {0, 9, 1, 0, 0}));
  ASSERT_TRUE(f.request(d, {0x81, 6, 0x2200, 0, 255}));
  EXPECT_EQ(f.mmio.sent_size[0], 63U);
  f.mmio.ack(0);
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(f.request(d, {0x21, 9, 0x200, 0, 1}));
  f.mmio.out(0, std::array<std::uint8_t, 1>{5});
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(d.leds(), 5U);
  ASSERT_TRUE(f.request(d, {0x21, 11, 0, 0, 0}));
  ASSERT_TRUE(f.request(d, {0xA1, 3, 0, 0, 1}));
  EXPECT_EQ(f.mmio.sent[0][0], 0U);
  f.mmio.ack(0);
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.send_report({2, 0, 4, 0, 0, 0, 0, 0}).has_value());
  ASSERT_TRUE(d.poll().has_value());
  ASSERT_TRUE(d.poll().has_value());
  EXPECT_EQ(f.mmio.sent[1][2], 4U);
  f.mmio.ack(1);
  ASSERT_TRUE(d.poll().has_value());
  auto stalls = f.mmio.stalls;
  ASSERT_TRUE(f.request(d, {0x21, 11, 2, 0, 0}));
  EXPECT_EQ(f.mmio.stalls, stalls + 1);
}
TEST(nrf_usb, bootloader_handoff_requires_explicit_abi_and_valid_vectors,
     "REQ-0068") {
  fixture f;
  ASSERT_TRUE(f.init());
  EXPECT_FALSE(f.usb.request_bootloader({0xF0000, 0x100000, 0xB1, false, true})
                   .has_value());
  f.mmio.poke(0xF0000, 0x20040000);
  f.mmio.poke(0xF0004, 0xF0101);
  ASSERT_TRUE(f.usb.request_bootloader({0xF0000, 0x100000, 0xB1, true, true})
                  .has_value());
  EXPECT_EQ(f.mmio.resets, 1U);
  EXPECT_EQ(f.mmio.peek(0x4000051C), 0xB1U);
}

TEST(nrf_usb, independent_radio_and_usb_retain_shared_clock_until_both_release,
     "REQ-0074", "REQ-0062", "REQ-0068", "REQ-0069") {
  fixture f;
  ASSERT_TRUE(f.init());
  f.mmio.poke(0x40001788, 1);
  radio_controller<decltype(f.context), decltype(f.buffers), usb_address, 64>
      radio{f.context, f.clocks, f.buffers, 100, {&f.mmio}};
  ASSERT_TRUE(radio.initialize().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 2U);
  f.mmio.cause(256);
  ASSERT_TRUE(f.usb.poll().has_value());
  ASSERT_TRUE(f.usb.enter_suspend().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  EXPECT_EQ(f.mmio.peek(0x40000408), 1U);
  ASSERT_TRUE(f.usb.shutdown().has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::radio));
  ASSERT_TRUE(radio.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
} // namespace
