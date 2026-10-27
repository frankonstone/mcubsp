#include <mcubsp/testing/description_suite.hpp>

#include <cstddef>
#include <cstdint>
#include <mcutest/mcutest.hpp>

namespace {

struct counting_contract {
    std::size_t& invocation_count;

    template<typename Description>
    bool operator()() {
        static_assert(sizeof(Description) > 0);
        ++invocation_count;
        return true;
    }
};

TEST(description_suite, applies_one_contract_to_every_supplied_type) {
    std::size_t invocation_count{};
    const auto accepted = mcubsp::testing::for_each_description<
        std::uint8_t,
        std::uint16_t,
        std::uint32_t>(counting_contract{invocation_count});

    EXPECT_TRUE(accepted);
    EXPECT_EQ(invocation_count, 3U);
}

} // namespace
