#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/adc.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/adc.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

namespace board = mcuhw::board::arduino_uno_r4_minima;
using binding = mcuhw::backend::ra4m1::adc14_binding<
    mcuhw::backend::ra4m1::adc_input<0, board::a1_an000>,
    mcuhw::backend::ra4m1::adc_input<1, board::a2_an001>>;
using mmio_type = mcuhw::testing::recording_mmio<0x60000, 384>;
using execution_type = mcuhw::execution::fake_policy;
using backend_type = mcuhw::backend::ra4m1::adc14_backend<
    binding, mmio_type, execution_type>;

[[nodiscard]] bool has_write(std::span<const mcuhw::testing::mmio_operation> ops,
                             std::uintptr_t address, std::uint64_t value) {
    for (const auto& operation : ops) {
        if (operation.kind == mcuhw::testing::mmio_operation_kind::write
            && operation.address == address && operation.value == value) {
            return true;
        }
    }
    return false;
}

TEST(adc_contract, validates_channels_and_sample_timing, "REQ-0029") {
    EXPECT_TRUE(mcuhw::driver::make_adc_channel(0U).has_value());
    EXPECT_TRUE(mcuhw::driver::make_adc_channel(14U).has_value());
    EXPECT_TRUE(mcuhw::driver::make_adc_channel(19U).has_value());
    EXPECT_TRUE(mcuhw::driver::make_adc_channel(20U).error()
                == mcuhw::driver::adc_error::invalid_channel);

    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    const auto rejected = backend.initialize({.sample_states = 5U});
    ASSERT_FALSE(rejected.has_value());
    EXPECT_TRUE(rejected.error()
                == mcuhw::driver::adc_error::invalid_sample_time);
}

TEST(ra4m1_adc, configures_resolution_sampling_and_analog_pins,
     "REQ-0029") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize({
        .resolution = mcuhw::driver::adc_resolution::bits14,
        .sample_states = 21U,
    }).has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C00EU, 0x0006U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C0E0U, 21U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C0E1U, 21U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0800U, 1U << 15U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0804U, 1U << 15U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_adc, finite_scan_returns_raw_results_in_requested_order,
     "REQ-0017", "REQ-0029") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    const std::array channels{
        mcuhw::driver::adc_channel{1U}, mcuhw::driver::adc_channel{0U}};
    std::array<std::uint16_t, 2> results{};
    auto started = backend.convert(channels, results);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C004U, 0x0003U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0x8000U));
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C000U, 0U).has_value());
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C020U, 0x0123U).has_value());
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C022U, 0x3ABCU).has_value());
    EXPECT_TRUE(backend.handle_scan_end_interrupt());
    const auto complete = transfer.take();
    ASSERT_TRUE(complete.has_value());
    EXPECT_EQ(complete->samples, 2U);
    EXPECT_EQ(results[0], 0x3ABCU);
    EXPECT_EQ(results[1], 0x0123U);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_adc, rejects_bad_sequences_and_timeout_quiesces_converter,
     "REQ-0019", "REQ-0029") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    const std::array duplicate{
        mcuhw::driver::adc_channel{0U}, mcuhw::driver::adc_channel{0U}};
    std::array<std::uint16_t, 2> values{};
    const auto rejected = backend.convert(duplicate, values);
    ASSERT_FALSE(rejected.has_value());
    EXPECT_TRUE(rejected.error().cause
                == mcuhw::driver::adc_error::duplicate_channel);

    const std::array channels{mcuhw::driver::adc_channel{0U}};
    auto started = backend.convert(channels,
                                   std::span<std::uint16_t>{values}.first(1));
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(mcuhw::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C004U, 0U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

} // namespace
