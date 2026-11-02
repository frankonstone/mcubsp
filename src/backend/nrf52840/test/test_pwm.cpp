#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/pwm.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
template <unsigned I = 0> struct fixture {
  using binding = pwm_binding<I, {0, 6}, {0, 8}, {0, 7}>;
  nrf52840_test::fake_analog mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<decltype(mmio)> gpio{mmio, resources};
  system_context<decltype(mmio), execution::fake_policy> context{
      mmio, execution, gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy> buffers{resources, execution};
  pwm_controller<binding, decltype(context), decltype(buffers),
                 nrf52840_test::analog_address, 16>
      pwm{context, clocks, buffers, 100};
  fixture() {
    mmio.execution = &execution;
    mmio.pwm_base = binding::base;
  }
  ~fixture() {
    (void)pwm.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize(pwm_configuration c = {},
                  pwm_alignment alignment = pwm_alignment::edge) {
    return clocks.initialize().has_value() &&
           pwm.initialize(*solve_pwm_timing({16000000, 1000, 0, 15}, alignment),
                          c)
               .has_value();
  }
};
TEST(nrf_pwm,
     solves_native_edge_and_center_timing_and_rejects_unattainable_rates,
     "REQ-0063") {
  auto edge = solve_pwm_timing({16000000, 1000, 0, 15}),
       center =
           solve_pwm_timing({16000000, 1000, 0, 15}, pwm_alignment::center);
  ASSERT_TRUE(edge.has_value());
  ASSERT_TRUE(center.has_value());
  EXPECT_EQ(edge->period_counts, 16000U);
  EXPECT_EQ(center->period_counts, 8000U);
  EXPECT_FALSE(solve_pwm_timing({64000000, 1000, 0, 15}).has_value());
  EXPECT_FALSE(solve_pwm_timing({16000000, 0, 0, 15}).has_value());
  EXPECT_FALSE(solve_pwm_timing({16000000, 1000, 0, 16}).has_value());
}
TEST(nrf_pwm,
     every_instance_and_decoder_has_explicit_register_and_pin_configuration,
     "REQ-0063", "REQ-0062") {
  for (unsigned mode = 0; mode < 4; ++mode) {
    fixture f;
    ASSERT_TRUE(f.initialize({static_cast<pwm_decoder>(mode), true}));
    EXPECT_EQ(f.mmio.peek(f.mmio.pwm_base + 0x510), mode | 256U);
    EXPECT_EQ(f.mmio.peek(f.mmio.pwm_base + 0x560), 6U);
  }
  auto run = []<unsigned I>() {
    fixture<I> f;
    ASSERT_TRUE(f.initialize());
    std::array<std::uint16_t, 4> values{0, 1000, 8000, 16000};
    auto op = f.pwm.play({values});
    ASSERT_TRUE(op.has_value());
    f.mmio.poke(f.mmio.pwm_base + 0x104, 1);
    ASSERT_TRUE(f.pwm.poll());
    ASSERT_TRUE(op->take().has_value());
  };
  run.template operator()<1>();
  run.template operator()<2>();
  run.template operator()<3>();
}
TEST(nrf_pwm, sequence_end_does_not_return_dma_before_stopped, "REQ-0063",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint16_t, 4> values{};
  auto op = f.pwm.play({values});
  ASSERT_TRUE(op.has_value());
  f.mmio.poke(f.mmio.pwm_base + 0x110, 1);
  ASSERT_TRUE(f.pwm.poll());
  EXPECT_FALSE(op->ready());
  EXPECT_TRUE(f.pwm.running());
  f.mmio.poke(f.mmio.pwm_base + 0x104, 1);
  ASSERT_TRUE(f.pwm.poll());
  auto result = op->take();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->ended_sequences, 1U);
}
TEST(nrf_pwm,
     loops_and_repeat_shortcuts_obey_anomaly_183_and_next_step_contract,
     "REQ-0063") {
  fixture f;
  ASSERT_TRUE(f.initialize({pwm_decoder::common, true}));
  std::array<std::uint16_t, 2> two{100, 200};
  std::array<std::uint16_t, 1> one{300};
  EXPECT_FALSE(f.pwm.play({two}, {one}, {0, 2, true}).has_value());
  auto op = f.pwm.play({two}, {two}, {1, 3, true});
  ASSERT_TRUE(op.has_value());
  EXPECT_EQ(f.mmio.peek(f.mmio.pwm_base + 0x200), 8U);
  EXPECT_EQ(f.mmio.peek(f.mmio.pwm_base + 0x514), 3U);
  ASSERT_TRUE(f.pwm.next_step().has_value());
  ASSERT_TRUE(op->cancel().has_value());
  (void)op->take();
}
TEST(nrf_pwm, waveform_top_and_sequence_lengths_are_validated_before_start,
     "REQ-0063", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize({pwm_decoder::waveform}));
  std::array<std::uint16_t, 4> values{100, 200, 300, 2};
  EXPECT_FALSE(f.pwm.play({values}).has_value());
  values[3] = 16000;
  auto op = f.pwm.play({values});
  ASSERT_TRUE(op.has_value());
  ASSERT_TRUE(op->cancel().has_value());
  (void)op->take();
  std::array<std::uint16_t, 3> bad{};
  EXPECT_FALSE(f.pwm.play({bad}).has_value());
  EXPECT_EQ(f.mmio.pwm_starts, 1U);
}
TEST(nrf_pwm, failed_start_and_stop_keep_clock_and_buffers_until_late_ack,
     "REQ-0063", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint16_t, 4> values{};
  f.mmio.failed_address = f.mmio.pwm_base + 8;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  auto op = f.pwm.play({values});
  ASSERT_TRUE(op.has_value());
  f.mmio.stop_responds = false;
  EXPECT_FALSE(op->cancel().has_value());
  (void)op->take();
  EXPECT_FALSE(f.pwm.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 1U);
  f.mmio.poke(f.mmio.pwm_base + 0x104, 1);
  ASSERT_TRUE(f.pwm.shutdown().has_value());
  EXPECT_EQ(f.clocks.hf_clients(), 0U);
}
TEST(nrf_pwm,
     terminal_ack_error_retains_latched_quiescence_without_a_second_stop,
     "REQ-0063", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  std::array<std::uint16_t, 4> values{};
  auto op = f.pwm.play({values});
  ASSERT_TRUE(op.has_value());
  f.mmio.poke(f.mmio.pwm_base + 0x104, 1);
  f.mmio.fail_event_ack = f.mmio.pwm_base + 0x104;
  EXPECT_FALSE(f.pwm.poll());
  f.mmio.stop_responds = false;
  ASSERT_TRUE(f.pwm.poll());
  EXPECT_FALSE(op->take().has_value());
  ASSERT_TRUE(f.pwm.shutdown().has_value());
}
} // namespace
