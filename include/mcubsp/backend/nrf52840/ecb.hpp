#pragma once
// @req-file REQ-0071 REQ-0062
// PS v1.11 6.7: ENDECB and ERRORECB both prove EasyDMA quiescence.
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
namespace mcubsp::backend::nrf52840 {
struct alignas(4) ecb_data {
  std::array<std::uint8_t, 16> key{}, cleartext{}, ciphertext{};
};
static_assert(sizeof(ecb_data) == 48);
template <class Context, class Buffers, class Address = native_dma_address,
          unsigned PollBudget = 1000000>
class ecb_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;

public:
  ecb_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                 execution::tick stop_ticks = 1'280'000,
                 Address a = {}) noexcept
      : s_(c, clocks, b, a), stop_ticks_(stop_ticks) {}
  ecb_controller(const ecb_controller &) = delete;
  ecb_controller &operator=(const ecb_controller &) = delete;
  ~ecb_controller() {
    if (dma_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result initialize() noexcept {
    if (!stop_ticks_ || stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto r = s_.acquire(peripheral::ecb, 0x4000E000, {}, std::nullopt);
    if (!r)
      return r;
    s_.publish();
    if (!s_.write(0x308, 3) || !clear())
      return fail();
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result start(ecb_data &block) noexcept {
    if (!initialized_ || dma_ || fault_ || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    auto lease = s_.prepare(&block, sizeof block, sizeof block, {48, 1, 4},
                            dma_access::read_write);
    if (!lease)
      return std::unexpected(system_error::invalid);
    dma_ = *lease;
    if (!clear() || !s_.write(0x504, s_.address(&block, sizeof block))) {
      s_.discard(dma_);
      return fail();
    }
    if (!s_.buffers.arm(*dma_)) {
      s_.discard(dma_);
      return fail();
    }
    terminal_ = error_ = false;
    if (!s_.write(0, 1))
      return fail();
    return {};
  }
  // true is completion; an error return after terminal DMA cleanup describes
  // arbitration/abort, never success. ECB has lower AES-core priority than
  // CCM/AAR.
  [[nodiscard]] system_value<bool> poll() noexcept {
    return s_.context.execution.critical([&]() -> system_value<bool> {
      if (!dma_)
        return false;
      auto end = s_.read(0x100), error = s_.read(0x104);
      if (!end || !error)
        return std::unexpected(system_error::mmio);
      terminal_ = terminal_ || *end || *error;
      error_ = error_ || *error;
      if (!terminal_)
        return false;
      if (!clear() || !s_.release_quiet(dma_))
        return std::unexpected(system_error::mmio);
      if (error_)
        return std::unexpected(system_error::faulted);
      return true;
    });
  }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (dma_) {
      if (!s_.write(4, 1))
        return fail();
      if (!s_.bounded(
              [&] {
                (void)poll();
                return !dma_;
              },
              stop_ticks_, PollBudget))
        return std::unexpected(system_error::timeout);
    }
    return {};
  }
  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, 3) ? system_result{} : fail();
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (!s_.owned())
      return {};
    auto r = cancel();
    if (!r)
      return r;
    if (!s_.write(0x308, 3) || !clear() || !s_.write(0x504, 0))
      return fail();
    r = s_.release();
    if (r)
      initialized_ = fault_ = false;
    return r;
  }
  [[nodiscard]] bool active() const noexcept { return dma_.has_value(); }

private:
  bool clear() { return s_.write(0x100, 0) && s_.write(0x104, 0); }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  session s_;
  execution::tick stop_ticks_;
  std::optional<dma_buffer_token> dma_;
  bool initialized_{}, fault_{}, terminal_{}, error_{};
};
} // namespace mcubsp::backend::nrf52840
