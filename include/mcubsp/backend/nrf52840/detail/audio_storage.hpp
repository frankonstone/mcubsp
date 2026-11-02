#pragma once
// @req-file REQ-0067 REQ-0062
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
namespace mcubsp::backend::nrf52840 {
template <class T> struct audio_block {
  std::span<const T> tx;
  std::span<T> rx;
  std::uint64_t sequence{};
};
namespace detail {
// Fixed leases and publication bookkeeping; only the I2S/PDM controller may
// advance slots after its concrete pointer-update/STOPPED proof.
template <class Session, class T, unsigned Capacity> class audio_storage {
public:
  explicit audio_storage(Session &s) : s_(s) {}
  static constexpr int guard = Capacity, none = -1;
  [[nodiscard]] system_result prepare(audio_block<T> b, bool tx, bool rx,
                                      std::size_t count,
                                      mcu::nrf52840::dma_constraints limits,
                                      bool tail = false) {
    if ((tx ? b.tx.size() != count : !b.tx.empty()) ||
        (rx ? b.rx.size() != count : !b.rx.empty()))
      return std::unexpected(system_error::invalid);
    unsigned slot = tail ? Capacity : 0;
    if (!tail) {
      for (; slot < Capacity; ++slot)
        if (slots_[slot].state == free)
          break;
    }
    if (slot > Capacity || (!tail && slot == Capacity) ||
        slots_[slot].state != free)
      return std::unexpected(system_error::busy);
    auto &v = slots_[slot];
    v.block = b;
    if (tx) {
      auto p = s_.prepare(b.tx.data(), b.tx.size_bytes(), count, limits,
                          dma_access::read);
      if (!p)
        return std::unexpected(system_error::invalid);
      v.tx = *p;
    }
    if (rx) {
      auto p = s_.prepare(b.rx.data(), b.rx.size_bytes(), count, limits,
                          dma_access::write);
      if (!p) {
        s_.discard(v.tx);
        return std::unexpected(system_error::invalid);
      }
      v.rx = *p;
    }
    v.order = ++order_;
    v.state = queued;
    return {};
  }
  int next() const {
    int found = none;
    for (unsigned i = 0; i < Capacity; ++i)
      if (slots_[i].state == queued &&
          (found == none || slots_[i].order < slots_[found].order))
        found = i;
    return found;
  }
  bool arm(int i) {
    auto &v = slots_[i];
    if (v.state != queued)
      return false;
    v.state = armed;
    return (!v.tx || s_.buffers.arm(*v.tx)) && (!v.rx || s_.buffers.arm(*v.rx));
  }
  const audio_block<T> &block(int i) const { return slots_[i].block; }
  bool publish_quiet(int i) {
    auto &v = slots_[i];
    if (!s_.release_quiet(v.tx) || !s_.release_quiet(v.rx))
      return false;
    v.state = complete;
    return true;
  }
  std::optional<audio_block<T>> take() {
    int found = none;
    for (unsigned i = 0; i < Capacity; ++i)
      if (slots_[i].state == complete &&
          (found == none || slots_[i].order < slots_[found].order))
        found = i;
    if (found == none)
      return {};
    auto result = slots_[found].block;
    slots_[found].state = free;
    return result;
  }
  bool release_all_quiet() {
    for (auto &v : slots_) {
      if (v.state == armed) {
        if (!s_.release_quiet(v.tx) || !s_.release_quiet(v.rx))
          return false;
      } else if (v.state == queued) {
        s_.discard(v.tx);
        s_.discard(v.rx);
      }
      if (v.state != complete)
        v.state = free;
    }
    return true;
  }
  bool empty() const {
    for (auto &v : slots_)
      if (v.state != free)
        return false;
    return true;
  }
  bool active_leases() const {
    for (auto &v : slots_)
      if (v.tx || v.rx)
        return true;
    return false;
  }

private:
  enum slot_state { free, queued, armed, complete };
  struct slot {
    audio_block<T> block{};
    std::optional<dma_buffer_token> tx, rx;
    std::uint64_t order{};
    slot_state state{free};
  };
  Session &s_;
  std::array<slot, Capacity + 1> slots_{};
  std::uint64_t order_{};
};
} // namespace detail
} // namespace mcubsp::backend::nrf52840
