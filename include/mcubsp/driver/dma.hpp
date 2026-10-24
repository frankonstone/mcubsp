#pragma once

#include <cstddef>
#include <cstdint>

namespace mcubsp::driver {

enum class dma_width : std::uint8_t {
    bits8 = 1U,
    bits16 = 2U,
    bits32 = 4U,
};

[[nodiscard]] constexpr std::size_t bytes(dma_width width) noexcept {
    return static_cast<std::size_t>(width);
}

enum class dma_error : std::uint8_t {
    already_owned,
    not_initialized,
    busy,
    invalid_width,
    invalid_count,
    invalid_alignment,
    inaccessible_memory,
    invalid_peripheral,
    unsupported_event,
    transfer_escape,
    mmio,
};

enum class dma_peripheral_direction : std::uint8_t {
    memory_to_peripheral,
    peripheral_to_memory,
};

// The source and destination storage remain caller-owned until the returned
// transfer reaches a terminal state and is consumed.
struct dma_memory_copy {
    std::uintptr_t source;
    std::uintptr_t destination;
    std::uint32_t count;
    dma_width width;
};

struct dma_peripheral_transfer {
    std::uintptr_t memory;
    std::uintptr_t peripheral;
    std::uint32_t count;
    dma_width width;
    std::uint16_t event;
    dma_peripheral_direction direction;
};

struct dma_transfer_result {
    std::uint32_t units;
    dma_width width;
};

} // namespace mcubsp::driver
