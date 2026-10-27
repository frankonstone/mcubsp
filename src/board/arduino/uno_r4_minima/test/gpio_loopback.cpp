#include <mcutest/mcutest.hpp>

#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/driver/gpio.hpp>

#include "support.hpp"

namespace {

mcubsp::driver::gpio_pin output{
    mcubsp::board_test::gpio, mcubsp::board::arduino::uno_r4_minima_bindings::d2};
mcubsp::driver::gpio_pin input{
    mcubsp::board_test::gpio, mcubsp::board::arduino::uno_r4_minima_bindings::d3};
bool initialized{};
bool observed_low{};
bool observed_high{};

TEST(uno_gpio, d2_to_d3_fixture_loopback, "REQ-0021", "REQ-0036") {
    ASSERT_TRUE(initialized);
    ASSERT_TRUE(output.clear().has_value());
    auto level = input.read();
    ASSERT_TRUE(level.has_value());
    observed_low = *level == mcubsp::driver::gpio_level::low;
    EXPECT_TRUE(observed_low);
    ASSERT_TRUE(output.set().has_value());
    level = input.read();
    ASSERT_TRUE(level.has_value());
    observed_high = *level == mcubsp::driver::gpio_level::high;
    EXPECT_TRUE(observed_high);
    ASSERT_TRUE(output.clear().has_value());
}

constexpr char metadata[] =
    "MCUBSP_META board=Arduino_UNO_R4_Minima suite=gpio_loopback "
    "firmware=pln-0005 fixture=D2_GPIO_OUT_to_D3_GPIO_IN "
    "toolchain=arm-none-eabi-gcc-15.2.1 iclk_hz=48000000 "
    "pclka_hz=48000000 pclkb_hz=24000000 pclkd_hz=48000000 "
    "interface=J-Link_SWD_RTT\n";
constexpr mcubsp::board_test::evidence_metadata evidence{
    .suite = "gpio_loopback",
    .firmware = "pln-0005",
    .fixture = "D2 GPIO output jumpered to D3 GPIO input",
    .coverage_build_id = 0x4d485701U,
};

} // namespace

int main() {
    mcubsp::board_test::start(metadata);
    const mcubsp::driver::gpio_configuration output_configuration{
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::low,
    };
    const mcubsp::driver::gpio_configuration input_configuration{
        .direction = mcubsp::driver::gpio_direction::input,
        .pull = mcubsp::driver::gpio_pull::none,
    };
    initialized = output.initialize(output_configuration).has_value()
        && input.initialize(input_configuration).has_value();
    mcubsp::board_test::finish(evidence, [&](auto& observations) {
        observations("initialized", initialized);
        observations("output_pin", "D2 / P301");
        observations("input_pin", "D3 / P104");
        observations("low_level_observed", observed_low);
        observations("high_level_observed", observed_high);
    });
}
