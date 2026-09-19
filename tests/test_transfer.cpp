#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/core/transfer.hpp>
#include <mcuhw/execution/bare_metal_policy.hpp>
#include <mcuhw/execution/deadline.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {

enum class device_error : std::uint8_t { transport, overrun };

using fake_policy = mcuhw::execution::fake_policy;
using value_pool = mcuhw::core::operation_pool<int, device_error, 1,
                                               fake_policy>;
using void_pool = mcuhw::core::operation_pool<void, device_error, 1,
                                              fake_policy>;

struct quiesce_context {
    std::uint32_t calls{};
    mcuhw::core::quiesce_result result{
        mcuhw::core::quiesce_result::reusable};
    value_pool* complete_pool{};
    bool late_completion_published{};
};

mcuhw::core::quiesce_result quiesce_value(
    void* opaque, mcuhw::core::operation_id id) noexcept {
    auto& context = *static_cast<quiesce_context*>(opaque);
    ++context.calls;
    if (context.complete_pool != nullptr) {
        context.late_completion_published =
            context.complete_pool->complete(id, 99);
    }
    return context.result;
}

struct wait_context {
    fake_policy* policy;
    value_pool* pool;
    mcuhw::core::operation_id id;
    mcuhw::execution::fake_wait_stage selected;
    bool fired{};
    bool ran_in_critical{};
};

void complete_at_wait_stage(
    void* opaque, mcuhw::execution::fake_wait_stage stage) noexcept {
    auto& context = *static_cast<wait_context*>(opaque);
    if (!context.fired && stage == context.selected) {
        context.fired = true;
        context.ran_in_critical = context.policy->in_critical();
        static_cast<void>(context.pool->complete(context.id, 42));
    }
}

struct transfer_bare_hooks {
    using critical_token = std::uint32_t;
    static inline mcuhw::execution::tick clock{};
    static inline std::uint32_t depth{};
    static inline std::uint32_t polls{};

    static void reset() noexcept {
        clock = 0;
        depth = 0;
        polls = 0;
    }
    static critical_token enter_critical() noexcept {
        const auto previous = depth;
        ++depth;
        return previous;
    }
    static void leave_critical(critical_token previous) noexcept {
        depth = previous;
    }
    static mcuhw::execution::tick now() noexcept { return clock; }
    static bool in_isr() noexcept { return false; }
    static void poll() noexcept {
        ++polls;
        ++clock;
    }
    static void notify() noexcept {}
    static void active_transfer_destroyed() noexcept {}
};

} // namespace

TEST(transfer, void_completion_is_inspectable_and_consumed_once,
     "REQ-0017", "REQ-0020") {
    fake_policy policy;
    void_pool pool{policy};
    auto started = pool.try_start({.quiesce = void_pool::already_quiescent});
    ASSERT_TRUE(started.has_value());
    auto operation = std::move(*started);

    EXPECT_FALSE(operation.ready());
    const auto pending = operation.inspect();
    ASSERT_FALSE(pending.has_value());
    EXPECT_TRUE(pending.error().kind
                == mcuhw::core::transfer_error_kind::pending);

    EXPECT_TRUE(pool.complete(operation.id()));
    EXPECT_FALSE(pool.complete(operation.id()));
    EXPECT_TRUE(operation.ready());
    EXPECT_TRUE(operation.inspect().has_value());
    EXPECT_TRUE(operation.inspect().has_value());
    EXPECT_TRUE(operation.take().has_value());

    const auto consumed = operation.take();
    ASSERT_FALSE(consumed.has_value());
    EXPECT_TRUE(consumed.error().kind
                == mcuhw::core::transfer_error_kind::consumed);
}

TEST(transfer, value_storage_stays_stable_when_handle_moves,
     "REQ-0017", "REQ-0018") {
    fake_policy policy;
    value_pool pool{policy};
    auto started = pool.try_start({.quiesce = value_pool::already_quiescent});
    ASSERT_TRUE(started.has_value());
    const auto id = started->id();
    auto moved = std::move(*started);
    EXPECT_FALSE(started->valid());

    EXPECT_TRUE(pool.complete(id, 73));
    const auto completion_won = moved.cancel();
    ASSERT_TRUE(completion_won.has_value());
    EXPECT_FALSE(*completion_won);
    const auto first = moved.inspect();
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(**first, 73);
    const auto second = moved.inspect();
    ASSERT_TRUE(second.has_value());
    EXPECT_TRUE(*first == *second);

    const auto consumed = moved.take();
    ASSERT_TRUE(consumed.has_value());
    EXPECT_EQ(*consumed, 73);
}

