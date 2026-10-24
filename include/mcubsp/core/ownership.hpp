#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>

namespace mcubsp::core {

enum class ownership_error : std::uint8_t { invalid_resource, busy };

// Fixed-storage ownership for explicitly enumerated peripheral instances.
// @req REQ-0014
template<std::size_t ResourceCount, typename ExecutionPolicy>
class peripheral_ownership {
    static_assert(ResourceCount > 0U);

public:
    static constexpr std::size_t capacity = ResourceCount;

    class lease {
    public:
        lease(const lease&) = delete;
        lease& operator=(const lease&) = delete;

        lease(lease&& other) noexcept
            : owner_(other.owner_), resource_(other.resource_) {
            other.owner_ = nullptr;
        }

        lease& operator=(lease&&) = delete;

        ~lease() { release(); }

        void release() noexcept {
            if (owner_ != nullptr) {
                owner_->release(resource_);
                owner_ = nullptr;
            }
        }

        [[nodiscard]] bool owns_resource() const noexcept {
            return owner_ != nullptr;
        }

        [[nodiscard]] std::size_t resource() const noexcept {
            return resource_;
        }

    private:
        friend class peripheral_ownership;

        lease(peripheral_ownership& owner, std::size_t resource) noexcept
            : owner_(&owner), resource_(resource) {}

        peripheral_ownership* owner_;
        std::size_t resource_;
    };

    explicit peripheral_ownership(ExecutionPolicy& policy) noexcept
        : policy_(policy) {}

    // @req REQ-0014
    [[nodiscard]] std::expected<lease, ownership_error>
    try_claim(std::size_t resource) noexcept {
        if (resource >= ResourceCount) {
            return std::unexpected(ownership_error::invalid_resource);
        }
        const bool claimed = policy_.critical([this, resource] {
            if (owned_[resource]) {
                return false;
            }
            owned_[resource] = true;
            return true;
        });
        if (!claimed) {
            return std::unexpected(ownership_error::busy);
        }
        return lease{*this, resource};
    }

    [[nodiscard]] bool owned(std::size_t resource) noexcept {
        if (resource >= ResourceCount) {
            return false;
        }
        return policy_.critical(
            [this, resource] { return owned_[resource]; });
    }

private:
    void release(std::size_t resource) noexcept {
        policy_.critical([this, resource] { owned_[resource] = false; });
    }

    ExecutionPolicy& policy_;
    std::array<bool, ResourceCount> owned_{};
};

} // namespace mcubsp::core
