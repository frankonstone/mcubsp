#pragma once
// @req-file REQ-0018 REQ-0019 REQ-0062
#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/backend/nrf52840/resources.hpp>

namespace mcubsp::backend::nrf52840 {
enum class dma_access : std::uint8_t { read, write, read_write };
enum class dma_buffer_state : std::uint8_t { free, prepared, armed, faulted };
struct dma_buffer_token {
    const void *pool{};
    std::size_t slot{};
    std::uint32_t generation{};
};

// Fixed bookkeeping, not a DMA engine. It never programs PTR/MAXCNT or polls
// hardware. Each owning driver supplies its own constraints and quiescence
// check (including final wire activity), then publishes its operation result.
// CryptoCell is NOT EasyDMA: use an explicit source-reviewed memory contract.
template<class Execution, std::size_t Capacity = 8> class dma_buffer_pool {
    static_assert(Capacity > 0);
  public:
    dma_buffer_pool(resource_registry &resources, Execution &execution) noexcept : resources_(resources), execution_(execution) {}
    dma_buffer_pool(const dma_buffer_pool &) = delete;
    dma_buffer_pool &operator=(const dma_buffer_pool &) = delete;
    ~dma_buffer_pool() {
        // A returning diagnostic policy (native tests) still retains every
        // active claim; a target policy must stop rather than detach buffers.
        execution_.critical([&] {
            for (const auto &s : slots_)
                if (s.state != dma_buffer_state::free) { execution_.active_transfer_destroyed(); return; }
            if (resources_.buffer_pool_ == this) resources_.buffer_pool_ = nullptr;
        });
    }
    template<class Constraints>
    [[nodiscard]] std::expected<dma_buffer_token, coordination_error>
    prepare(resource_token owner, Constraints constraints,
            std::uintptr_t address, std::size_t capacity_bytes, std::size_t count,
            dma_access access) noexcept {
        if (!constraints.unit_bytes || count == 0 || count > capacity_bytes / constraints.unit_bytes ||
            count * constraints.unit_bytes > std::numeric_limits<std::uintptr_t>::max() - address ||
            !constraints.accepts(address, capacity_bytes, count) || static_cast<unsigned>(access) > 2)
            return std::unexpected(coordination_error::invalid);
        return execution_.critical([&]() -> std::expected<dma_buffer_token, coordination_error> {
            if (!resources_.valid(owner)) return std::unexpected(coordination_error::stale);
            if (resources_.buffer_pool_ && resources_.buffer_pool_ != this)
                return std::unexpected(coordination_error::busy);
            const auto bytes = count * constraints.unit_bytes;
            std::size_t free = Capacity;
            for (std::size_t i = 0; i < Capacity; ++i) {
                auto &s = slots_[i];
                if (s.state == dma_buffer_state::free) {
                    if (free == Capacity && s.generation != std::numeric_limits<std::uint32_t>::max()) free = i;
                    continue;
                }
                if (address < s.address + s.bytes && s.address < address + bytes &&
                    (access != dma_access::read || s.access != dma_access::read))
                    return std::unexpected(coordination_error::busy);
            }
            if (free == Capacity) return std::unexpected(coordination_error::exhausted);
            auto retained = resources_.retain(execution_, owner);
            if (!retained) return std::unexpected(retained.error());
            auto &s = slots_[free];
            resources_.buffer_pool_ = this;
            ++s.generation;
            s.owner = owner; s.address = address; s.bytes = bytes; s.count = count; s.access = access;
            s.state = dma_buffer_state::prepared;
            return dma_buffer_token{this, free, s.generation};
        });
    }
    // Call BEFORE issuing any START that could commit, even if its MMIO write
    // subsequently reports an error. The buffer then requires explicit proof.
    [[nodiscard]] coordination_result arm(dma_buffer_token t) noexcept {
        return execution_.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            auto &s = slots_[t.slot];
            if (s.state != dma_buffer_state::prepared) return std::unexpected(coordination_error::active);
            arch::data_memory_barrier(); s.state = dma_buffer_state::armed;
            return {};
        });
    }
    [[nodiscard]] coordination_result discard_prepared(dma_buffer_token t) noexcept {
        return execution_.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            if (slots_[t.slot].state != dma_buffer_state::prepared) return std::unexpected(coordination_error::active);
            return release(t);
        });
    }
    // Check must be one bounded, nonblocking observation and return
    // expected<bool, coordination_error>. false keeps ownership for later
    // progress/deadline handling. An error faults the lease; recover explicitly.
    template<class Check> [[nodiscard]] coordination_result
    finish(dma_buffer_token t, std::size_t actual_count, Check &&quiescent) noexcept {
        return execution_.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            auto &s = slots_[t.slot];
            if (s.state != dma_buffer_state::armed) return std::unexpected(coordination_error::faulted);
            if (actual_count > s.count) { s.state = dma_buffer_state::faulted; return std::unexpected(coordination_error::count); }
            auto quiet = quiescent();
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            if (s.state != dma_buffer_state::armed) return std::unexpected(coordination_error::faulted);
            if (!quiet) { s.state = dma_buffer_state::faulted; return std::unexpected(quiet.error()); }
            if (!*quiet) return std::unexpected(coordination_error::not_quiescent);
            arch::data_memory_barrier();
            return release(t);
        });
    }
    // Cancellation/failed START/overrun recovery must establish hardware
    // quiescence before returning caller memory, independently of byte counts.
    template<class Check> [[nodiscard]] coordination_result recover(dma_buffer_token t, Check &&quiescent) noexcept {
        return execution_.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            auto quiet = quiescent();
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            if (!quiet) { slots_[t.slot].state = dma_buffer_state::faulted; return std::unexpected(quiet.error()); }
            if (!*quiet) return std::unexpected(coordination_error::not_quiescent);
            arch::data_memory_barrier();
            return release(t);
        });
    }
    [[nodiscard]] bool valid(dma_buffer_token t) const noexcept {
        return t.pool == this && t.slot < Capacity && slots_[t.slot].state != dma_buffer_state::free &&
               slots_[t.slot].generation == t.generation;
    }
  private:
    [[nodiscard]] coordination_result release(dma_buffer_token t) noexcept {
        if (!valid(t)) return std::unexpected(coordination_error::stale);
        auto r = resources_.drop(execution_, slots_[t.slot].owner);
        if (!r) return r;
        slots_[t.slot].state = dma_buffer_state::free;
        return {};
    }
    struct slot {
        resource_token owner{}; std::uintptr_t address{}; std::size_t bytes{}, count{};
        std::uint32_t generation{}; dma_access access{}; dma_buffer_state state{};
    };
    resource_registry &resources_; Execution &execution_;
    std::array<slot, Capacity> slots_{};
};
} // namespace mcubsp::backend::nrf52840
