#include <cstdint>

#include <mcutest/mcutest.hpp>

#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/gpio.hpp>

#include "support.hpp"

extern "C" std::uint32_t mcuhw_uno_clock_status;

namespace {

mcuhw::driver::gpio_pin led{
    mcuhw::board_test::gpio,
    mcuhw::board::arduino_uno_r4_minima::led_builtin_d13};
bool led_initialized{};

TEST(uno_startup, owns_vectors_and_48mhz_clock, "REQ-0031", "REQ-0036") {
    EXPECT_EQ(mcuhw_uno_clock_status, 0U);
    EXPECT_EQ(*reinterpret_cast<volatile const std::uint32_t*>(0xE000'ED08U),
              0x0000'4000U);
    EXPECT_EQ(*reinterpret_cast<volatile const std::uint8_t*>(0x4001'E026U),
              0U);
    EXPECT_EQ(*reinterpret_cast<volatile const std::uint32_t*>(0x4001'E020U),
              0x1001'0100U);
    EXPECT_EQ(*reinterpret_cast<volatile const std::uint8_t*>(0x4001'E031U),
              1U);
}

TEST(uno_gpio, initializes_builtin_led_low_before_output, "REQ-0021") {
    ASSERT_TRUE(led_initialized);
    const auto port =
        *reinterpret_cast<volatile const std::uint32_t*>(0x4004'0020U);
    EXPECT_EQ(port & (1U << 11U), 1U << 11U);
    EXPECT_EQ(port & (1U << (16U + 11U)), 0U);
}

TEST(uno_gpio, builtin_led_set_read_and_clear, "REQ-0021") {
    ASSERT_TRUE(led_initialized);
    ASSERT_TRUE(led.set().has_value());
    const auto high = led.read();
    ASSERT_TRUE(high.has_value());
    EXPECT_TRUE(*high == mcuhw::driver::gpio_level::high);
    ASSERT_TRUE(led.clear().has_value());
    const auto low = led.read();
    ASSERT_TRUE(low.has_value());
    EXPECT_TRUE(*low == mcuhw::driver::gpio_level::low);
}

TEST(uno_gpio, builtin_led_access_preserves_adjacent_pins, "REQ-0021") {
    ASSERT_TRUE(led_initialized);
    constexpr std::uint32_t owned_bits =
        (1U << 11U) | (1U << (16U + 11U));
    const auto before =
        *reinterpret_cast<volatile const std::uint32_t*>(0x4004'0020U);
    ASSERT_TRUE(led.set().has_value());
    ASSERT_TRUE(led.clear().has_value());
    const auto after =
        *reinterpret_cast<volatile const std::uint32_t*>(0x4004'0020U);
    EXPECT_EQ(before & ~owned_bits, after & ~owned_bits);
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=startup_gpio "
    "firmware=pln-0005 toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 pclkb_hz=24000000 "
    "pclkd_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "startup_gpio",
    .firmware = "pln-0005",
    .fixture = "built-in LED D13",
    .coverage_build_id = 0x4d485701U,
};

} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    const mcuhw::driver::gpio_configuration configuration{
        .direction = mcuhw::driver::gpio_direction::output,
        .initial_level = mcuhw::driver::gpio_level::low,
        .output_type = mcuhw::driver::gpio_output_type::push_pull,
        .pull = mcuhw::driver::gpio_pull::none,
        .drive = mcuhw::driver::gpio_drive::middle,
    };
    led_initialized = led.initialize(configuration).has_value();
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("clock_status", mcuhw_uno_clock_status);
        observations("vector_table_address",
                     *reinterpret_cast<volatile const std::uint32_t*>(
                         0xE000'ED08U));
        observations("led_initialized", led_initialized);
        observations("led_port", "P111 / D13 / active-high");
        observations("initial_level", "low");
    });
}
