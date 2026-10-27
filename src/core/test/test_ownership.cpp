#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/core/ownership.hpp>
#include <mcubsp/execution/fake_policy.hpp>

TEST(peripheral_ownership, rejects_duplicate_and_invalid_claims,
     "REQ-0014") {
    mcubsp::execution::fake_policy policy;
    mcubsp::core::peripheral_ownership<2, mcubsp::execution::fake_policy>
        ownership{policy};

    auto first = ownership.try_claim(0);
    ASSERT_TRUE(first.has_value());
    EXPECT_TRUE(ownership.owned(0));

    const auto duplicate = ownership.try_claim(0);
    ASSERT_FALSE(duplicate.has_value());
    EXPECT_TRUE(duplicate.error()
                == mcubsp::core::ownership_error::busy);

    const auto invalid = ownership.try_claim(2);
    ASSERT_FALSE(invalid.has_value());
    EXPECT_TRUE(invalid.error()
                == mcubsp::core::ownership_error::invalid_resource);
    EXPECT_FALSE(ownership.owned(2));
}

TEST(peripheral_ownership, move_preserves_unique_claim_and_release_reuses_it,
     "REQ-0014") {
    mcubsp::execution::fake_policy policy;
    mcubsp::core::peripheral_ownership<1, mcubsp::execution::fake_policy>
        ownership{policy};

    auto claimed = ownership.try_claim(0);
    ASSERT_TRUE(claimed.has_value());
    auto moved = std::move(*claimed);
    EXPECT_FALSE(claimed->owns_resource());
    EXPECT_TRUE(moved.owns_resource());

    moved.release();
    EXPECT_FALSE(ownership.owned(0));
    const auto reused = ownership.try_claim(0);
    EXPECT_TRUE(reused.has_value());
}
