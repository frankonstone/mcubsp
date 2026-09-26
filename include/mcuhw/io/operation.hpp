#pragma once

// @req-file REQ-0017 REQ-0018 REQ-0019 REQ-0045
#include <chrono>
#include <utility>
#include <mcuhw/io/core.hpp>

namespace mcuhw::io {

template<class NativeTransfer, class Context>
class operation {
public:
    using value_type = typename NativeTransfer::take_result::value_type;
    operation(NativeTransfer&& transfer, Context& context) noexcept
        : transfer_(std::move(transfer)), context_(&context) { context_->retain(); }
    operation(const operation&) = delete;
    operation& operator=(const operation&) = delete;
    operation(operation&& other) noexcept
        : transfer_(std::move(other.transfer_)), context_(std::exchange(other.context_, nullptr)) {}
    operation& operator=(operation&&) = delete;
    ~operation() {
        // The native transfer destructor enforces the active-destruction
        // contract. Keep its context alive through that check.
        if (context_) {
            if (!transfer_.ready()) {
                context_->execution().active_transfer_destroyed();
                __builtin_trap();
            }
            context_->drop();
        }
    }
    [[nodiscard]] bool ready() const noexcept { return transfer_.ready(); }
    [[nodiscard]] result<void> cancel() noexcept {
        if (context_ && context_->execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        auto value = transfer_.cancel();
        if (!value) return std::unexpected(to_error(value.error()));
        return {};
    }
    [[nodiscard]] result<value_type> take() noexcept {
        if (context_ && context_->execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        auto value = transfer_.take();
        // Pending does not consume the transfer and must keep the runtime alive.
        if (value || value.error().kind != core::transfer_error_kind::pending) drop();
        if (!value) return std::unexpected(to_error(value.error()));
        if constexpr (!std::is_void_v<value_type>) return std::move(*value);
        else return {};
    }
    [[nodiscard]] result<void> wait_until(execution::deadline limit) noexcept {
        const auto value = transfer_.wait_until(limit);
        if (!value) return std::unexpected(to_error(value.error()));
        return {};
    }
    [[nodiscard]] result<value_type> finish_until(execution::deadline limit) noexcept {
        if (!context_) return std::unexpected(error{errc::invalid_operation});
        if (context_->execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        const auto waited = transfer_.wait_until(limit);
        // wait_from_isr cannot happen after the guard in this single-core
        // policy. Other terminal results must be consumed, including timeout.
        if (!waited && !transfer_.ready()) return std::unexpected(to_error(waited.error()));
        return take();
    }
    template<class Rep, class Period>
    [[nodiscard]] result<value_type> finish(std::chrono::duration<Rep, Period> timeout) noexcept {
        if (!context_) return std::unexpected(error{errc::invalid_operation});
        const auto deadline = context_->deadline_after(timeout);
        if (!deadline) return std::unexpected(deadline.error());
        return finish_until(*deadline);
    }
private:
    void drop() noexcept { if (context_) { context_->drop(); context_ = nullptr; } }
    NativeTransfer transfer_;
    Context* context_;
};

template<class Started, class Context>
[[nodiscard]] auto wrap_operation(Started&& started, Context& context) noexcept {
    using transfer_type = typename std::remove_cvref_t<Started>::value_type;
    using result_type = result<operation<transfer_type, Context>>;
    if (!started) return result_type{std::unexpected(to_error(started.error()))};
    return result_type{std::in_place, std::move(*started), context};
}

} // namespace mcuhw::io
