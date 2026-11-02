#pragma once
// @req-file REQ-0067 REQ-0062
// PS v1.11 6.11; anomalies 55,170,194,196.
#include <mcubsp/backend/nrf52840/detail/audio_storage.hpp>
#include <mcubsp/mcu/nrf52840/product/i2s.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
template <core::pin_id Sck, core::pin_id Lrck,
          core::pin_id Din = detail::disconnected,
          core::pin_id Dout = detail::disconnected,
          core::pin_id Mck = detail::disconnected>
struct i2s_binding {
  static constexpr std::array pins{Mck, Sck, Lrck, Din, Dout};
  static_assert(Sck != detail::disconnected && Lrck != detail::disconnected &&
                (Din != detail::disconnected || Dout != detail::disconnected));
  static_assert(
      [] {
        for (unsigned i = 0; i < 5; ++i)
          if (pins[i] != detail::disconnected) {
            if (!mcu::nrf52840::gpio_supported(pins[i]) ||
                mcu::nrf52840::product::silicon_low_frequency_only(pins[i]))
              return false;
            for (unsigned j = 0; j < i; ++j)
              if (pins[i] == pins[j])
                return false;
          }
        return true;
      }(),
      "I2S requires distinct high-frequency GPIOs");
};
inline constexpr std::array<unsigned, 13> i2s_divisors{
    8, 10, 11, 15, 16, 21, 23, 30, 31, 32, 42, 63, 125};
inline constexpr std::array<std::uint32_t, 13> i2s_mck_words{
    0x20000000, 0x18000000, 0x16000000, 0x11000000, 0x10000000,
    0xC000000,  0xB000000,  0x8800000,  0x8400000,  0x8000000,
    0x6000000,  0x4100000,  0x20C0000};
inline constexpr std::array<unsigned, 9> i2s_ratios{32,  48,  64,  96, 128,
                                                    192, 256, 384, 512};
struct i2s_timing {
  unsigned divisor{}, ratio{}, sample_hz{}, error_ppm{};
};
[[nodiscard]] constexpr system_value<i2s_timing>
solve_i2s_timing(unsigned sample_hz, unsigned width = 16,
                 unsigned tolerance_ppm = 10000) noexcept {
  if (!sample_hz || (width != 8 && width != 16 && width != 24))
    return std::unexpected(system_error::invalid);
  std::optional<i2s_timing> best;
  for (auto d : i2s_divisors)
    for (auto r : i2s_ratios)
      if (r % (2 * width) == 0) {
        auto desired = std::uint64_t(sample_hz) * d * r;
        auto delta =
            desired > 32'000'000 ? desired - 32'000'000 : 32'000'000 - desired;
        auto ppm = (delta * 1'000'000 + desired / 2) / desired;
        if (ppm <= tolerance_ppm && (!best || ppm < best->error_ppm))
          best =
              i2s_timing{d, r, unsigned((32'000'000ULL + d * r / 2) / (d * r)),
                         unsigned(ppm)};
      }
  if (!best)
    return std::unexpected(system_error::invalid);
  return *best;
}
enum class i2s_channels : unsigned { stereo, left, right };
struct i2s_configuration {
  bool slave{}, generate_master_clock{true}, receive{true}, transmit{true},
      aligned_format{}, right_aligned_input{};
  unsigned width{16};
  i2s_channels channels{i2s_channels::stereo};
  i2s_timing timing{32, 32, 31250, 0};
  // The application supplies the execution clock rate and (for slave mode)
  // the fastest external LRCK. Service must occur more often than one block.
  unsigned ticks_per_second{64'000'000}, maximum_external_sample_hz{};
};
struct audio_stream_result {
  std::uint64_t completed_blocks{};
  bool exhausted{}, service_overrun{}, cancelled{};
};
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned Capacity = 4,
          unsigned PollBudget = 1000000>
