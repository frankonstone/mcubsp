#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0045
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <span>
#include <mcubsp/io/execution.hpp>

namespace mcubsp::io::detail {

// Shared by every facade view of one target, independent of configuration,
// MMIO or execution type. Raw expert backends are outside this composition.
template<class Board> inline bool target_active{};
template<class Board, class = void> struct target_key { using type = Board; };
template<class Board> struct target_key<Board, std::void_t<typename Board::target_identity>> {
    using type = typename Board::target_identity;
};

template<class Board, class Mmio, class Policy>
class context {
    using target = typename target_key<Board>::type;
public:
    using board_type = Board;
    using mmio_type = Mmio;
    using execution_type = progress_policy<Policy>;
    static constexpr auto default_timeout = std::chrono::milliseconds{1000};

    context(Mmio& mmio, Policy& policy, std::uint32_t frequency) noexcept
        : mmio_(mmio), execution_(policy), frequency_(frequency) {}
    context(const context&) = delete;
    context& operator=(const context&) = delete;

    Mmio& mmio() noexcept { return mmio_; }
    execution_type& execution() noexcept { return execution_; }
    const execution_type& execution() const noexcept { return execution_; }
    [[nodiscard]] bool started() const noexcept { return started_; }

    [[nodiscard]] result<void> activate() noexcept {
        if (execution_.in_isr()) return std::unexpected(error{errc::invalid_context});
        if (frequency_ == 0U) return std::unexpected(error{errc::invalid_timeout});
        return execution_.critical([this]() -> result<void> {
            if (started_) return std::unexpected(error{errc::already_started});
            if (target_active<target>) return std::unexpected(error{errc::busy});
            target_active<target> = true;
            started_ = true;
            return {};
        });
    }
    [[nodiscard]] result<void> deactivate() noexcept {
        return execution_.critical([this]() -> result<void> {
            if (!started_) return {};
            if (handles_ != 0U) return std::unexpected(error{errc::busy});
            for (auto word : pins_) if (word != 0U) return std::unexpected(error{errc::busy});
            started_ = false;
            target_active<target> = false;
            return {};
        });
    }

    [[nodiscard]] result<void> claim(std::span<const core::pin_id> pins) noexcept {
        if (execution_.in_isr()) return std::unexpected(error{errc::invalid_context});
        return execution_.critical([&]() -> result<void> {
            if (!started_) return std::unexpected(error{errc::not_started});
            for (std::size_t i = 0; i < pins.size(); ++i) {
                const auto index = Board::pin_index(pins[i]);
                if (index >= Board::pin_slots) return std::unexpected(error{errc::invalid_pin});
                if ((pins_[index / 32U] & (1U << (index % 32U))) != 0U)
                    return std::unexpected(error{errc::pin_in_use});
                for (std::size_t j = 0; j < i; ++j)
                    if (Board::pin_index(pins[j]) == index)
                        return std::unexpected(error{errc::pin_in_use});
            }
            for (auto pin : pins) {
                const auto index = Board::pin_index(pin);
                pins_[index / 32U] |= 1U << (index % 32U);
            }
            return {};
        });
    }
    void release(std::span<const core::pin_id> pins) noexcept {
        execution_.critical([&] {
            for (auto pin : pins) {
                const auto index = Board::pin_index(pin);
                if (index < Board::pin_slots)
                    pins_[index / 32U] &= ~(1U << (index % 32U));
            }
        });
    }
    void retain() noexcept {
        execution_.critical([this] {
            if (handles_ == 0xFFFFU) __builtin_trap();
            ++handles_;
        });
    }
    void drop() noexcept {
        execution_.critical([this] {
            if (handles_ == 0U) __builtin_trap();
            --handles_;
        });
    }
    template<class Rep, class Period>
    [[nodiscard]] result<mcubsp::execution::deadline>
    deadline_after(std::chrono::duration<Rep, Period> interval) noexcept {
        if (!started_) return std::unexpected(error{errc::not_started});
        if (execution_.in_isr()) return std::unexpected(error{errc::invalid_context});
        auto ticks = duration_ticks(interval, frequency_);
        if (!ticks) return std::unexpected(ticks.error());
        return mcubsp::execution::deadline{static_cast<mcubsp::execution::tick>(execution_.now() + *ticks)};
    }
private:
    Mmio& mmio_;
    execution_type execution_;
    std::array<std::uint32_t, (Board::pin_slots + 31U) / 32U> pins_{};
    std::uint32_t frequency_;
    std::uint16_t handles_{};
    bool started_{};
};

} // namespace mcubsp::io::detail
