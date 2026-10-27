#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/gpio.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {

constexpr std::uintptr_t io_base = 0x4004'0000U;
using policy_type = mcubsp::testing::recording_mmio<0xD08, 48>;
using backend_type = mcubsp::backend::ra4m1::gpio_backend<policy_type>;
using pin_type = mcubsp::driver::gpio_pin<backend_type>;

TEST(ra4m1_gpio, initializes_level_before_output_and_relocks_pfs,
     "REQ-0009", "REQ-0021") {
    policy_type policy{io_base};
    backend_type backend{policy};
    pin_type led{backend, {1, 11}};
    mcubsp::driver::gpio_configuration configuration{
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::high,
        .output_type = mcubsp::driver::gpio_output_type::push_pull,
        .pull = mcubsp::driver::gpio_pull::none,
        .drive = mcubsp::driver::gpio_drive::middle,
    };

    ASSERT_TRUE(led.initialize(configuration).has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 7U);
    EXPECT_TRUE(operations[0].kind
                == mcubsp::testing::mmio_operation_kind::read);
    EXPECT_EQ(operations[0].address, 0x4004'086CU);
    EXPECT_EQ(operations[1].address, 0x4004'0028U);
    EXPECT_EQ(operations[1].value, 1U << 11U);
    EXPECT_EQ(operations[2].address, 0x4004'0D03U);
    EXPECT_EQ(operations[2].value, 0U);
    EXPECT_EQ(operations[3].value, 0x40U);
    EXPECT_EQ(operations[4].address, 0x4004'086CU);
    EXPECT_EQ(operations[4].value, (1U << 10U) | (1U << 2U) | 1U);
    EXPECT_EQ(operations[5].value, 0U);
    EXPECT_EQ(operations[6].value, 0x80U);
}

TEST(ra4m1_gpio, set_and_clear_use_single_pin_atomic_alias_writes,
     "REQ-0021") {
    policy_type policy{io_base};
    backend_type backend{policy};
    pin_type led{backend, {1, 11}};
    const mcubsp::driver::gpio_configuration configuration{
        .direction = mcubsp::driver::gpio_direction::output,
    };
    ASSERT_TRUE(led.initialize(configuration).has_value());
    policy.clear_operations();

    ASSERT_TRUE(led.set().has_value());
    ASSERT_TRUE(led.clear().has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 2U);
    EXPECT_EQ(operations[0].address, 0x4004'0028U);
    EXPECT_EQ(operations[0].value, 1U << 11U);
    EXPECT_EQ(operations[1].address, 0x4004'0028U);
    EXPECT_EQ(operations[1].value, 1U << (16U + 11U));
    EXPECT_EQ(operations[0].value & ~(1U << 11U), 0U);
    EXPECT_EQ(operations[1].value & ~(1U << (16U + 11U)), 0U);
}

TEST(ra4m1_gpio, read_uses_the_pin_input_data_without_modifying_the_port,
     "REQ-0021") {
    policy_type policy{io_base};
    backend_type backend{policy};
    pin_type input{backend, {1, 10}};
    ASSERT_TRUE(input.initialize({}).has_value());
    ASSERT_TRUE(policy.write<std::uint32_t>(0x4004'0024U, 1U << 10U)
                    .has_value());
    policy.clear_operations();

    const auto level = input.read();
    ASSERT_TRUE(level.has_value());
    EXPECT_TRUE(*level == mcubsp::driver::gpio_level::high);
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 1U);
    EXPECT_TRUE(operations[0].kind
                == mcubsp::testing::mmio_operation_kind::read);
    EXPECT_EQ(operations[0].address, 0x4004'0024U);
}

TEST(ra4m1_gpio, ownership_and_double_initialization_are_deterministic,
     "REQ-0014") {
    policy_type policy{io_base};
    backend_type backend{policy};
    pin_type first{backend, {1, 11}};
    pin_type second{backend, {1, 11}};

    ASSERT_TRUE(first.initialize({}).has_value());
    const auto repeated = first.initialize({});
    ASSERT_FALSE(repeated.has_value());
    EXPECT_TRUE(repeated.error()
                == mcubsp::driver::gpio_error::already_initialized);
    const auto collision = second.initialize({});
    ASSERT_FALSE(collision.has_value());
    EXPECT_TRUE(collision.error() == mcubsp::driver::gpio_error::already_owned);

    ASSERT_TRUE(first.release().has_value());
    EXPECT_TRUE(second.initialize({}).has_value());
}

TEST(ra4m1_gpio, rejects_output_on_ra4m1_input_only_pins, "REQ-0021") {
    policy_type policy{io_base};
    backend_type backend{policy};
    pin_type p200{backend, {2, 0}};
    const mcubsp::driver::gpio_configuration output{
        .direction = mcubsp::driver::gpio_direction::output,
    };

    const auto result = p200.initialize(output);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error() == mcubsp::driver::gpio_error::invalid_pin);
    EXPECT_EQ(policy.operations().size(), 0U);
}

TEST(ra4m1_gpio, uses_p408_unique_drive_strength_bit, "REQ-0021") {
    policy_type policy{io_base};
    backend_type backend{policy};
    pin_type p408{backend, {4, 8}};
    mcubsp::driver::gpio_configuration configuration{
        .direction = mcubsp::driver::gpio_direction::output,
        .drive = mcubsp::driver::gpio_drive::middle,
    };

    ASSERT_TRUE(p408.initialize(configuration).has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 7U);
    EXPECT_EQ(operations[4].value, (1U << 11U) | (1U << 2U));
    EXPECT_EQ(operations[4].value & (1U << 10U), 0U);
}

} // namespace
