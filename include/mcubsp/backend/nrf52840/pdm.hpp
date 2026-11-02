#pragma once
// @req-file REQ-0067 REQ-0062
// PS v1.11 6.15. STARTED latches SAMPLE.PTR; END publishes the previous
// buffer. No AMOUNT register exists: stopped partial buffers remain
// unpublished.
#include <mcubsp/backend/nrf52840/detail/audio_storage.hpp>
#include <mcubsp/mcu/nrf52840/product/pdm.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
template <core::pin_id Clk, core::pin_id Din> struct pdm_binding {
  static constexpr std::array pins{Clk, Din};
  static_assert(Clk != Din && mcu::nrf52840::gpio_supported(Clk) &&
                    mcu::nrf52840::gpio_supported(Din) &&
                    !mcu::nrf52840::product::silicon_low_frequency_only(Clk) &&
                    !mcu::nrf52840::product::silicon_low_frequency_only(Din),
                "PDM requires distinct high-frequency GPIOs");
};
enum class pdm_frequency : unsigned {
  khz1000,
  khz1032,
  khz1067,
  khz1231,
  khz1280,
  khz1333
};
inline constexpr std::array<std::uint32_t, 6> pdm_clock_words{
    0x8000000, 0x8400000, 0x8800000, 0x9800000, 0xA000000, 0xA800000};
inline constexpr std::array<unsigned, 6> pdm_clock_divisors{32, 31, 30,
                                                            26, 25, 24};
struct pdm_configuration {
  pdm_frequency frequency{pdm_frequency::khz1032};
  bool mono{}, left_rising{}, ratio80{};
  unsigned gain_left{40}, gain_right{40}, ticks_per_second{64'000'000};
};
struct pdm_stream_result {
  std::uint64_t completed_blocks{};
  bool exhausted{}, service_overrun{}, cancelled{};
  // Typically about 50 samples at every START are transient; the attached
  // microphone can require a longer warmup. No automatic data is discarded.
  bool startup_transient{true};
};
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned Capacity = 4,
          unsigned PollBudget = 1000000>
class pdm_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;
  using storage = detail::audio_storage<session, std::int16_t, Capacity>;

