#pragma once
// @req-file REQ-0055 REQ-0073 REQ-0062
// Nordic PS 4413_417 v1.11 4.2.1-4.2.3, 4.7 and 6.1.
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <mcubsp/mcu/nrf52840/model.hpp>

namespace mcubsp::mcu::nrf52840::memory {
struct region {
    std::uintptr_t begin;
    std::size_t bytes;
    [[nodiscard]] constexpr bool contains(std::uintptr_t address, std::size_t count) const noexcept {
        return address >= begin && address - begin <= bytes && count <= bytes - (address - begin);
    }
};
inline constexpr region flash{0x00000000, 0x100000};
inline constexpr region code_ram{0x00800000, 0x40000};
inline constexpr region data_ram{0x20000000, 0x40000};
inline constexpr region qspi_xip{0x12000000, 0x8000000};
inline constexpr unsigned flash_page_bytes = 4096, flash_pages = 256;
inline constexpr bool data_cache_present = false;

// Code RAM and Data RAM alias the same storage on distinct CPU bus interfaces.
// This translation is a description, not a cache/barrier or DMA operation.
[[nodiscard]] constexpr std::optional<std::uintptr_t>
canonical_ram_address(std::uintptr_t address) noexcept {
    if (data_ram.contains(address, 1))
        return address;
    if (code_ram.contains(address, 1))
        return data_ram.begin + (address - code_ram.begin);
    return std::nullopt;
}

// EasyDMA accepts Data RAM addresses. A CPU code alias is not an EasyDMA pointer.
// CryptoCell's DMA engines have their own contracts; this does not describe them.
[[nodiscard]] constexpr bool easydma_buffer(std::uintptr_t address, std::size_t bytes,
                                           std::size_t alignment = 1) noexcept {
    return bytes != 0 && std::has_single_bit(alignment) && address % alignment == 0 &&
           data_ram.contains(address, bytes);
}

[[nodiscard]] constexpr std::optional<region> ram_section(unsigned block, unsigned section) noexcept {
    if (block < 8 && section < 2)
        return region{data_ram.begin + block * 0x2000 + section * 0x1000, 0x1000};
    if (block == 8 && section < 6)
        return region{data_ram.begin + 0x10000 + section * 0x8000, 0x8000};
    return std::nullopt;
}
} // namespace mcubsp::mcu::nrf52840::memory
