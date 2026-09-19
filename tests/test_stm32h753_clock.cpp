#include <cstdint>
#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/stm32h753/clock.hpp>
#include <mcuhw/mcu/stm32h753/system.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

constexpr std::uintptr_t rcc_base = mcuhw::mcu::stm32h753::rcc_base;
using policy_type = mcuhw::testing::recording_mmio<0x20, 16>;

TEST(stm32h753_clock, accepts_reset_hsi_without_dividers,
     "REQ-0009", "REQ-0031", "REQ-0033") {
    policy_type policy{rcc_base};
    ASSERT_TRUE(policy.write<std::uint32_t>(rcc_base, 0x0000'0005U)
                    .has_value());
    policy.clear_operations();
    mcuhw::backend::stm32h753::clock_backend clock{policy};

    EXPECT_TRUE(clock.verify_hsi_64mhz().has_value());
    const auto operations = policy.operations();
    ASSERT_EQ(operations.size(), 3U);
    EXPECT_EQ(operations[0].address, rcc_base);
    EXPECT_EQ(operations[1].address, rcc_base + 0x10U);
    EXPECT_EQ(operations[2].address, rcc_base + 0x18U);
}

TEST(stm32h753_clock, rejects_an_unready_hsi, "REQ-0009", "REQ-0033") {
    policy_type policy{rcc_base};
    ASSERT_TRUE(policy.write<std::uint32_t>(rcc_base, 0x0000'0001U)
                    .has_value());
    policy.clear_operations();
    mcuhw::backend::stm32h753::clock_backend clock{policy};

    const auto result = clock.verify_hsi_64mhz();
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error()
                == mcuhw::backend::stm32h753::clock_error::hsi_not_ready);
}

TEST(stm32h753_clock, rejects_a_divided_hsi,
     "REQ-0009", "REQ-0031", "REQ-0033") {
    policy_type policy{rcc_base};
    ASSERT_TRUE(policy.write<std::uint32_t>(rcc_base, 0x0000'000DU)
                    .has_value());
    policy.clear_operations();
    mcuhw::backend::stm32h753::clock_backend clock{policy};

    const auto result = clock.verify_hsi_64mhz();
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error()
                == mcuhw::backend::stm32h753::clock_error::unexpected_divider);
}

} // namespace
