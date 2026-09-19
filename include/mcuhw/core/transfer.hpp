#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

#include <mcuhw/execution/deadline.hpp>

namespace mcuhw::core {

struct operation_id {
    std::uint16_t slot;
    std::uint32_t generation;

    friend constexpr bool operator==(operation_id, operation_id) = default;
};

enum class quiesce_result : std::uint8_t {
    reusable,
    faulted,
};

enum class transfer_error_kind : std::uint8_t {
    pending,
    failure,
    timeout,
    cancelled,
    consumed,
    invalid_handle,
    invalid_operation,
    resource_exhausted,
    wait_from_isr,
    quiesce_failed,
};

template<typename Error>
struct transfer_error {
    transfer_error_kind kind;
    std::optional<Error> cause{};

    friend constexpr bool operator==(const transfer_error&,
                                     const transfer_error&) = default;
};

namespace detail {

enum class operation_state : std::uint8_t {
    free,
    pending,
    cancelling,
    succeeded,
    failed,
    timed_out,
    cancelled,
    faulted,
    retired,
};

template<typename T>
struct operation_value {
    std::optional<T> stored{};

    void reset_value() noexcept { stored.reset(); }
};

template<>
struct operation_value<void> {
    void reset_value() noexcept {}
};

template<typename Error>
[[nodiscard]] constexpr transfer_error<Error>
make_transfer_error(transfer_error_kind kind) noexcept {
    return transfer_error<Error>{.kind = kind};
}

template<typename Error>
[[nodiscard]] constexpr transfer_error<Error>
make_failure(Error error) noexcept {
    return transfer_error<Error>{
        .kind = transfer_error_kind::failure,
        .cause = std::move(error),
    };
}

} // namespace detail

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
class operation_pool;

// Move-only consumer for one stable operation-pool slot. Moving this handle
// never moves the state observed by completion/ISR code.
// @req REQ-0017 REQ-0018 REQ-0020
template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
class transfer {
public:
    using pool_type = operation_pool<T, Error, Capacity, ExecutionPolicy>;
    using error_type = transfer_error<Error>;
    using inspection_result = std::conditional_t<
        std::is_void_v<T>, std::expected<void, error_type>,
        std::expected<const T*, error_type>>;
    using take_result = std::conditional_t<
        std::is_void_v<T>, std::expected<void, error_type>,
        std::expected<T, error_type>>;

    transfer(const transfer&) = delete;
    transfer& operator=(const transfer&) = delete;

    transfer(transfer&& other) noexcept
        : pool_(other.pool_), id_(other.id_), consumed_(other.consumed_) {
        other.pool_ = nullptr;
        other.consumed_ = true;
    }

    transfer& operator=(transfer&&) = delete;

    ~transfer();

    [[nodiscard]] bool ready() const noexcept;
    [[nodiscard]] inspection_result inspect() const noexcept;
    [[nodiscard]] take_result take() noexcept;
    [[nodiscard]] inspection_result
    wait_until(execution::deadline limit) noexcept;
    [[nodiscard]] std::expected<bool, error_type> cancel() noexcept;

    [[nodiscard]] operation_id id() const noexcept { return id_; }
    [[nodiscard]] bool valid() const noexcept {
        return pool_ != nullptr && !consumed_;
    }

private:
    friend class operation_pool<T, Error, Capacity, ExecutionPolicy>;

    transfer(pool_type& pool, operation_id id) noexcept
        : pool_(&pool), id_(id) {}

    pool_type* pool_{};
    operation_id id_{};
    bool consumed_{};
};

// Fixed operation storage and the single publication point shared by polling,
// blocking and ISR completion paths.
// @req REQ-0014 REQ-0016 REQ-0017 REQ-0018 REQ-0019
template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
class operation_pool {
    static_assert(Capacity > 0U);
    static_assert(Capacity
                  <= static_cast<std::size_t>(
                      std::numeric_limits<std::uint16_t>::max()));
    static_assert(std::is_void_v<T> || std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_void_v<T> || std::is_nothrow_destructible_v<T>);
    static_assert(std::is_nothrow_copy_constructible_v<Error>);
    static_assert(std::is_nothrow_move_constructible_v<Error>);
    static_assert(std::is_nothrow_destructible_v<Error>);

public:
    static constexpr std::size_t capacity = Capacity;

