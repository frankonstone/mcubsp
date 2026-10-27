#include <chrono>
#include <mcutest/mcutest.hpp>
#include <mcubsp/backend/nrf52840/gpio.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/io/board.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {
using namespace mcubsp;
using mmio_type = testing::recording_mmio<0xB00, 32>;
using backend_type = backend::nrf52840::gpio_backend<mmio_type>;
using profile = board::others::promicro_nrf52840;
static_assert(mcu::nrf52840::gpio_registers::pin_cnf<31>::offset == 0x77CU);
static_assert(mcu::nrf52840::gpio_base({1, 15}) == 0x5000'0300U);
static_assert(profile::application_begin + profile::application_bytes == profile::storage_begin);
static_assert(profile::led_builtin == core::pin_id{0, 15});

TEST(nrf52840_gpio, sets_latch_before_direction_and_encodes_electrical_modes, "REQ-0053") {
    mmio_type mmio{0x5000'0000U};
    backend_type backend{mmio};
    driver::gpio_pin pin{backend, {0, 15}};
    ASSERT_TRUE(pin.initialize({.direction = driver::gpio_direction::output,
        .initial_level = driver::gpio_level::high,
        .output_type = driver::gpio_output_type::open_drain,
        .pull = driver::gpio_pull::up, .drive = driver::gpio_drive::middle}).has_value());
    const auto trace = mmio.operations();
    ASSERT_EQ(trace.size(), 2U);
    EXPECT_EQ(trace[0].address, 0x5000'0508U);
    EXPECT_EQ(trace[0].value, 1U << 15U);
    EXPECT_EQ(trace[1].address, 0x5000'073CU);
    EXPECT_EQ(trace[1].value, 0x70DU); // H0D1, pull-up, connected input, output.
    EXPECT_EQ(trace[0].width, 4U);
    EXPECT_EQ(trace[1].width, 4U);
}

TEST(nrf52840_gpio, uses_port_one_atomic_aliases_and_reads_input, "REQ-0053") {
    mmio_type mmio{0x5000'0000U};
    backend_type backend{mmio};
    driver::gpio_pin pin{backend, {1, 15}};
    ASSERT_TRUE(pin.initialize({.direction = driver::gpio_direction::output}).has_value());
    ASSERT_TRUE(mmio.write<std::uint32_t>(0x5000'0810U, 1U << 15U).has_value());
    mmio.clear_operations();
    ASSERT_TRUE(pin.set().has_value());
    ASSERT_TRUE(pin.clear().has_value());
    const auto read = pin.read();
    ASSERT_TRUE(read.has_value());
    EXPECT_TRUE(*read == driver::gpio_level::high);
    const auto trace = mmio.operations();
    ASSERT_EQ(trace.size(), 3U);
    EXPECT_EQ(trace[0].address, 0x5000'0808U);
    EXPECT_EQ(trace[1].address, 0x5000'080CU);
    EXPECT_EQ(trace[2].address, 0x5000'0810U);
    EXPECT_EQ(trace[0].value, 1U << 15U);
}

TEST(nrf52840_gpio, rejects_unavailable_reserved_and_duplicate_pins_without_mmio, "REQ-0053") {
    mmio_type mmio{0x5000'0000U};
    backend_type backend{mmio};
    EXPECT_FALSE(backend.claim({1, 16}, driver::gpio_direction::output).has_value());
    EXPECT_FALSE(backend.claim({0, 32}, driver::gpio_direction::output).has_value());
    for (auto index : {0U, 1U, 9U, 10U, 18U})
        EXPECT_FALSE(backend.claim({0, static_cast<std::uint16_t>(index)}, driver::gpio_direction::output).has_value());
    ASSERT_TRUE(backend.claim({0, 15}, driver::gpio_direction::output).has_value());
    const auto duplicate = backend.claim({0, 15}, driver::gpio_direction::output);
    ASSERT_FALSE(duplicate.has_value());
    EXPECT_TRUE(duplicate.error() == driver::gpio_error::already_owned);
    EXPECT_EQ(mmio.operations().size(), 0U);
}

TEST(nrf52840_gpio, input_configuration_and_release_preserve_ownership_rules, "REQ-0053") {
    mmio_type mmio{0x5000'0000U};
    backend_type backend{mmio};
    driver::gpio_pin input{backend, {0, 8}};
    ASSERT_TRUE(input.initialize({.pull = driver::gpio_pull::down}).has_value());
    EXPECT_EQ(mmio.operations()[0].value, 4U);
    EXPECT_FALSE(input.set().has_value());
    ASSERT_TRUE(input.release().has_value());
    EXPECT_EQ(mmio.operations().back().value, 0U);
    EXPECT_FALSE(backend.set({0, 8}).has_value());
    ASSERT_TRUE(input.initialize({.pull = driver::gpio_pull::up}).has_value());
    EXPECT_EQ(mmio.operations().back().value, 12U);
}

TEST(nrf52840_gpio, configuration_failure_rolls_back_claim_and_does_not_enable_output, "REQ-0053") {
    // OUTSET fits, PIN_CNF does not: the functional driver must release its claim.
    testing::recording_mmio<0x510, 8> mmio{0x5000'0000U};
    backend::nrf52840::gpio_backend backend{mmio};
    driver::gpio_pin pin{backend, {0, 15}};
    const auto configured = pin.initialize({.direction = driver::gpio_direction::output});
    ASSERT_FALSE(configured.has_value());
    EXPECT_TRUE(configured.error() == driver::gpio_error::mmio);
    EXPECT_FALSE(pin.initialized());
    ASSERT_TRUE(backend.claim({0, 15}, driver::gpio_direction::output).has_value());
    EXPECT_EQ(mmio.operations().size(), 1U);
}

TEST(nrf52840_gpio, propagates_latch_and_input_access_failure, "REQ-0053") {
    testing::recording_mmio<4, 8> mmio{0x5000'0000U};
    backend::nrf52840::gpio_backend backend{mmio};
    ASSERT_TRUE(backend.claim({0, 15}, driver::gpio_direction::output).has_value());
    const auto configured = backend.configure({0, 15}, {.direction = driver::gpio_direction::output});
    ASSERT_FALSE(configured.has_value());
    EXPECT_TRUE(configured.error() == driver::gpio_error::mmio);
    EXPECT_FALSE(backend.read({0, 15}).has_value());
    EXPECT_EQ(mmio.operations().size(), 0U);
}

TEST(nrf52840_board, facade_claims_aliases_releases_to_input_and_uses_borrowed_clock, "REQ-0053") {
    mmio_type mmio{0x5000'0000U};
    execution::fake_policy policy;
    auto hw = io::make_board<profile, {.serial = false}>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin().has_value());
    auto led = hw.output(hw.pins.led_builtin);
    ASSERT_TRUE(led.has_value());
    EXPECT_FALSE(hw.output(hw.pins.p0_15).has_value());
    EXPECT_FALSE(hw.output({0, 18}).has_value());
    EXPECT_FALSE(hw.shutdown().has_value());
    ASSERT_TRUE(hw.delay(std::chrono::milliseconds{5}).has_value());
    ASSERT_TRUE(led->release().has_value());
    EXPECT_EQ(mmio.operations().back().value, 0U);
    ASSERT_TRUE(hw.shutdown().has_value());
}
} // namespace
