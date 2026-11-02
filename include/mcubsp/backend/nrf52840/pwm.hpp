#pragma once
// @req-file REQ-0022 REQ-0063 REQ-0062
// Nordic PS v1.11 6.17. STOPPED, not SEQEND, is the release boundary.
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
#include <mcubsp/driver/pwm.hpp>
#include <mcubsp/mcu/nrf52840/product/pwm0.hpp>
namespace mcubsp::backend::nrf52840 {
template <unsigned I, core::pin_id P0, core::pin_id P1 = detail::disconnected,
          core::pin_id P2 = detail::disconnected,
          core::pin_id P3 = detail::disconnected>
struct pwm_binding {
  static_assert(I < 4, "PWM instance must be 0 through 3");
  static constexpr std::array pins{P0, P1, P2, P3};
  static_assert(
      [] {
        for (unsigned i = 0; i < 4; ++i) {
          if (pins[i] == detail::disconnected)
            continue;
          if (!mcu::nrf52840::gpio_supported(pins[i]))
            return false;
          for (unsigned j = 0; j < i; ++j)
            if (pins[i] == pins[j])
              return false;
        }
        return P0 != detail::disconnected;
      }(),
      "PWM pins must be available and distinct");
  static constexpr unsigned instance = I;
  static constexpr auto pin = P0;
  static constexpr auto peripheral_id =
      std::array{peripheral::pwm0, peripheral::pwm1, peripheral::pwm2,
                 peripheral::pwm3}[I];
  static constexpr std::uintptr_t base = std::array<std::uintptr_t, 4>{
      0x4001C000, 0x40021000, 0x40022000, 0x4002D000}[I];
};
enum class pwm_alignment : std::uint8_t { edge, center };
enum class pwm_decoder : std::uint8_t { common, grouped, individual, waveform };
struct pwm_timing : driver::pwm_timing {
  pwm_alignment alignment{};
};
[[nodiscard]] constexpr std::expected<pwm_timing, driver::pwm_error>
solve_pwm_timing(driver::pwm_timing_request r,
                 pwm_alignment alignment = pwm_alignment::edge) noexcept {
  using error = driver::pwm_error;
  if (r.clock_hz != 16'000'000)
    return std::unexpected(error::invalid_clock);
  if (r.counter_width != 15)
    return std::unexpected(error::invalid_counter_width);
  if (!r.target_hz || static_cast<unsigned>(alignment) > 1)
    return std::unexpected(error::invalid_frequency);
  std::optional<pwm_timing> best;
  unsigned factor = alignment == pwm_alignment::center ? 2 : 1;
  for (unsigned p = 0; p < 8; ++p) {
    auto divisor = std::uint64_t(r.target_hz) * (1U << p) * factor;
    auto top = (r.clock_hz + divisor / 2) / divisor;
    if (top < 3 || top > 32767)
      continue;
    auto denominator = (1U << p) * factor * top;
    auto product = std::uint64_t(r.target_hz) * denominator;
    auto difference =
        product > r.clock_hz ? product - r.clock_hz : r.clock_hz - product;
    auto ppm = (difference * 1'000'000 + product / 2) / product;
    if (ppm > r.maximum_error_ppm)
      continue;
    if (!best || ppm < best->error_ppm)
      best = pwm_timing{{r.clock_hz, r.target_hz,
                         static_cast<std::uint32_t>(
                             (r.clock_hz + denominator / 2) / denominator),
                         static_cast<std::uint32_t>(ppm), p, 15, top},
                        alignment};
  }
  if (!best)
    return std::unexpected(error::unattainable_tolerance);
  return *best;
}
struct pwm_configuration {
  pwm_decoder decoder{pwm_decoder::individual};
  bool next_step{};
  std::array<bool, 4> idle_high{};
};
struct pwm_sequence {
  std::span<const std::uint16_t> values;
  std::uint32_t refresh{}, end_delay{};
};
struct pwm_playback {
  unsigned first{}, loops{};
  bool repeat{};
};
struct pwm_result {
  std::uint32_t ended_sequences{};
  bool loops_done{};
};
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned StopPollBudget = 1000000>
class pwm_controller {
  using error = driver::pwm_error;
  using Session = detail::peripheral_session<Context, Buffers, Address>;
  using E = typename Session::execution_type;

public:
  using pool_type = core::operation_pool<pwm_result, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  pwm_controller(Context &c, clock_controller<Context> &clocks,
                 Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                 Address address = {}) noexcept
      : s_(c, clocks, buffers, address), operations_(c.execution),
        stop_ticks_(stop_ticks) {}
  pwm_controller(const pwm_controller &) = delete;
  pwm_controller &operator=(const pwm_controller &) = delete;
  ~pwm_controller() {
    if (tokens_[0] || tokens_[1])
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(pwm_timing timing, pwm_configuration config = {}) noexcept {
    if (s_.owned() || s_.context.execution.in_isr())
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::mmio);
    auto solved = solve_pwm_timing({timing.clock_hz, timing.target_hz,
                                    timing.error_ppm, timing.counter_width},
                                   timing.alignment);
    if (!solved || solved->prescaler != timing.prescaler ||
        solved->period_counts != timing.period_counts ||
        solved->achieved_hz != timing.achieved_hz || !stop_ticks_ ||
        stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(error::invalid_frequency);
    if (static_cast<unsigned>(config.decoder) > 3 ||
        (config.decoder == pwm_decoder::waveform &&
         Binding::pins[3] != detail::disconnected))
      return std::unexpected(error::invalid_duty);
    for (auto pin : Binding::pins)
      if (pin != detail::disconnected &&
          mcu::nrf52840::product::silicon_low_frequency_only(pin) &&
          timing.achieved_hz > 10000)
        return std::unexpected(error::invalid_frequency);
    if (!s_.acquire(Binding::peripheral_id, Binding::base, Binding::pins))
      return std::unexpected(error::already_owned);
    auto enabled = s_.read(0x500);
    if (!enabled || *enabled) {
      (void)s_.release();
      return std::unexpected(enabled ? error::already_owned : error::mmio);
    }
    timing_ = timing;
    config_ = config;
    for (unsigned i = 0; i < 4; ++i)
      if (Binding::pins[i] != detail::disconnected &&
          !s_.context.gpio.configure(
              Binding::pins[i],
              {.direction = driver::gpio_direction::output,
               .initial_level = config.idle_high[i] ? driver::gpio_level::high
                                                    : driver::gpio_level::low}))
        return init_failed();
    if (!s_.write(0x308, irq_mask) || !s_.clear(events) ||
        !s_.write(0x200, 0) ||
        !s_.write(0x504, static_cast<unsigned>(timing.alignment)) ||
        !s_.write(0x508, timing.period_counts) ||
        !s_.write(0x50C, timing.prescaler) ||
        !s_.write(0x510, static_cast<unsigned>(config.decoder) |
                             (config.next_step ? 256U : 0U)) ||
        !s_.write(0x514, 0))
      return init_failed();
    for (unsigned i = 0; i < 4; ++i)
      if (!s_.write(0x560 + 4 * i, Session::select(Binding::pins[i])))
        return init_failed();
    if (!s_.write(0x500, 1))
      return init_failed();
    ready_ = true;
    s_.publish();
    return {};
  }
  [[nodiscard]] start_result play(pwm_sequence zero, pwm_sequence one = {},
                                  pwm_playback playback = {}) noexcept {
    if (!ready_ || fault_ || s_.context.execution.in_isr())
      return failure(error::not_initialized);
    if (playback.first > 1 || playback.loops > 65535 ||
        (!playback.loops && playback.repeat) ||
        (playback.loops && (zero.values.empty() || one.values.empty())))
      return failure(error::invalid_duty);
    std::array seq{zero, one};
    if (seq[playback.first].values.empty())
      return failure(error::invalid_duty);
    const unsigned stride = config_.decoder == pwm_decoder::common    ? 1
                            : config_.decoder == pwm_decoder::grouped ? 2
                                                                      : 4;
    // Anomaly 183: no automatic restart of one-value sequence 1.
    if (playback.repeat && one.values.size() == 1)
      return failure(error::invalid_duty);
    for (auto q : seq)
      if (!q.values.empty()) {
        if (q.values.size() > 32767 || q.values.size() % stride ||
            q.refresh > 0xFFFFFF || q.end_delay > 0xFFFFFF)
          return failure(error::invalid_duty);
        if (config_.decoder == pwm_decoder::waveform)
          for (std::size_t i = 3; i < q.values.size(); i += 4) {
            auto top = q.values[i];
            if (top < 3 || top > 32767)
              return failure(error::invalid_duty);
            for (auto pin : Binding::pins)
              if (pin != detail::disconnected &&
                  mcu::nrf52840::product::silicon_low_frequency_only(pin) &&
                  16'000'000U /
                          ((1U << timing_.prescaler) * top *
                           (timing_.alignment == pwm_alignment::center ? 2U
                                                                       : 1U)) >
                      10000)
                return failure(error::invalid_frequency);
          }
      }
    return s_.context.execution.critical([&]() -> start_result {
      if (id_ || active_)
        return failure(error::already_running);
      for (unsigned i = 0; i < 2; ++i)
        if (!seq[i].values.empty()) {
          auto t =
              s_.prepare(seq[i].values.data(), seq[i].values.size_bytes(),
                         seq[i].values.size(), {32767, 2, 2}, dma_access::read);
          if (!t) {
            discard_all();
            return failure(error::invalid_duty);
          }
          tokens_[i] = *t;
        }
      auto op = operations_.try_start({.quiesce = &cancel, .context = this});
      if (!op) {
        discard_all();
        return std::unexpected(op.error());
      }
      id_ = op->id();
      quiet_ = false;
      result_ = {};
      pending_.reset();
      auto shorts = playback.loops
                        ? (playback.repeat ? (playback.first ? 8U : 4U) : 16U)
                        : (playback.first ? 2U : 1U);
      bool ok = s_.clear(events) && s_.write(0x200, shorts) &&
                s_.write(0x514, playback.loops);
      for (unsigned i = 0; i < 2 && ok; ++i) {
        auto off = 0x520 + 0x20 * i;
        auto q = seq[i];
        auto ptr = q.values.empty()
                       ? s_.address(seq[playback.first].values.data(),
                                    seq[playback.first].values.size_bytes())
                       : s_.address(q.values.data(), q.values.size_bytes());
        ok = s_.write(off, ptr) && s_.write(off + 4, q.values.size()) &&
             s_.write(off + 8, q.refresh) && s_.write(off + 12, q.end_delay);
      }
      if (!ok) {
        discard_all();
        finish(error::mmio);
        return op;
      }
      for (auto token : tokens_)
        if (token && !s_.buffers.arm(*token)) {
          (void)release_all();
          finish(error::mmio);
          return op;
        }
      active_ = true;
      if (!s_.write(8 + 4 * playback.first, 1)) {
        fault_ = true;
        pending_ = error::mmio;
      }
      return op;
    });
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_)
      return false;
    return s_.context.execution.critical([&] {
      bool progress = false;
      for (unsigned i = 0; i < 2; ++i) {
        auto end = s_.read(0x110 + 4 * i);
        if (!end) {
          fault_ = true;
          pending_ = error::mmio;
        } else if (*end) {
          result_.ended_sequences |= 1U << i;
          progress = true;
          if (!s_.write(0x110 + 4 * i, 0)) {
            fault_ = true;
            pending_ = error::mmio;
          }
        }
      }
      auto loop = s_.read(0x11C);
      if (!loop) {
        fault_ = true;
        pending_ = error::mmio;
      } else if (*loop) {
        result_.loops_done = true;
        progress = true;
        if (!s_.write(0x11C, 0)) {
          fault_ = true;
          pending_ = error::mmio;
        }
      }
      auto stopped = s_.read(0x104);
      if (!stopped) {
        fault_ = true;
        pending_ = error::mmio;
      }
      if (quiet_ || pending_ || (stopped && *stopped)) {
        if (!stop_step()) {
          if (fault_)
            (void)s_.write(0x308, irq_mask);
          return false;
        }
        finish(pending_);
        return true;
      }
      return progress;
    });
  }
  [[nodiscard]] std::expected<void, error> next_step() noexcept {
    if (!id_ || !config_.next_step || fault_)
      return std::unexpected(error::not_running);
    if (!s_.write(0x10, 1))
      return std::unexpected(error::mmio);
    return {};
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool on) noexcept {
    if (!ready_ || s_.context.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!s_.write(on ? 0x304 : 0x308, irq_mask))
      return std::unexpected(error::mmio);
    return {};
  }
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (s_.context.execution.in_isr() || id_)
      return std::unexpected(error::already_running);
    if (!s_.owned())
      return {};
    if (active_ &&
        !s_.bounded([&] { return stop_step(); }, stop_ticks_, StopPollBudget))
      return std::unexpected(error::mmio);
    if (!s_.write(0x308, irq_mask) || !s_.write(0x200, 0) || !s_.disabled())
      return std::unexpected(error::mmio);
    for (unsigned i = 0; i < 4; ++i)
      if (!s_.write(0x560 + 4 * i, 0x80000000))
        return std::unexpected(error::mmio);
    if (!s_.release())
      return std::unexpected(error::mmio);
    ready_ = false;
    return {};
  }
  [[nodiscard]] bool running() const noexcept { return active_; }
  [[nodiscard]] bool owns_resource() const noexcept { return s_.owned(); }

private:
  start_result failure(error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  void finish(std::optional<error> error) noexcept {
    auto id = *id_;
    id_.reset();
    if (error)
      (void)operations_.fail(id, *error);
    else
      (void)operations_.complete(id, result_);
  }
  void discard_all() noexcept {
    for (auto &t : tokens_)
      s_.discard(t);
  }
  bool release_all() noexcept {
    for (auto &t : tokens_)
      if (!s_.release_quiet(t))
        return false;
    return true;
  }
  bool stop_step() noexcept {
    if (!quiet_) {
      auto stop = s_.read(0x104);
      if (!stop) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      if (!*stop) {
        if (!s_.write(0x200, 0) || !s_.write(4, 1)) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        stop = s_.read(0x104);
        if (!stop || !*stop)
          return false;
      }
      quiet_ = true;
    }
    if (!release_all()) {
      fault_ = true;
      return false;
    }
    active_ = false;
    if (!s_.clear(events)) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    return true;
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &self = *static_cast<pwm_controller *>(p);
    if (!self.id_ || *self.id_ != id)
      return core::quiesce_result::reusable;
    bool quiet = self.s_.bounded([&] { return self.stop_step(); },
                                 self.stop_ticks_, StopPollBudget);
    self.id_.reset();
    self.fault_ |= !quiet;
    if (!quiet)
      (void)self.s_.write(0x308, irq_mask);
    return quiet ? core::quiesce_result::reusable
                 : core::quiesce_result::faulted;
  }
  std::expected<void, error> init_failed() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(error::mmio);
  }
  inline static constexpr std::array<unsigned, 7> events{
      0x104, 0x108, 0x10C, 0x110, 0x114, 0x118, 0x11C};
  static constexpr auto irq_mask =
      mcu::nrf52840::product::pwm0::intenclr::writable_mask;
  Session s_;
  pool_type operations_;
  execution::tick stop_ticks_;
  pwm_timing timing_{};
  pwm_configuration config_{};
  std::array<std::optional<dma_buffer_token>, 2> tokens_{};
  std::optional<core::operation_id> id_;
  std::optional<error> pending_;
  pwm_result result_{};
  bool ready_{}, active_{}, quiet_{}, fault_{};
};
} // namespace mcubsp::backend::nrf52840
