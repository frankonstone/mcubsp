#pragma once

// @req-file REQ-0015 REQ-0016 REQ-0045
#include <cstdint>
#include <utility>
#include <mcubsp/execution/deadline.hpp>

namespace mcubsp::execution {

// Borrow synchronization/notification from Policy and time from an independent
// monotonic Clock. Clock supplies now() and static ticks_per_second. A 1 kHz
// always-on clock permits intervals up to 24.8 days, including across sleep.
// The application owns clock initialization, sleep continuity and frequency.
// Neither the borrowed objects nor this adapter may move while in use.
template<class Policy, class Clock>
class monotonic_policy {
public:
    using wait_token = typename Policy::wait_token;
    static constexpr std::uint32_t ticks_per_second = Clock::ticks_per_second;
    static_assert(ticks_per_second > 0U);
    monotonic_policy(Policy& policy, Clock& clock) noexcept : policy_(policy), clock_(clock) {}
    monotonic_policy(const monotonic_policy&) = delete;
    monotonic_policy& operator=(const monotonic_policy&) = delete;
    template<class F> decltype(auto) critical(F&& function) noexcept {
        return policy_.critical(std::forward<F>(function));
    }
    [[nodiscard]] tick now() const noexcept { return static_cast<tick>(clock_.now()); }
    [[nodiscard]] bool in_isr() const noexcept { return policy_.in_isr(); }
    [[nodiscard]] wait_token prepare_wait() noexcept { return policy_.prepare_wait(); }
    void wait_once(wait_token token, deadline limit) noexcept {
        // A deadline in Clock units must never be passed to Policy. Use one
        // bounded policy wait step, then let the caller recheck Clock time.
        if (!reached(now(), limit))
            policy_.wait_once(token, deadline{static_cast<tick>(policy_.now() + 1U)});
    }
    void notify() noexcept { policy_.notify(); }
    void active_transfer_destroyed() noexcept { policy_.active_transfer_destroyed(); }
    template<class Diagnostic> void contract_violation(const Diagnostic& diagnostic) noexcept {
        if constexpr (requires { policy_.contract_violation(diagnostic); })
            policy_.contract_violation(diagnostic);
    }
private:
    Policy& policy_;
    Clock& clock_;
};

} // namespace mcubsp::execution
