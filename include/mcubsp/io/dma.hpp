#pragma once

// @req-file REQ-0013 REQ-0014 REQ-0018 REQ-0030 REQ-0045
#include <chrono>
#include <cstdint>
#include <span>
#include <tuple>
#include <utility>
#include <mcubsp/io/operation.hpp>

namespace mcubsp::io {

template<class Channel> struct dma_selection { Channel& channel; };
template<class Receive, class Transmit>
struct duplex_dma_selection { Receive& receive; Transmit& transmit; };
template<class Channel>
[[nodiscard]] dma_selection<Channel> use_dma(Channel& channel) noexcept { return {channel}; }
template<class Receive, class Transmit>
[[nodiscard]] duplex_dma_selection<Receive, Transmit>
use_dma(Receive& receive, Transmit& transmit) noexcept { return {receive, transmit}; }

namespace detail {

template<class Context, unsigned Channel>
class dma_endpoint {
    using Board = typename Context::board_type;
public:
    using backend_type = typename Board::template dma_backend<Channel,
        typename Context::mmio_type, typename Context::execution_type>;
    explicit dma_endpoint(Context& context) noexcept
        : context_(context), backend_(context.mmio(), context.execution()) {}
    dma_endpoint(const dma_endpoint&) = delete;
    dma_endpoint& operator=(const dma_endpoint&) = delete;

    class lease {
    public:
        lease(const lease&) = delete;
        lease& operator=(const lease&) = delete;
        lease(lease&& other) noexcept : endpoint_(std::exchange(other.endpoint_, nullptr)) {}
        lease& operator=(lease&&) = delete;
        ~lease() {
            if (endpoint_ && !release()) {
                endpoint_->context_.execution().fail({"dma", "live resources or failed cleanup during destruction"});
                __builtin_trap();
            }
        }
        [[nodiscard]] bool valid() const noexcept { return endpoint_ != nullptr; }
        // Borrowed expert access: the lease and runtime must outlive all uses.
        backend_type& backend() noexcept {
            if (!endpoint_) __builtin_trap();
            return endpoint_->backend_;
        }
        [[nodiscard]] auto copy_async(driver::dma_memory_copy request) noexcept {
            using R = decltype(wrap_operation(backend().copy(request), endpoint_->context_));
            if (!endpoint_) return R{std::unexpected(error{errc::not_started})};
            if (endpoint_->context_.execution().in_isr()) return R{std::unexpected(error{errc::invalid_context})};
            return wrap_operation(backend().copy(request), endpoint_->context_);
        }
        template<class Rep, class Period>
        [[nodiscard]] result<driver::dma_transfer_result> copy(
            driver::dma_memory_copy request,
            std::chrono::duration<Rep, Period> timeout) noexcept {
            if (!endpoint_) return std::unexpected(error{errc::not_started});
            const auto deadline = endpoint_->context_.deadline_after(timeout);
            if (!deadline) return std::unexpected(deadline.error());
            auto started = copy_async(request);
            if (!started) return std::unexpected(started.error());
            return started->finish_until(*deadline);
        }
        [[nodiscard]] result<void> release() noexcept {
            if (!endpoint_) return {};
            if (endpoint_->context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
            const auto stopped = endpoint_->backend_.shutdown();
            if (!stopped) return std::unexpected(to_error(stopped.error()));
            endpoint_->leased_ = false;
            endpoint_->context_.drop();
            endpoint_ = nullptr;
            return {};
        }
    private:
        friend class dma_endpoint;
        explicit lease(dma_endpoint& endpoint) noexcept : endpoint_(&endpoint) {}
        dma_endpoint* endpoint_;
    };

    [[nodiscard]] result<lease> acquire() noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!context_.started()) return std::unexpected(error{errc::not_started});
        if (leased_) return std::unexpected(error{errc::busy});
        const auto started = backend_.initialize();
        if (!started) return std::unexpected(to_error(started.error()));
        leased_ = true;
        context_.retain();
        return lease{*this};
    }
    void poll() noexcept {
        if (context_.execution().in_isr()) return;
        if (leased_) Board::template poll_dma<Channel>(context_.mmio(), backend_);
    }
private:
    Context& context_;
    backend_type backend_;
    bool leased_{};
};

template<class Context, unsigned Channel, bool Enabled>
struct dma_slot {
    explicit dma_slot(Context&) noexcept {}
    void poll() noexcept {}
};
template<class Context, unsigned Channel>
struct dma_slot<Context, Channel, true> : dma_endpoint<Context, Channel> {
    using dma_endpoint<Context, Channel>::dma_endpoint;
};

template<class Context, std::uint8_t Mask, class Sequence> class dma_storage;
template<class Context, std::uint8_t Mask, std::size_t... Channel>
class dma_storage<Context, Mask, std::index_sequence<Channel...>>
    : public dma_slot<Context, Channel, ((Mask & (1U << Channel)) != 0U)>... {
public:
    explicit dma_storage(Context& context) noexcept
        : dma_slot<Context, Channel, ((Mask & (1U << Channel)) != 0U)>(context)... {}
    template<unsigned Index>
    [[nodiscard]] auto channel() noexcept {
        static_assert(Index < 8 && (Mask & (1U << Index)),
                      "DMA channel is not selected in board_config.dma");
        return static_cast<dma_slot<Context, Index, true>&>(*this).acquire();
    }
    void poll() noexcept {
        (dma_slot<Context, Channel, ((Mask & (1U << Channel)) != 0U)>::poll(), ...);
    }
};
} // namespace detail

template<class Context, std::uint8_t Mask>
using dma_bank = detail::dma_storage<Context, Mask, std::make_index_sequence<8>>;

} // namespace mcubsp::io
