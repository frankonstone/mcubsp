#pragma once
// @req-file REQ-0071 REQ-0062
// PS v1.11 6.2; the three-byte radio header precedes the six-byte address.
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
namespace mcubsp::backend::nrf52840 {
using identity_resolving_key = std::array<std::uint8_t, 16>;
struct alignas(4) aar_packet {
  std::array<std::uint8_t, 3> header{};
  std::array<std::uint8_t, 6> address{};
};
struct aar_result {
  bool resolved{};
  unsigned key_index{};
};
template <class Context, class Buffers, class Address = native_dma_address>
class aar_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;

public:
  aar_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                 Address a = {}) noexcept
      : s_(c, clocks, b, a) {}
  aar_controller(const aar_controller &) = delete;
  aar_controller &operator=(const aar_controller &) = delete;
  ~aar_controller() {
    if (active_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result initialize() noexcept {
    auto r = s_.acquire_routed(peripheral::aar, 77, endpoint_domain::aar,
                               0x4000F000);
    if (!r)
      return r;
    auto enabled = s_.read(0x500);
    if (!enabled || *enabled) {
      (void)s_.release();
      return std::unexpected(enabled ? system_error::busy : system_error::mmio);
    }
    s_.publish();
    if (!s_.write(0x308, 7) || !clear() || !s_.write(0x500, 3))
      return fail();
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result
  resolve(std::span<const identity_resolving_key> keys,
          const aar_packet &packet, std::span<std::uint8_t> scratch) noexcept {
    if (!initialized_ || active_ || fault_ || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (keys.empty() || keys.size() > 16 || scratch.size() < 3 ||
        (packet.address[5] & 0xC0) != 0x40)
      return std::unexpected(system_error::invalid);
    if (!prepare(0, keys.data(), keys.size_bytes(), dma_access::read) ||
        !prepare(1, &packet, 9, dma_access::read) ||
        !prepare(2, scratch.data(), 3, dma_access::write)) {
      discard();
      return std::unexpected(system_error::invalid);
    }
    if (!clear() || !s_.write(0x504, keys.size()) ||
        !s_.write(0x508, s_.address(keys.data(), keys.size_bytes())) ||
        !s_.write(0x510, s_.address(&packet, 9)) ||
        !s_.write(0x514, s_.address(scratch.data(), 3))) {
      discard();
      return fail();
    }
    terminal_ = resolved_ = false;
    key_count_ = keys.size();
    active_ = true;
    for (auto &lease : leases_)
      if (!s_.buffers.arm(*lease))
        return fail();
    if (!s_.write(0, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_value<std::optional<aar_result>> poll() noexcept {
    return s_.context.execution.critical(
        [&]() -> system_value<std::optional<aar_result>> {
          if (!active_)
            return std::optional<aar_result>{};
          auto end = s_.read(0x100), yes = s_.read(0x104), no = s_.read(0x108);
          if (!end || !yes || !no)
            return std::unexpected(system_error::mmio);
          terminal_ = terminal_ || *end || *yes || *no;
          resolved_ = resolved_ || *yes;
          if (!terminal_)
            return std::optional<aar_result>{};
          auto status = s_.read(0x400);
          if (!status)
            return std::unexpected(system_error::mmio);
          if (!clear() || !release())
            return std::unexpected(system_error::mmio);
          active_ = false;
          if (resolved_ && *status >= key_count_)
            return std::unexpected(system_error::faulted);
          return std::optional{aar_result{resolved_, *status}};
        });
  }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (s_.routes_referenced())
      return std::unexpected(system_error::busy);
    if (!active_)
      return {};
    // No STOPPED event exists. STOP+ENABLE disabled readback prevents future
    // DMA; never use a RESOLVED/NOTRESOLVED acknowledgement as cancellation.
    if (!s_.write(8, 1) || !s_.disabled() || !release())
      return fail();
    active_ = false;
    initialized_ = false;
    return {};
  }
  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, 7) ? system_result{} : fail();
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    auto r = cancel();
    if (!r)
      return r;
    if (!s_.write(0x308, 7) || !s_.disabled() || !clear())
      return fail();
    r = s_.release();
    if (r)
      initialized_ = fault_ = false;
    return r;
  }

  [[nodiscard]] system_endpoint event_endpoint(unsigned offset) const noexcept {
    return s_.endpoint(offset, endpoint_kind::event);
  }
  [[nodiscard]] system_endpoint task_endpoint(unsigned offset) const noexcept {
    return active_ ? s_.endpoint(offset, endpoint_kind::task)
                   : system_endpoint{};
  }

private:
  bool prepare(unsigned i, const void *p, std::size_t bytes,
               dma_access access) {
    auto r = s_.prepare(p, bytes, bytes, {256, 1, 1}, access);
    if (!r)
      return false;
    leases_[i] = *r;
    return true;
  }
  void discard() {
    for (auto &v : leases_)
      s_.discard(v);
  }
  bool release() {
    for (auto &v : leases_)
      if (!s_.release_quiet(v))
        return false;
    return true;
  }
  bool clear() {
    return s_.write(0x100, 0) && s_.write(0x104, 0) && s_.write(0x108, 0);
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  session s_;
  std::array<std::optional<dma_buffer_token>, 3> leases_{};
  unsigned key_count_{};
  bool initialized_{}, active_{}, fault_{}, terminal_{}, resolved_{};
};
} // namespace mcubsp::backend::nrf52840
