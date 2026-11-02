#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/i2s.hpp>
#include <mcubsp/backend/nrf52840/pdm.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_audio : public nrf52840_test::fake_analog {
public:
  bool responds{true};
  std::optional<std::uintptr_t> failed_write;
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto r = fake_analog::write<T>(a, v);
    if (!r)
      return r;
    if (a == 0x40025000 && v) {
      if (peek(0x40025508))
        poke(0x40025104, 1);
      if (peek(0x4002550C))
        poke(0x40025114, 1);
    }
    if (a == 0x40025004 && v && responds) {
      poke(0x40025108, 1);
      poke(0x40025104, 1);
      poke(0x40025114, 1);
    }
    if (a == 0x4001D000 && v)
      poke(0x4001D100, 1);
    if (a == 0x4001D004 && v && responds)
      poke(0x4001D104, 1);
    if (failed_write == a) {
      failed_write.reset();
      return std::unexpected(core::mmio_error::invalid_value);
    }
    return {};
  }
  void i2s_boundary() {
    if (peek(0x40025508))
      poke(0x40025104, 1);
    if (peek(0x4002550C))
      poke(0x40025114, 1);
  }
  void pdm_boundary() {
    poke(0x4001D100, 1);
    poke(0x4001D108, 1);
  }
};
using ib =
    i2s_binding<core::pin_id{0, 20}, core::pin_id{0, 21}, core::pin_id{0, 22},
                core::pin_id{0, 23}, core::pin_id{0, 24}>;