class i2s_controller {
  static_assert(Capacity > 0);
  using session = detail::peripheral_session<Context, Buffers, Address>;
  using storage = detail::audio_storage<session, std::uint32_t, Capacity>;

public:
  i2s_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                 execution::tick stop_ticks = 1'280'000,
                 Address a = {}) noexcept
      : s_(c, clocks, b, a), storage_(s_), stop_ticks_(stop_ticks) {}
  i2s_controller(const i2s_controller &) = delete;
  i2s_controller &operator=(const i2s_controller &) = delete;
  ~i2s_controller() {
    if (storage_.active_leases())
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result initialize(i2s_configuration c = {}) noexcept {
    if (!valid(c) || !stop_ticks_ ||
        stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto p = s_.context.read(0x10000130), r = s_.context.read(0x10000134);
    if (!p || !r)
      return std::unexpected(system_error::mmio);
    if (!mcu::nrf52840::serial_revision{*p, *r}.reviewed_modern())
      return std::unexpected(system_error::unsupported);
    auto claimed = s_.acquire(peripheral::i2s, base, Binding::pins);
    if (!claimed)
      return claimed;
    config_ = c;
    s_.publish();
    for (unsigned i = 0; i < 5; ++i)
      if (Binding::pins[i] != detail::disconnected) {
        bool output = i == 0 || i == 4 || (!c.slave && (i == 1 || i == 2));
        if (!s_.context.gpio.configure(
                Binding::pins[i],
                {.direction = output ? driver::gpio_direction::output
                                     : driver::gpio_direction::input}))
          return fail();
      }
    if (!s_.write(0x308, 38) || !clear_events() || !s_.write(0x504, c.slave) ||
        !s_.write(0x508, c.receive) || !s_.write(0x50C, c.transmit) ||
        !s_.write(0x510, c.generate_master_clock) ||
        !s_.write(0x514, i2s_mck_words[divisor_index(c.timing.divisor)]) ||
        !s_.write(0x518, ratio_index(c.timing.ratio)) ||
        !s_.write(0x51C, c.width == 8    ? 0
                         : c.width == 16 ? 1
                                         : 2) ||
        !s_.write(0x520, c.right_aligned_input) ||
        !s_.write(0x524, c.aligned_format) ||
        !s_.write(0x528, static_cast<unsigned>(c.channels)))
      return fail();
    // 170: never infer CONNECT from readback. 196: connect only after claims,
    // directly before ENABLE; every shutdown explicitly disconnects all pins.
    for (unsigned i = 0; i < 5; ++i)
      if (!s_.write(0x560 + 4 * i, session::select(Binding::pins[i])))
        return fail();
    if (!s_.write(0x500, 1))
      return fail();
    initialized_ = true;
    return {};
  }
  // Tail is a caller-owned discard/silence block, same size as every payload.
  // It absorbs the unavoidable handoff before STOP and is never published.
  [[nodiscard]] system_result start(audio_block<std::uint32_t> first,
                                    audio_block<std::uint32_t> tail,
                                    std::uint64_t maximum_blocks = 1) noexcept {
    if (!initialized_ || running_ || fault_ || !storage_.empty() ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    words_ = config_.receive ? first.rx.size() : first.tx.size();
    if (!words_ || words_ > 0x3FFF || !maximum_blocks ||
        (config_.width == 24 && config_.channels == i2s_channels::stereo &&
         words_ % 2))
      return std::unexpected(system_error::invalid);
    auto samples_per_word = config_.width == 24 ? 1U : 32 / config_.width;
    auto samples_per_frame = config_.channels == i2s_channels::stereo ? 2U : 1U;
    auto rate =
        config_.slave
            ? config_.maximum_external_sample_hz
            : unsigned((32'000'000ULL +
                        config_.timing.divisor * config_.timing.ratio - 1) /
                       (config_.timing.divisor * config_.timing.ratio));
    auto ticks = std::uint64_t(words_) * samples_per_word *
                 config_.ticks_per_second /
                 (samples_per_frame * std::uint64_t(rate));
    if (ticks < 2 || ticks > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    block_ticks_ = ticks;
    max_blocks_ = maximum_blocks;
    result_ = {};
    auto a = storage_.prepare(tail, config_.transmit, config_.receive, words_,
                              {0x3FFF, 4, 4}, true);
    if (!a)
      return a;
    auto b = storage_.prepare(first, config_.transmit, config_.receive, words_,
                              {0x3FFF, 4, 4});
    if (!b) {
      (void)storage_.release_all_quiet();
      return b;
    }
    current_ = storage::none;
    pending_ = storage_.next();
    if (!clear_events() || !s_.write(0x550, words_)) {
      (void)storage_.release_all_quiet();
      return fail();
    }
    running_ = true;
    stopping_ = stopped_seen_ = false;
    // Tail is armed too: hardware may latch it at the next boundary.
    if (!storage_.arm(storage::guard) || !arm_pointer(pending_))
      return fail();
    running_ = true;
    stopping_ = stopped_seen_ = false;
    seen_ = 0;
    handoff_at_.reset();
    last_poll_ = s_.context.execution.now();
    if (!s_.write(0, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_result
  enqueue(audio_block<std::uint32_t> block) noexcept {
    return s_.context.execution.critical([&]() -> system_result {
      if (!running_ || stopping_ || fault_)
        return std::unexpected(system_error::busy);
      return storage_.prepare(block, config_.transmit, config_.receive, words_,
                              {0x3FFF, 4, 4});
    });
  }
  [[nodiscard]] std::optional<audio_block<std::uint32_t>>
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
      for (auto [off, bit] : std::array<std::pair<unsigned, unsigned>, 2>{
               {{0x104, 1}, {0x114, 2}}}) {
        if ((bit == 1 && !config_.receive) || (bit == 2 && !config_.transmit))
          continue;
        auto r = s_.read(off);
        if (!r)
          return fail();
        if (*r) {
          if (!handoff_at_)
            handoff_at_ = now;
          seen_ |= bit;
          if (!s_.write(off, 0))
            return fail();
        }
      }
      unsigned needed =
          (config_.receive ? 1U : 0U) | (config_.transmit ? 2U : 0U);
      if (seen_ != needed)
        return {};
      // Exactly one latch per direction since the previous observation is
      // mandatory. A missed service interval stops without publishing memory.
      if (static_cast<execution::tick>(s_.context.execution.now() -
                                       last_poll_) >= block_ticks_) {
        result_.service_overrun = true;
        return request_stop();
      }
      seen_ = 0;
      handoff_at_.reset();
      if (current_ != storage::none) {
        if (!storage_.publish_quiet(current_))
          return fail();
        ++result_.completed_blocks;
      }
      current_ = pending_;
      if (current_ == storage::guard) {
        return request_stop();
      }
      if (result_.completed_blocks + 1 >= max_blocks_)
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
      return {};
    });
  }
  [[nodiscard]] system_result interrupt_mode(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, 38) ? system_result{} : fail();
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
    if (!s_.write(0x308, 38) || !s_.disabled())
      return fail();
    for (unsigned i = 0; i < 5; ++i)
      if (!s_.write(0x560 + 4 * i, 0x80000000U))
        return fail();
    if (!clear_events())
      return fail();
    r = s_.release();
    if (r) {
      initialized_ = fault_ = false;
    }
    return r;
  }
  [[nodiscard]] bool running() const noexcept { return running_; }
  [[nodiscard]] bool owns_resource() const noexcept { return s_.owned(); }
  [[nodiscard]] audio_stream_result result() const noexcept { return result_; }

private:
  static unsigned divisor_index(unsigned d) {
    for (unsigned i = 0; i < 13; ++i)
      if (i2s_divisors[i] == d)
        return i;
    return 13;
  }
  static unsigned ratio_index(unsigned r) {
    for (unsigned i = 0; i < 9; ++i)
      if (i2s_ratios[i] == r)
        return i;
    return 9;
  }
  static bool valid(i2s_configuration c) {
    return (c.width == 8 || c.width == 16 || c.width == 24) &&
           unsigned(c.channels) <= 2 && (c.receive || c.transmit) &&
           (!c.receive || Binding::pins[3] != detail::disconnected) &&
           (!c.transmit || Binding::pins[4] != detail::disconnected) &&
           (c.slave || c.generate_master_clock) &&
           (!c.slave || c.maximum_external_sample_hz) && c.ticks_per_second &&
           divisor_index(c.timing.divisor) < 13 &&
           ratio_index(c.timing.ratio) < 9 &&
           c.timing.ratio % (2 * c.width) == 0;
  }
  bool clear_events() {
    return s_.write(0x104, 0) && s_.write(0x108, 0) && s_.write(0x114, 0);
  }
  bool write_pointer(int slot) {
    auto &b = storage_.block(slot);
    return (!config_.receive ||
            s_.write(0x538, s_.address(b.rx.data(), b.rx.size_bytes()))) &&
           (!config_.transmit ||
            s_.write(0x540, s_.address(b.tx.data(), b.tx.size_bytes())));
  }
  bool arm_pointer(int slot) {
    return storage_.arm(slot) && write_pointer(slot);
  }
  system_result request_stop() {
    stopping_ = true;
    if (!s_.write(4, 1) || !s_.write(0x38, 1) || !s_.write(0x3C, 1))
      return fail();
    return {};
  }
  system_result observe_stop() {
    auto stopped = s_.read(0x108);
    if (!stopped)
      return fail();
    if (*stopped)
      stopped_seen_ = true;
    if (!stopped_seen_)
      return std::unexpected(system_error::busy);
    if (!s_.write(0x108, 0) || !s_.write(0x38, 1) || !s_.write(0x3C, 1) ||
        !storage_.release_all_quiet())
      return fail();
    // 55: ignore PTRUPD after STOP; no partial word count exists in hardware.
    if (!clear_events())
      return fail();
    running_ = stopping_ = false;
    return {};
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x40025000;
  session s_;
  storage storage_;
  i2s_configuration config_{};
  audio_stream_result result_{};
  std::size_t words_{};
  std::uint64_t max_blocks_{};
  int current_{storage::none}, pending_{storage::none};
  execution::tick stop_ticks_, block_ticks_{}, last_poll_{};
  std::optional<execution::tick> handoff_at_;
  unsigned seen_{};
  bool initialized_{}, running_{}, stopping_{}, stopped_seen_{}, fault_{};
};
} // namespace mcubsp::backend::nrf52840
