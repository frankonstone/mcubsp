#include <mcubsp/mcu/nrf52840/catalogue.hpp>
#include <mcubsp/mcu/nrf52840/product.hpp>
#include <mcubsp/mcu/nrf52840/system.hpp>
#include <mcubsp/mcu/nrf52840/memory.hpp>
#include <mcubsp/mcu/nrf52840/core_registers.hpp>
#include <mcubsp/testing/recording_mmio.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
namespace n = mcu::nrf52840;
TEST(nrf_description, source_catalogue_accounts_for_all_legacy_mdk_instances, "REQ-0055") {
    EXPECT_EQ(n::instance_inventory.size(), 74U);
    unsigned registers{}, fields{};
    for (auto &p : n::instance_inventory) {
        EXPECT_TRUE(p.base % 4 == 0);
        EXPECT_TRUE(p.irq < 0 || n::valid_irq(static_cast<unsigned>(p.irq)));
        registers += p.registers;
        fields += p.fields;
    }
    EXPECT_EQ(registers, 2142U);
    EXPECT_EQ(fields, 5949U);
    EXPECT_FALSE(n::complete_product_description); // stepping/errata and board identity gate remains open
}
TEST(nrf_description, shared_addresses_do_not_hide_real_instance_differences, "REQ-0055") {
    EXPECT_EQ(n::instances::spim0::base, n::instances::twim0::base);
    EXPECT_EQ(n::instances::uart0::base, n::instances::uarte0::base);
    EXPECT_EQ(n::instances::comp::base, n::instances::lpcomp::base);
    EXPECT_EQ(n::timer_channels[0], 4U);
    EXPECT_EQ(n::timer_channels[4], 6U);
    EXPECT_EQ(n::ram_section_count(0), 2U);
    EXPECT_EQ(n::ram_section_count(8), 6U);
}
TEST(nrf_description, typed_register_fields_preserve_commands_aliases_and_masks, "REQ-0055") {
    using namespace n::registers;
    EXPECT_TRUE(clock::tasks_hfclkstart::access == core::access_semantics::command);
    EXPECT_TRUE(clock::events_hfclkstarted::access == core::access_semantics::write_zero_to_clear);
    EXPECT_TRUE(power::resetreas::access == core::access_semantics::write_one_to_clear);
    EXPECT_TRUE(ppi::chenset::access == core::access_semantics::set_alias);
    EXPECT_EQ(gpiote::config0::offset, 0x510U);
    EXPECT_EQ(gpiote::config0_fields::port::mask, 0x2000U);
    EXPECT_EQ(timer0::cc5::offset, 0x554U);
    EXPECT_EQ(rtc0::counter::writable_mask, 0U);
    EXPECT_TRUE(usbd::eventcause::access == core::access_semantics::write_one_to_clear);
    EXPECT_TRUE(usbd::epstall::access == core::access_semantics::write_only);
    EXPECT_TRUE(uart0::rxd::access == core::access_semantics::read_clear);
}
TEST(nrf_description, product_crypto_supplement_preserves_current_addresses_and_field_semantics,
     "REQ-0055") {
    namespace p = n::product;
    EXPECT_EQ(p::crypto_inventory.size(), 11U);
    unsigned registers{}, fields{};
    for (const auto &block : p::crypto_inventory) {
        EXPECT_EQ(block.base, 0x5002B000U);
        registers += block.registers;
        fields += block.fields;
    }
    EXPECT_EQ(registers, 199U);
    EXPECT_EQ(fields, 345U);
    EXPECT_EQ(p::cc_aes::aes_key_0_0::offset, 0x400U);
    EXPECT_EQ(p::cc_aes::aes_hw_flags::reset_value, 0x108U);
    EXPECT_EQ(p::cc_chacha::chacha_control_fields::use_iv_96bit::mask, 1U << 10);
    EXPECT_EQ(p::cc_pka::pka_l_0_fields::opsize::mask, 0x1FFFU);
    EXPECT_EQ(p::cc_host_rgf::base + p::cc_host_rgf::host_iot_kdr0::offset,
              n::instances::cc_host_rgf::base + n::registers::cc_host_rgf::host_iot_kdr0::offset);
    EXPECT_TRUE(p::cc_host_rgf::host_iot_kdr1::write_once);
    EXPECT_TRUE(p::cc_host_rgf::host_iot_lcs_fields::lcs_is_valid::access ==
                core::access_semantics::read_only);
    EXPECT_EQ(p::cc_host_rgf::host_iot_lcs::writable_mask, 7U);
    EXPECT_TRUE(p::cc_rng::ehr_data_5::access == core::access_semantics::read_clear);
    EXPECT_TRUE(p::cc_rng::rng_icr::access == core::access_semantics::clear_alias);
    EXPECT_TRUE(p::cc_pka::pka_sram_rdata::access == core::access_semantics::read_clear);
    EXPECT_TRUE(p::cc_rng_sram::sram_data::access ==
                core::access_semantics::read_write_side_effect);
    EXPECT_EQ(p::cc_host_rgf::host_boot_fields::aes_exists_local::mask, 1U << 30);
    EXPECT_EQ(p::cc_host_rgf::host_boot_fields::only_encrypt_local::mask, 1U << 29);
}
TEST(nrf_description, current_product_tables_cover_fields_instances_and_read_only_overrides,
     "REQ-0055") {
    namespace p = n::product;
    unsigned registers{}, fields{}, debug_ports{};
    for (const auto &block : p::block_inventory) {
        registers += block.registers;
        fields += block.fields;
        debug_ports += block.debug_access_port;
    }
    EXPECT_EQ(p::block_inventory.size(), 53U);
    EXPECT_EQ(registers, 1563U);
    EXPECT_EQ(fields, 4476U);
    EXPECT_EQ(debug_ports, 1U);
    EXPECT_EQ(p::instances::fpu::base, 0x40026000U);
    EXPECT_EQ(p::instances::fpu::irq, 38);
    EXPECT_EQ(p::instances::cc_host_rgf::base, 0x5002B000U);
    EXPECT_EQ(p::gpio::out_fields::pin31::mask, 0x80000000U);
    EXPECT_EQ(p::ppi::chg_5_fields::ch31::mask, 0x80000000U);
    EXPECT_EQ(p::clock::lfrcmode::writable_mask, 1U);
    EXPECT_EQ(n::registers::clock::lfrcmode::writable_mask, 1U);
    EXPECT_TRUE(p::clock::lfrcmode_fields::status::access == core::access_semantics::read_only);
    EXPECT_TRUE(n::registers::clock::lfrcmode_fields::status::access == core::access_semantics::read_only);
    EXPECT_TRUE(p::mwu::perregion_0_substatwa::access == core::access_semantics::write_one_to_clear);
    EXPECT_TRUE(p::nfct::framestatus_rx::access == core::access_semantics::write_one_to_clear);
    EXPECT_TRUE(p::usbd::epstatus::access == core::access_semantics::write_one_to_clear);
    EXPECT_TRUE(p::uart0::rxd::access == core::access_semantics::read_clear);
    EXPECT_TRUE(p::gpio::outset_status::access == core::access_semantics::read_only);
    EXPECT_EQ(p::ctrl_ap::access_port, 1U);
    using radio_mode = p::radio::mode_fields::mode_encoding;
    EXPECT_TRUE(p::radio::mode_fields::mode_values::try_encode(radio_mode::value_ble_1mbit).has_value());
    EXPECT_FALSE(p::radio::mode_fields::mode_values::try_encode(static_cast<radio_mode>(14)).has_value());
}
TEST(nrf_description, package_bonds_and_electrical_constraints_do_not_imply_board_exposure,
     "REQ-0055") {
    namespace p = n::product;
    EXPECT_TRUE(p::bonded(p::package::aqfn73, {0, 15}));
    EXPECT_TRUE(p::bonded(p::package::wlcsp94, {0, 15}));
    EXPECT_FALSE(p::bonded(p::package::qfn48, {0, 15}));
    EXPECT_FALSE(p::bonded(p::package::aqfn73, {1, 16}));
    EXPECT_FALSE(p::bonded(static_cast<p::package>(3), {0, 0}));
    EXPECT_TRUE(p::digital_route_supported(p::package::aqfn73, {1, 5}, 10000));
    EXPECT_FALSE(p::digital_route_supported(p::package::aqfn73, {1, 5}, 10001));
    EXPECT_FALSE(p::digital_route_supported(p::package::aqfn73, {1, 5}, 100, true));
    EXPECT_TRUE(p::digital_route_supported(p::package::aqfn73, {1, 0}, 2000000, true));
    EXPECT_TRUE(p::package_from_ficr(0x2004) == p::package::aqfn73);
    EXPECT_TRUE(p::package_from_ficr(0x2000) == p::package::qfn48);
    EXPECT_TRUE(p::package_from_ficr(0x2005) == p::package::wlcsp94);
    EXPECT_FALSE(p::package_from_ficr(0xFFFFFFFF).has_value());
    EXPECT_FALSE(p::package_from_ficr(0).has_value());
}
TEST(nrf_description, ram_aliases_dma_ranges_and_power_sections_have_distinct_meanings,
     "REQ-0055", "REQ-0073", "REQ-0062") {
    namespace mem = n::memory;
    EXPECT_TRUE(mem::canonical_ram_address(0x00800010) == 0x20000010U);
    EXPECT_TRUE(mem::canonical_ram_address(0x2003FFFF) == 0x2003FFFFU);
    EXPECT_FALSE(mem::canonical_ram_address(0x20040000).has_value());
    EXPECT_TRUE(mem::easydma_buffer(0x2003FFFC, 4, 4));
    EXPECT_FALSE(mem::easydma_buffer(0x2003FFFC, 5, 4));
    EXPECT_FALSE(mem::easydma_buffer(0x00800000, 4, 4));
    EXPECT_FALSE(mem::easydma_buffer(0x00026000, 4, 4));
    EXPECT_FALSE(mem::easydma_buffer(0x20000001, 4, 4));
    EXPECT_FALSE(mem::easydma_buffer(0x20000000, 4, 0));
    EXPECT_FALSE(mem::easydma_buffer(0x20000000, 4, 3));
    EXPECT_FALSE(mem::easydma_buffer(0x20000000, static_cast<std::size_t>(-1)));
    EXPECT_FALSE(mem::easydma_buffer(0x20040000, 0));
    auto last = mem::ram_section(8, 5);
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(last->begin, 0x20038000U);
    EXPECT_EQ(last->bytes, 0x8000U);
    EXPECT_FALSE(mem::ram_section(0, 2).has_value());
    EXPECT_FALSE(mem::ram_section(8, 6).has_value());
}
TEST(nrf_description, selected_core_geometry_and_debug_comparator_differences_are_explicit,
     "REQ-0055", "REQ-0073") {
    namespace c = n::core_registers;
    EXPECT_EQ(c::inventory.size(), 12U);
    unsigned registers{};
    for (auto &block : c::inventory) registers += block.registers;
    EXPECT_EQ(registers, 382U);
    EXPECT_EQ(c::nvic::iser1::writable_mask, 0x0000A7FFU);
    EXPECT_EQ(c::nvic::ip47::writable_mask, 0xE0U);
    EXPECT_TRUE(c::nvic::iabr0::access == core::access_semantics::read_only);
    EXPECT_EQ(c::scb::cpacr::writable_mask, 0x00F00000U);
    EXPECT_EQ(c::debug::dcrsr_fields::regsel::width, 7U);
    EXPECT_TRUE(c::core_register_selector(33));
    EXPECT_TRUE(c::core_register_selector(95));
    EXPECT_FALSE(c::core_register_selector(19));
    EXPECT_FALSE(c::core_register_selector(96));
    EXPECT_EQ(c::tpi::devtype_fields::subtype::mask, 0xF0U);
    EXPECT_EQ(c::tpi::ffcr::writable_mask, 2U);
    EXPECT_TRUE(c::tpi::ffcr_fields::trigin::access == core::access_semantics::read_only);
    EXPECT_TRUE((c::dwt::function0::writable_mask & 0x80U) != 0);
    EXPECT_TRUE((c::dwt::function1::writable_mask & 0x80U) == 0);
    EXPECT_TRUE((c::dwt::function2::writable_mask & 0xF0F00U) == 0);
    EXPECT_TRUE(c::dwt::function1_fields::lnk1ena::access == core::access_semantics::read_only);
    EXPECT_EQ(c::dwt::pid0::offset, 0xFE0U);
    EXPECT_EQ(c::tpi::pid0::offset, 0xFE0U);
    EXPECT_EQ(c::fpb::comp0::writable_mask, 0xDFFFFFFDU);
    EXPECT_EQ(c::fpb::comp6::writable_mask, 0x1FFFFFFDU);
    EXPECT_EQ(c::fpb::comp0::reset_known_mask, 1U);
    EXPECT_FALSE(c::dwt::ctrl::reset_fully_known);
    EXPECT_EQ(c::etm::etmtesseicr::writable_mask, 0xF000FU);
    EXPECT_EQ(c::etm::etmsr::writable_mask, 12U);
    EXPECT_EQ(c::etm::etmsr_fields::trigger::mask, 8U);
    EXPECT_EQ(c::etm::etmidr_fields::major::extract(c::etm::etmidr::reset_value), 2U);
    EXPECT_EQ(c::etm::etmidr_fields::minor::extract(c::etm::etmidr::reset_value), 5U);
    EXPECT_FALSE(c::etm::etmccr::reset_fully_known);
}
TEST(nrf_description, keyed_core_writes_reject_missing_keys_and_reserved_bits_without_mmio,
     "REQ-0055", "REQ-0073") {
    namespace c = n::core_registers;
    testing::recording_mmio<512, 8> policy{c::scb::base};
    core::register_ref<c::scb::aircr, decltype(policy)> aircr{policy, c::scb::base};
    EXPECT_FALSE(aircr.issue(4).has_value());
    EXPECT_FALSE(aircr.issue(0x05FA0008).has_value());
    EXPECT_EQ(policy.operations().size(), 0U);
    ASSERT_TRUE(aircr.issue(0x05FA0004).has_value());
    ASSERT_EQ(policy.operations().size(), 1U);
    EXPECT_EQ(policy.operations()[0].value, 0x05FA0004U);
    policy.clear_operations();
    core::register_ref<c::debug::dhcsr, decltype(policy)> dhcsr{policy, c::debug::base};
    EXPECT_FALSE(dhcsr.write(1).has_value());
    EXPECT_FALSE(dhcsr.write(0xA05F0010).has_value());
    EXPECT_EQ(policy.operations().size(), 0U);
    ASSERT_TRUE(dhcsr.write(0xA05F0001).has_value());
    ASSERT_EQ(policy.operations().size(), 1U);
    EXPECT_EQ(policy.operations()[0].value, 0xA05F0001U);
}
TEST(nrf_description, core_read_side_effects_and_stimulus_widths_do_not_gain_rmw_access,
     "REQ-0055", "REQ-0073") {
    namespace c = n::core_registers;
    using policy_type = testing::recording_mmio<4096, 8>;
    static_assert(!core::register_detail::readable<c::systick::ctrl>);
    static_assert(!core::register_detail::readable<c::dwt::function0>);
    static_assert(!core::register_detail::safely_modifiable<c::debug::dhcsr>);
    static_assert(!core::register_detail::safely_modifiable<c::fpb::ctrl>);
    static_assert(!core::register_detail::directly_writable<c::nvic::iabr0>);
    EXPECT_TRUE(c::systick::ctrl_fields::countflag::access == core::access_semantics::read_clear);
    EXPECT_TRUE(c::dwt::function0_fields::matched::access == core::access_semantics::read_clear);
    policy_type policy{c::itm::base};
    core::register_ref<c::itm::port0_u8, policy_type> byte{policy, c::itm::base};
    core::register_ref<c::itm::port0_u16, policy_type> half{policy, c::itm::base};
    core::register_ref<c::itm::port0_u32, policy_type> word{policy, c::itm::base};
    ASSERT_TRUE(byte.write(0xA5).has_value());
    ASSERT_TRUE(half.write(0xA55A).has_value());
    ASSERT_TRUE(word.write(0x12345678).has_value());
    ASSERT_EQ(policy.operations().size(), 3U);
    EXPECT_EQ(policy.operations()[0].width, 1U);
    EXPECT_EQ(policy.operations()[1].width, 2U);
    EXPECT_EQ(policy.operations()[2].width, 4U);
}
} // namespace
