#include <cstddef>
#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcuhw/mcu/ra4m1/description.hpp>
#include <mcuhw/testing/description_suite.hpp>

namespace {

namespace ra = mcuhw::mcu::ra4m1;
using mcuhw::core::electrical_mode;

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

TEST(ra4m1_description, identifies_exact_device_package_and_sources,
     "REQ-0005") {
    EXPECT_TRUE(ra::identity.vendor == "Renesas");
    EXPECT_TRUE(ra::identity.device == "R7FA4M1AB3CFM#AA0");
    EXPECT_TRUE(ra::identity.package == "PLQP0064KB-C, 64-pin LQFP");
    EXPECT_TRUE(ra::identity.provenance.document
                == "R01UH0887EJ0110 / R01DS0355EJ0110");
    EXPECT_EQ(ra::source_gaps.size(), 1U);
    EXPECT_TRUE(ra::source_gaps[0].id == "GAP-RA4M1-001");
}

TEST(ra4m1_description, records_audited_increment_and_deferred_remainder,
     "REQ-0006") {
    ASSERT_EQ(ra::review_ledger.size(), 13U);
    for (const auto& review : ra::review_ledger) {
        EXPECT_TRUE(!review.provenance.document.empty());
        EXPECT_TRUE(!review.provenance.revision.empty());
        EXPECT_TRUE(review.described_registers <= review.register_denominator);
        EXPECT_TRUE(review.state == ra::coverage_state::verified);
    }
    EXPECT_TRUE(!ra::review_ledger[7].outstanding.empty());
}

TEST(ra4m1_description, preserves_real_register_widths_aliases_and_side_effects,
     "REQ-0007") {
    static_assert(ra::p000pfs::width == 4U);
    static_assert(ra::p000pfs_ha::width == 2U);
    static_assert(ra::p000pfs_by::width == 1U);
    static_assert(std::same_as<ra::p000pfs_ha::canonical_register, ra::p000pfs>);
    static_assert(ra::sci_parity_error::access
                  == mcuhw::core::access_semantics::write_zero_to_clear);
    static_assert(ra::spi_mode_fault::access
                  == mcuhw::core::access_semantics::write_zero_to_clear);
    static_assert(ra::hococr2::offset == 0x37U);
    static_assert(ra::hoco_frequency::mask == 0x38U);
    static_assert(!ra::hococr2::reset_fully_known);
    EXPECT_EQ(ra::dmac_registers.size(), 12U);
    EXPECT_EQ(ra::gpt_registers.size(), 24U);
    EXPECT_EQ(ra::spi_registers.size(), 13U);
    EXPECT_TRUE(ra::sci_rdr::access == mcuhw::core::access_semantics::read_only);
}

TEST(ra4m1_description, carries_protection_groups_without_executing_sequences,
     "REQ-0009") {
    ASSERT_EQ(ra::protections.size(), 4U);
    EXPECT_EQ(ra::protections[0].control_register_offset, 0x3FEU);
    EXPECT_EQ(ra::protections[2].control_register_offset, 0xD03U);
    EXPECT_TRUE(ra::protections[2].relock_required);
    EXPECT_EQ(ra::prcr::required_one_mask, 0xA500U);
    EXPECT_EQ(ra::pwpr::reset_value, 0x80U);
}

TEST(ra4m1_description, validates_package_and_source_backed_pfs_routes,
     "REQ-0010") {
    constexpr mcuhw::core::pin_id p400{4, 0};
    constexpr mcuhw::core::pin_id absent_p600{6, 0};
    static_assert(ra::pin_database::valid<p400,
        ra::signal_id(ra::signal::iic0_scl), electrical_mode::open_drain>);
    static_assert(!ra::pin_database::valid<absent_p600,
        ra::signal_id(ra::signal::gpio_output), electrical_mode::push_pull>);
    static_assert(ra::pfs_address({4, 8}) == 0x4004'0920U);
    EXPECT_EQ(ra::package_pins.size(), 52U);
    EXPECT_TRUE(mcuhw::core::route_exists(
        ra::pin_routes, {1, 0}, ra::signal_id(ra::signal::spi0_miso)));
    EXPECT_TRUE(!mcuhw::core::route_exists(
        ra::pin_routes, {4, 0}, ra::signal_id(ra::signal::spi0_miso)));
}

TEST(ra4m1_description, models_clock_limits_domains_and_shared_module_stops,
     "REQ-0011") {
    const auto pclkb = mcuhw::core::derive_frequency(
        mcuhw::core::hertz{48'000'000U}, ra::clock_edges[2], {2, 0, 1},
        ra::clock_nodes[8]);
    ASSERT_TRUE(pclkb.has_value());
    EXPECT_EQ(pclkb->count, 24'000'000U);
    const auto too_fast = mcuhw::core::derive_frequency(
        mcuhw::core::hertz{64'000'000U}, ra::clock_edges[2], {1, 0, 1},
        ra::clock_nodes[8]);
    EXPECT_TRUE(!too_fast.has_value());
    EXPECT_EQ(ra::mstpcrd::reset_value, 0xFFFF'FFFFU);
}

TEST(ra4m1_description, keeps_icu_event_numbers_separate_from_cpu_lines,
     "REQ-0012") {
    static_assert(ra::event_number(ra::event::gpt0_compare_a) == 0x057U);
    static_assert(ra::interrupt_database::valid_cpu_route(
        ra::event::gpt0_compare_a, 31));
    static_assert(!ra::interrupt_database::valid_cpu_route(
        ra::event::gpt0_compare_a, 32));
    EXPECT_TRUE(ra::event_number(ra::event::sci0_rxi) != 0U);
    EXPECT_EQ(ra::ielsr0::offset, 0x300U);
    EXPECT_EQ(ra::delsr0::offset, 0x280U);
}

TEST(ra4m1_description, keeps_dmac_and_dtc_distinct_and_checks_sram,
     "REQ-0013") {
    static_assert(ra::dmac_constraints.engine
                  != ra::dtc_constraints.engine);
    static_assert(sizeof(ra::dtc_transfer_data) == 16U);
    const auto accepted = mcuhw::core::validate_dma_transfer(
        ra::dma_requests[0], ra::memory_regions[1], ra::dma_reachability[1],
        0x2000'0000U, 16U, 4U,
        mcuhw::core::dma_direction::peripheral_to_memory);
    EXPECT_TRUE(accepted.has_value());
    const auto misaligned = mcuhw::core::validate_dma_transfer(
        ra::dma_requests[0], ra::memory_regions[1], ra::dma_reachability[1],
        0x2000'0001U, 16U, 4U,
        mcuhw::core::dma_direction::peripheral_to_memory);
    EXPECT_TRUE(!misaligned.has_value());
}

TEST(ra4m1_description, preserves_gpt_instance_width_and_two_output_identities,
     "REQ-0007") {
    static_assert(ra::gpt32_capabilities::contains<
        ra::peripheral_capability::pwm_output_a>);
    static_assert(ra::gpt32_capabilities::contains<
        ra::peripheral_capability::pwm_output_b>);
    static_assert(ra::gpt32_capabilities::provenance.document
                  == "R01UH0887EJ0110");
    EXPECT_EQ(ra::instances.size(), 29U);
    EXPECT_EQ(ra::gpt_instances[0].counter_width, 32U);
    EXPECT_EQ(ra::gpt_instances[2].counter_width, 16U);
    EXPECT_TRUE(ra::gpt_instances[0].outputs[0] == ra::gpt_output::a);
    EXPECT_TRUE(ra::gpt_instances[0].outputs[1] == ra::gpt_output::b);
}

TEST(ra4m1_description, instantiates_the_shared_description_contract) {
    static_assert(mcuhw::testing::for_each_description<ra::description>(
        real_description_contract{}));
    EXPECT_TRUE(true);
}

} // namespace
