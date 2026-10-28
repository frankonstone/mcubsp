#include "support/source_backed_examples.hpp"

#include <cstdint>
#include <limits>
#include <mcutest/mcutest.hpp>

#include <mcubsp/core/register.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {

using namespace mcubsp;
using host_policy = testing::recording_mmio<512, 16>;

template<typename Reference>
concept supports_read = requires(Reference& reference) { reference.read(); };

template<typename Reference>
concept supports_modify = requires(Reference& reference) {
    reference.modify(typename Reference::value_type{},
                     typename Reference::value_type{});
};

template<typename Reference>
concept supports_write = requires(Reference& reference) {
    reference.write(typename Reference::value_type{});
};

TEST(register_metadata, represents_real_width_alias_and_reset_differences,
     "REQ-0007") {
    using namespace contract_trials::ra4m1;
    EXPECT_EQ(p000pfs::width, 4U);
    EXPECT_EQ(p000pfs_ha::width, 2U);
    EXPECT_EQ(p000pfs_by::width, 1U);
    EXPECT_TRUE((std::same_as<p000pfs_ha::canonical_register, p000pfs>));
    EXPECT_TRUE((std::same_as<p000pfs_by::canonical_register, p000pfs>));
    EXPECT_TRUE(!p000pfs::reset_fully_known);
    EXPECT_TRUE(pidr::access == core::access_semantics::read_only);
    EXPECT_EQ(psel::mask, 0x1F00'0000U);
}

TEST(field_encoding, handles_full_width_and_rejects_invalid_enumerators,
     "REQ-0007", "REQ-0008") {
    using full_register = core::register_descriptor<
        std::uint32_t, 0, core::access_semantics::read_write>;
    using full_field = core::field_descriptor<full_register, 0, 32>;
    static_assert(full_field::mask == std::numeric_limits<std::uint32_t>::max());

    using drive = contract_trials::ra4m1::p408_drive;
    using drive_field = contract_trials::ra4m1::p408_drive_field;
    static_assert(drive_field::template encode_constant<drive::iic_fast>()
                  == 0x800U);
    const auto prohibited = drive_field::try_encode(drive::prohibited);
    EXPECT_TRUE(!prohibited.has_value());
    EXPECT_TRUE(prohibited.error() == core::mmio_error::invalid_value);
}

TEST(register_access, w1c_clear_has_no_unwanted_read, "REQ-0008") {
    constexpr std::uintptr_t exti_base = 0x5800'0000U;
    host_policy policy{exti_base};
    core::register_ref<contract_trials::stm32h753::exti_cpupr1, host_policy>
        pending{policy, exti_base};

    ASSERT_TRUE(pending.clear_bits(1U << 17U).has_value());
    ASSERT_EQ(policy.operations().size(), 1U);
    EXPECT_TRUE(policy.operations()[0].kind
                == testing::mmio_operation_kind::write);
    EXPECT_EQ(policy.operations()[0].address, exti_base + 0x88U);
    EXPECT_EQ(policy.operations()[0].width, 4U);
    EXPECT_EQ(policy.operations()[0].value, 1U << 17U);

    const auto reserved = pending.clear_bits(1U << 22U);
    EXPECT_TRUE(!reserved.has_value());
    EXPECT_EQ(policy.operations().size(), 1U);
}

TEST(register_access, set_and_clear_aliases_are_write_only_commands,
     "REQ-0008") {
    constexpr std::uintptr_t gpio_base = 0x6000'0000U;
    testing::recording_mmio<32, 4> policy{gpio_base};
    using set_reference = core::register_ref<
        contract_trials::esp32p4::gpio_out_w1ts, decltype(policy)>;
    using clear_reference = core::register_ref<
        contract_trials::esp32p4::gpio_out_w1tc, decltype(policy)>;
    static_assert(!supports_read<set_reference>);
    static_assert(!supports_read<clear_reference>);

    set_reference set{policy, gpio_base};
    clear_reference clear{policy, gpio_base};
    ASSERT_TRUE(set.issue(1U << 5U).has_value());
    ASSERT_TRUE(clear.issue(1U << 7U).has_value());
    ASSERT_EQ(policy.operations().size(), 2U);
    EXPECT_EQ(policy.operations()[0].address, gpio_base + 0x08U);
    EXPECT_EQ(policy.operations()[1].address, gpio_base + 0x0CU);
}

TEST(register_access, rmw_is_available_only_when_declared_safe, "REQ-0008") {
    using safe_register = core::register_descriptor<
        std::uint32_t, 0, core::access_semantics::read_write,
        0xFFU, true, core::reserved_bit_policy::preserve>;
    using unsafe_reference = core::register_ref<
        contract_trials::stm32h753::exti_cpupr1, host_policy>;
    using safe_reference = core::register_ref<safe_register, host_policy>;
    static_assert(!supports_modify<unsafe_reference>);
    static_assert(supports_modify<safe_reference>);

    constexpr std::uintptr_t base = 0x4000'0000U;
    host_policy policy{base};
    ASSERT_TRUE(policy.write<std::uint32_t>(base, 0xA5U).has_value());
    policy.clear_operations();
    safe_reference value{policy, base};
    ASSERT_TRUE(value.modify(0x05U, 0x02U).has_value());
    ASSERT_EQ(policy.operations().size(), 2U);
    EXPECT_TRUE(policy.operations()[0].kind
                == testing::mmio_operation_kind::read);
    EXPECT_TRUE(policy.operations()[1].kind
                == testing::mmio_operation_kind::write);
    EXPECT_EQ(policy.operations()[1].value, 0xA2U);
}

TEST(register_access, distinguishes_w0c_and_read_clear_side_effects,
     "REQ-0008") {
    using w0c_register = core::register_descriptor<
        std::uint8_t, 0, core::access_semantics::write_zero_to_clear, 0x0FU>;
    using read_clear_register = core::register_descriptor<
        std::uint8_t, 1, core::access_semantics::read_clear, 0U>;
    constexpr std::uintptr_t base = 0x4000'0000U;
    testing::recording_mmio<4, 4> policy{base};
    core::register_ref<w0c_register, decltype(policy)> flags{policy, base};
    core::register_ref<read_clear_register, decltype(policy)> status{policy, base};

    ASSERT_TRUE(flags.clear_bits(0x05U).has_value());
    ASSERT_TRUE(policy.write<std::uint8_t>(base + 1U, 0xA5U).has_value());
    policy.clear_operations();
    const auto observed = status.read_and_clear();
    ASSERT_TRUE(observed.has_value());
    EXPECT_EQ(*observed, 0xA5U);
    ASSERT_EQ(policy.operations().size(), 1U);
    EXPECT_TRUE(policy.operations()[0].kind
                == testing::mmio_operation_kind::read);

    policy.clear_operations();
    ASSERT_TRUE(flags.clear_bits(0x05U).has_value());
    ASSERT_EQ(policy.operations().size(), 1U);
    EXPECT_EQ(policy.operations()[0].value, 0x0AU);
}

TEST(register_access, reserved_preserve_disables_unsafe_direct_write,
     "REQ-0008") {
    using preserve_register = core::register_descriptor<
        std::uint32_t, 0, core::access_semantics::read_write,
        0xFFU, true, core::reserved_bit_policy::preserve>;
    using preserve_reference = core::register_ref<preserve_register, host_policy>;
    static_assert(!supports_write<preserve_reference>);
    static_assert(supports_modify<preserve_reference>);
    EXPECT_TRUE(true);
}

TEST(register_access, read_write_data_port_requires_explicit_side_effect_read,
     "REQ-0007", "REQ-0008") {
    using data_port = core::register_descriptor<
        std::uint32_t, 0, core::access_semantics::read_write_side_effect,
        0xFFU, true>; // Even an erroneous RMW-safe flag must not permit modify.
    using reference = core::register_ref<data_port, host_policy>;
    static_assert(!supports_read<reference>);
    static_assert(!supports_modify<reference>);
    static_assert(supports_write<reference>);
    constexpr std::uintptr_t base = 0x5002BF00;
    host_policy policy{base};
    reference data{policy, base};
    ASSERT_TRUE(data.write(0xA5).has_value());
    EXPECT_FALSE(data.write(0x100).has_value());
    auto value = data.read_with_side_effect();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 0xA5U);
    ASSERT_EQ(policy.operations().size(), 2U);
    EXPECT_TRUE(policy.operations()[0].kind == testing::mmio_operation_kind::write);
    EXPECT_TRUE(policy.operations()[1].kind == testing::mmio_operation_kind::read);
    EXPECT_EQ(policy.operations()[1].width, 4U);
}

} // namespace
