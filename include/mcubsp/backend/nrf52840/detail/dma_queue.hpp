#pragma once
// @req-file REQ-0018 REQ-0019 REQ-0062
#include <algorithm>
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
namespace mcubsp::backend::nrf52840 {
template <class T> struct dma_block {
  std::span<T> storage;
  std::size_t count;
  std::uint64_t sequence;
};
namespace detail {
// Fixed queue and leases. publish_quiet is called ONLY by the concrete driver
// after its documented DMA completion/stop proof. No peripheral is programmed.
template <class Session, class T, std::size_t Capacity = 4> class dma_queue {
  static_assert(Capacity > 0);

public:
  dma_queue(Session &session, mcu::nrf52840::dma_constraints constraints,
            dma_access access) noexcept
      : s_(session), constraints_(constraints), access_(access) {}
  [[nodiscard]] coordination_result push(std::span<T> buffer) noexcept {
    if (buffer.empty() ||
        sequence_ == std::numeric_limits<std::uint64_t>::max())
      return std::unexpected(coordination_error::invalid);
    auto a = reinterpret_cast<std::uintptr_t>(buffer.data());
    if (buffer.size_bytes() > std::numeric_limits<std::uintptr_t>::max() - a)
      return std::unexpected(coordination_error::invalid);
    unsigned free = Capacity;
    for (unsigned i = 0; i < Capacity; ++i) {
      if (slots_[i].status == state::free) {
        if (free == Capacity)
          free = i;
        continue;
      }
      auto b = reinterpret_cast<std::uintptr_t>(slots_[i].data.data());
      if (a < b + slots_[i].data.size_bytes() && b < a + buffer.size_bytes())
        return std::unexpected(coordination_error::busy);
    }
    if (free == Capacity)
      return std::unexpected(coordination_error::exhausted);
    auto token = s_.prepare(buffer.data(), buffer.size_bytes(), buffer.size(),
                            constraints_, access_);
    if (!token)
      return std::unexpected(token.error());
    slots_[free] = {buffer, *token, 0, ++sequence_, state::queued};
    return {};
  }
  [[nodiscard]] coordination_result activate(std::size_t maximum) noexcept {
    if (active_ || !maximum)
      return std::unexpected(coordination_error::active);
    unsigned next = Capacity;
    for (unsigned i = 0; i < Capacity; ++i)
      if (slots_[i].status == state::queued &&
          (next == Capacity || slots_[i].sequence < slots_[next].sequence))
        next = i;
    if (next == Capacity)
      return std::unexpected(coordination_error::exhausted);
    if (!s_.buffers.arm(*slots_[next].token))
      return std::unexpected(coordination_error::faulted);
    slots_[next].status = state::active;
    active_ = next;
    active_count_ = std::min(maximum, slots_[next].data.size());
    return {};
  }
  [[nodiscard]] std::span<T> current() const noexcept {
    return active_ ? slots_[*active_].data.first(active_count_)
                   : std::span<T>{};
  }
  [[nodiscard]] bool has_active() const noexcept { return active_.has_value(); }
  [[nodiscard]] bool publish_quiet(std::size_t count) noexcept {
    if (!active_)
      return true;
    auto &slot = slots_[*active_];
    if (count > active_count_ || !s_.release_quiet(slot.token))
      return false;
    slot.count = count;
    slot.status = state::complete;
    active_.reset();
    return true;
  }
  [[nodiscard]] bool discard_queued() noexcept {
    for (auto &slot : slots_)
      if (slot.status == state::queued) {
        s_.discard(slot.token);
        if (slot.token)
          return false;
        slot.status = state::free;
      }
    return true;
  }
  [[nodiscard]] std::optional<dma_block<T>> take() noexcept {
    unsigned next = Capacity;
    for (unsigned i = 0; i < Capacity; ++i)
      if (slots_[i].status == state::complete &&
          (next == Capacity || slots_[i].sequence < slots_[next].sequence))
        next = i;
    if (next == Capacity)
      return {};
    auto &slot = slots_[next];
    dma_block<T> block{slot.data, slot.count, slot.sequence};
    slot.status = state::free;
    return block;
  }
  [[nodiscard]] bool empty() const noexcept {
    for (auto &slot : slots_)
      if (slot.status != state::free)
        return false;
    return true;
  }

private:
  enum class state : std::uint8_t { free, queued, active, complete };
  struct slot {
    std::span<T> data;
    std::optional<dma_buffer_token> token;
    std::size_t count{};
    std::uint64_t sequence{};
    state status{};
  };
  Session &s_;
  mcu::nrf52840::dma_constraints constraints_;
  dma_access access_;
  std::array<slot, Capacity> slots_{};
  std::optional<unsigned> active_;
  std::size_t active_count_{};
  std::uint64_t sequence_{};
};
} // namespace detail
} // namespace mcubsp::backend::nrf52840
