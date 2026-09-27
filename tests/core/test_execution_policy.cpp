#include <cstdint>
#include <limits>

#include <mcutest/mcutest.hpp>

#include <mcuhw/execution/bare_metal_policy.hpp>
#include <mcuhw/execution/deadline.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {

struct bare_hooks {
    using critical_token = std::uint32_t;

    static inline mcuhw::execution::tick clock{};
    static inline std::uint32_t critical_depth{};
    static inline std::uint32_t polls{};
    static inline std::uint32_t notifications{};
    static inline bool isr{};

    static void reset() noexcept {
        clock = 0;
        critical_depth = 0;
        polls = 0;
        notifications = 0;
        isr = false;
    }

    [[nodiscard]] static critical_token enter_critical() noexcept {
        const auto previous = critical_depth;
        ++critical_depth;
        return previous;
    }

    static void leave_critical(critical_token previous) noexcept {
        critical_depth = previous;
    }

    [[nodiscard]] static mcuhw::execution::tick now() noexcept {
        return clock;
    }
    [[nodiscard]] static bool in_isr() noexcept { return isr; }
    static void poll() noexcept {
        ++polls;
        ++clock;
    }
    static void notify() noexcept { ++notifications; }
    static void active_transfer_destroyed() noexcept {}
};

} // namespace

TEST(deadline, validates_half_range_and_wraps_through_zero,
     "REQ-0015", "REQ-0019") {
    using mcuhw::execution::deadline;
    using mcuhw::execution::make_deadline;
    using mcuhw::execution::maximum_deadline_interval;
    using mcuhw::execution::reached;

    constexpr auto near_wrap =
        std::numeric_limits<mcuhw::execution::tick>::max() - 2U;
    const auto wrapped = make_deadline(near_wrap, 4U);
    ASSERT_TRUE(wrapped.has_value());
    EXPECT_EQ(wrapped->value, 1U);
    EXPECT_FALSE(reached(near_wrap, *wrapped));
    EXPECT_FALSE(reached(0U, *wrapped));
    EXPECT_TRUE(reached(1U, *wrapped));

    const auto too_far = make_deadline(0U, maximum_deadline_interval + 1U);
    EXPECT_FALSE(too_far.has_value());
    EXPECT_TRUE(reached(7U, deadline{7U}));
}

TEST(execution_policy, bare_metal_uses_target_hooks_and_notification_tokens,
     "REQ-0012", "REQ-0015", "REQ-0016", "REQ-0041") {
    bare_hooks::reset();
    mcuhw::execution::bare_metal_policy<bare_hooks> policy;

    const auto value = policy.critical([] {
        EXPECT_EQ(bare_hooks::critical_depth, 1U);
        return 17U;
    });
    EXPECT_EQ(value, 17U);
    EXPECT_EQ(bare_hooks::critical_depth, 0U);

    const auto stale = policy.prepare_wait();
    policy.notify();
    policy.wait_once(stale, mcuhw::execution::deadline{10U});
    EXPECT_EQ(bare_hooks::polls, 0U);
    EXPECT_EQ(bare_hooks::notifications, 1U);

    const auto current = policy.prepare_wait();
    policy.wait_once(current, mcuhw::execution::deadline{10U});
    EXPECT_EQ(bare_hooks::polls, 1U);
    EXPECT_EQ(bare_hooks::clock, 1U);

    bare_hooks::isr = true;
    EXPECT_TRUE(policy.in_isr());
}