    using transfer_type = transfer<T, Error, Capacity, ExecutionPolicy>;
    using error_type = transfer_error<Error>;
    using inspection_result = typename transfer_type::inspection_result;
    using take_result = typename transfer_type::take_result;
    using quiesce_function = quiesce_result (*)(void*, operation_id) noexcept;

    struct start_options {
        quiesce_function quiesce;
        void* context{};
    };

    explicit operation_pool(ExecutionPolicy& policy) noexcept : policy_(policy) {}

    [[nodiscard]] static quiesce_result
    already_quiescent(void*, operation_id) noexcept {
        return quiesce_result::reusable;
    }

    // @req REQ-0014 REQ-0017 REQ-0018
    [[nodiscard]] std::expected<transfer_type, error_type>
    try_start(start_options options) noexcept {
        if (options.quiesce == nullptr) {
            return std::unexpected(detail::make_transfer_error<Error>(
                transfer_error_kind::invalid_operation));
        }
        return policy_.critical([this, options]()
                                    -> std::expected<transfer_type, error_type> {
            for (std::size_t index = 0; index < Capacity; ++index) {
                auto& current = slots_[index];
                if (current.state != detail::operation_state::free) {
                    continue;
                }
                ++current.generation;
                if (current.generation == 0U) {
                    ++current.generation;
                }
                current.reset_value();
                current.failure.reset();
                current.quiesce = options.quiesce;
                current.context = options.context;
                current.state = detail::operation_state::pending;
                const operation_id id{
                    .slot = static_cast<std::uint16_t>(index),
                    .generation = current.generation,
                };
                return transfer_type{*this, id};
            }
            return std::unexpected(detail::make_transfer_error<Error>(
                transfer_error_kind::resource_exhausted));
        });
    }

    // @req REQ-0012 REQ-0017
    template<typename U = T>
        requires(!std::is_void_v<U>)
    [[nodiscard]] bool complete(operation_id id, U value) noexcept
    {
        const bool published = policy_.critical([this, id, &value] {
            auto* current = pending_slot(id);
            if (current == nullptr) {
                return false;
            }
            current->stored.emplace(std::move(value));
            current->state = detail::operation_state::succeeded;
            return true;
        });
        if (published) {
            policy_.notify();
        }
        return published;
    }

    // @req REQ-0012 REQ-0017
    template<typename U = T>
        requires(std::is_void_v<U>)
    [[nodiscard]] bool complete(operation_id id) noexcept
    {
        const bool published = policy_.critical([this, id] {
            auto* current = pending_slot(id);
            if (current == nullptr) {
                return false;
            }
            current->state = detail::operation_state::succeeded;
            return true;
        });
        if (published) {
            policy_.notify();
        }
        return published;
    }

    // @req REQ-0012 REQ-0017
    [[nodiscard]] bool fail(operation_id id, Error error) noexcept {
        const bool published = policy_.critical([this, id, &error] {
            auto* current = pending_slot(id);
            if (current == nullptr) {
                return false;
            }
            current->failure.emplace(error);
            current->state = detail::operation_state::failed;
            return true;
        });
        if (published) {
            policy_.notify();
        }
        return published;
    }

private:
    friend class transfer<T, Error, Capacity, ExecutionPolicy>;

    struct slot : detail::operation_value<T> {
        detail::operation_state state{detail::operation_state::free};
        std::uint32_t generation{};
        std::optional<Error> failure{};
        quiesce_function quiesce{};
        void* context{};
    };

    [[nodiscard]] slot* find_slot(operation_id id) noexcept {
        if (id.slot >= Capacity) {
            return nullptr;
        }
        auto& current = slots_[id.slot];
        return current.generation == id.generation ? &current : nullptr;
    }

    [[nodiscard]] const slot* find_slot(operation_id id) const noexcept {
        if (id.slot >= Capacity) {
            return nullptr;
        }
        const auto& current = slots_[id.slot];
        return current.generation == id.generation ? &current : nullptr;
    }

