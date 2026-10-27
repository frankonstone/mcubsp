#include <concepts>
#include <type_traits>

#include <mcutest/mcutest.hpp>

#include <mcubsp/mcu/esp32p4/description.hpp>
#include <mcubsp/mcu/ra4m1/description.hpp>
#include <mcubsp/mcu/stm32h753/description.hpp>
#include <mcubsp/testing/description_suite.hpp>

#if defined(MCUBSP_TARGET) || defined(MCUBSP_MCU) \
    || defined(MCUBSP_TARGET_RA4M1) || defined(MCUBSP_TARGET_STM32H753) \
    || defined(MCUBSP_TARGET_ESP32P4)
#error "description metadata must not require a global MCU-selection macro"
#endif

namespace {

namespace ra = mcubsp::mcu::ra4m1;
namespace stm = mcubsp::mcu::stm32h753;
namespace esp = mcubsp::mcu::esp32p4;

TEST(three_description_gate, instantiates_one_complete_contract_for_all_targets,
     "REQ-0001", "REQ-0003", "REQ-0004", "REQ-0006", "REQ-0033",
     "REQ-0034", "REQ-0038", "REQ-0040") {
    static_assert(mcubsp::testing::for_each_description<
        ra::description, stm::description, esp::description>(
            mcubsp::testing::complete_description_contract{}));
    static_assert(!std::same_as<ra::description, stm::description>);
    static_assert(!std::same_as<stm::description, esp::description>);
    EXPECT_TRUE(true);
}

TEST(three_description_gate, reconciles_exact_source_denominators_and_gaps,
     "REQ-0005", "REQ-0006", "REQ-0033", "REQ-0034", "REQ-0040") {
    static_assert(mcubsp::core::complete_source_inventory(
        std::span{ra::complete_source_inventory}, 48U));
    static_assert(mcubsp::core::complete_source_inventory(
        std::span{stm::complete_source_inventory}, 61U));
    static_assert(mcubsp::core::complete_source_inventory(
        std::span{esp::complete_source_inventory}, 68U, 1U));

    EXPECT_EQ(ra::description::source_units.size(), 48U);
    EXPECT_EQ(stm::description::source_units.size(), 61U);
    EXPECT_EQ(esp::description::source_units.size(), 68U);
    EXPECT_TRUE(esp::complete_source_inventory.back().unit == "I3C");
    EXPECT_TRUE(esp::complete_source_inventory.back().description
                == mcubsp::core::description_coverage_state::unavailable_in_source);
}

TEST(three_description_gate, preserves_native_routing_clock_and_irq_models,
    "REQ-0010", "REQ-0011", "REQ-0012", "REQ-0040") {
    static_assert(ra::pin_routes[0].kind
                  == mcubsp::core::pin_route_kind::alternate_function);
    static_assert(stm::pin_routes[0].kind
                  == mcubsp::core::pin_route_kind::alternate_function);
    static_assert(ra::pin_routes[0].vendor_encoding
                  != stm::pin_routes[0].vendor_encoding);
    static_assert(esp::qfn104_pin_database::valid<
        mcubsp::core::pin_id{0, 20}, esp::signal_id(esp::signal::ledc_ch0),
        mcubsp::core::electrical_mode::push_pull>);

    EXPECT_TRUE(ra::clock_edges[0].kind
                == mcubsp::core::clock_route_kind::selectable);
    EXPECT_TRUE(!ra::clock_edges[0].allows_fractional_divider);
    EXPECT_TRUE(esp::clock_edges[3].allows_fractional_divider);
    EXPECT_TRUE(ra::interrupt_routes[0].kind
                == mcubsp::core::interrupt_route_kind::selectable_line);
    EXPECT_TRUE(stm::interrupt_routes[0].kind
                == mcubsp::core::interrupt_route_kind::fixed_line);
    EXPECT_TRUE(esp::interrupt_routes[0].kind
                == mcubsp::core::interrupt_route_kind::interrupt_matrix);
}

TEST(three_description_gate, preserves_dma_memory_and_timer_differences,
     "REQ-0004", "REQ-0013", "REQ-0033", "REQ-0034", "REQ-0040") {
    static_assert(ra::dmac_constraints.engine
                  == mcubsp::core::dma_engine_kind::ra_dmac);
    static_assert(stm::mdma_constraints.engine
                  == mcubsp::core::dma_engine_kind::stm32_mdma);
    static_assert(esp::gdma_axi_constraints.engine
                  == mcubsp::core::dma_engine_kind::esp_gdma_axi);
    EXPECT_TRUE(ra::dmac_constraints.maximum_count
                != stm::dma12_constraints.maximum_count);
    EXPECT_TRUE(stm::memory_regions[3].cache
                != esp::memory_regions[4].cache);
    EXPECT_EQ(ra::gpt_instances[0].outputs.size(), 2U);
    EXPECT_EQ(stm::timer_instances[0].output_count, 7U);
    EXPECT_EQ(esp::timers[4].output_count, 6U);
    EXPECT_TRUE(!ra::source_gaps.empty());
    EXPECT_TRUE(!stm::source_gaps.empty());
    EXPECT_TRUE(!esp::source_gaps.empty());
}

TEST(three_description_gate, records_traceable_gate_evidence,
     "REQ-0039", "REQ-0040") {
    EXPECT_TRUE(ra::complete_source_inventory.front().provenance.document
                == "R01UH0887EJ0110");
    EXPECT_TRUE(stm::complete_source_inventory.front().provenance.document
                == "DS12117");
    EXPECT_TRUE(esp::complete_source_inventory.front().provenance.document
                == "ESP32-P4 Series Datasheet");
}

} // namespace