public:
  pdm_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                 execution::tick stop_ticks = 1'280'000,
                 Address a = {}) noexcept
      : s_(c, clocks, b, a), storage_(s_), stop_ticks_(stop_ticks) {}
  pdm_controller(const pdm_controller &) = delete;
  pdm_controller &operator=(const pdm_controller &) = delete;
  ~pdm_controller() {
    if (storage_.active_leases())
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result initialize(pdm_configuration c = {}) noexcept {
    if (unsigned(c.frequency) > 5 || c.gain_left > 80 || c.gain_right > 80 ||
        !c.ticks_per_second || !stop_ticks_ ||
        stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto p = s_.context.read(0x10000130), r = s_.context.read(0x10000134);
    if (!p || !r)
      return std::unexpected(system_error::mmio);
    // Engineering A's RATIO register is nonfunctional (128).
    if (!mcu::nrf52840::serial_revision{*p, *r}.reviewed_modern())
      return std::unexpected(system_error::unsupported);
    auto claim = s_.acquire(peripheral::pdm, base, Binding::pins);
    if (!claim)
      return claim;
    config_ = c;
    s_.publish();
    if (!s_.context.gpio.configure(
            Binding::pins[0], {.direction = driver::gpio_direction::output}) ||
        !s_.context.gpio.configure(
            Binding::pins[1], {.direction = driver::gpio_direction::input}) ||
        !s_.write(0x308, 7) || !clear_events() ||
        !s_.write(0x504, pdm_clock_words[unsigned(c.frequency)]) ||
        !s_.write(0x508, (c.mono ? 1U : 0U) | (c.left_rising ? 2U : 0U)) ||
        !s_.write(0x518, c.gain_left) || !s_.write(0x51C, c.gain_right) ||
        !s_.write(0x520, c.ratio80) ||
        !s_.write(0x540, session::select(Binding::pins[0])) ||
        !s_.write(0x544, session::select(Binding::pins[1])) ||
        !s_.write(0x500, 1))
      return fail();
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result start(std::span<std::int16_t> first,
                                    std::span<std::int16_t> discard_tail,
                                    std::uint64_t maximum_blocks = 1) noexcept {
    if (!initialized_ || running_ || fault_ || !storage_.empty() ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    samples_ = first.size();
    if (!samples_ || samples_ > 0x7FFF || (!config_.mono && samples_ % 2) ||
        !maximum_blocks)
      return std::unexpected(system_error::invalid);
    auto ticks = std::uint64_t(samples_) *
                 pdm_clock_divisors[unsigned(config_.frequency)] *
                 (config_.ratio80 ? 80 : 64) * config_.ticks_per_second /
                 (32'000'000ULL * (config_.mono ? 1 : 2));
    if (ticks < 2 || ticks > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    block_ticks_ = ticks;
    maximum_blocks_ = maximum_blocks;
    result_ = {};
    auto tail = storage_.prepare({{}, discard_tail, 0}, false, true, samples_,
                                 {0x7FFF, 2, 4}, true);
    if (!tail)
      return tail;
    auto front =
        storage_.prepare({{}, first, 0}, false, true, samples_, {0x7FFF, 2, 4});
    if (!front) {
      (void)storage_.release_all_quiet();
      return front;
    }
    current_ = previous_ = storage::none;
    pending_ = storage_.next();
    started_seen_ = end_seen_ = false;
    handoff_at_.reset();
    if (!clear_events() || !s_.write(0x564, samples_)) {
      (void)storage_.release_all_quiet();
      return fail();
    }
    running_ = true;
    stopping_ = stopped_seen_ = false;
    if (!storage_.arm(storage::guard) || !storage_.arm(pending_) ||
        !write_pointer(pending_))
      return fail();
    last_poll_ = s_.context.execution.now();
    if (!s_.write(0, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_result enqueue(std::span<std::int16_t> buffer,
                                      std::uint64_t sequence = 0) noexcept {
    return s_.context.execution.critical([&]() -> system_result {
      if (!running_ || stopping_ || fault_)
        return std::unexpected(system_error::busy);
      return storage_.prepare({{}, buffer, sequence}, false, true, samples_,
                              {0x7FFF, 2, 4});
    });
  }
  [[nodiscard]] std::optional<audio_block<std::int16_t>>
  take_completed() noexcept {
    return s_.context.execution.critical([&] { return storage_.take(); });
  }
  [[nodiscard]] system_result poll() noexcept {
    return s_.context.execution.critical([&]() -> system_result {
      if (!running_)
        return {};
      if (stopping_)
        return observe_stop();
      auto now = s_.context.execution.now();
      if (static_cast<execution::tick>(now - last_poll_) >= block_ticks_) {
        result_.service_overrun = true;
        return request_stop();
      }
      last_poll_ = now;
      if (handoff_at_ &&
          static_cast<execution::tick>(now - *handoff_at_) >= block_ticks_) {
        result_.service_overrun = true;
        return request_stop();
      }
      // END can arrive immediately before STARTED. Latch both before clearing;
      // a lost MMIO acknowledgement must never lose the ownership transition.
      auto started = s_.read(0x100), ended = s_.read(0x108);
      if (!started || !ended)
        return fail();
      if (!handoff_at_ && (*ended || (*started && current_ != storage::none)))
        handoff_at_ = now;
      started_seen_ = started_seen_ || *started;
      end_seen_ = end_seen_ || *ended;
      if ((*started && !s_.write(0x100, 0)) || (*ended && !s_.write(0x108, 0)))
        return fail();
      // END means a complete block, but its address may still be the buffered
      // pointer if foreground missed STARTED. Require the next latch as well.
      if (started_seen_) {
        if (previous_ != storage::none) {
          result_.service_overrun = true;
          return request_stop();
        }
        previous_ = current_;
        current_ = pending_;
        started_seen_ = false;
        if (current_ != storage::guard) {
          if (result_.completed_blocks + (previous_ == storage::none ? 1 : 2) >=
              maximum_blocks_)
            pending_ = storage::guard;
          else {
            pending_ = storage_.next();
            if (pending_ == storage::none) {
              pending_ = storage::guard;
              result_.exhausted = true;
            }
          }
          if (pending_ != storage::guard && !storage_.arm(pending_))
            return fail();
          if (!write_pointer(pending_))
            return fail();
        }
      }
      if (end_seen_ && previous_ != storage::none) {
        if (static_cast<execution::tick>(s_.context.execution.now() -
                                         last_poll_) >= block_ticks_) {
          result_.service_overrun = true;
          return request_stop();
        }
        if (!storage_.publish_quiet(previous_))
          return fail();
        ++result_.completed_blocks;
        previous_ = storage::none;
        end_seen_ = false;
        handoff_at_.reset();
      }
      if (current_ == storage::guard && previous_ == storage::none)
        return request_stop();
      return {};
    });
  }
  [[nodiscard]] system_result interrupt_mode(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, 7) ? system_result{} : fail();
  }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (running_) {
      result_.cancelled = true;
      if (!stopped_seen_)
        (void)request_stop();
      if (!s_.bounded(
              [&] {
                auto r = observe_stop();
                return r && !running_;
              },
              stop_ticks_, PollBudget))
        return std::unexpected(system_error::timeout);
    }
    return {};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    auto r = cancel();
    if (!r)
      return r;
    if (!s_.write(0x308, 7) || !s_.disabled() ||
        !s_.write(0x540, 0x80000000U) || !s_.write(0x544, 0x80000000U) ||
        !clear_events())
      return fail();
    r = s_.release();
    if (r)
      initialized_ = fault_ = false;
    return r;
  }
  [[nodiscard]] bool running() const noexcept { return running_; }
  [[nodiscard]] bool owns_resource() const noexcept { return s_.owned(); }
  [[nodiscard]] pdm_stream_result result() const noexcept { return result_; }

private:
  bool clear_events() {
    return s_.write(0x100, 0) && s_.write(0x104, 0) && s_.write(0x108, 0);
  }
  bool write_pointer(int i) {
    auto &b = storage_.block(i);
    return s_.write(0x560, s_.address(b.rx.data(), b.rx.size_bytes()));
  }
  system_result request_stop() {
    stopping_ = true;
    if (!s_.write(4, 1))
      return fail();
    return {};
  }
  system_result observe_stop() {
    auto r = s_.read(0x104);
    if (!r)
      return fail();
    stopped_seen_ = stopped_seen_ || *r;
    if (!stopped_seen_)
      return std::unexpected(system_error::busy);
    if (!s_.write(0x104, 0) || !storage_.release_all_quiet() || !clear_events())
      return fail();
    running_ = stopping_ = false;
    return {};
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x4001D000;
  session s_;
  storage storage_;
  pdm_configuration config_{};
  pdm_stream_result result_{};
  std::size_t samples_{};
  std::uint64_t maximum_blocks_{};
  int current_{storage::none}, pending_{storage::none},
      previous_{storage::none};
  execution::tick stop_ticks_, block_ticks_{}, last_poll_{};
  std::optional<execution::tick> handoff_at_;
  bool initialized_{}, running_{}, stopping_{}, stopped_seen_{}, fault_{},
      started_seen_{}, end_seen_{};
};
} // namespace mcubsp::backend::nrf52840
