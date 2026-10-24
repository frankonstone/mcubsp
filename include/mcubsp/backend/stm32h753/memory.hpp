#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>

#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/mcu/stm32h753/description.hpp>

namespace mcubsp::backend::stm32h753 {

inline constexpr std::size_t cortex_m7_cache_line = 32U;

enum class dma_buffer_error : std::uint8_t {
    empty,
    overflow,
    outside_memory,
    unreachable,
    wrong_access,
    cache_line_alignment,
    mmio,
};

enum class dma_buffer_access : std::uint8_t {
    dma_reads,
    dma_writes,
    bidirectional,
};

struct dma_buffer_plan {
    std::uint16_t region_id;
    std::uintptr_t address;
    std::size_t size;
    bool clean_before;
    bool invalidate_before;
    bool invalidate_after;
};

// Cacheable DMA buffers must own complete M7 cache lines. This prevents a
// maintenance operation from discarding or writing back an unrelated object
// sharing the first or last line.
// @req REQ-0009 REQ-0013 REQ-0033
[[nodiscard]] constexpr std::expected<dma_buffer_plan, dma_buffer_error>
plan_dma_buffer(core::dma_engine_kind engine, std::uintptr_t address,
                std::size_t size, dma_buffer_access access) noexcept {
    if (size == 0U) return std::unexpected(dma_buffer_error::empty);
    if (address > static_cast<std::uintptr_t>(-1) - (size - 1U)) {
        return std::unexpected(dma_buffer_error::overflow);
    }
    const auto end = address + size;
    const core::memory_region* region{};
    for (const auto& candidate : mcu::stm32h753::memory_regions) {
        if (address >= candidate.begin
            && end <= candidate.begin + candidate.size) {
            region = &candidate; break;
        }
    }
    if (region == nullptr) return std::unexpected(dma_buffer_error::outside_memory);
    const core::dma_reachability* route{};
    for (const auto& candidate : mcu::stm32h753::dma_reachability) {
        if (candidate.engine == engine && candidate.region_id == region->id) {
            route = &candidate; break;
        }
    }
    if (route == nullptr) return std::unexpected(dma_buffer_error::unreachable);
    const bool needs_read = access != dma_buffer_access::dma_writes;
    const bool needs_write = access != dma_buffer_access::dma_reads;
    if ((needs_read && (!region->readable || !route->can_read))
        || (needs_write && (!region->writable || !route->can_write))) {
        return std::unexpected(dma_buffer_error::wrong_access);
    }
    const bool cached = region->cache == core::cache_policy::software_maintained;
    if (cached && ((address % cortex_m7_cache_line) != 0U
                   || (size % cortex_m7_cache_line) != 0U)) {
        return std::unexpected(dma_buffer_error::cache_line_alignment);
    }
    return dma_buffer_plan{
        .region_id = region->id,
        .address = address,
        .size = size,
        .clean_before = cached && needs_read,
        .invalidate_before = cached && needs_write,
        .invalidate_after = cached && needs_write,
    };
}

// The SCB maintenance registers operate by virtual address. The caller uses
// prepare before enabling DMA and complete only after the engine is quiescent.
template<typename MmioPolicy>
class dma_cache_maintenance {
public:
    explicit dma_cache_maintenance(MmioPolicy& mmio) noexcept : mmio_(mmio) {}

    [[nodiscard]] std::expected<void, dma_buffer_error> prepare(
        dma_buffer_plan plan) noexcept {
        if (plan.invalidate_before && !maintain(dccimvac, plan)) return failure();
        if (plan.clean_before && !plan.invalidate_before
            && !maintain(dccmvac, plan)) return failure();
        arch::data_synchronization_barrier();
        return {};
    }

    [[nodiscard]] std::expected<void, dma_buffer_error> complete(
        dma_buffer_plan plan) noexcept {
        arch::data_synchronization_barrier();
        if (plan.invalidate_after && !maintain(dcimvac, plan)) return failure();
        arch::data_synchronization_barrier();
        arch::instruction_barrier();
        return {};
    }

private:
    static constexpr std::uintptr_t dcimvac = 0xE000'EF5CU;
    static constexpr std::uintptr_t dccmvac = 0xE000'EF68U;
    static constexpr std::uintptr_t dccimvac = 0xE000'EF70U;

    [[nodiscard]] bool maintain(std::uintptr_t command,
                                dma_buffer_plan plan) noexcept {
        for (std::size_t offset = 0U; offset < plan.size;
             offset += cortex_m7_cache_line) {
            if (!mmio_.template write<std::uint32_t>(
                    command, static_cast<std::uint32_t>(plan.address + offset))) {
                return false;
            }
        }
        return true;
    }
    [[nodiscard]] static std::expected<void, dma_buffer_error> failure() noexcept {
        return std::unexpected(dma_buffer_error::mmio);
    }
    MmioPolicy& mmio_;
};

} // namespace mcubsp::backend::stm32h753
