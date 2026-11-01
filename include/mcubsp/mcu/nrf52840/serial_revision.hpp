#pragma once
// @req-file REQ-0055 REQ-0059 REQ-0060 REQ-0062
#include <cstdint>
namespace mcubsp::mcu::nrf52840 {
// nrfx v3.2.0 nrf52_erratas.h predicates 58/127/131 use FICR words
// 0x10000130 and 0x10000134. These are distinct from INFO.VARIANT and ROM ID.
// Recognize only the revisions explicitly enumerated by that pinned source.
struct serial_revision {
  std::uint32_t part{}, revision{};
  [[nodiscard]] constexpr bool reviewed_modern() const noexcept {
    return part == 8 && revision >= 1 && revision <= 5;
  }
  [[nodiscard]] constexpr unsigned uarte_maximum_count() const noexcept {
    return reviewed_modern() ? 65535U : 255U;
  }
};
} // namespace mcubsp::mcu::nrf52840
