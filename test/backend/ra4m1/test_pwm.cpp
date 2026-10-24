#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/gpt.hpp>
#include <mcubsp/board/arduino_uno_r4_minima.hpp>
#include <mcubsp/driver/pwm.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {

namespace ra = mcubsp::mcu::ra4m1;
using d3_binding = mcubsp::backend::ra4m1::gpt_binding<
    1, ra::gpt_output::b,
    mcubsp::board::arduino_uno_r4_minima::d3>;
using mmio_type = mcubsp::testing::recording_mmio<0x60000, 192>;
using execution_type = mcubsp::execution::fake_policy;
using backend_type = mcubsp::backend::ra4m1::gpt_pwm_backend<
    d3_binding, mmio_type, execution_type>;
using pwm_type = mcubsp::driver::pwm_channel<backend_type>;

constexpr auto timing_20khz = mcubsp::backend::ra4m1::solve_pwm_timing({
    .clock_hz = 48'000'000U,
    .target_hz = 20'000U,
    .maximum_error_ppm = 0U,
    .counter_width = 32U,
});
static_assert(timing_20khz.has_value());
static_assert(timing_20khz->period_counts == 2400U);

[[nodiscard]] bool has_write(std::span<const mcubsp::testing::mmio_operation> ops,
                             std::uintptr_t address,
                             std::uint64_t value) {
    for (const auto& operation : ops) {
        if (operation.kind == mcubsp::testing::mmio_operation_kind::write
            && operation.address == address && operation.value == value) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool has_write_to(
    std::span<const mcubsp::testing::mmio_operation> ops,
    std::uintptr_t address) {
    for (const auto& operation : ops) {
        if (operation.kind == mcubsp::testing::mmio_operation_kind::write
            && operation.address == address) {
            return true;
        }
    }
    return false;
}

TEST(pwm_solver, solves_exact_20khz_against_the_real_48mhz_clock,
     "REQ-0011", "REQ-0022") {
    ASSERT_TRUE(timing_20khz.has_value());
    EXPECT_EQ(timing_20khz->prescaler, 1U);
    EXPECT_EQ(timing_20khz->prescaler_encoding, 0U);
    EXPECT_EQ(timing_20khz->period_counts, 2400U);
    EXPECT_EQ(timing_20khz->period_register, 2399U);
    EXPECT_EQ(timing_20khz->achieved_hz, 20'000U);
    EXPECT_EQ(timing_20khz->error_ppm, 0U);
}

TEST(pwm_solver, uses_real_counter_widths_and_legal_prescalers,
     "REQ-0004", "REQ-0022") {
    const auto gpt32 = mcubsp::backend::ra4m1::solve_pwm_timing({
        48'000'000U, 1U, 0U, 32U});
    const auto gpt16 = mcubsp::backend::ra4m1::solve_pwm_timing({
        48'000'000U, 1U, 0U, 16U});
    ASSERT_TRUE(gpt32.has_value());
    ASSERT_TRUE(gpt16.has_value());
    EXPECT_EQ(gpt32->prescaler, 1U);
    EXPECT_EQ(gpt32->period_counts, 48'000'000U);
    EXPECT_EQ(gpt16->prescaler, 1024U);
    EXPECT_EQ(gpt16->period_counts, 46'875U);
}

TEST(pwm_solver, rejects_zero_invalid_range_and_unattainable_tolerance,
     "REQ-0022") {
    EXPECT_TRUE(mcubsp::backend::ra4m1::solve_pwm_timing(
                    {48'000'000U, 0U, 10U, 32U})
                    .error()
                == mcubsp::driver::pwm_error::invalid_frequency);
    EXPECT_TRUE(mcubsp::backend::ra4m1::solve_pwm_timing(
                    {0U, 20'000U, 10U, 32U})
                    .error()
                == mcubsp::driver::pwm_error::invalid_clock);
    EXPECT_TRUE(mcubsp::backend::ra4m1::solve_pwm_timing(
                    {48'000'000U, 20'000U, 10U, 24U})
                    .error()
                == mcubsp::driver::pwm_error::invalid_counter_width);
    EXPECT_TRUE(mcubsp::backend::ra4m1::solve_pwm_timing(
                    {48'000'000U, 7'000'000U, 0U, 32U})
                    .error()
                == mcubsp::driver::pwm_error::unattainable_tolerance);
    EXPECT_TRUE(mcubsp::backend::ra4m1::solve_pwm_timing(
                    {100'000'000U, 1U, 1'000'000U, 16U})
                    .error()
                == mcubsp::driver::pwm_error::unattainable_tolerance);
}

TEST(pwm_solver, rounds_interior_duty_and_models_endpoints_explicitly,
     "REQ-0022") {
    const auto zero = mcubsp::driver::solve_pwm_duty(
        2400U, mcubsp::driver::pwm_duty::percent(0U));
    const auto full = mcubsp::driver::solve_pwm_duty(
        2400U, mcubsp::driver::pwm_duty::percent(100U));
    const auto third = mcubsp::driver::solve_pwm_duty(
        10U, {1U, 3U});
    ASSERT_TRUE(zero.has_value());
    ASSERT_TRUE(full.has_value());
    ASSERT_TRUE(third.has_value());
    EXPECT_TRUE(zero->mode == mcubsp::driver::pwm_duty_mode::force_low);
    EXPECT_TRUE(full->mode == mcubsp::driver::pwm_duty_mode::force_high);
    EXPECT_TRUE(third->mode == mcubsp::driver::pwm_duty_mode::compare);
    EXPECT_EQ(third->compare_counts, 3U);
    EXPECT_TRUE(mcubsp::driver::solve_pwm_duty(2400U, {2U, 1U}).error()
                == mcubsp::driver::pwm_error::invalid_duty);
    EXPECT_TRUE(mcubsp::driver::solve_pwm_duty(2400U, {1U, 0U}).error()
                == mcubsp::driver::pwm_error::invalid_duty);
}

TEST(ra4m1_pwm, configures_real_gpt1b_pin_period_compare_and_stopped_low,
     "REQ-0004", "REQ-0022") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    pwm_type pwm{backend};

    ASSERT_TRUE(pwm.initialize(*timing_20khz,
                               mcubsp::driver::pwm_duty::percent(25U))
                    .has_value());
    const auto operations = mmio.operations();
    EXPECT_TRUE(has_write(operations, 0x4007'8164U, 2399U));
    EXPECT_TRUE(has_write(operations, 0x4007'8100U, 0x0000'A500U));
    EXPECT_TRUE(has_write(operations, 0x4007'8100U, 0x0000'A501U));
    EXPECT_TRUE(has_write(operations, 0x4007'8150U, 600U));
    EXPECT_TRUE(has_write(operations, 0x4007'8158U, 600U));
    EXPECT_TRUE(has_write(operations, 0x4007'8140U, 1U << 18U));
    EXPECT_TRUE(has_write(operations, 0x4007'8134U,
                          (0b1'10'01U << 16U) | (1U << 24U)));
    EXPECT_TRUE(has_write(operations, 0x4004'0850U,
                          (0x03U << 24U) | (1U << 16U)));
    ASSERT_TRUE(pwm.release().has_value());
}

TEST(ra4m1_pwm, runtime_updates_use_the_buffer_and_start_stop_are_explicit,
     "REQ-0022") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    pwm_type pwm{backend};
    ASSERT_TRUE(pwm.initialize(*timing_20khz,
                               mcubsp::driver::pwm_duty::percent(25U))
                    .has_value());
    ASSERT_TRUE(pwm.start().has_value());
    mmio.clear_operations();

    ASSERT_TRUE(pwm.update_duty(mcubsp::driver::pwm_duty::percent(75U))
                    .has_value());
    const auto update = mmio.operations();
    EXPECT_TRUE(has_write(update, 0x4007'8158U, 1800U));
    EXPECT_FALSE(has_write_to(update, 0x4007'8150U));

    mmio.clear_operations();
    ASSERT_TRUE(pwm.stop().has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'812CU, 0U));
    ASSERT_TRUE(pwm.release().has_value());
}

TEST(ra4m1_pwm, endpoint_updates_use_synchronized_gpt_duty_control,
     "REQ-0022") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    pwm_type pwm{backend};
    ASSERT_TRUE(pwm.initialize(*timing_20khz,
                               mcubsp::driver::pwm_duty::percent(50U))
                    .has_value());
    ASSERT_TRUE(pwm.start().has_value());
    mmio.clear_operations();

    ASSERT_TRUE(pwm.update_duty(mcubsp::driver::pwm_duty::percent(0U))
                    .has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'8130U,
                          1U | (0b10U << 24U)));
    mmio.clear_operations();
    ASSERT_TRUE(pwm.update_duty(mcubsp::driver::pwm_duty::percent(100U))
                    .has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'8130U,
                          1U | (0b11U << 24U)));
    ASSERT_TRUE(pwm.stop().has_value());
    ASSERT_TRUE(pwm.release().has_value());
}

TEST(ra4m1_pwm, overflow_notification_uses_the_fixed_transfer_state_machine,
     "REQ-0014", "REQ-0022") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    pwm_type pwm{backend};
    ASSERT_TRUE(pwm.initialize(*timing_20khz,
                               mcubsp::driver::pwm_duty::percent(50U))
                    .has_value());
    ASSERT_TRUE(pwm.start().has_value());
    auto started = pwm.next_period();
    ASSERT_TRUE(started.has_value());
    auto period = std::move(*started);
    EXPECT_FALSE(period.ready());
    ASSERT_TRUE(mmio.write<std::uint32_t>(0x4007'813CU, 1U << 6U)
                    .has_value());
    EXPECT_TRUE(backend.handle_overflow_interrupt());
    EXPECT_TRUE(period.ready());
    EXPECT_TRUE(period.take().has_value());
    EXPECT_EQ(execution.notification_count(), 1U);
    ASSERT_TRUE(pwm.stop().has_value());
    ASSERT_TRUE(pwm.release().has_value());
}

TEST(ra4m1_pwm, one_owner_per_gpt_instance_is_enforced, "REQ-0014") {
    mmio_type first_mmio{0x4001'E000U};
    mmio_type second_mmio{0x4001'E000U};
    execution_type first_execution;
    execution_type second_execution;
    backend_type first_backend{first_mmio, first_execution};
    backend_type second_backend{second_mmio, second_execution};
    pwm_type first{first_backend};
    pwm_type second{second_backend};

    ASSERT_TRUE(first.initialize(*timing_20khz,
                                 mcubsp::driver::pwm_duty::percent(50U))
                    .has_value());
    const auto collision = second.initialize(
        *timing_20khz, mcubsp::driver::pwm_duty::percent(50U));
    ASSERT_FALSE(collision.has_value());
    EXPECT_TRUE(collision.error() == mcubsp::driver::pwm_error::already_owned);
    ASSERT_TRUE(first.release().has_value());
    ASSERT_TRUE(second.initialize(*timing_20khz,
                                  mcubsp::driver::pwm_duty::percent(50U))
                    .has_value());
    ASSERT_TRUE(second.release().has_value());
}

} // namespace