    [[nodiscard]] slot* pending_slot(operation_id id) noexcept {
        auto* current = find_slot(id);
        if (current == nullptr
            || current->state != detail::operation_state::pending) {
            return nullptr;
        }
        return current;
    }

    [[nodiscard]] static error_type state_error(const slot& current) noexcept {
        switch (current.state) {
        case detail::operation_state::pending:
        case detail::operation_state::cancelling:
            return detail::make_transfer_error<Error>(
                transfer_error_kind::pending);
        case detail::operation_state::failed:
            return detail::make_failure<Error>(*current.failure);
        case detail::operation_state::timed_out:
            return detail::make_transfer_error<Error>(
                transfer_error_kind::timeout);
        case detail::operation_state::cancelled:
            return detail::make_transfer_error<Error>(
                transfer_error_kind::cancelled);
        case detail::operation_state::faulted:
            return detail::make_transfer_error<Error>(
                transfer_error_kind::quiesce_failed);
        case detail::operation_state::free:
        case detail::operation_state::retired:
            return detail::make_transfer_error<Error>(
                transfer_error_kind::invalid_handle);
        case detail::operation_state::succeeded:
            break;
        }
        return detail::make_transfer_error<Error>(
            transfer_error_kind::invalid_handle);
    }

    [[nodiscard]] inspection_result inspect(operation_id id) noexcept {
        return policy_.critical([this, id]() -> inspection_result {
            const auto* current = find_slot(id);
            if (current == nullptr) {
                return std::unexpected(detail::make_transfer_error<Error>(
                    transfer_error_kind::invalid_handle));
            }
            if (current->state != detail::operation_state::succeeded) {
                return std::unexpected(state_error(*current));
            }
            if constexpr (std::is_void_v<T>) {
                return {};
            } else {
                return &*current->stored;
            }
        });
    }

    [[nodiscard]] bool ready(operation_id id) noexcept {
        return policy_.critical([this, id] {
            const auto* current = find_slot(id);
            return current != nullptr
                && current->state != detail::operation_state::pending
                && current->state != detail::operation_state::cancelling;
        });
    }

    [[nodiscard]] take_result take(operation_id id) noexcept {
        return policy_.critical([this, id]() -> take_result {
            auto* current = find_slot(id);
            if (current == nullptr) {
                return std::unexpected(detail::make_transfer_error<Error>(
                    transfer_error_kind::invalid_handle));
            }
            if (current->state == detail::operation_state::succeeded) {
                if constexpr (std::is_void_v<T>) {
                    release_slot(*current);
                    return {};
                } else {
                    T result = std::move(*current->stored);
                    release_slot(*current);
                    return result;
                }
            }

            const auto error = state_error(*current);
            if (current->state == detail::operation_state::failed
                || current->state == detail::operation_state::timed_out
                || current->state == detail::operation_state::cancelled) {
                release_slot(*current);
            } else if (current->state == detail::operation_state::faulted) {
                retire_slot(*current);
            }
            return std::unexpected(error);
        });
    }

    // @req REQ-0018 REQ-0019
    [[nodiscard]] std::expected<bool, error_type>
    terminate(operation_id id, detail::operation_state terminal) noexcept {
        quiesce_function quiesce{};
        void* context{};
        const auto reserved = policy_.critical([this, id, &quiesce, &context] {
            auto* current = find_slot(id);
            if (current == nullptr) {
                return -1;
            }
            if (current->state != detail::operation_state::pending) {
                return 0;
            }
            current->state = detail::operation_state::cancelling;
            quiesce = current->quiesce;
            context = current->context;
            return 1;
        });
        if (reserved < 0) {
            return std::unexpected(detail::make_transfer_error<Error>(
                transfer_error_kind::invalid_handle));
        }
        if (reserved == 0) {
            return false;
        }

        const auto quiesced = quiesce(context, id);
        policy_.critical([this, id, terminal, quiesced] {
            auto* current = find_slot(id);
            if (current == nullptr
                || current->state != detail::operation_state::cancelling) {
                return;
            }
            current->state = quiesced == quiesce_result::reusable
                ? terminal
                : detail::operation_state::faulted;
        });
        policy_.notify();
        if (quiesced == quiesce_result::faulted) {
            return std::unexpected(detail::make_transfer_error<Error>(
                transfer_error_kind::quiesce_failed));
        }
        return true;
    }

