#include <cstddef>
#include <cstdint>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/gpt.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/driver/pwm.hpp>

#include "support.hpp"

namespace {

using binding = mcuhw::backend::ra4m1::gpt_binding<
    1, mcuhw::mcu::ra4m1::gpt_output::b,
    mcuhw::board::arduino_uno_r4_minima::d3>;
using backend_type = mcuhw::backend::ra4m1::gpt_pwm_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};
mcuhw::driver::pwm_channel pwm{backend};
mcuhw::driver::gpio_pin observer{
    mcuhw::board_test::gpio, mcuhw::board::arduino_uno_r4_minima::d2};

constexpr auto timing = mcuhw::backend::ra4m1::solve_pwm_timing({
    .clock_hz = 48'000'000U,
    .target_hz = 20'000U,
    .maximum_error_ppm = 0U,
    .counter_width = binding::metadata.counter_width,
});
static_assert(timing.has_value());
static_assert(timing->period_counts == 2400U);

struct waveform_capture {
    std::uint32_t period_cycles;
    std::uint32_t high_cycles;
};

bool initialized{};
waveform_capture duty_25_observation{};
waveform_capture duty_75_observation{};

[[nodiscard]] bool MCUCOV_NO_INSTRUMENT observed_level() noexcept {
    return (*reinterpret_cast<volatile const std::uint32_t*>(0x4004'0024U)
            & (1U << 5U)) != 0U;
}

[[nodiscard]] bool MCUCOV_NO_INSTRUMENT wait_for_level(
    bool level, std::uint32_t start, std::uint32_t budget,
    std::uint32_t& timestamp) noexcept {
    while (static_cast<std::uint32_t>(
               mcuhw::board_test::execution_hooks::now() - start)
           < budget) {
        if (observed_level() == level) {
            timestamp = mcuhw::board_test::execution_hooks::now();
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool MCUCOV_NO_INSTRUMENT capture_waveform(
    waveform_capture& capture) noexcept {
    constexpr std::uint32_t budget = 480'000U;
    const auto start = mcuhw::board_test::execution_hooks::now();
    std::uint32_t ignored{};
    std::uint32_t rising{};
    std::uint32_t falling{};
    std::uint32_t next_rising{};
    if (!wait_for_level(false, start, budget, ignored)
        || !wait_for_level(true, start, budget, rising)
        || !wait_for_level(false, rising, budget, falling)
        || !wait_for_level(true, falling, budget, next_rising)) {
        return false;
    }
    capture.period_cycles = next_rising - rising;
    capture.high_cycles = falling - rising;
    return true;
}

void write_decimal(std::uint32_t value) noexcept {
    char digits[10];
    std::uint32_t count{};
    do {
        digits[count++] = static_cast<char>('0' + value % 10U);
        value /= 10U;
    } while (value != 0U);
    while (count != 0U) {
        --count;
        mcuhw::testing::rtt::write(&digits[count], 1U);
    }
}

template<std::size_t Length>
void write_register(const char (&label)[Length],
                    std::uintptr_t address) noexcept {
    mcuhw::testing::rtt::write(label, Length - 1U);
    write_decimal(*reinterpret_cast<volatile const std::uint32_t*>(address));
    constexpr char newline[] = "\n";
    mcuhw::testing::rtt::write(newline, 1U);
}

template<std::size_t Length>
void write_capture(const char (&label)[Length],
                   const waveform_capture& capture) noexcept {
    mcuhw::testing::rtt::write(label, Length - 1U);
    constexpr char period[] = " period_cycles=";
    mcuhw::testing::rtt::write(period, sizeof(period) - 1U);
    write_decimal(capture.period_cycles);
    constexpr char high[] = " high_cycles=";
    mcuhw::testing::rtt::write(high, sizeof(high) - 1U);
    write_decimal(capture.high_cycles);
    constexpr char newline[] = "\n";
    mcuhw::testing::rtt::write(newline, 1U);
}

TEST(uno_pwm, d3_to_d2_measures_start_buffered_update_and_stop,
     "REQ-0022", "REQ-0036") {
    ASSERT_TRUE(initialized);
    ASSERT_TRUE(pwm.start().has_value());
    write_register("MCUHW_PWM gtcr=", 0x4007'812CU);
    write_register("MCUHW_PWM gtcnt=", 0x4007'8148U);
    write_register("MCUHW_PWM gtpr=", 0x4007'8164U);
    write_register("MCUHW_PWM gtior=", 0x4007'8134U);
    write_register("MCUHW_PWM p104pfs=", 0x4004'0850U);
    write_register("MCUHW_PWM mstpcrd=", 0x4004'7008U);

    ASSERT_TRUE(capture_waveform(duty_25_observation));
    write_capture("MCUHW_PWM duty=25", duty_25_observation);
    EXPECT_TRUE(duty_25_observation.period_cycles >= 2280U
                && duty_25_observation.period_cycles <= 2520U);
    EXPECT_TRUE(duty_25_observation.high_cycles >= 480U
                && duty_25_observation.high_cycles <= 720U);

    ASSERT_TRUE(pwm.update_duty(mcuhw::driver::pwm_duty::percent(75U))
                    .has_value());
    waveform_capture discarded{};
    ASSERT_TRUE(capture_waveform(discarded));
    ASSERT_TRUE(capture_waveform(duty_75_observation));
    write_capture("MCUHW_PWM duty=75", duty_75_observation);
    EXPECT_TRUE(duty_75_observation.period_cycles >= 2280U
                && duty_75_observation.period_cycles <= 2520U);
    EXPECT_TRUE(duty_75_observation.high_cycles >= 1680U
                && duty_75_observation.high_cycles <= 1920U);

    ASSERT_TRUE(pwm.stop().has_value());
    EXPECT_FALSE(observed_level());
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=pwm "
    "firmware=pln-0007 fixture=D3_GPT1B_to_D2_GPIO target_hz=20000 "
    "duty_percent=25_to_75 tolerance_percent=5 "
    "toolchain=arm-none-eabi-gcc-15.2.1 iclk_hz=48000000 "
    "pclka_hz=48000000 pclkb_hz=24000000 pclkd_hz=48000000 "
    "interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "pwm",
    .firmware = "pln-0007",
    .fixture = "D3 GPT1B output jumpered to D2 GPIO input",
    .coverage_build_id = 0x4d485701U,
};

} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    const mcuhw::driver::gpio_configuration observer_configuration{
        .direction = mcuhw::driver::gpio_direction::input,
        .pull = mcuhw::driver::gpio_pull::none,
    };
    initialized = observer.initialize(observer_configuration).has_value()
        && pwm.initialize(*timing, mcuhw::driver::pwm_duty::percent(25U))
               .has_value();
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("initialized", initialized);
        observations("timer", "GPT1 channel B");
        observations("target_frequency_hz", 20'000U);
        observations("configured_period_counts", timing->period_counts);
        observations.object("duty_25_percent", [&](auto& capture) {
            capture("period_cycles", duty_25_observation.period_cycles);
            capture("high_cycles", duty_25_observation.high_cycles);
        });
        observations.object("duty_75_percent", [&](auto& capture) {
            capture("period_cycles", duty_75_observation.period_cycles);
            capture("high_cycles", duty_75_observation.high_cycles);
        });
        observations("stopped_level", observed_level() ? "high" : "low");
    });
}
