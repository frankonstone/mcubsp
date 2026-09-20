#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/esp32p4/gpio.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

constexpr std::uintptr_t gpio_base = 0x500E'0000U;
constexpr std::uintptr_t io_mux_base = 0x500E'1000U;
using policy_type = mcuhw::testing::recording_mmio<0x1200, 48>;
using backend_type = mcuhw::backend::esp32p4::gpio_backend<policy_type>;
using pin_type = mcuhw::driver::gpio_pin<backend_type>;

TEST(esp32p4_gpio, latches_then_configures_then_enables_lower_bank_output,
     "REQ-0009", "REQ-0034") {
    policy_type policy{gpio_base};
    backend_type backend{policy};
    pin_type pin{backend, {0, 7}};

    ASSERT_TRUE(pin.initialize({
        .direction = mcuhw::driver::gpio_direction::output,
        .initial_level = mcuhw::driver::gpio_level::high,
        .output_type = mcuhw::driver::gpio_output_type::open_drain,
        .pull = mcuhw::driver::gpio_pull::up,
        .drive = mcuhw::driver::gpio_drive::middle,
    }).has_value());

    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 8U);
    EXPECT_EQ(operations[0].address, gpio_base + 0x08U);
    EXPECT_EQ(operations[0].value, 1U << 7U);
    EXPECT_EQ(operations[1].address, io_mux_base + 0x20U);
    EXPECT_EQ(operations[2].address, gpio_base + 0x90U);
    EXPECT_EQ(operations[3].address, gpio_base + 0x574U);
    EXPECT_EQ(operations[4].address, gpio_base + 0x90U);
    EXPECT_EQ(operations[4].value, 1U << 2U);
    EXPECT_EQ(operations[5].value, 256U | (1U << 10U));
    EXPECT_EQ(operations[6].value,
              (1U << 8U) | (0x2U << 10U) | (1U << 12U));
    EXPECT_EQ(operations[7].address, gpio_base + 0x24U);
    EXPECT_EQ(operations[7].value, 1U << 7U);
}

TEST(esp32p4_gpio, upper_bank_uses_atomic_aliases_and_input_register,
     "REQ-0009", "REQ-0034") {
    policy_type policy{gpio_base};
    backend_type backend{policy};
    pin_type pin{backend, {0, 45}};
    ASSERT_TRUE(pin.initialize({
        .direction = mcuhw::driver::gpio_direction::output,
    }).has_value());
    policy.clear_operations();

    ASSERT_TRUE(pin.set().has_value());
    ASSERT_TRUE(pin.clear().has_value());
    const auto level = pin.read();
    ASSERT_TRUE(level.has_value());
    EXPECT_TRUE(*level == mcuhw::driver::gpio_level::low);

    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 3U);
    EXPECT_EQ(operations[0].address, gpio_base + 0x14U);
    EXPECT_EQ(operations[0].value, 1U << 13U);
    EXPECT_EQ(operations[1].address, gpio_base + 0x18U);
    EXPECT_EQ(operations[1].value, 1U << 13U);
    EXPECT_EQ(operations[2].address, gpio_base + 0x40U);
}

TEST(esp32p4_gpio, rejects_invalid_and_duplicate_claims,
     "REQ-0004", "REQ-0014", "REQ-0034") {
    policy_type policy{gpio_base};
    backend_type backend{policy};
    pin_type first{backend, {0, 8}};
    pin_type duplicate{backend, {0, 8}};
    pin_type invalid{backend, {1, 8}};

    ASSERT_TRUE(first.initialize({}).has_value());
    const auto duplicate_result = duplicate.initialize({});
    ASSERT_FALSE(duplicate_result.has_value());
    EXPECT_TRUE(duplicate_result.error()
                == mcuhw::driver::gpio_error::already_owned);
    const auto invalid_result = invalid.initialize({});
    ASSERT_FALSE(invalid_result.has_value());
    EXPECT_TRUE(invalid_result.error()
                == mcuhw::driver::gpio_error::invalid_pin);
}

TEST(esp32p4_gpio, leaves_native_usb_phy_pins_to_usb_integration,
     "REQ-0004", "REQ-0034") {
    policy_type policy{gpio_base};
    backend_type backend{policy};
    pin_type usb_dm{backend, {0, 24}};

    const auto result = usb_dm.initialize({
        .direction = mcuhw::driver::gpio_direction::output,
    });
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error()
                == mcuhw::driver::gpio_error::unsupported_configuration);
    EXPECT_EQ(policy.operations().size(), 0U);
}

} // namespace