    void abandon(operation_id id) noexcept {
        const bool violation = policy_.critical([this, id] {
            auto* current = find_slot(id);
            if (current == nullptr) {
                return false;
            }
            if (current->state == detail::operation_state::pending
                || current->state == detail::operation_state::cancelling) {
                retire_slot(*current);
                return true;
            }
            if (current->state == detail::operation_state::faulted) {
                retire_slot(*current);
            } else if (current->state != detail::operation_state::free
                       && current->state
                           != detail::operation_state::retired) {
                release_slot(*current);
            }
            return false;
        });
        if (violation) {
            policy_.active_transfer_destroyed();
        }
    }

    static void release_slot(slot& current) noexcept {
        current.reset_value();
        current.failure.reset();
        current.quiesce = nullptr;
        current.context = nullptr;
        current.state = detail::operation_state::free;
    }

    static void retire_slot(slot& current) noexcept {
        current.reset_value();
        current.failure.reset();
        current.quiesce = nullptr;
        current.context = nullptr;
        current.state = detail::operation_state::retired;
    }

    ExecutionPolicy& policy_;
    std::array<slot, Capacity> slots_{};
};

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
transfer<T, Error, Capacity, ExecutionPolicy>::~transfer() {
    if (pool_ != nullptr && !consumed_) {
        pool_->abandon(id_);
    }
}

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
bool transfer<T, Error, Capacity, ExecutionPolicy>::ready() const noexcept {
    return consumed_ || (pool_ != nullptr && pool_->ready(id_));
}

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
// @req REQ-0017
auto transfer<T, Error, Capacity, ExecutionPolicy>::inspect() const noexcept
    -> inspection_result {
    if (consumed_) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::consumed));
    }
    if (pool_ == nullptr) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::invalid_handle));
    }
    return pool_->inspect(id_);
}

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
// @req REQ-0017
auto transfer<T, Error, Capacity, ExecutionPolicy>::take() noexcept
    -> take_result {
    if (consumed_) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::consumed));
    }
    if (pool_ == nullptr) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::invalid_handle));
    }
    auto result = pool_->take(id_);
    if (result || result.error().kind == transfer_error_kind::failure
        || result.error().kind == transfer_error_kind::timeout
        || result.error().kind == transfer_error_kind::cancelled
        || result.error().kind == transfer_error_kind::quiesce_failed) {
        consumed_ = true;
    }
    return result;
}

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
// @req REQ-0015 REQ-0016 REQ-0017 REQ-0019
auto transfer<T, Error, Capacity, ExecutionPolicy>::wait_until(
    execution::deadline limit) noexcept -> inspection_result {
    if (consumed_) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::consumed));
    }
    if (pool_ == nullptr) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::invalid_handle));
    }
    if (pool_->policy_.in_isr()) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::wait_from_isr));
    }

    for (;;) {
        const auto wait_token = pool_->policy_.prepare_wait();
        auto current = pool_->inspect(id_);
        if (current || current.error().kind != transfer_error_kind::pending) {
            return current;
        }
        if (execution::reached(pool_->policy_.now(), limit)) {
            auto timed_out = pool_->terminate(
                id_, detail::operation_state::timed_out);
            if (!timed_out) {
                return std::unexpected(timed_out.error());
            }
            continue;
        }
        pool_->policy_.wait_once(wait_token, limit);
    }
}

template<typename T, typename Error, std::size_t Capacity,
         typename ExecutionPolicy>
// @req REQ-0018 REQ-0019
auto transfer<T, Error, Capacity, ExecutionPolicy>::cancel() noexcept
    -> std::expected<bool, error_type> {
    if (consumed_) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::consumed));
    }
    if (pool_ == nullptr) {
        return std::unexpected(detail::make_transfer_error<Error>(
            transfer_error_kind::invalid_handle));
    }
    return pool_->terminate(id_, detail::operation_state::cancelled);
}

} // namespace mcuhw::core
