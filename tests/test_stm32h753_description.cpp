#include <concepts>
#include <cstdint>
#include <type_traits>

#include <mcutest/mcutest.hpp>

#include <mcuhw/mcu/stm32h753/description.hpp>
#include <mcuhw/testing/description_suite.hpp>

namespace {

namespace stm = mcuhw::mcu::stm32h753;

struct real_description_contract {
    template<typename Description>
    constexpr bool operator()() const {
        return !Description::device.vendor.empty()
            && !Description::memories.empty()
            && !Description::pins.empty()
            && !Description::routes.empty()
            && !Description::clocks.empty()
            && !Description::irqs.empty()
            && !Description::transfers.empty()
            && Description::representative_register::width != 0U
            && Description::representative_field::width != 0U;
    }
};

TEST(stm32h753_description, is_declarative_metadata_only, "REQ-0003") {
    static_assert(std::is_trivial_v<stm::description>);
    static_assert(std::is_trivially_copyable_v<stm::description>);
    EXPECT_TRUE(true);
}

TEST(stm32h753_description, identifies_exact_part_packages_revisions_and_sources,
     "REQ-0005", "REQ-0033") {
    EXPECT_TRUE(stm::identity.vendor == "STMicroelectronics");
    EXPECT_TRUE(stm::identity.device == "STM32H753VI");
    EXPECT_TRUE(stm::identity.package == "LQFP100");
    EXPECT_EQ(stm::supported_packages.size(), 8U);
    EXPECT_EQ(stm::silicon_revisions.size(), 3U);
    EXPECT_EQ(stm::silicon_revisions[2].rev_id, 0x2003U);
    EXPECT_EQ(stm::core_properties.maximum_frequency_hz, 480'000'000U);
    EXPECT_EQ(stm::core_properties.instruction_cache_bytes, 16U * 1024U);
    EXPECT_TRUE(stm::source_gaps[0].state == "correction-overlay-applied");
}

TEST(stm32h753_description, records_reviewed_slices_and_deferred_breadth,
     "REQ-0006") {
    ASSERT_EQ(stm::review_ledger.size(), 14U);
    for (const auto& review : stm::review_ledger) {
        EXPECT_TRUE(review.state == stm::coverage_state::verified);
        EXPECT_TRUE(review.described_registers <= review.register_denominator);
        EXPECT_TRUE(!review.provenance.document.empty());
    }
    EXPECT_TRUE(!stm::review_ledger[3].outstanding.empty());
}

TEST(stm32h753_description, exposes_native_capability_differences,
     "REQ-0004") {
    static_assert(stm::advanced_timer_capabilities::contains<
        stm::peripheral_capability::complementary_outputs>);
    static_assert(!stm::general_timer32_capabilities::contains<
        stm::peripheral_capability::complementary_outputs>);
    static_assert(stm::mdma_capabilities::contains<
        stm::peripheral_capability::tcm_access>);
    static_assert(!stm::dma12_capabilities::contains<
        stm::peripheral_capability::tcm_access>);
    EXPECT_EQ(stm::instances.size(), 49U);
}

TEST(stm32h753_description, preserves_register_offsets_widths_and_side_effects,
     "REQ-0007") {
    static_assert(stm::gpio_afrh::offset == 0x24U);
    static_assert(stm::dma_lifcr::access
                  == mcuhw::core::access_semantics::write_one_to_clear);
    static_assert(stm::tim_update_flag::access
                  == mcuhw::core::access_semantics::write_zero_to_clear);
    static_assert(stm::i2c_icr::access
                  == mcuhw::core::access_semantics::write_one_to_clear);
    static_assert(stm::usart_isr::access
                  == mcuhw::core::access_semantics::read_only);
    static_assert(stm::spi_ifcr::access
                  == mcuhw::core::access_semantics::write_one_to_clear);
    static_assert(stm::adc_dr::access
                  == mcuhw::core::access_semantics::read_only);
    EXPECT_EQ(stm::gpio_registers.size(), 10U);
    EXPECT_EQ(stm::timer_registers.size(), 17U);
    EXPECT_EQ(stm::dma_registers.size(), 16U);
}

TEST(stm32h753_description, validates_package_membership_and_af_routes,
     "REQ-0010") {
    using mcuhw::core::electrical_mode;
    constexpr mcuhw::core::pin_id pa8{0, 8};
    constexpr mcuhw::core::pin_id pk0{10, 0};
    static_assert(stm::lqfp100_pin_database::valid<pa8,
        stm::signal_id(stm::signal::tim1_ch1), electrical_mode::push_pull>);
    static_assert(!stm::lqfp100_pin_database::valid<pk0,
        stm::signal_id(stm::signal::gpio_output), electrical_mode::push_pull>);
    static_assert(stm::package_contains(stm::package_variant::lqfp208, pk0,
                                        stm::package_memberships));
    EXPECT_TRUE(mcuhw::core::route_exists(
        stm::pin_routes, {0, 5}, stm::signal_id(stm::signal::spi1_sck)));
    EXPECT_EQ(stm::package_pins[5].package_position, 67U);
}

TEST(stm32h753_description, models_clock_domains_corrected_adc_limit_and_reset,
     "REQ-0011") {
    EXPECT_EQ(stm::clock_nodes[15].maximum.count, 80'000'000U);
    const auto hclk = mcuhw::core::derive_frequency(
        mcuhw::core::hertz{480'000'000U}, stm::clock_edges[4], {2, 0, 1},
        stm::clock_nodes[8]);
    ASSERT_TRUE(hclk.has_value());
    EXPECT_EQ(hclk->count, 240'000'000U);
    EXPECT_EQ(stm::reset_controls[1].register_offset, stm::rcc_base + 0x80U);
    EXPECT_EQ(stm::instances[11].kernel_clock,
              static_cast<std::uint16_t>(stm::clock_id::timy_ker));
}

TEST(stm32h753_description, keeps_fixed_nvic_lines_distinct_from_events,
     "REQ-0012") {
    static_assert(stm::event_number(stm::event::tim2) != 28U);
    static_assert(stm::interrupt_database::valid_cpu_route(stm::event::tim2, 28));
    static_assert(!stm::interrupt_database::valid_cpu_route(stm::event::tim2, 29));
    static_assert(stm::interrupt_database::valid_cpu_route(stm::event::tim8_break, 43));
    static_assert(stm::interrupt_database::valid_cpu_route(stm::event::tim12, 43));
    EXPECT_EQ(stm::interrupt_routes.size(), 36U);
}

TEST(stm32h753_description, separates_dma_engines_reachability_and_cache_policy,
     "REQ-0013") {
    static_assert(stm::dma12_constraints.engine != stm::bdma_constraints.engine);
    static_assert(stm::dma12_constraints.engine != stm::mdma_constraints.engine);
    const auto accepted = mcuhw::core::validate_dma_transfer(
        stm::dma_requests[0], stm::memory_regions[4], stm::dma_reachability[2],
        0x2400'0000U, 16U, 4U,
        mcuhw::core::dma_direction::peripheral_to_memory);
    EXPECT_TRUE(accepted.has_value());
    const mcuhw::core::dma_request mdma_request{0, stm::mdma_constraints,
                                                stm::dma_source};
    const auto tcm = mcuhw::core::validate_dma_transfer(
        mdma_request, stm::memory_regions[3], stm::dma_reachability[12],
        0x2000'0000U, 16U, 4U,
        mcuhw::core::dma_direction::peripheral_to_memory);
    EXPECT_TRUE(tcm.has_value());
    EXPECT_TRUE(stm::memory_regions[4].cache
                == mcuhw::core::cache_policy::software_maintained);
}

TEST(stm32h753_description, instantiates_shared_description_contract) {
    static_assert(mcuhw::testing::for_each_description<stm::description>(
        real_description_contract{}));
    EXPECT_TRUE(true);
}

} // namespace