TEST(transfer, typed_failure_is_repeatable_then_consumed_once,
     "REQ-0017") {
    fake_policy policy;
    value_pool pool{policy};
    auto started = pool.try_start({.quiesce = value_pool::already_quiescent});
    ASSERT_TRUE(started.has_value());
    auto operation = std::move(*started);
    EXPECT_TRUE(pool.fail(operation.id(), device_error::overrun));

    const auto first = operation.inspect();
    ASSERT_FALSE(first.has_value());
    EXPECT_TRUE(first.error().kind
                == mcuhw::core::transfer_error_kind::failure);
    ASSERT_TRUE(first.error().cause.has_value());
    EXPECT_TRUE(*first.error().cause == device_error::overrun);
    const auto repeated = operation.inspect();
    EXPECT_TRUE(repeated.error() == first.error());

    const auto taken = operation.take();
    ASSERT_FALSE(taken.has_value());
    EXPECT_TRUE(taken.error().kind
                == mcuhw::core::transfer_error_kind::failure);
    EXPECT_TRUE(operation.inspect().error().kind
                == mcuhw::core::transfer_error_kind::consumed);
}

TEST(transfer, fixed_capacity_and_generation_reject_late_completion,
     "REQ-0014", "REQ-0018", "REQ-0019", "REQ-0035") {
    fake_policy policy;
    value_pool pool{policy};
    quiesce_context quiesce;
    auto first = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    ASSERT_TRUE(first.has_value());
    auto operation = std::move(*first);
    const auto old_id = operation.id();

    const auto exhausted = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    ASSERT_FALSE(exhausted.has_value());
    EXPECT_TRUE(exhausted.error().kind
                == mcuhw::core::transfer_error_kind::resource_exhausted);

    const auto cancelled = operation.cancel();
    ASSERT_TRUE(cancelled.has_value());
    EXPECT_TRUE(*cancelled);
    EXPECT_EQ(quiesce.calls, 1U);
    EXPECT_TRUE(operation.take().error().kind
                == mcuhw::core::transfer_error_kind::cancelled);

    auto second = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    ASSERT_TRUE(second.has_value());
    auto reused = std::move(*second);
    EXPECT_NE(old_id.generation, reused.id().generation);
    EXPECT_FALSE(pool.complete(old_id, 1));
    EXPECT_TRUE(pool.complete(reused.id(), 2));
    EXPECT_EQ(*reused.take(), 2);
}

TEST(transfer, wait_protocol_accepts_completion_at_every_race_window,
     "REQ-0016", "REQ-0017", "REQ-0035") {
    constexpr std::array stages{
        mcuhw::execution::fake_wait_stage::after_prepare,
        mcuhw::execution::fake_wait_stage::before_wait,
        mcuhw::execution::fake_wait_stage::during_wait,
    };

    for (const auto stage : stages) {
        fake_policy policy;
        value_pool pool{policy};
        auto started = pool.try_start(
            {.quiesce = value_pool::already_quiescent});
        ASSERT_TRUE(started.has_value());
        auto operation = std::move(*started);
        wait_context context{
            .policy = &policy,
            .pool = &pool,
            .id = operation.id(),
            .selected = stage,
        };
        policy.set_wait_hook(complete_at_wait_stage, &context);
        const auto result = operation.wait_until(
            *mcuhw::execution::make_deadline(policy.now(), 10U));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(**result, 42);
        EXPECT_TRUE(context.fired);
        EXPECT_FALSE(context.ran_in_critical);
        EXPECT_EQ(*operation.take(), 42);
    }
}

TEST(transfer, completion_before_wait_registration_is_observed,
     "REQ-0016", "REQ-0017") {
    fake_policy policy;
    value_pool pool{policy};
    auto started = pool.try_start({.quiesce = value_pool::already_quiescent});
    ASSERT_TRUE(started.has_value());
    auto operation = std::move(*started);
    EXPECT_TRUE(pool.complete(operation.id(), 8));

    const auto result = operation.wait_until(
        *mcuhw::execution::make_deadline(policy.now(), 1U));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(**result, 8);
    EXPECT_EQ(*operation.take(), 8);
}

