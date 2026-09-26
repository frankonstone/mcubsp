#pragma once

// @req-file REQ-0015 REQ-0016 REQ-0019 REQ-0045
#include <chrono>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <mcuhw/io/core.hpp>

namespace mcuhw::io::detail {

// An execution policy remains the sole authority for time and synchronization.
// The adapter only guarantees bounded peripheral progress before a wait step.
template<class Policy>
class progress_policy {
public:
    using wait_token = typename Policy::wait_token;
    explicit progress_policy(Policy& policy) noexcept : policy_(policy) {}
    progress_policy(const progress_policy&) = delete;
    progress_policy& operator=(const progress_policy&) = delete;
    template<class F> decltype(auto) critical(F&& f) noexcept {
        return policy_.critical(std::forward<F>(f));
    }
    [[nodiscard]] execution::tick now() const noexcept { return policy_.now(); }
    [[nodiscard]] bool in_isr() const noexcept { return policy_.in_isr(); }
    [[nodiscard]] wait_token prepare_wait() noexcept { return policy_.prepare_wait(); }
    void wait_once(wait_token token, execution::deadline limit) noexcept {
        poll();
        policy_.wait_once(token, limit);
    }
    void notify() noexcept { policy_.notify(); }
    void active_transfer_destroyed() noexcept { policy_.active_transfer_destroyed(); }
    void set_progress(void (*function)(void*) noexcept, void* context) noexcept {
        progress_ = function;
        context_ = context;
    }
    void poll() noexcept { if (progress_) progress_(context_); }
    Policy& underlying() noexcept { return policy_; }
private:
    Policy& policy_;
    void (*progress_)(void*) noexcept {};
    void* context_{};
};

// Integer duration conversion: round up so a timeout cannot expire early.
// Bound the count before multiplication. This uses integer arithmetic only
// and never pulls floating-point conversion into an embedded application.
template<std::integral Rep, class Period>
[[nodiscard]] constexpr result<execution::tick>
duration_ticks(std::chrono::duration<Rep, Period> duration,
               std::uint32_t frequency) noexcept {
    if (frequency == 0U || duration.count() < 0)
        return std::unexpected(error{errc::invalid_timeout});
    const auto count = static_cast<std::uint64_t>(duration.count());
    // Restrict to ordinary integer nanoseconds through seconds. This keeps
    // the embedded conversion allocation-free and avoids floating runtime.
    static_assert(Period::num == 1 && Period::den <= 1'000'000'000LL,
                  "use integer seconds, milliseconds, microseconds or nanoseconds");
    constexpr auto denominator = static_cast<std::uint64_t>(Period::den);
    constexpr auto maximum = execution::maximum_deadline_interval;
    const auto maximum_count = static_cast<std::uint64_t>(maximum) * denominator / frequency;
    if (count > maximum_count)
        return std::unexpected(error{errc::invalid_timeout});
    const auto numerator = count * frequency;
    return static_cast<execution::tick>(numerator / denominator
                                       + (numerator % denominator != 0U));
}

template<class T, bool Borrowed> struct storage;
template<class T> struct storage<T, false> {
    [[no_unique_address]] T value{};
    T& get() noexcept { return value; }
};
template<class T> struct storage<T, true> {
    T& value;
    explicit storage(T& object) noexcept : value(object) {}
    T& get() noexcept { return value; }
};

} // namespace mcuhw::io::detail
