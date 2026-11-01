#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/spis.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_spis : public nrf52840_test::fake_system {
public:
  std::uintptr_t base{0x40003000};
  unsigned releases{}, acquires{};
  std::uint32_t interrupts{};
  bool fail_ack{};
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto previous = peek(a);
    auto r = nrf52840_test::fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a == base + 0x304)
      interrupts |= v;
    if (a == base + 0x308)
      interrupts &= ~v;
    if (a == base + 0x500 && v == 2)
      poke(base + 0x400, 1);
    if (a == base + 0x440)
      poke(a, previous & ~v);
    if (a == base + 0x028 && v) {
      ++releases;
      poke(base + 0x400, 0);
    }
    if (a == base + 0x024 && v) {
      ++acquires;
      auto sem = peek(base + 0x400);
      if (sem == 0 || sem == 1) {
        poke(base + 0x400, 1);
        poke(base + 0x128, 1);
      } else
        poke(base + 0x400, 3);
    }
    if (a == base + 0x104 && !v && previous && fail_ack) {
      fail_ack = false;
      return std::unexpected(core::mmio_error::out_of_range);
    }
    return r;
  }
  void busy() { poke(base + 0x400, 2); }
  void complete(unsigned tx, unsigned rx, unsigned status = 0) {
    poke(base + 0x400, 1);
    poke(base + 0x128, 1);
    poke(base + 0x104, 1);
    poke(base + 0x54C, tx);
    poke(base + 0x53C, rx);
    poke(base + 0x440, status);
  }
};
template <unsigned I = 0> struct fixture {
  using binding = spis_binding<I, {0, 6}, {0, 8}, {0, 7}, {0, 12}>;
  fake_spis mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_spis> gpio{mmio, resources};
  system_context<fake_spis, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 4> buffers{resources, execution};
  spis_controller<binding, decltype(context), decltype(buffers),
                  native_dma_address, 4>
      spi{context, clocks, buffers, 100};
  fixture() { mmio.base = binding::base; }
  ~fixture() {
    (void)spi.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(spis_configuration config = {}) {
    return clocks.initialize().has_value() &&
           spi.initialize(config).has_value();
  }
};
std::span<std::uint8_t> ram(std::uintptr_t a = 0x20001000, std::size_t n = 8) {
  return {reinterpret_cast<std::uint8_t *>(a), n};
}
TEST(nrf_spis, all_instances_modes_and_default_characters_are_native,
     "REQ-0060") {
  for (unsigned mode = 0; mode < 4; ++mode) {
    fixture f;
    ASSERT_TRUE(f.initialize({static_cast<driver::spi_mode>(mode),
                              driver::spi_bit_order::least_significant_first,
                              0x55, 0xAA}));
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 2U);
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x554),
              1U | ((mode & 1) << 1) | ((mode & 2) << 1));
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x55C), 0x55U);
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x5C0), 0xAAU);
  }
  fixture<1> one;
  ASSERT_TRUE(one.initialize());
  fixture<2> two;
  ASSERT_TRUE(two.initialize());
}
TEST(nrf_spis, end_acquire_prevents_automatic_buffer_reuse_and_reports_status,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.spi.transfer(ram(), ram(0x20002000));
  ASSERT_TRUE(op.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x200), 4U);
  f.mmio.complete(8, 8, 3);
  ASSERT_TRUE(f.spi.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->transmitted, 8U);
  EXPECT_EQ(result->received, 8U);
  EXPECT_TRUE(result->overread);
  EXPECT_TRUE(result->overflow);
  EXPECT_EQ(f.mmio.releases, 1U);
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x400), 1U);
}
TEST(nrf_spis, alias_and_invalid_dma_spans_fail_without_releasing_semaphore,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  EXPECT_FALSE(
      f.resources.claim(f.execution, peripheral::spi0, &f).has_value());
  EXPECT_FALSE(f.spi.transfer(ram(0), {}).has_value());
  EXPECT_FALSE(f.spi.transfer(ram(0x2003FFFC), {}).has_value());
  EXPECT_FALSE(f.spi.transfer(ram(), ram()).has_value());
  EXPECT_EQ(f.mmio.releases, 0U);
}
TEST(nrf_spis, cancel_before_chip_select_returns_buffers_and_no_stale_amount,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(f.mmio.base + 0x54C, 7);
  auto op = f.spi.transfer(ram(), {});
  ASSERT_TRUE(op.has_value());
  ASSERT_TRUE(op->cancel().has_value());
  (void)op->take();
  EXPECT_EQ(f.spi.transferred_count().transmitted, 0U);
  auto next = f.spi.transfer(ram(), {});
  ASSERT_TRUE(next.has_value());
  ASSERT_TRUE(next->cancel().has_value());
  (void)next->take();
}
TEST(nrf_spis, cancel_during_cs_holds_buffers_and_clock_until_cpu_semaphore,
     "REQ-0060", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.spi.transfer(ram(), ram(0x20002000));
  ASSERT_TRUE(op.has_value());
  f.mmio.busy();
  EXPECT_FALSE(op->cancel().has_value());
  (void)op->take();
  EXPECT_FALSE(f.spi.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  auto owner = f.resources.claim(f.execution, peripheral::uarte1, &f);
  ASSERT_TRUE(owner.has_value());
  EXPECT_FALSE(f.buffers
                   .prepare(*owner, mcu::nrf52840::dma_constraints{16, 1, 1},
                            0x20002000, 8, 8, dma_access::write)
                   .has_value());
  ASSERT_TRUE(f.resources.release(f.execution, *owner).has_value());
  f.mmio.complete(3, 3);
  ASSERT_TRUE(f.spi.shutdown().has_value());
  EXPECT_TRUE(f.mmio.wrote(f.mmio.base + 0xA4, 1));
  EXPECT_TRUE(f.mmio.wrote(f.mmio.base + 0xAC, 1));
}
TEST(nrf_spis, failed_release_after_commit_requires_semaphore_proof, "REQ-0060",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.failed_address = f.mmio.base + 0x28;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  auto op = f.spi.transfer(ram(), {});
  ASSERT_TRUE(op.has_value());
  f.mmio.busy();
  EXPECT_FALSE(f.spi.poll());
  f.mmio.complete(2, 0);
  ASSERT_TRUE(f.spi.poll());
  EXPECT_FALSE(op->take().has_value());
}
TEST(nrf_spis, terminal_ack_failure_preserves_latched_quiescence, "REQ-0060",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.spi.interrupt_delivery(true).has_value());
  EXPECT_EQ(f.mmio.interrupts, 0x402U);
  auto op = f.spi.transfer(ram(), {});
  ASSERT_TRUE(op.has_value());
  f.mmio.complete(8, 0);
  f.mmio.fail_ack = true;
  EXPECT_FALSE(f.spi.poll());
  ASSERT_TRUE(f.spi.poll());
  EXPECT_FALSE(op->take().has_value());
  EXPECT_FALSE(f.mmio.peek(f.mmio.base + 0x104));
  EXPECT_FALSE(f.mmio.peek(f.mmio.base + 0x128));
}
} // namespace
