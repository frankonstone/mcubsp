#pragma once
// @req-file REQ-0062
#include <cstddef>
#include <cstdint>
namespace mcubsp::backend::nrf52840 {
// Simulator policies translate host storage; native policies preserve pointers.
// Peripheral-specific constraints are always checked after translation.
struct native_dma_address {
  [[nodiscard]] std::uintptr_t operator()(const void *p,
                                          std::size_t) const noexcept {
    return reinterpret_cast<std::uintptr_t>(p);
  }
};
} // namespace mcubsp::backend::nrf52840
