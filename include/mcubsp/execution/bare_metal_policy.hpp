#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include <mcubsp/execution/deadline.hpp>

namespace mcubsp::execution {

// Hooks bind target-owned time, interrupt masking, ISR detection and polling.
// poll() runs with the policy's critical section released. It shall make
// bounded progress and shall not use WFI without an independently validated
// event handshake. The initial contract is single-core.
template<typename Hooks>
// @req REQ-0012 REQ-0015 REQ-0016 REQ-0041
class bare_metal_policy {
public:
    using wait_token = std::uint32_t;

    // @req REQ-0015 REQ-0016 REQ-0041
    template<typename Function>
    decltype(auto) critical(Function&& function) noexcept {
        const auto token = Hooks::enter_critical();
        if constexpr (std::is_void_v<decltype(
                          std::forward<Function>(function)())>) {
            std::forward<Function>(function)();
            Hooks::leave_critical(token);
        } else {
            auto result = std::forward<Function>(function)();
            Hooks::leave_critical(token);
            return result;
        }
    }

    [[nodiscard]] tick now() const noexcept { return Hooks::now(); }
    [[nodiscard]] bool in_isr() const noexcept { return Hooks::in_isr(); }

    // @req REQ-0015 REQ-0016 REQ-0041
    [[nodiscard]] wait_token prepare_wait() noexcept {
        return critical([this] { return notification_epoch_; });
    }

    // @req REQ-0015 REQ-0016 REQ-0041
    void wait_once(wait_token token, deadline limit) noexcept {
        const bool notified = critical(
            [this, token] { return notification_epoch_ != token; });
        if (!notified && !reached(now(), limit)) {
            Hooks::poll();
        }
    }

    // @req REQ-0012 REQ-0015 REQ-0016 REQ-0041
    void notify() noexcept {
        critical([this] { ++notification_epoch_; });
        Hooks::notify();
    }

    [[noreturn]] void active_transfer_destroyed() noexcept {
        Hooks::active_transfer_destroyed();
        for (;;) {
        }
    }

private:
    wait_token notification_epoch_{};
};

} // namespace mcubsp::execution
