#pragma once
// @req-file REQ-0018 REQ-0059 REQ-0062
#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <mcubsp/driver/uart.hpp>
#include <optional>
#include <span>
namespace mcubsp::backend::nrf52840 {
struct received_block {
  std::span<std::uint8_t> storage;
  std::size_t count;
  std::uint64_t sequence;
};
namespace detail {
// Bookkeeping only. The owning driver supplies critical sections and proves
// memory quiescent before publish/discard. Completed blocks are never reused.
template <std::size_t Capacity> class rx_queue {
  static_assert(Capacity > 0);

public:
  enum class state : std::uint8_t { free, queued, active, complete };
  [[nodiscard]] std::expected<unsigned, driver::uart_error>
  push(std::span<std::uint8_t> bytes) noexcept {
    if (bytes.empty() || !bytes.data())
      return std::unexpected(driver::uart_error::empty_transfer);
    auto begin = reinterpret_cast<std::uintptr_t>(bytes.data());
    if (bytes.size() > std::numeric_limits<std::uintptr_t>::max() - begin)
      return std::unexpected(driver::uart_error::dma);
    unsigned free = Capacity;
    for (unsigned i = 0; i < Capacity; ++i) {
      if (slots_[i].status == state::free) {
        if (free == Capacity)
          free = i;
        continue;
      }
      auto other = reinterpret_cast<std::uintptr_t>(slots_[i].bytes.data());
      if (begin < other + slots_[i].bytes.size() &&
          other < begin + bytes.size())
        return std::unexpected(driver::uart_error::already_owned);
    }
    if (free == Capacity ||
        sequence_ == std::numeric_limits<std::uint64_t>::max())
      return std::unexpected(driver::uart_error::busy);
    slots_[free] = {bytes, 0, ++sequence_, state::queued};
    return free;
  }
  [[nodiscard]] std::optional<unsigned> activate() noexcept {
    unsigned next = Capacity;
    for (unsigned i = 0; i < Capacity; ++i) {
      if (slots_[i].status == state::active)
        return {};
      if (slots_[i].status == state::queued &&
          (next == Capacity || slots_[i].sequence < slots_[next].sequence))
        next = i;
    }
    if (next == Capacity)
      return {};
    slots_[next].status = state::active;
    return next;
  }
  [[nodiscard]] bool publish(unsigned i, std::size_t count) noexcept {
    if (i >= Capacity || slots_[i].status != state::active ||
        count > slots_[i].bytes.size())
      return false;
    slots_[i].count = count;
    slots_[i].status = state::complete;
    return true;
  }
  [[nodiscard]] std::optional<received_block> take() noexcept {
    unsigned next = Capacity;
    for (unsigned i = 0; i < Capacity; ++i)
      if (slots_[i].status == state::complete &&
          (next == Capacity || slots_[i].sequence < slots_[next].sequence))
        next = i;
    if (next == Capacity)
      return {};
    auto &slot = slots_[next];
    received_block block{slot.bytes, slot.count, slot.sequence};
    slot.status = state::free;
    return block;
  }
  [[nodiscard]] std::span<std::uint8_t> buffer(unsigned i) const noexcept {
    return slots_[i].bytes;
  }
  [[nodiscard]] state status(unsigned i) const noexcept {
    return slots_[i].status;
  }
  void discard(unsigned i) noexcept { slots_[i].status = state::free; }
  [[nodiscard]] bool empty() const noexcept {
    for (const auto &s : slots_)
      if (s.status != state::free)
        return false;
    return true;
  }

private:
  struct slot {
    std::span<std::uint8_t> bytes{};
    std::size_t count{};
    std::uint64_t sequence{};
    state status{};
  };
  std::array<slot, Capacity> slots_{};
  std::uint64_t sequence_{};
};
} // namespace detail
} // namespace mcubsp::backend::nrf52840
