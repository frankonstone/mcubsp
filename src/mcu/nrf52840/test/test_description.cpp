#include <mcubsp/mcu/nrf52840/catalogue.hpp>
#include <mcubsp/mcu/nrf52840/product.hpp>
#include <mcubsp/mcu/nrf52840/system.hpp>
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
    EXPECT_FALSE(n::complete_product_description); // latest CC_* / mode audit is still open
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
    EXPECT_EQ(fields, 343U);
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
}
} // namespace
