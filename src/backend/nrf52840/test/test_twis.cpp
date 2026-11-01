#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/twis.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_twis : public nrf52840_test::fake_system {
public:
  std::uintptr_t base{0x40003000};
  bool stop_responds{true}, fail_ack{};
  unsigned resumes{}, prepares{};
  std::uint32_t interrupts{};
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
    if (a == base + 0x4D0)
      poke(a, previous & ~v);
    if (a == base + 0x14 && v && stop_responds)
      poke(base + 0x104, 1);
    if (a == base + 0x20 && v)
      ++resumes;
    if (a == base + 0x30 && v) {
      ++prepares;
      poke(base + 0x14C, 1);
      poke(base + 0x53C, 0);
    }
    if (a == base + 0x34 && v) {
      ++prepares;
      poke(base + 0x150, 1);
      poke(base + 0x54C, 0);
    }
    if (a == base + 0x104 && !v && previous && fail_ack) {
      fail_ack = false;
      return std::unexpected(core::mmio_error::out_of_range);
    }
    return r;
  }
  void request(bool read, unsigned slot = 0) {
    poke(base + (read ? 0x168 : 0x164), 1);
    poke(base + 0x4D4, slot);
  }
  void complete(unsigned tx, unsigned rx, unsigned bits = 0) {
    poke(base + 0x54C, tx);
    poke(base + 0x53C, rx);
    poke(base + 0x4D0, bits);
    if (bits)
      poke(base + 0x124, 1);
    poke(base + 0x104, 1);
  }
};
template <unsigned I = 0> struct fixture {
  using binding = twis_binding<I, {0, 6}, {0, 8}>;
  fake_twis mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_twis> gpio{mmio, resources};
  system_context<fake_twis, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 4> buffers{resources, execution};
  twis_controller<binding, decltype(context), decltype(buffers),
                  native_dma_address, 4>
      twi{context, clocks, buffers, 100};
  fixture() {
    mmio.base = binding::base;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 3);
  }
  ~fixture() {
    (void)twi.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(twis_configuration c = {}) {
    return clocks.initialize().has_value() && twi.initialize(c).has_value();
  }
};
std::span<std::uint8_t> ram(std::uintptr_t a = 0x20001000, std::size_t n = 8) {
  return {reinterpret_cast<std::uint8_t *>(a), n};
}
TEST(nrf_twis, both_instances_configure_two_addresses_before_enabling,
     "REQ-0061") {
  fixture f;
  ASSERT_TRUE(f.initialize({{0x42}, driver::i2c_address{0x43}, 0xA5}));
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x594), 3U);
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 0U);
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x5C0), 0xA5U);
  fixture<1> one;
  ASSERT_TRUE(one.initialize());
}
TEST(nrf_twis, write_read_repeated_start_stretches_until_each_explicit_resume,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize({{0x42}, driver::i2c_address{0x43}}));
  auto op = f.twi.listen(ram(0x20002000, 4), ram(0x20001000, 2));
  ASSERT_TRUE(op.has_value());
  EXPECT_EQ(f.mmio.prepares, 0U);
  f.mmio.request(false, 1);
  ASSERT_TRUE(f.twi.poll());
  auto request = f.twi.pending_request();
  ASSERT_TRUE(request.has_value());
  EXPECT_EQ(request->address.value, 0x43U);
  EXPECT_TRUE(request->direction == twis_direction::master_write);
  ASSERT_TRUE(f.twi.resume().has_value());
  f.mmio.poke(f.mmio.base + 0x53C, 2);
  f.mmio.request(true);
  ASSERT_TRUE(f.twi.poll());
  request = f.twi.pending_request();
  ASSERT_TRUE(request.has_value());
  EXPECT_EQ(request->previous.bytes_read, 2U);
  EXPECT_FALSE(op->ready());
  ASSERT_TRUE(f.twi.resume().has_value());
  f.mmio.complete(4, 2, 4);
  ASSERT_TRUE(f.twi.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->received, 2U);
  EXPECT_EQ(result->transmitted, 4U);
  EXPECT_EQ(result->phases, 2U);
  EXPECT_EQ(result->error_source, 4U);
  EXPECT_EQ(f.mmio.resumes, 2U);
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 0U);
}
TEST(nrf_twis, dynamic_response_is_supplied_only_while_address_is_pending,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.twi.listen();
  ASSERT_TRUE(op.has_value());
  EXPECT_FALSE(f.twi.provide_transmit(ram()).has_value());
  f.mmio.request(true);
  ASSERT_TRUE(f.twi.poll());
  EXPECT_FALSE(f.twi.resume().has_value());
  ASSERT_TRUE(f.twi.provide_transmit(ram()).has_value());
  ASSERT_TRUE(f.twi.resume().has_value());
  EXPECT_FALSE(f.twi.provide_transmit(ram()).has_value());
  f.mmio.complete(5, 0);
  ASSERT_TRUE(f.twi.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->transmitted, 5U);
}
TEST(nrf_twis, overflow_and_overread_report_counts_and_stop_without_reuse,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.twi.listen({}, ram());
  ASSERT_TRUE(op.has_value());
  f.mmio.request(false);
  ASSERT_TRUE(f.twi.poll());
  ASSERT_TRUE(f.twi.resume().has_value());
  f.mmio.poke(f.mmio.base + 0x53C, 8);
  f.mmio.poke(f.mmio.base + 0x4D0, 1);
  f.mmio.poke(f.mmio.base + 0x124, 1);
  ASSERT_TRUE(f.twi.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->received, 8U);
  EXPECT_EQ(result->error_source, 1U);
  EXPECT_EQ(f.mmio.prepares, 1U);
}
TEST(nrf_twis, duplicate_direction_cannot_reuse_a_buffer, "REQ-0061",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.twi.listen({}, ram());
  ASSERT_TRUE(op.has_value());
  f.mmio.request(false);
  ASSERT_TRUE(f.twi.poll());
  ASSERT_TRUE(f.twi.resume().has_value());
  f.mmio.request(false);
  ASSERT_TRUE(f.twi.poll());
  EXPECT_FALSE(f.twi.resume().has_value());
  EXPECT_EQ(f.mmio.prepares, 1U);
  ASSERT_TRUE(op->cancel().has_value());
  (void)op->take();
}
TEST(nrf_twis, alias_invalid_address_ram_and_unreviewed_revision_are_rejected,
     "REQ-0055", "REQ-0061", "REQ-0062") {
  {
    fixture f;
    EXPECT_FALSE(f.initialize({{0x01}}));
  }
  {
    fixture f;
    f.mmio.poke(0x10000134, 0);
    EXPECT_FALSE(f.initialize());
    EXPECT_EQ(f.mmio.prepares, 0U);
  }
  {
    fixture f;
    ASSERT_TRUE(f.initialize());
    EXPECT_FALSE(
        f.resources.claim(f.execution, peripheral::twim0, &f).has_value());
    EXPECT_FALSE(f.twi.listen(ram(0)).has_value());
    EXPECT_FALSE(f.twi.listen(ram(), ram()).has_value());
    EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 0U);
  }
}
TEST(nrf_twis, cancellation_keeps_dma_leases_until_stopped_and_disable,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.twi.listen({}, ram());
  ASSERT_TRUE(op.has_value());
  f.mmio.request(false);
  ASSERT_TRUE(f.twi.poll());
  ASSERT_TRUE(f.twi.resume().has_value());
  f.mmio.stop_responds = false;
  EXPECT_FALSE(op->cancel().has_value());
  (void)op->take();
  EXPECT_FALSE(f.twi.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  auto owner = f.resources.claim(f.execution, peripheral::uarte1, &f);
  ASSERT_TRUE(owner.has_value());
  EXPECT_FALSE(f.buffers
                   .prepare(*owner, mcu::nrf52840::dma_constraints{16, 1, 1},
                            0x20001000, 8, 8, dma_access::write)
                   .has_value());
  ASSERT_TRUE(f.resources.release(f.execution, *owner).has_value());
  f.mmio.complete(0, 3);
  ASSERT_TRUE(f.twi.shutdown().has_value());
}
TEST(nrf_twis, prepare_failure_after_commit_requires_stop_before_result,
     "REQ-0061", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  auto op = f.twi.listen(ram(), {});
  ASSERT_TRUE(op.has_value());
  f.mmio.request(true);
  ASSERT_TRUE(f.twi.poll());
  f.mmio.failed_address = f.mmio.base + 0x34;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  EXPECT_FALSE(f.twi.resume().has_value());
  EXPECT_FALSE(op->ready());
  ASSERT_TRUE(f.twi.poll());
  EXPECT_FALSE(op->take().has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.base + 0x500), 0U);
}
TEST(nrf_twis, terminal_ack_failure_does_not_lose_stop_proof, "REQ-0061",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  ASSERT_TRUE(f.twi.interrupt_delivery(true).has_value());
  auto op = f.twi.listen();
  ASSERT_TRUE(op.has_value());
  f.mmio.complete(0, 0);
  f.mmio.fail_ack = true;
  EXPECT_FALSE(f.twi.poll());
  f.mmio.stop_responds = false;
  ASSERT_TRUE(f.twi.poll());
  EXPECT_FALSE(op->take().has_value());
}
TEST(nrf_twis, aborted_address_is_reported_even_when_stop_arrives_before_poll,
     "REQ-0061", "REQ-0062") {
  fixture<1> f;
  ASSERT_TRUE(f.initialize({{0x42}, driver::i2c_address{0x43}}));
  auto op = f.twi.listen();
  ASSERT_TRUE(op.has_value());
  f.mmio.request(true, 1);
  f.mmio.complete(0, 0);
  ASSERT_TRUE(f.twi.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->address_slot, 1U);
  EXPECT_EQ(result->phases, 1U);
  EXPECT_TRUE(result->last_direction == twis_direction::master_read);
  EXPECT_EQ(f.mmio.prepares, 0U);
}
} // namespace