TEST(transfer, timeout_wraps_quiesces_and_allows_reuse,
     "REQ-0018", "REQ-0019", "REQ-0035") {
    fake_policy policy;
    policy.set_time(std::numeric_limits<mcuhw::execution::tick>::max() - 2U);
    value_pool pool{policy};
    quiesce_context quiesce;
    auto started = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    ASSERT_TRUE(started.has_value());
    auto operation = std::move(*started);
    const auto limit = mcuhw::execution::make_deadline(policy.now(), 4U);
    ASSERT_TRUE(limit.has_value());

    const auto result = operation.wait_until(*limit);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_EQ(policy.now(), 1U);
    EXPECT_EQ(quiesce.calls, 1U);
    EXPECT_TRUE(operation.take().error().kind
                == mcuhw::core::transfer_error_kind::timeout);

    const auto reused = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    EXPECT_TRUE(reused.has_value());
}

TEST(transfer, cancellation_reserves_completion_and_fault_retires_slot,
     "REQ-0017", "REQ-0018", "REQ-0019", "REQ-0035") {
    fake_policy policy;
    value_pool pool{policy};
    quiesce_context quiesce{
        .result = mcuhw::core::quiesce_result::faulted,
        .complete_pool = &pool,
    };
    auto started = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    ASSERT_TRUE(started.has_value());
    auto operation = std::move(*started);

    const auto cancelled = operation.cancel();
    ASSERT_FALSE(cancelled.has_value());
    EXPECT_TRUE(cancelled.error().kind
                == mcuhw::core::transfer_error_kind::quiesce_failed);
    EXPECT_FALSE(quiesce.late_completion_published);
    EXPECT_TRUE(operation.inspect().error().kind
                == mcuhw::core::transfer_error_kind::quiesce_failed);
    EXPECT_TRUE(operation.take().error().kind
                == mcuhw::core::transfer_error_kind::quiesce_failed);

    const auto unavailable = pool.try_start(
        {.quiesce = quiesce_value, .context = &quiesce});
    ASSERT_FALSE(unavailable.has_value());
    EXPECT_TRUE(unavailable.error().kind
                == mcuhw::core::transfer_error_kind::resource_exhausted);
}

TEST(transfer, active_destruction_is_rejected_and_isr_wait_is_forbidden,
     "REQ-0016", "REQ-0018") {
    fake_policy policy;
    value_pool pool{policy};
    {
        auto started = pool.try_start(
            {.quiesce = value_pool::already_quiescent});
        ASSERT_TRUE(started.has_value());
        auto operation = std::move(*started);
        policy.set_isr(true);
        const auto wait = operation.wait_until(
            *mcuhw::execution::make_deadline(policy.now(), 2U));
        ASSERT_FALSE(wait.has_value());
        EXPECT_TRUE(wait.error().kind
                    == mcuhw::core::transfer_error_kind::wait_from_isr);
        policy.set_isr(false);
    }
    EXPECT_EQ(policy.destruction_violations(), 1U);
    const auto retired = pool.try_start(
        {.quiesce = value_pool::already_quiescent});
    EXPECT_FALSE(retired.has_value());
}

TEST(transfer, bare_metal_policy_drives_bounded_polling_timeout,
     "REQ-0015", "REQ-0016", "REQ-0019", "REQ-0041") {
    transfer_bare_hooks::reset();
    using policy_type =
        mcuhw::execution::bare_metal_policy<transfer_bare_hooks>;
    using pool_type =
        mcuhw::core::operation_pool<void, device_error, 1, policy_type>;
    policy_type policy;
    pool_type pool{policy};
    auto started = pool.try_start({.quiesce = pool_type::already_quiescent});
    ASSERT_TRUE(started.has_value());
    auto operation = std::move(*started);

    const auto result = operation.wait_until(
        *mcuhw::execution::make_deadline(transfer_bare_hooks::clock, 3U));
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_EQ(transfer_bare_hooks::polls, 3U);
    EXPECT_TRUE(operation.take().error().kind
                == mcuhw::core::transfer_error_kind::timeout);
}

static_assert(!std::is_copy_constructible_v<value_pool::transfer_type>);
static_assert(std::is_move_constructible_v<value_pool::transfer_type>);
