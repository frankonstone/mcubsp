#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include <mcubsp/execution/deadline.hpp>

namespace mcubsp::execution {

enum class fake_wait_stage : std::uint8_t {
    after_prepare,
    before_wait,
    during_wait,
};

// Deterministic native policy. Tests inject completion at exact wait stages and
// advance a wrapping monotonic clock without wall-clock sleeps.
// @req REQ-0015 REQ-0016 REQ-0035
class fake_policy {
public:
    using wait_token = std::uint32_t;
    using wait_hook = void (*)(void*, fake_wait_stage) noexcept;

    // @req REQ-0015 REQ-0016 REQ-0035
    template<typename Function>
    decltype(auto) critical(Function&& function) noexcept {
        ++critical_depth_;
        if constexpr (std::is_void_v<decltype(
                          std::forward<Function>(function)())>) {
            std::forward<Function>(function)();
            --critical_depth_;
        } else {
            auto result = std::forward<Function>(function)();
            --critical_depth_;
            return result;
        }
    }

    [[nodiscard]] tick now() const noexcept { return now_; }
    [[nodiscard]] bool in_isr() const noexcept { return in_isr_; }
    [[nodiscard]] bool in_critical() const noexcept {
        return critical_depth_ != 0U;
    }

    // @req REQ-0015 REQ-0016 REQ-0035
    [[nodiscard]] wait_token prepare_wait() noexcept {
        const auto token = critical([this] { return notification_epoch_; });
        invoke_hook(fake_wait_stage::after_prepare);
        return token;
    }

    // @req REQ-0015 REQ-0016 REQ-0035
    void wait_once(wait_token token, deadline limit) noexcept {
        invoke_hook(fake_wait_stage::before_wait);
        if (critical([this, token] { return notification_epoch_ != token; })
            || reached(now_, limit)) {
            return;
        }
        invoke_hook(fake_wait_stage::during_wait);
        if (!critical([this, token] { return notification_epoch_ != token; })
            && !reached(now_, limit)) {
            ++now_;
        }
    }

    // @req REQ-0015 REQ-0016
    void notify() noexcept {
        critical([this] { ++notification_epoch_; });
        ++notification_count_;
    }

    void active_transfer_destroyed() noexcept { ++destruction_violations_; }

    void set_time(tick value) noexcept { now_ = value; }
    void advance(tick amount = 1U) noexcept {
        now_ = static_cast<tick>(now_ + amount);
    }
    void set_isr(bool value) noexcept { in_isr_ = value; }
    void set_wait_hook(wait_hook hook, void* context = nullptr) noexcept {
        wait_hook_ = hook;
        wait_context_ = context;
    }

    [[nodiscard]] std::uint32_t notification_count() const noexcept {
        return notification_count_;
    }
    [[nodiscard]] std::uint32_t destruction_violations() const noexcept {
        return destruction_violations_;
    }

private:
    void invoke_hook(fake_wait_stage stage) noexcept {
        if (wait_hook_ != nullptr) {
            wait_hook_(wait_context_, stage);
        }
    }

    tick now_{};
    wait_token notification_epoch_{};
    wait_hook wait_hook_{};
    void* wait_context_{};
    std::uint32_t notification_count_{};
    std::uint32_t destruction_violations_{};
    std::uint32_t critical_depth_{};
    bool in_isr_{};
};

} // namespace mcubsp::execution
