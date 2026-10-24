#include "support/source_backed_examples.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <mcutest/mcutest.hpp>

#include <mcubsp/core/metadata.hpp>
#include <mcubsp/core/units.hpp>

namespace {

using namespace mcubsp;

TEST(capabilities, exposes_absent_target_features_at_compile_and_run_time,
     "REQ-0004") {
    using output = contract_trials::ra4m1::gpt_output;
    using outputs = contract_trials::ra4m1::gpt320_outputs;
    static_assert(outputs::contains<output::a>);
    static_assert(outputs::contains<output::b>);
    static_assert(!outputs::contains<output::c>);
    using output_a = core::capability_request<outputs, output::a>;
    EXPECT_TRUE(sizeof(output_a) > 0);
    EXPECT_TRUE(outputs::require(output::a).has_value());
    const auto unsupported = outputs::require(output::c);
    EXPECT_TRUE(!unsupported.has_value());
    EXPECT_TRUE(unsupported.error() == core::metadata_error::unsupported);
}

TEST(peripheral_metadata, retains_multiple_relationships_and_protection,
     "REQ-0007", "REQ-0009") {
    static constexpr std::array<std::uint16_t, 2> clocks{1, 2};
    static constexpr std::array<std::uint16_t, 1> resets{3};
    static constexpr std::array<std::uint16_t, 3> events{4, 5, 6};
    static constexpr std::array<std::uint16_t, 2> requests{7, 8};
    static constexpr core::peripheral_metadata peripheral{
        {9, 0}, clocks, resets, events, requests,
        contract_trials::ra4m1::io_source};
    static constexpr core::protection_requirement protection{
        1, 0x503, true, contract_trials::ra4m1::protection_source};

    EXPECT_EQ(peripheral.clock_inputs.size(), 2U);
    EXPECT_EQ(peripheral.event_sources.size(), 3U);
    EXPECT_EQ(peripheral.dma_requests.size(), 2U);
    EXPECT_EQ(protection.control_register_offset, 0x503U);
    EXPECT_TRUE(protection.relock_required);
}

TEST(pin_metadata, distinguishes_package_availability_route_and_composition,
     "REQ-0010") {
    constexpr core::source_ref source{
        "contract trial", "source-backed shapes", "see doc/concept/register-model.md"};
    constexpr core::pin_id p0{0, 0};
    constexpr core::pin_id p1{0, 1};
    constexpr auto digital_modes = static_cast<std::uint16_t>(
        core::mode_mask(core::electrical_mode::input)
        | core::mode_mask(core::electrical_mode::push_pull));
    constexpr std::array package{
        core::package_pin{p0, 1, digital_modes, source},
        core::package_pin{p1, 2, core::mode_mask(core::electrical_mode::analog), source},
    };
    constexpr core::signal_id gpio_output{{1, 0}, 1};
    constexpr std::array routes{
        core::pin_route{p0, gpio_output, core::pin_route_kind::fixed, 0,
                        contract_trials::ra4m1::io_source},
        core::pin_route{p1, gpio_output,
                        core::pin_route_kind::alternate_function, 7,
                        contract_trials::stm32h753::gpio_source},
        core::pin_route{{0, 2}, gpio_output,
                        core::pin_route_kind::programmable_matrix, 1,
                        contract_trials::esp32p4::routing_source},
    };

    static_assert(core::pins_are_unique<p0, p1>());
    static_assert(!core::pins_are_unique<p0, p0>());
    using valid_composition = core::pin_composition<p0, p1>;
    using valid_binding = core::pin_binding<
        contract_trials::ra4m1::trial_pin_database,
        contract_trials::ra4m1::trial_pin,
        contract_trials::ra4m1::trial_gpio_output,
        core::electrical_mode::push_pull>;
    EXPECT_TRUE(sizeof(valid_composition) > 0);
    EXPECT_TRUE(sizeof(valid_binding) > 0);
    EXPECT_TRUE(core::package_contains(package, p0,
                                       core::electrical_mode::push_pull));
    EXPECT_TRUE(!core::package_contains(package, p1,
                                        core::electrical_mode::push_pull));
    EXPECT_TRUE(core::route_exists(routes, p0, gpio_output));
    EXPECT_TRUE(!core::route_exists(routes, core::pin_id{0, 3}, gpio_output));
}

TEST(clock_metadata, supports_multiple_sources_and_fractional_dividers,
     "REQ-0011") {
    constexpr core::clock_node peripheral{3, core::hertz{1'000'000},
                                         core::hertz{100'000'000},
                                         contract_trials::ra4m1::clock_source};
    constexpr core::clock_edge fixed_parent{
        1, 3, 1, 16, false, core::clock_route_kind::selectable, 0x01,
        contract_trials::stm32h753::clock_source};
    constexpr core::clock_edge fractional_parent{
        2, 3, 1, 256, true, core::clock_route_kind::selectable, 0x02,
        contract_trials::esp32p4::clock_source};

    const auto integer = core::derive_frequency(
        core::hertz{160'000'000}, fixed_parent, {2, 0, 1}, peripheral);
    ASSERT_TRUE(integer.has_value());
    EXPECT_EQ(integer->count, 80'000'000U);
    const auto fractional = core::derive_frequency(
        core::hertz{160'000'000}, fractional_parent, {2, 1, 2}, peripheral);
    ASSERT_TRUE(fractional.has_value());
    EXPECT_EQ(fractional->count, 64'000'000U);
    EXPECT_TRUE(!core::derive_frequency(
                     core::hertz{160'000'000}, fixed_parent, {2, 1, 2}, peripheral)
                     .has_value());
}

TEST(interrupt_metadata, preserves_fixed_selectable_and_matrix_routing,
     "REQ-0012") {
    constexpr std::array routes{
        core::interrupt_route{1, 5, core::interrupt_route_kind::selectable_line,
                              0x17, true,
                              contract_trials::ra4m1::interrupt_source},
        core::interrupt_route{2, 12, core::interrupt_route_kind::fixed_line,
                              0, false, contract_trials::stm32h753::exti_source},
        core::interrupt_route{3, 18, core::interrupt_route_kind::interrupt_matrix,
                              0x44, true,
                              contract_trials::esp32p4::interrupt_source},
    };
    EXPECT_TRUE(core::interrupt_route_exists(routes, 1, 5));
    EXPECT_TRUE(!core::interrupt_route_exists(routes, 1, 12));
    EXPECT_TRUE(core::interrupt_routes_conflict(
        routes[0], core::interrupt_route{4, 5,
                                         core::interrupt_route_kind::selectable_line,
                                         0x18, true,
                                         contract_trials::ra4m1::interrupt_source}));
}

TEST(dma_metadata, keeps_engines_distinct_and_checks_buffer_constraints,
     "REQ-0013") {
    constexpr core::dma_constraints constraints{
        core::dma_engine_kind::esp_gdma_axi,
        static_cast<std::uint8_t>(core::dma_direction::memory_to_peripheral),
        static_cast<std::uint8_t>(core::dma_width_bit(1) | core::dma_width_bit(4)),
        4,
        1024,
    };
    constexpr core::dma_request request{
        7, constraints, contract_trials::esp32p4::dma_source};
    constexpr core::memory_region region{
        2, 0x3FF0'0000U, 0x1000U, true, true,
        core::cache_policy::software_maintained,
        contract_trials::stm32h753::memory_source};
    constexpr core::dma_reachability reachable{
        core::dma_engine_kind::esp_gdma_axi, 2, true, true};
    constexpr core::dma_reachability wrong_engine{
        core::dma_engine_kind::esp_gdma_ahb, 2, true, true};
    static_assert(core::valid_static_dma_transfer<
                  constraints, 4, 16, core::dma_direction::memory_to_peripheral>);
    static_assert(core::dma_engine_kind::ra_dmac
                  != core::dma_engine_kind::ra_dtc);
    static_assert(core::dma_engine_kind::esp_gdma_ahb
                  != core::dma_engine_kind::esp_gdma_axi);

    EXPECT_TRUE(core::validate_dma_transfer(
                    request, region, reachable, region.begin, 16, 4,
                    core::dma_direction::memory_to_peripheral)
                    .has_value());
    EXPECT_TRUE(!core::validate_dma_transfer(
                     request, region, reachable, region.begin + 1U, 16, 4,
                     core::dma_direction::memory_to_peripheral)
                     .has_value());
    EXPECT_TRUE(!core::validate_dma_transfer(
                     request, region, wrong_engine, region.begin, 16, 4,
                     core::dma_direction::memory_to_peripheral)
                     .has_value());
}

TEST(checked_units, reports_overflow_and_division_by_zero,
     "REQ-0011", "REQ-0013") {
    const auto overflow = core::checked_add(
        core::hertz{std::numeric_limits<std::uint64_t>::max()}, core::hertz{1});
    const auto divide_zero = core::checked_divide(core::bytes{64}, 0);
    const auto multiply_overflow = core::checked_multiply(
        core::bytes{std::numeric_limits<std::uint64_t>::max()}, 2);
    EXPECT_TRUE(!overflow.has_value());
    EXPECT_TRUE(overflow.error() == core::arithmetic_error::overflow);
    EXPECT_TRUE(!divide_zero.has_value());
    EXPECT_TRUE(divide_zero.error() == core::arithmetic_error::divide_by_zero);
    EXPECT_TRUE(!multiply_overflow.has_value());
    EXPECT_TRUE(multiply_overflow.error() == core::arithmetic_error::overflow);
}

} // namespace
