#include "source_backed_examples.hpp"

#include <array>
#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcuhw/core/protected_access.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

TEST(protected_access, preserves_ra4m1_unlock_write_relock_order,
     "REQ-0009") {
    constexpr std::uintptr_t pfs_base = 0x4004'0800U;
    constexpr std::size_t pwpr_offset = 0x503U;
    constexpr std::array steps{
        mcuhw::core::write_step{pwpr_offset, mcuhw::core::access_width::bits8,
                                0x00U},
        mcuhw::core::write_step{pwpr_offset, mcuhw::core::access_width::bits8,
                                0x40U},
        mcuhw::core::write_step{0x00U, mcuhw::core::access_width::bits32,
                                0x0001'0004U},
        mcuhw::core::write_step{pwpr_offset, mcuhw::core::access_width::bits8,
                                0x00U},
        mcuhw::core::write_step{pwpr_offset, mcuhw::core::access_width::bits8,
                                0x80U},
    };
    mcuhw::testing::recording_mmio<0x508, 8> policy{pfs_base};

    ASSERT_TRUE(mcuhw::core::execute_write_sequence(
                    policy, pfs_base, std::span<const mcuhw::core::write_step>{steps})
                    .has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 5U);
    EXPECT_EQ(operations[0].address, pfs_base + pwpr_offset);
    EXPECT_EQ(operations[0].width, 1U);
    EXPECT_EQ(operations[0].value, 0x00U);
    EXPECT_EQ(operations[1].value, 0x40U);
    EXPECT_EQ(operations[2].address, pfs_base);
    EXPECT_EQ(operations[2].width, 4U);
    EXPECT_EQ(operations[2].value, 0x0001'0004U);
    EXPECT_EQ(operations[3].value, 0x00U);
    EXPECT_EQ(operations[4].value, 0x80U);
    for (const auto& operation : operations) {
        EXPECT_TRUE(operation.kind == mcuhw::testing::mmio_operation_kind::write);
    }
}

TEST(protected_access, rejects_values_wider_than_the_declared_access,
     "REQ-0009") {
    constexpr std::array steps{
        mcuhw::core::write_step{0, mcuhw::core::access_width::bits8, 0x100U},
    };
    mcuhw::testing::recording_mmio<4, 1> policy{0x4000'0000U};
    const auto result = mcuhw::core::execute_write_sequence(
        policy, 0x4000'0000U,
        std::span<const mcuhw::core::write_step>{steps});
    EXPECT_TRUE(!result.has_value());
    EXPECT_TRUE(result.error() == mcuhw::core::mmio_error::invalid_value);
    EXPECT_EQ(policy.operations().size(), 0U);
}

} // namespace
