#include <concepts>
#include <cstdint>
#include <type_traits>

#include <mcutest/mcutest.hpp>

#include <mcuhw/mcu/esp32p4/description.hpp>
#include <mcuhw/testing/description_suite.hpp>

namespace {

namespace esp = mcuhw::mcu::esp32p4;

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

TEST(esp32p4_description, is_declarative_metadata_only, "REQ-0003") {
    static_assert(std::is_trivial_v<esp::description>);
    static_assert(std::is_trivially_copyable_v<esp::description>);
    EXPECT_TRUE(true);
}

TEST(esp32p4_description, identifies_parts_package_revisions_and_sources,
     "REQ-0005", "REQ-0034") {
    EXPECT_TRUE(esp::identity.vendor == "Espressif Systems");
    EXPECT_TRUE(esp::identity.device == "ESP32-P4NRW16X");
    EXPECT_TRUE(esp::identity.package == "QFN104");
    ASSERT_EQ(esp::supported_parts.size(), 2U);
    EXPECT_EQ(esp::supported_parts[0].in_package_psram_bytes, 16U * 1024U * 1024U);
    EXPECT_EQ(esp::supported_parts[1].in_package_psram_bytes, 32U * 1024U * 1024U);
    EXPECT_EQ(esp::silicon_revisions.size(), 3U);
    EXPECT_TRUE(esp::source_gaps[0].state
                == "unavailable-in-source-exclusion-accepted");
}

TEST(esp32p4_description, preserves_hp_lp_ownership_and_memory_aliases,
     "REQ-0034", "REQ-0013") {
    EXPECT_EQ(esp::cpu_cores.size(), 3U);
    EXPECT_TRUE(esp::cpu_cores[1].held_in_reset_after_power_up);
    EXPECT_TRUE(esp::cpu_cores[2].owner == esp::cpu_domain::lp_cpu);
    EXPECT_EQ(esp::memory_regions[4].begin, 0x4FF0'0000U);
    EXPECT_EQ(esp::memory_regions[4].size, 768U * 1024U);
    EXPECT_TRUE(esp::memory_regions[2].cache
                == mcuhw::core::cache_policy::software_maintained);
    EXPECT_TRUE(esp::memory_regions[8].cache
                == mcuhw::core::cache_policy::uncached);
    EXPECT_TRUE(esp::memory_details[6].lp_cpu_access);
}

TEST(esp32p4_description, exposes_real_timer_and_pwm_differences,
     "REQ-0004", "REQ-0034") {
    static_assert(esp::ledc_capabilities::contains<
        esp::peripheral_capability::pwm_eight_outputs>);
    static_assert(!esp::ledc_capabilities::contains<
        esp::peripheral_capability::complementary_outputs>);
    static_assert(esp::mcpwm_capabilities::contains<
        esp::peripheral_capability::complementary_outputs>);
    static_assert(esp::mcpwm_capabilities::contains<
        esp::peripheral_capability::dead_time>);
    EXPECT_TRUE(esp::timers[3].kind == esp::timer_kind::led_pwm);
    EXPECT_EQ(esp::timers[3].output_count, 8U);
    EXPECT_TRUE(esp::timers[4].kind == esp::timer_kind::motor_pwm);
    EXPECT_EQ(esp::timers[4].output_count, 6U);
}

TEST(esp32p4_description, preserves_register_widths_offsets_and_side_effects,
     "REQ-0007") {
    static_assert(esp::gpio_out_w1ts::offset == 0x08U);
    static_assert(esp::gpio_out_w1ts::access
                  == mcuhw::core::access_semantics::set_alias);
    static_assert(esp::gpio_out_w1tc::access
                  == mcuhw::core::access_semantics::clear_alias);
    static_assert(esp::gpio_out1_w1ts::offset == 0x14U);
    static_assert(esp::gpio_enable1_w1tc::offset == 0x34U);
    static_assert(esp::gpio_input1::offset == 0x40U);
    static_assert(esp::uart_int_clear::access
                  == mcuhw::core::access_semantics::write_one_to_clear);
    static_assert(esp::uart_clock_divider::offset == 0x14U);
    static_assert(esp::uart_status::access
                  == mcuhw::core::access_semantics::read_only);
    static_assert(esp::uart_register_update::offset == 0x98U);
    static_assert(esp::i2c_int_clear::offset == 0x24U);
    static_assert(esp::i2s_tx_configuration::offset == 0x24U);
    static_assert(esp::i2s_rx_eof_number::offset == 0x64U);
    static_assert(esp::ledc_hpoint::width == 20U);
    static_assert(esp::mcpwm_dead_time0::offset == 0x58U);
    EXPECT_EQ(esp::gpio_registers.size(), 19U);
    EXPECT_EQ(esp::timer_registers.size(), 14U);
}

TEST(esp32p4_description, validates_qfn104_io_mux_matrix_and_analog_routes,
     "REQ-0010") {
    using mcuhw::core::electrical_mode;
    static_assert(esp::qfn104_pin_database::valid<mcuhw::core::pin_id{0, 37},
        esp::signal_id(esp::signal::uart0_tx), electrical_mode::push_pull>);
    static_assert(esp::qfn104_pin_database::valid<mcuhw::core::pin_id{0, 23},
        esp::signal_id(esp::signal::i2c0_sda), electrical_mode::open_drain>);
    static_assert(esp::qfn104_pin_database::valid<mcuhw::core::pin_id{0, 13},
        esp::signal_id(esp::signal::i2s0_mclk), electrical_mode::push_pull>);
    static_assert(esp::qfn104_pin_database::valid<mcuhw::core::pin_id{0, 16},
        esp::signal_id(esp::signal::adc1_ch0), electrical_mode::analog>);
    static_assert(!esp::qfn104_pin_database::valid<mcuhw::core::pin_id{0, 16},
        esp::signal_id(esp::signal::adc1_ch0), electrical_mode::input>);
    EXPECT_EQ(esp::package_pins.size(), 55U);
    EXPECT_EQ(esp::package_pins[0].package_position, 104U);
    EXPECT_TRUE(esp::pin_details[15].lp_io);
    EXPECT_TRUE(!esp::pin_details[16].lp_io);
    EXPECT_TRUE(esp::pin_details[34].strapping);
}

TEST(esp32p4_description, models_hp_lp_clocks_resets_and_source_conflict,
     "REQ-0011", "REQ-0034") {
    EXPECT_EQ(esp::clock_nodes[8].maximum.count, 400'000'000U);
    EXPECT_EQ(esp::clock_nodes[10].maximum.count, 200'000'000U);
    EXPECT_EQ(esp::clock_nodes[11].maximum.count, 100'000'000U);
    const auto apb = mcuhw::core::derive_frequency(
        mcuhw::core::hertz{200'000'000U}, esp::clock_edges[6], {2, 0, 1},
        esp::clock_nodes[11]);
    ASSERT_TRUE(apb.has_value());
    EXPECT_EQ(apb->count, 100'000'000U);
    EXPECT_EQ(esp::reset_controls[0].mask, 1U << 28U);
    EXPECT_EQ(esp::reset_controls[5].mask,
              (1U << 8U) | (1U << 13U));
    EXPECT_EQ(esp::uart_clock_controls[0].system_gate_mask, 1U << 18U);
    EXPECT_EQ(esp::uart_clock_controls[0].apb_gate_mask, 1U << 7U);
    EXPECT_EQ(esp::uart_clock_controls[0].source_register,
              esp::hp_clkrst_base + 0x68U);
    EXPECT_EQ(esp::uart_clock_controls[0].divider_register,
              esp::hp_clkrst_base + 0x6CU);
    EXPECT_EQ(esp::spi_clock_controls[0].system_gate_mask, 1U);
    EXPECT_EQ(esp::spi_clock_controls[0].apb_gate_mask, 1U << 19U);
    EXPECT_EQ(esp::spi_clock_controls[0].reset_register, esp::hp_rst_en2);
    EXPECT_EQ(esp::spi_clock_controls[0].reset_mask, 1U << 7U);
    EXPECT_EQ(esp::instances[22].clock_gate_register,
              esp::hp_soc_clock_ctrl1);
    EXPECT_EQ(esp::instances[22].clock_gate_mask, 1U << 3U);
    EXPECT_EQ(esp::instances[22].reset_register, esp::hp_rst_en1);
    EXPECT_EQ(esp::instances[22].reset_mask, 1U << 1U);
    EXPECT_EQ(esp::reset_controls[9].mask, 1U << 1U);
    EXPECT_TRUE(esp::source_gaps[2].state == "source-conflict-recorded");
}

TEST(esp32p4_description, separates_peripheral_sources_from_matrix_lines,
     "REQ-0012") {
    static_assert(esp::event_number(esp::event::uart0) == 31U);
    static_assert(esp::interrupt_database::valid_cpu_route(esp::event::uart0, 7));
    static_assert(esp::interrupt_database::valid_cpu_route(esp::event::uart0, 31));
    static_assert(!esp::interrupt_database::valid_cpu_route(esp::event::uart0, 32));
    EXPECT_EQ(esp::interrupt_targets.size(), 2U);
    EXPECT_EQ(esp::interrupt_targets[0].input_count, 32U);
    EXPECT_TRUE(!esp::interrupt_routes[0].exclusive_line);
}

TEST(esp32p4_description, separates_dma_engines_requests_and_reachability,
     "REQ-0013") {
    static_assert(esp::gdma_ahb_constraints.engine
                  != esp::gdma_axi_constraints.engine);
    static_assert(esp::dma_engines[1].kind != esp::dma_engines[2].kind);
    static_assert(esp::dma_engines[2].kind != esp::dma_engines[3].kind);
    EXPECT_EQ(esp::dma_engines[0].channel_count, 6U);
    EXPECT_EQ(esp::dma_engines[0].descriptor_alignment, 4U);
    EXPECT_EQ(esp::dma_engines[1].descriptor_alignment, 8U);
    EXPECT_EQ(esp::dma_engines[2].descriptor_alignment, 64U);
    EXPECT_EQ(esp::dma_engines[3].descriptor_alignment, 8U);
    EXPECT_TRUE(!esp::dma_engines[0].descriptors_may_be_external);
    EXPECT_TRUE(esp::dma_engines[1].descriptors_may_be_external);
    const auto accepted = mcuhw::core::validate_dma_transfer(
        esp::dma_requests[4], esp::memory_regions[4], esp::dma_reachability[2],
        0x4FF0'0000U, 32U, 4U,
        mcuhw::core::dma_direction::peripheral_to_memory);
    EXPECT_TRUE(accepted.has_value());
}

TEST(esp32p4_description, records_reviewed_slices_and_unavailable_i3c,
     "REQ-0006", "REQ-0034") {
    ASSERT_EQ(esp::review_ledger.size(), 18U);
    for (const auto& review : esp::review_ledger) {
        EXPECT_TRUE(review.described_registers <= review.register_denominator);
        EXPECT_TRUE(!review.provenance.document.empty());
    }
    EXPECT_TRUE(esp::review_ledger.back().state
                == esp::coverage_state::unavailable_in_source);
    static_assert(!esp::i3c_capabilities::contains<
        esp::peripheral_capability::i2c_multi_master>);
}

TEST(esp32p4_description, instantiates_shared_description_contract) {
    static_assert(mcuhw::testing::for_each_description<esp::description>(
        real_description_contract{}));
    EXPECT_TRUE(true);
}

} // namespace
