#include <mcuhw/testing/recording_mmio.hpp>

#include <cstdint>
#include <mcutest/mcutest.hpp>
#include <mcutest/output.hpp>

namespace {

constexpr std::uintptr_t peripheral_base = 0x4000'0000U;
using policy = mcuhw::testing::recording_mmio<32, 8>;

TEST(recording_mmio, records_width_value_and_order, "REQ-0008") {
    policy mmio{peripheral_base};

    const auto write8 = mmio.write<std::uint8_t>(peripheral_base, 0xA5U);
    const auto write32 = mmio.write<std::uint32_t>(peripheral_base + 4U,
                                                  0x1234'5678U);
    EXPECT_TRUE(write8.has_value());
    EXPECT_TRUE(write32.has_value());

    const auto operations = mmio.operations();
    EXPECT_EQ(operations.size(), 2U);
    EXPECT_TRUE(operations[0].kind
                == mcuhw::testing::mmio_operation_kind::write);
    EXPECT_EQ(operations[0].width, 1U);
    EXPECT_EQ(operations[0].value, 0xA5U);
    EXPECT_TRUE(operations[1].kind
                == mcuhw::testing::mmio_operation_kind::write);
    EXPECT_EQ(operations[1].address, peripheral_base + 4U);
    EXPECT_EQ(operations[1].width, 4U);
    EXPECT_EQ(operations[1].value, 0x1234'5678U);
}

TEST(recording_mmio, reads_local_storage_not_target_memory, "REQ-0008") {
    policy mmio{peripheral_base};
    ASSERT_TRUE(mmio.write<std::uint32_t>(peripheral_base + 8U,
                                         0xCAFE'BABEU).has_value());
    mmio.clear_operations();

    const auto value = mmio.read<std::uint32_t>(peripheral_base + 8U);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 0xCAFE'BABEU);
    ASSERT_EQ(mmio.operations().size(), 1U);
    EXPECT_TRUE(mmio.operations()[0].kind
                == mcuhw::testing::mmio_operation_kind::read);
}

TEST(recording_mmio, rejects_invalid_access_without_trace, "REQ-0008") {
    policy mmio{peripheral_base};

    const auto misaligned = mmio.read<std::uint32_t>(peripheral_base + 1U);
    const auto outside = mmio.write<std::uint32_t>(peripheral_base + 32U, 1U);
    EXPECT_TRUE(!misaligned.has_value());
    EXPECT_TRUE(misaligned.error() == mcuhw::testing::mmio_error::misaligned);
    EXPECT_TRUE(!outside.has_value());
    EXPECT_TRUE(outside.error() == mcuhw::testing::mmio_error::out_of_range);
    EXPECT_EQ(mmio.operations().size(), 0U);
}

TEST(recording_mmio, reports_fixed_trace_capacity, "REQ-0002", "REQ-0008") {
    mcuhw::testing::recording_mmio<4, 1> mmio{peripheral_base};
    ASSERT_TRUE(mmio.write<std::uint8_t>(peripheral_base, 1U).has_value());

    const auto overflow = mmio.write<std::uint8_t>(peripheral_base + 1U, 2U);
    EXPECT_TRUE(!overflow.has_value());
    EXPECT_TRUE(overflow.error() == mcuhw::testing::mmio_error::trace_full);
    EXPECT_EQ(mmio.operations().size(), 1U);

    mmio.clear_operations();
    const auto untouched = mmio.read<std::uint8_t>(peripheral_base + 1U);
    ASSERT_TRUE(untouched.has_value());
    EXPECT_EQ(*untouched, 0U);
}

} // namespace

int main() {
    mcutest::Runner<mcutest::JsonOutput> runner;
    return mcutest::run_json_output(runner);
}