using pb = pdm_binding<core::pin_id{0, 17}, core::pin_id{0, 19}>;
struct fixture {
  fake_audio mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_audio> gpio{mmio, resources};
  system_context<fake_audio, execution::fake_policy> context{mmio, execution,
                                                             gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 16> buffers{resources, execution};
  i2s_controller<ib, decltype(context), decltype(buffers),
                 nrf52840_test::analog_address, 4, 64>
      i2s{context, clocks, buffers, 100};
  pdm_controller<pb, decltype(context), decltype(buffers),
                 nrf52840_test::analog_address, 4, 64>
      pdm{context, clocks, buffers, 100};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 5);
  }
  ~fixture() {
    mmio.responds = true;
    (void)i2s.shutdown();
    (void)pdm.shutdown();
    (void)clocks.shutdown();
  }
  audio_block<std::uint32_t> block(unsigned i) {
    return {
        {reinterpret_cast<const std::uint32_t *>(0x20001000 + 0x100 * i), 16},
        {reinterpret_cast<std::uint32_t *>(0x20004000 + 0x100 * i), 16},
        i};
  }
  std::span<std::int16_t> samples(unsigned i) {
    return {reinterpret_cast<std::int16_t *>(0x20008000 + 0x100 * i), 32};
  }
};
TEST(nrf_audio, i2s_clock_solver_checks_ratio_width_and_tolerance, "REQ-0067") {
  EXPECT_FALSE(solve_i2s_timing(44100, 16, 0).has_value());
  auto t = solve_i2s_timing(16000, 16, 9000);
  ASSERT_TRUE(t.has_value());
  EXPECT_TRUE(t->error_ppm <= 9000);
  EXPECT_FALSE(solve_i2s_timing(0).has_value());
  for (unsigned width : std::array{8U, 16U, 24U}) {
    auto p = solve_i2s_timing(16000, width, 100000);
    ASSERT_TRUE(p.has_value());
    EXPECT_EQ(p->ratio % (2 * width), 0U);
  }
}
TEST(nrf_audio, i2s_finite_duplex_uses_tail_then_stopped_and_disconnects_pins,
     "REQ-0067", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.i2s.initialize().has_value());
  ASSERT_TRUE(f.i2s.start(f.block(0), f.block(4)).has_value());
  ASSERT_TRUE(f.i2s.poll().has_value());
  EXPECT_FALSE(f.i2s.take_completed().has_value());
  f.mmio.i2s_boundary();
  ASSERT_TRUE(f.i2s.poll().has_value());
  auto completed = f.i2s.take_completed();
  ASSERT_TRUE(completed.has_value());
  EXPECT_EQ(completed->sequence, 0U);
  EXPECT_TRUE(f.i2s.running());
  ASSERT_TRUE(f.i2s.poll().has_value());
  EXPECT_FALSE(f.i2s.running());
  EXPECT_EQ(f.i2s.result().completed_blocks, 1U);
  ASSERT_TRUE(f.i2s.shutdown().has_value());
  EXPECT_TRUE(f.mmio.wrote(0x40025038, 1));
  EXPECT_TRUE(f.mmio.wrote(0x4002503C, 1));
  for (unsigned i = 0; i < 5; ++i)
    EXPECT_EQ(f.mmio.peek(0x40025560 + 4 * i), 0x80000000U);
}
TEST(nrf_audio, i2s_streaming_requeues_completed_blocks_with_fixed_storage,
     "REQ-0067") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.i2s.initialize().has_value());
  ASSERT_TRUE(f.i2s.start(f.block(0), f.block(4), 3).has_value());
  ASSERT_TRUE(f.i2s.enqueue(f.block(1)).has_value());
  ASSERT_TRUE(f.i2s.poll().has_value());
  f.mmio.i2s_boundary();
  ASSERT_TRUE(f.i2s.poll().has_value());
  auto first = f.i2s.take_completed();
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(f.i2s.enqueue(*first).has_value());
  // Queue must already contain the next block when the handoff occurs.
  EXPECT_TRUE(f.i2s.result()
                  .exhausted); // only block 1 was queued at the preceding latch
  f.mmio.i2s_boundary();
  ASSERT_TRUE(f.i2s.poll().has_value());
  ASSERT_TRUE(f.i2s.poll().has_value());
  EXPECT_EQ(f.i2s.result().completed_blocks, 2U);
}
TEST(nrf_audio, i2s_failed_start_and_stop_never_release_active_ram, "REQ-0067",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.i2s.initialize().has_value());
  f.mmio.failed_write = 0x40025000;
  EXPECT_FALSE(f.i2s.start(f.block(0), f.block(4)).has_value());
  f.mmio.responds = false;
  EXPECT_FALSE(f.i2s.cancel().has_value());
  EXPECT_TRUE(f.i2s.running());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::i2s));
  f.mmio.poke(0x40025108, 1);
  ASSERT_TRUE(f.i2s.cancel().has_value());
  EXPECT_FALSE(f.i2s.take_completed().has_value());
}
TEST(nrf_audio, i2s_late_poll_stops_without_inventing_completed_counts,
     "REQ-0067") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.i2s.initialize().has_value());
  ASSERT_TRUE(f.i2s.start(f.block(0), f.block(4)).has_value());
  f.execution.advance(100000);
  ASSERT_TRUE(f.i2s.poll().has_value());
  EXPECT_TRUE(f.i2s.result().service_overrun);
  ASSERT_TRUE(f.i2s.poll().has_value());
  EXPECT_FALSE(f.i2s.take_completed().has_value());
}
TEST(nrf_audio, pdm_mono_stereo_gains_ratios_and_finite_handoff, "REQ-0067") {
  for (bool mono : std::array{false, true}) {
    fixture f;
    ASSERT_TRUE(f.clocks.initialize().has_value());
    ASSERT_TRUE(f.pdm
                    .initialize({.frequency = pdm_frequency::khz1280,
                                 .mono = mono,
                                 .left_rising = true,
                                 .ratio80 = true,
                                 .gain_left = 12,
                                 .gain_right = 72})
                    .has_value());
    EXPECT_EQ(f.mmio.peek(0x4001D508), mono ? 3U : 2U);
    EXPECT_EQ(f.mmio.peek(0x4001D520), 1U);
    EXPECT_EQ(f.mmio.peek(0x4001D518), 12U);
    ASSERT_TRUE(f.pdm.start(f.samples(0), f.samples(4)).has_value());
    ASSERT_TRUE(f.pdm.poll().has_value());
    EXPECT_FALSE(f.pdm.take_completed().has_value());
    f.mmio.pdm_boundary();
    ASSERT_TRUE(f.pdm.poll().has_value());
    auto b = f.pdm.take_completed();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(b->rx.size(), 32U);
    ASSERT_TRUE(f.pdm.poll().has_value());
    EXPECT_FALSE(f.pdm.running());
    EXPECT_EQ(f.pdm.result().completed_blocks, 1U);
  }
}
TEST(nrf_audio, pdm_requires_started_and_end_before_publication, "REQ-0067",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.pdm.initialize().has_value());
  ASSERT_TRUE(f.pdm.start(f.samples(0), f.samples(4)).has_value());
  ASSERT_TRUE(f.pdm.poll().has_value());
  f.mmio.poke(0x4001D108, 1);
  ASSERT_TRUE(f.pdm.poll().has_value());
  EXPECT_FALSE(f.pdm.take_completed().has_value());
  f.mmio.poke(0x4001D100, 1);
  ASSERT_TRUE(f.pdm.poll().has_value());
  EXPECT_TRUE(f.pdm.take_completed().has_value());
  ASSERT_TRUE(f.pdm.poll().has_value());
}
TEST(nrf_audio, pdm_streaming_bounds_failed_stop_and_late_service, "REQ-0067",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.pdm.initialize().has_value());
  EXPECT_FALSE(
      f.pdm.start(f.samples(0).first(31), f.samples(4).first(31)).has_value());
  ASSERT_TRUE(f.pdm.start(f.samples(0), f.samples(4), 2).has_value());
  ASSERT_TRUE(f.pdm.enqueue(f.samples(1), 1).has_value());
  ASSERT_TRUE(f.pdm.poll().has_value());
  f.mmio.pdm_boundary();
  ASSERT_TRUE(f.pdm.poll().has_value());
  EXPECT_TRUE(f.pdm.take_completed().has_value());
  f.mmio.responds = false;
  EXPECT_FALSE(f.pdm.cancel().has_value());
  EXPECT_TRUE(f.pdm.running());
  f.mmio.poke(0x4001D104, 1);
  ASSERT_TRUE(f.pdm.cancel().has_value());
  EXPECT_FALSE(f.pdm.take_completed().has_value());
}
TEST(nrf_audio, analog_audio_and_qspi_owners_share_clock_and_pin_contracts,
     "REQ-0067", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.i2s.initialize().has_value());
  ASSERT_TRUE(f.pdm.initialize().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 2U);
  ASSERT_TRUE(f.i2s.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  ASSERT_TRUE(f.pdm.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}

TEST(nrf_audio, duplex_partial_handoff_expires_even_with_frequent_poll,
     "REQ-0067", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.i2s.initialize().has_value());
  ASSERT_TRUE(f.i2s.start(f.block(0), f.block(4), 3).has_value());
  ASSERT_TRUE(f.i2s.enqueue(f.block(1)).has_value());
  ASSERT_TRUE(f.i2s.poll().has_value());
  f.mmio.poke(0x40025104, 1);
  ASSERT_TRUE(f.i2s.poll().has_value());
  for (unsigned i = 0; i < 20 && !f.i2s.result().service_overrun; ++i) {
    f.execution.advance(4000);
    ASSERT_TRUE(f.i2s.poll().has_value());
  }
  EXPECT_TRUE(f.i2s.result().service_overrun);
  EXPECT_FALSE(f.i2s.take_completed().has_value());
}
TEST(nrf_audio, pdm_end_without_next_started_has_bounded_lifetime, "REQ-0067",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.pdm.initialize().has_value());
  ASSERT_TRUE(f.pdm.start(f.samples(0), f.samples(4), 3).has_value());
  ASSERT_TRUE(f.pdm.enqueue(f.samples(1)).has_value());
  ASSERT_TRUE(f.pdm.poll().has_value());
  f.mmio.poke(0x4001D108, 1);
  ASSERT_TRUE(f.pdm.poll().has_value());
  for (unsigned i = 0; i < 40 && !f.pdm.result().service_overrun; ++i) {
    f.execution.advance(4000);
    ASSERT_TRUE(f.pdm.poll().has_value());
  }
  EXPECT_TRUE(f.pdm.result().service_overrun);
  EXPECT_FALSE(f.pdm.take_completed().has_value());
}
} // namespace
