#pragma once
// @req-file REQ-0029 REQ-0064 REQ-0062
// Nordic PS v1.11 6.23; anomalies 212/237/241/252/266/276.
#include <mcubsp/backend/nrf52840/detail/dma_queue.hpp>
#include <mcubsp/driver/adc.hpp>
#include <mcubsp/mcu/nrf52840/product/saadc.hpp>
namespace mcubsp::backend::nrf52840 {
[[nodiscard]] constexpr std::optional<core::pin_id>
analog_input_pin(unsigned ain) noexcept {
  constexpr std::array<core::pin_id, 8> pins{
      {{0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 28}, {0, 29}, {0, 30}, {0, 31}}};
  return ain < 8 ? std::optional{pins[ain]} : std::nullopt;
}
enum class saadc_input : std::uint8_t {
  disconnected,
  ain0,
  ain1,
  ain2,
  ain3,
  ain4,
  ain5,
  ain6,
  ain7,
  vdd,
  vddh_div5 = 13
};
enum class saadc_gain : std::uint8_t {
  one_sixth,
  one_fifth,
  one_quarter,
  one_third,
  one_half,
  one,
  two,
  four
};
enum class saadc_reference : std::uint8_t { internal_0v6, vdd_quarter };
enum class saadc_acquisition : std::uint8_t {
  us3,
  us5,
  us10,
  us15,
  us20,
  us40
};
enum class saadc_resistor : std::uint8_t {
  bypass,
  pulldown,
  pullup,
  mid_supply
};
struct saadc_channel_configuration {
  unsigned channel{};
  saadc_input positive{saadc_input::ain0}, negative{saadc_input::disconnected};
  saadc_gain gain{saadc_gain::one_sixth};
  saadc_reference reference{saadc_reference::internal_0v6};
  saadc_acquisition acquisition{saadc_acquisition::us10};
  saadc_resistor positive_resistor{}, negative_resistor{};
  bool differential{}, burst{};
  std::int16_t low_limit{-32768}, high_limit{32767};
};
struct saadc_configuration {
  unsigned resolution_bits{12}, oversample_log2{}, timer_cc{};
  bool automatic_sample{true};
  unsigned minimum_supply_mv{};
};
struct saadc_limits {
  std::uint8_t high{}, low{};
};
template <class Context, class Buffers, class Address = native_dma_address,
          std::size_t QueueCapacity = 4, unsigned StopPollBudget = 1000000>
class saadc_controller {
  using error = driver::adc_error;
  using Session = detail::peripheral_session<Context, Buffers, Address, 16>;
  using E = typename Session::execution_type;

public:
  using pool_type =
      core::operation_pool<driver::adc_sequence_result, error, 1, E>;
  using calibration_pool = core::operation_pool<void, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  using calibration_result =
      std::expected<typename calibration_pool::transfer_type,
                    typename calibration_pool::error_type>;
  saadc_controller(Context &c, clock_controller<Context> &clocks,
                   Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                   Address address = {}) noexcept
      : s_(c, clocks, buffers, address),
        queue_(s_, {32767, 2, 2}, dma_access::write), operations_(c.execution),
        calibrations_(c.execution), stop_ticks_(stop_ticks) {}
  saadc_controller(const saadc_controller &) = delete;
  saadc_controller &operator=(const saadc_controller &) = delete;
  ~saadc_controller() {
    if (queue_.has_active())
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(std::span<const saadc_channel_configuration> channels,
             saadc_configuration config = {}) noexcept {
    if (s_.owned() || s_.context.execution.in_isr())
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::mmio);
    if (channels.empty() || channels.size() > 8)
      return std::unexpected(error::empty_sequence);
    if (!stop_ticks_ || stop_ticks_ > execution::maximum_deadline_interval ||
        config.oversample_log2 > 8 ||
        (config.resolution_bits != 8 && config.resolution_bits != 10 &&
         config.resolution_bits != 12 && config.resolution_bits != 14) ||
        (config.resolution_bits == 14 && !config.oversample_log2) ||
        (config.timer_cc && (channels.size() != 1 || config.timer_cc < 80 ||
                             config.timer_cc > 2047)))
      return std::unexpected(error::invalid_sample_time);
    std::array<core::pin_id, 16> pins{};
    unsigned pin_count = 0;
    std::uint8_t mask = 0;
    auto valid_input = [](saadc_input input) {
      auto v = static_cast<unsigned>(input);
      return v <= 9 || v == 13;
    };
    for (auto channel : channels) {
      auto c = channel;
      auto gain = static_cast<unsigned>(c.gain),
           acq = static_cast<unsigned>(c.acquisition);
      if (c.channel > 7 || !valid_input(c.positive) ||
          c.positive == saadc_input::disconnected || !valid_input(c.negative) ||
          (!c.differential && c.negative != saadc_input::disconnected) ||
          (c.differential && c.negative == saadc_input::disconnected) ||
          gain > 7 || acq > 5 || static_cast<unsigned>(c.reference) > 1 ||
          static_cast<unsigned>(c.positive_resistor) > 3 ||
          static_cast<unsigned>(c.negative_resistor) > 3 ||
          c.low_limit > c.high_limit)
        return std::unexpected(error::invalid_channel);
      if (mask & (1U << c.channel))
        return std::unexpected(error::duplicate_channel);
      mask |= 1U << c.channel;
      if (((c.positive == saadc_input::vddh_div5 ||
            c.negative == saadc_input::vddh_div5) &&
           acq < 2) ||
          (channels.size() > 1 && config.oversample_log2 && !c.burst) ||
          (c.reference == saadc_reference::vdd_quarter &&
           (gain == 4 || gain == 5) && config.minimum_supply_mv <= 2100))
        return std::unexpected(error::invalid_sample_time);
      constexpr std::array us{3U, 5U, 10U, 15U, 20U, 40U};
      if (config.timer_cc &&
          config.timer_cc <=
              16U * (us[acq] + 2U) *
                  (c.burst ? (1U << config.oversample_log2) : 1U))
        return std::unexpected(error::invalid_sample_time);
      for (auto input : std::array{c.positive, c.negative}) {
        auto value = static_cast<unsigned>(input);
        if (value >= 1 && value <= 8) {
          auto pin = *analog_input_pin(value - 1);
          bool duplicate = false;
          for (unsigned i = 0; i < pin_count; ++i)
            duplicate |= pins[i] == pin;
          if (!duplicate)
            pins[pin_count++] = pin;
        }
      }
    }
    if (!s_.acquire(peripheral::saadc, base,
                    std::span<const core::pin_id>{pins.data(), pin_count}))
      return std::unexpected(error::already_owned);
    for (unsigned i = 0; i < pin_count; ++i)
      if (!s_.analog_pin(pins[i]))
        return init_failed();
    config_ = config;
    mask_ = mask;
    channel_count_ = channels.size();
    channels_ = {};
    for (auto c : channels)
      channels_[c.channel] = c;
    calibrated_ = false;
    if (!reset_analog() || !program() || !clear_events() ||
        !s_.write(0x308, irq_mask))
      return init_failed();
    ready_ = true;
    s_.publish();
    return {};
  }
  [[nodiscard]] calibration_result calibrate() noexcept {
    if (!ready_ || fault_ || s_.context.execution.in_isr())
      return fail_start<calibration_result>(error::not_initialized);
    return s_.context.execution.critical([&]() -> calibration_result {
      if (id_ || cal_id_ || active_ || calibrating_)
        return fail_start<calibration_result>(error::busy);
      auto op = calibrations_.try_start(
          {.quiesce = &cancel_calibration, .context = this});
      if (!op)
        return std::unexpected(op.error());
      cal_id_ = op->id();
      calibrated_ = false;
      cal_quiet_ = false;
      // Always calibrate at TACQ >=10 us, then restore each channel's setting.
      bool ok = s_.write(0x110, 0);
      for (unsigned i = 0; i < 8 && ok; ++i)
        if (mask_ & (1U << i))
          ok = s_.write(0x518 + 16 * i, channel_word(channels_[i], true));
      if (!ok) {
        auto id = *cal_id_;
        cal_id_.reset();
        (void)calibrations_.fail(id, error::mmio);
        fault_ = true;
        return op;
      }
      calibrating_ = true;
      if (!s_.write(0xC, 1))
        fault_ = true;
      return op;
    });
  }
  [[nodiscard]] start_result sample(std::span<std::int16_t> buffer) noexcept {
    const std::array blocks{buffer};
    return sample_stream(blocks, buffer.size());
  }
  [[nodiscard]] start_result
  sample_stream(std::span<const std::span<std::int16_t>> initial,
                std::size_t maximum_samples) noexcept {
    if (!ready_ || !calibrated_ || fault_ || s_.context.execution.in_isr())
      return fail_start<start_result>(error::not_initialized);
    if (initial.empty() || initial.size() > QueueCapacity || !maximum_samples ||
        maximum_samples % channel_count_)
      return fail_start<start_result>(error::result_size_mismatch);
    return s_.context.execution.critical([&]() -> start_result {
      if (id_ || cal_id_ || active_ || !queue_.empty())
        return fail_start<start_result>(error::busy);
      for (auto block : initial)
        if (block.empty() || block.size() % channel_count_ ||
            !queue_.push(block)) {
          (void)queue_.discard_queued();
          return fail_start<start_result>(error::dma);
        }
      auto op = operations_.try_start({.quiesce = &cancel, .context = this});
      if (!op) {
        (void)queue_.discard_queued();
        return std::unexpected(op.error());
      }
      id_ = op->id();
      total_ = 0;
      maximum_ = maximum_samples;
      limits_ = {};
      pending_.reset();
      quiet_ = false;
      if (!clear_events() || !start_buffer()) {
        fault_ = true;
        pending_ = error::mmio;
      }
      return op;
    });
  }
  [[nodiscard]] std::expected<void, error>
  queue_buffer(std::span<std::int16_t> buffer) noexcept {
    return s_.context.execution.critical([&]() -> std::expected<void, error> {
      if (!id_ || fault_ || stopping_ || buffer.size() % channel_count_)
        return std::unexpected(error::busy);
      if (!queue_.push(buffer))
        return std::unexpected(error::dma);
      return {};
    });
  }
  [[nodiscard]] std::optional<dma_block<std::int16_t>> take_buffer() noexcept {
    return s_.context.execution.critical([&] { return queue_.take(); });
  }
  [[nodiscard]] std::expected<void, error> trigger_sample() noexcept {
    if (!id_ || !started_ || config_.timer_cc || stopping_)
      return std::unexpected(error::busy);
    if (!s_.write(4, 1)) {
      fault_ = true;
      pending_ = error::mmio;
      return std::unexpected(error::mmio);
    }
    return {};
  }
  [[nodiscard]] bool poll() noexcept {
    return s_.context.execution.critical([&] {
      if (cal_id_) {
        if (!calibration_step())
          return false;
        auto id = *cal_id_;
        cal_id_.reset();
        if (fault_)
          (void)calibrations_.fail(id, error::mmio);
        else
          (void)calibrations_.complete(id);
        return true;
      }
      if (!id_)
        return false;
      if (!sample_limits()) {
        fault_ = true;
        pending_ = error::mmio;
      }
      if (pending_ || stopping_ || quiet_) {
        if (!stop_step()) {
          if (fault_)
            (void)s_.write(0x308, irq_mask);
          return false;
        }
        finish();
        return true;
      }
      auto start = s_.read(0x100), end = s_.read(0x104);
      if (!start || !end) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      if (*start && !started_) {
        started_ = true;
        if (!s_.write(0x100, 0)) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        if (!*end && (config_.timer_cc || config_.automatic_sample))
          if (!s_.write(4, 1)) {
            fault_ = true;
            pending_ = error::mmio;
            return false;
          }
      }
      if (*end) {
        auto amount = s_.read(0x634);
        if (!amount) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        auto size = queue_.current().size();
        auto count = std::min<std::size_t>(*amount, size);
        if (*amount != size)
          pending_ = error::dma;
        // END is the source-defined DMA boundary. No automatic END->START.
        if (!queue_.publish_quiet(count)) {
          fault_ = true;
          pending_ = error::dma;
          return false;
        }
        total_ += count;
        if (!s_.write(0x104, 0)) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        if (!pending_ && total_ < maximum_) {
          if (!start_buffer()) {
            pending_ = error::conversion_overrun;
            stopping_ = true;
          } else
            return true;
        }
        stopping_ = true;
        if (!stop_step())
          return false;
        finish();
        return true;
      }
      if (started_ && !config_.timer_cc && config_.automatic_sample) {
        auto busy = s_.read(0x400);
        if (!busy) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        if (!(*busy & 1U)) {
          if (!s_.write(4, 1)) {
            fault_ = true;
            pending_ = error::mmio;
          }
          return true;
        }
      }
      return false;
    });
  }
  [[nodiscard]] std::expected<void, error> stop() noexcept {
    if (!id_)
      return std::unexpected(error::not_initialized);
    stopping_ = true;
    return {};
  }
  [[nodiscard]] saadc_limits take_limits() noexcept {
    return s_.context.execution.critical([&] {
      auto r = limits_;
      limits_ = {};
      return r;
    });
  }
  [[nodiscard]] std::size_t transferred_count() const noexcept {
    return s_.context.execution.critical([&] { return total_; });
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool on) noexcept {
    if (!ready_ || s_.context.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!s_.write(on ? 0x304 : 0x308, on ? irq_mask & ~12U : irq_mask))
      return std::unexpected(error::mmio);
    return {};
  }
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (s_.context.execution.in_isr() || id_ || cal_id_)
      return std::unexpected(error::busy);
    if (!s_.owned())
      return {};
    if (calibrating_ && !s_.bounded([&] { return calibration_step(); },
                                    stop_ticks_, StopPollBudget))
      return std::unexpected(error::mmio);
    if (active_ &&
        !s_.bounded([&] { return stop_step(); }, stop_ticks_, StopPollBudget))
      return std::unexpected(error::mmio);
    if (!s_.write(0x308, irq_mask) || !s_.disabled() || !reset_analog() ||
        !queue_.discard_queued())
      return std::unexpected(error::mmio);
    if (!s_.release())
      return std::unexpected(error::mmio);
    ready_ = false;
    return {};
  }
  [[nodiscard]] bool calibrated() const noexcept { return calibrated_; }
  [[nodiscard]] bool owns_resource() const noexcept { return s_.owned(); }

private:
  template <class R> static R fail_start(error e) noexcept {
    return std::unexpected(
        typename R::error_type{core::transfer_error_kind::failure, e});
  }
  static std::uint32_t channel_word(saadc_channel_configuration c,
                                    bool calibration = false) noexcept {
    auto tacq = static_cast<unsigned>(c.acquisition);
    if (calibration && tacq < 2)
      tacq = 2;
    return static_cast<unsigned>(c.positive_resistor) |
           (static_cast<unsigned>(c.negative_resistor) << 4) |
           (static_cast<unsigned>(c.gain) << 8) |
           (static_cast<unsigned>(c.reference) << 12) | (tacq << 16) |
           (c.differential ? 1U << 20 : 0U) | (c.burst ? 1U << 24 : 0U);
  }
  bool reset_analog() noexcept {
    // Nordic anomalies 212/241/276: preserve offset trims across power reset.
    if (!trims_) {
      std::array<std::uint32_t, 3> values{};
      for (unsigned i = 0; i < 3; ++i) {
        auto value = s_.read(0x640 + 4 * i);
        if (!value)
          return false;
        values[i] = *value;
      }
      trims_ = values;
    }
    if (!s_.write(0xFFC, 0) || !s_.read(0xFFC) || !s_.write(0xFFC, 1))
      return false;
    for (unsigned i = 0; i < 3; ++i)
      if (!s_.write(0x640 + 4 * i, (*trims_)[i]))
        return false;
    trims_.reset();
    return true;
  }
  bool program() noexcept {
    for (unsigned i = 0; i < 8; ++i) {
      bool enabled = mask_ & (1U << i);
      auto c = channels_[i];
      if (!s_.write(0x510 + 16 * i,
                    enabled ? static_cast<unsigned>(c.positive) : 0) ||
          !s_.write(0x514 + 16 * i,
                    enabled ? static_cast<unsigned>(c.negative) : 0) ||
          !s_.write(0x518 + 16 * i, enabled ? channel_word(c) : 0) ||
          !s_.write(0x51C + 16 * i,
                    (std::uint32_t(std::uint16_t(c.high_limit)) << 16) |
                        std::uint16_t(c.low_limit)))
        return false;
    }
    return s_.write(0x5F0, (config_.resolution_bits - 8) / 2) &&
           s_.write(0x5F4, config_.oversample_log2) &&
           s_.write(0x5F8,
                    config_.timer_cc ? config_.timer_cc | (1U << 12) : 0) &&
           s_.write(0x500, 1);
  }
  bool clear_events() noexcept {
    for (unsigned off = 0x100; off <= 0x154; off += 4)
      if (!s_.write(off, 0))
        return false;
    return true;
  }
  bool calibration_step() noexcept {
    if (!cal_quiet_) {
      auto done = s_.read(0x110);
      if (!done || !*done)
        return false;
      cal_quiet_ = true;
    }
    for (unsigned i = 0; i < 8; ++i)
      if ((mask_ & (1U << i)) &&
          !s_.write(0x518 + 16 * i, channel_word(channels_[i]))) {
        fault_ = true;
        return false;
      }
    if (!s_.write(0x110, 0)) {
      fault_ = true;
      return false;
    }
    calibrating_ = false;
    calibrated_ = !fault_;
    return true;
  }
  bool start_buffer() noexcept {
    started_ = start_issued_ = false;
    stopping_ = false;
    quiet_ = false;
    if (!s_.write(0x100, 0) || !s_.write(0x104, 0) || !s_.write(0x114, 0)) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    if (!queue_.activate(maximum_ - total_))
      return false;
    auto buffer = queue_.current();
    if (!s_.write(0x62C, s_.address(buffer.data(), buffer.size_bytes())) ||
        !s_.write(0x630, buffer.size())) {
      active_ = true;
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    active_ = start_issued_ = true;
    if (!s_.write(0, 1)) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    return true;
  }
  bool sample_limits() noexcept {
    for (unsigned i = 0; i < 8; ++i)
      if (mask_ & (1U << i))
        for (unsigned d = 0; d < 2; ++d) {
          auto off = 0x118 + 8 * i + 4 * d;
          auto event = s_.read(off);
          if (!event)
            return false;
          if (*event) {
            if (!s_.write(off, 0))
              return false;
            if (d)
              limits_.low |= 1U << i;
            else
              limits_.high |= 1U << i;
          }
        }
    return true;
  }
  bool stop_step() noexcept {
    if (!quiet_) {
      auto stopped = s_.read(0x114);
      if (!stopped) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      if (!*stopped) {
        if (!s_.write(8, 1)) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        stopped = s_.read(0x114);
        if (!stopped || !*stopped)
          return false;
      }
      quiet_ = true;
    }
    if (queue_.has_active()) {
      auto amount = s_.read(0x634), start = s_.read(0x100);
      if (!amount || !start) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      std::size_t count = (start_issued_ && (started_ || *start)) ? *amount : 0;
      if (count > queue_.current().size()) {
        pending_ = error::dma;
        count = queue_.current().size();
      }
      if (!queue_.publish_quiet(count)) {
        fault_ = true;
        return false;
      }
      total_ += count;
    }
    if (!clear_events() || !queue_.discard_queued()) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    active_ = false;
    return true;
  }
  void finish() noexcept {
    auto id = *id_;
    id_.reset();
    if (pending_)
      (void)operations_.fail(id, *pending_);
    else
      (void)operations_.complete(id, driver::adc_sequence_result{total_});
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &self = *static_cast<saadc_controller *>(p);
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
  static core::quiesce_result
  cancel_calibration(void *p, core::operation_id id) noexcept {
    auto &self = *static_cast<saadc_controller *>(p);
    if (!self.cal_id_ || *self.cal_id_ != id)
      return core::quiesce_result::reusable;
    bool quiet = self.s_.bounded([&] { return self.calibration_step(); },
                                 self.stop_ticks_, StopPollBudget);
    self.cal_id_.reset();
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
  static constexpr std::uintptr_t base = 0x40007000;
  static constexpr auto irq_mask =
      mcu::nrf52840::product::saadc::intenclr::writable_mask;
  Session s_;
  detail::dma_queue<Session, std::int16_t, QueueCapacity> queue_;
  pool_type operations_;
  calibration_pool calibrations_;
  execution::tick stop_ticks_;
  std::array<saadc_channel_configuration, 8> channels_{};
  saadc_configuration config_{};
  std::uint8_t mask_{};
  std::size_t channel_count_{};
  std::optional<core::operation_id> id_, cal_id_;
  std::optional<error> pending_;
  std::optional<std::array<std::uint32_t, 3>> trims_;
  saadc_limits limits_{};
  std::size_t total_{}, maximum_{};
  bool ready_{}, fault_{}, active_{}, quiet_{}, started_{}, start_issued_{},
      stopping_{}, calibrating_{}, cal_quiet_{}, calibrated_{};
};
} // namespace mcubsp::backend::nrf52840
