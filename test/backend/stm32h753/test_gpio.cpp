#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/stm32h753/gpio.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {

constexpr std::uintptr_t peripheral_base = 0x5802'0000U;
constexpr std::uintptr_t gpioe_base = 0x5802'1000U;
constexpr std::uintptr_t ahb4enr = 0x5802'44E0U;
using policy_type = mcubsp::testing::recording_mmio<0x4500, 48>;
using backend_type = mcubsp::backend::stm32h753::gpio_backend<policy_type>;
using pin_type = mcubsp::driver::gpio_pin<backend_type>;

TEST(stm32h753_gpio, enables_port_and_latches_level_before_output,
     "REQ-0009", "REQ-0021", "REQ-0033") {
    policy_type policy{peripheral_base};
    backend_type backend{policy};
    pin_type led{backend, {4, 1}};
    const mcubsp::driver::gpio_configuration configuration{
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::high,
        .output_type = mcubsp::driver::gpio_output_type::push_pull,
        .pull = mcubsp::driver::gpio_pull::none,
        .drive = mcubsp::driver::gpio_drive::middle,
    };

    ASSERT_TRUE(led.initialize(configuration).has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 11U);
    EXPECT_EQ(operations[0].address, ahb4enr);
    EXPECT_EQ(operations[1].address, ahb4enr);
    EXPECT_EQ(operations[1].value, 1U << 4U);
    EXPECT_EQ(operations[2].address, gpioe_base + 0x18U);
    EXPECT_EQ(operations[2].value, 1U << 1U);
    EXPECT_EQ(operations[10].address, gpioe_base + 0x00U);
    EXPECT_EQ(operations[10].value, 1U << 2U);
}

TEST(stm32h753_gpio, set_and_clear_use_atomic_bsrr_writes,
     "REQ-0021", "REQ-0033") {
    policy_type policy{peripheral_base};
    backend_type backend{policy};
    pin_type led{backend, {4, 1}};
    ASSERT_TRUE(led.initialize({
        .direction = mcubsp::driver::gpio_direction::output,
    }).has_value());
    policy.clear_operations();

    ASSERT_TRUE(led.set().has_value());
    ASSERT_TRUE(led.clear().has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 2U);
    EXPECT_EQ(operations[0].address, gpioe_base + 0x18U);
    EXPECT_EQ(operations[0].value, 1U << 1U);
    EXPECT_EQ(operations[1].value, 1U << 17U);
}

TEST(stm32h753_gpio, rejects_a_pin_absent_from_lqfp144,
     "REQ-0010", "REQ-0021", "REQ-0033") {
    policy_type policy{peripheral_base};
    backend_type backend{policy};
    pin_type unavailable{backend, {10, 0}};

    const auto result = unavailable.initialize({});
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error() == mcubsp::driver::gpio_error::invalid_pin);
    EXPECT_EQ(policy.operations().size(), 0U);
}

} // namespace
