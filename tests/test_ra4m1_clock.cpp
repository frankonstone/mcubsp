#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/clock.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

constexpr std::uintptr_t system_base = 0x4001'E000U;
using policy_type = mcuhw::testing::recording_mmio<0x400, 32>;

TEST(ra4m1_clock, applies_verified_48mhz_sequence_and_relocks,
     "REQ-0009", "REQ-0031") {
    policy_type policy{system_base};
    ASSERT_TRUE(policy.write<std::uint8_t>(system_base + 0x37U, 0x20U)
                    .has_value());
    ASSERT_TRUE(policy.write<std::uint8_t>(system_base + 0x3CU, 0x01U)
                    .has_value());
    policy.clear_operations();
    mcuhw::backend::ra4m1::clock_backend backend{policy};

    ASSERT_TRUE(backend.configure_hoco_48mhz(2U).has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 13U);
    EXPECT_EQ(operations[0].address, system_base + 0x37U);
    EXPECT_EQ(operations[1].address, system_base + 0x3FEU);
    EXPECT_EQ(operations[1].value, 0xA503U);
    EXPECT_EQ(operations[2].address, system_base + 0x36U);
    EXPECT_EQ(operations[3].address, system_base + 0x3CU);
    EXPECT_EQ(operations[4].address, system_base + 0xA0U);
    EXPECT_EQ(operations[5].address, system_base + 0xA0U);
    EXPECT_EQ(operations[6].address, system_base + 0x31U);
    EXPECT_EQ(operations[6].value, 1U);
    EXPECT_EQ(operations[7].address, system_base + 0x20U);
    EXPECT_EQ(operations[7].value, 0x1001'0100U);
    EXPECT_EQ(operations[8].address, system_base + 0x26U);
    EXPECT_EQ(operations[9].address, system_base + 0x26U);
    EXPECT_EQ(operations[10].address, system_base + 0x20U);
    EXPECT_EQ(operations[11].address, system_base + 0x31U);
    EXPECT_EQ(operations[12].address, system_base + 0x3FEU);
    EXPECT_EQ(operations[12].value, 0xA500U);
}

TEST(ra4m1_clock, refuses_a_non_48mhz_option_setting_without_writes,
     "REQ-0009", "REQ-0031") {
    policy_type policy{system_base};
    ASSERT_TRUE(policy.write<std::uint8_t>(system_base + 0x37U, 0x10U)
                    .has_value());
    policy.clear_operations();
    mcuhw::backend::ra4m1::clock_backend backend{policy};

    const auto result = backend.configure_hoco_48mhz(2U);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error()
                == mcuhw::backend::ra4m1::clock_error::incompatible_hoco_frequency);
    ASSERT_EQ(policy.operations().size(), 1U);
    EXPECT_TRUE(policy.operations()[0].kind
                == mcuhw::testing::mmio_operation_kind::read);
}

} // namespace
