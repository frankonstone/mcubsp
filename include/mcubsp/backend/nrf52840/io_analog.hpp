#pragma once
// @req-file REQ-0022 REQ-0029 REQ-0045 REQ-0063 REQ-0064 REQ-0062
#include <mcubsp/backend/nrf52840/io.hpp>
#include <mcubsp/backend/nrf52840/pwm.hpp>
#include <mcubsp/backend/nrf52840/saadc.hpp>
namespace mcubsp::backend::nrf52840 {
template <unsigned... Inputs> struct saadc_binding {
  static_assert(sizeof...(Inputs) > 0 && sizeof...(Inputs) <= 8,
                "SAADC binding requires one through eight inputs");
  static_assert(((Inputs < 8) && ...),
                "SAADC binding inputs must be AIN0 through AIN7");
  static constexpr std::array<std::uint8_t, sizeof...(Inputs)> channels{
      Inputs...};
  static constexpr std::array<core::pin_id, sizeof...(Inputs)> pins{
      *analog_input_pin(Inputs)...};
  static_assert(
      [] {
        for (unsigned i = 0; i < channels.size(); ++i)
          for (unsigned j = 0; j < i; ++j)
            if (channels[i] == channels[j])
              return false;
        return true;
      }(),
      "SAADC binding inputs must be distinct");
};
namespace detail {
template <class M, class = void> struct io_dma_address {
  using type = native_dma_address;
};
template <class M>
struct io_dma_address<M, std::void_t<typename M::dma_address_type>> {
  using type = typename M::dma_address_type;
};
} // namespace detail
// The common single-output PWM facade uses private RAM sequences. Updating
// duty explicitly stops at a period boundary before modifying those samples.
template <class Binding, class M, class E> class io_pwm_backend {
  static_assert(
      Binding::pins[1] == detail::disconnected &&
          Binding::pins[2] == detail::disconnected &&
          Binding::pins[3] == detail::disconnected,
      "PWM facade binding selects one output; native PWM exposes four");
  using S = io_services<M, E>;
  using C = decltype(std::declval<S &>().context);
  using B = decltype(std::declval<S &>().buffers);
  using Native =
      pwm_controller<Binding, C, B, typename detail::io_dma_address<M>::type>;
  using error = driver::pwm_error;

public:
  using period_pool = core::operation_pool<void, error, 1, E>;
  using period_transfer = typename period_pool::transfer_type;
  using transfer_error = typename period_pool::error_type;
  using period_start_result = std::expected<period_transfer, transfer_error>;
  explicit io_pwm_backend(S &services) noexcept
      : s_(services), pwm_(services.context, services.clocks, services.buffers),
        periods_(services.context.execution) {}
  [[nodiscard]] std::expected<void, error> claim() noexcept {
    if (claimed_ || s_.context.execution.in_isr())
      return std::unexpected(error::already_owned);
    if (!s_.ensure())
      return std::unexpected(error::not_initialized);
    claimed_ = true;
    return {};
  }
  [[nodiscard]] std::expected<void, error>
  configure(pwm_timing timing, driver::pwm_compare compare) noexcept {
    if (!claimed_)
      return std::unexpected(error::not_initialized);
    timing_ = timing;
    set_compare(compare);
    return pwm_.initialize(timing_, {pwm_decoder::individual});
  }
  [[nodiscard]] std::expected<void, error> start() noexcept {
    if (operation_)
      return std::unexpected(error::already_running);
    auto op = pwm_.play({values_}, {values_}, {0, 1, true});
    if (!op)
      return std::unexpected(op.error().cause.value_or(error::mmio));
    operation_.emplace(std::move(*op));
    return {};
  }
  [[nodiscard]] std::expected<void, error> stop() noexcept {
    if (period_id_) {
      (void)periods_.fail(*period_id_, error::not_running);
      period_id_.reset();
    }
    if (!operation_)
      return {};
    auto quiet = operation_->cancel();
    (void)operation_->take();
    operation_.reset();
    if (!quiet)
      return std::unexpected(error::mmio);
    return {};
  }
  [[nodiscard]] std::expected<void, error>
  update_duty(driver::pwm_compare compare, bool running) noexcept {
    if (running) {
      auto quiet = stop();
      if (!quiet)
        return quiet;
    }
    set_compare(compare);
    return running ? start() : std::expected<void, error>{};
  }
  [[nodiscard]] period_start_result next_period() noexcept {
    if (!operation_)
      return std::unexpected(transfer_error{core::transfer_error_kind::failure,
                                            error::not_running});
    if (period_id_)
      return std::unexpected(transfer_error{core::transfer_error_kind::failure,
                                            error::already_running});
    auto op = periods_.try_start({.quiesce = &cancel_period, .context = this});
    if (!op)
      return std::unexpected(op.error());
    period_id_ = op->id();
    if (!s_.context.clear_event(Binding::base + 0x118)) {
      auto id = *period_id_;
      period_id_.reset();
      (void)periods_.fail(id, error::mmio);
    }
    return op;
  }
  bool poll() noexcept {
    bool progress = pwm_.poll();
    auto event = s_.context.read(Binding::base + 0x118);
    if (event && *event && s_.context.clear_event(Binding::base + 0x118)) {
      progress = true;
      if (period_id_) {
        auto id = *period_id_;
        period_id_.reset();
        (void)periods_.complete(id);
      }
    } else if (!event && period_id_) {
      auto id = *period_id_;
      period_id_.reset();
      (void)periods_.fail(id, error::mmio);
    }
    return progress;
  }
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    auto quiet = stop();
    if (!quiet)
      return quiet;
    return pwm_.shutdown();
  }
  void release() noexcept {
    if (!pwm_.owns_resource())
      claimed_ = false;
  }
  [[nodiscard]] bool needs_cleanup() const noexcept {
    return pwm_.owns_resource();
  }

private:
  void set_compare(driver::pwm_compare compare) noexcept {
    values_.fill(static_cast<std::uint16_t>(0x8000U | compare.compare_counts));
  }
  static core::quiesce_result cancel_period(void *p,
                                            core::operation_id id) noexcept {
    auto &self = *static_cast<io_pwm_backend *>(p);
    if (self.period_id_ && *self.period_id_ == id)
      self.period_id_.reset();
    return core::quiesce_result::reusable;
  }
  S &s_;
  Native pwm_;
  period_pool periods_;
  pwm_timing timing_{};
  std::array<std::uint16_t, 4> values_{};
  std::optional<typename Native::pool_type::transfer_type> operation_;
  std::optional<core::operation_id> period_id_;
  bool claimed_{};
};
// Compatible common ADC scans use private signed DMA storage. Negative offset
// samples clamp to zero at the unsigned facade boundary. Native SAADC retains
// signed/differential/streaming operation and all reference/gain choices.
template <class Binding, class M, class E> class io_adc_backend {
  using S = io_services<M, E>;
  using C = decltype(std::declval<S &>().context);
  using B = decltype(std::declval<S &>().buffers);
  using Native =
      saadc_controller<C, B, typename detail::io_dma_address<M>::type>;
  using error = driver::adc_error;

public:
  using pool_type =
      core::operation_pool<driver::adc_sequence_result, error, 1, E>;
  using transfer_type = typename pool_type::transfer_type;
  using start_result =
      std::expected<transfer_type, typename pool_type::error_type>;
  explicit io_adc_backend(S &services) noexcept
      : s_(services), adc_(services.context, services.clocks, services.buffers),
        operations_(services.context.execution) {}
  [[nodiscard]] std::expected<void, error>
  initialize(driver::adc_configuration config = {}) noexcept {
    if (ready_ || s_.context.execution.in_isr())
      return std::unexpected(error::already_owned);
    if (config.reference != driver::adc_reference::vrefh0 ||
        config.result_format !=
            driver::adc_result_format::right_aligned_unsigned ||
        (config.resolution != driver::adc_resolution::bits12 &&
         config.resolution != driver::adc_resolution::bits14) ||
        config.sample_states != 13)
      return std::unexpected(error::invalid_sample_time);
    if (!s_.ensure())
      return std::unexpected(error::not_initialized);
    std::array<saadc_channel_configuration, Binding::channels.size()>
        channels{};
    for (unsigned i = 0; i < channels.size(); ++i) {
      channels[i].channel = i;
      channels[i].positive = static_cast<saadc_input>(Binding::channels[i] + 1);
      channels[i].burst = config.resolution == driver::adc_resolution::bits14;
    }
    auto initialized = adc_.initialize(
        channels,
        {.resolution_bits = static_cast<unsigned>(config.resolution),
         .oversample_log2 =
             config.resolution == driver::adc_resolution::bits14 ? 2U : 0U});
    if (!initialized)
      return initialized;
    auto cal = adc_.calibrate();
    if (!cal)
      return std::unexpected(cal.error().cause.value_or(error::mmio));
    calibration_.emplace(std::move(*cal));
    ready_ = true;
    return {};
  }
  [[nodiscard]] start_result
  convert(std::span<const driver::adc_channel> channels,
          std::span<std::uint16_t> output) noexcept {
    if (!ready_ || fault_ || s_.context.execution.in_isr())
      return failure(error::not_initialized);
    if (output.size() != Binding::channels.size() ||
        channels.size() != Binding::channels.size())
      return failure(error::result_size_mismatch);
    for (unsigned i = 0; i < channels.size(); ++i)
      if (channels[i].value != Binding::channels[i])
        return failure(error::invalid_channel);
    if (id_)
      return failure(error::busy);
    auto op = operations_.try_start({.quiesce = &cancel, .context = this});
    if (!op)
      return std::unexpected(op.error());
    id_ = op->id();
    output_ = output;
    if (!calibration_)
      start_scan();
    return op;
  }
  bool poll() noexcept {
    bool progress = adc_.poll();
    if (calibration_ && calibration_->ready()) {
      auto result = calibration_->take();
      calibration_.reset();
      progress = true;
      if (!result) {
        fault_ = true;
        if (id_)
          finish(error::mmio);
        return true;
      }
      if (id_)
        start_scan();
    }
    if (scan_ && scan_->ready()) {
      auto result = scan_->take();
      scan_.reset();
      auto block = adc_.take_buffer();
      if (!result || !block || result->samples != output_.size()) {
        if (id_)
          finish(result ? error::dma
                        : result.error().cause.value_or(error::mmio));
        return true;
      }
      if (id_) {
        for (unsigned i = 0; i < output_.size(); ++i)
          output_[i] = block->storage[i] < 0
                           ? 0U
                           : static_cast<std::uint16_t>(block->storage[i]);
        finish({});
      }
      progress = true;
    }
    return progress;
  }
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (id_)
      return std::unexpected(error::busy);
    if (calibration_) {
      auto quiet = calibration_->cancel();
      (void)calibration_->take();
      calibration_.reset();
      if (!quiet)
        return std::unexpected(error::mmio);
    }
    if (scan_) {
      auto quiet = scan_->cancel();
      (void)scan_->take();
      scan_.reset();
      if (!quiet)
        return std::unexpected(error::mmio);
    }
    auto quiet = adc_.shutdown();
    if (!quiet)
      return quiet;
    while (adc_.take_buffer()) {
    }
    ready_ = false;
    return {};
  }
  [[nodiscard]] bool needs_cleanup() const noexcept {
    return adc_.owns_resource();
  }

private:
  start_result failure(error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  void start_scan() noexcept {
    auto scan = adc_.sample(
        std::span<std::int16_t>{scratch_}.first(Binding::channels.size()));
    if (!scan) {
      finish(scan.error().cause.value_or(error::mmio));
      return;
    }
    scan_.emplace(std::move(*scan));
  }
  void finish(std::optional<error> error) noexcept {
    auto id = *id_;
    id_.reset();
    auto count = output_.size();
    output_ = {};
    if (error)
      (void)operations_.fail(id, *error);
    else
      (void)operations_.complete(id, driver::adc_sequence_result{count});
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &self = *static_cast<io_adc_backend *>(p);
    if (!self.id_ || *self.id_ != id)
      return core::quiesce_result::reusable;
    self.output_ = {};
    self.id_.reset();
    bool quiet = true;
    if (self.calibration_) {
      quiet = self.calibration_->cancel().has_value();
      (void)self.calibration_->take();
      self.calibration_.reset();
    }
    if (self.scan_) {
      quiet = self.scan_->cancel().has_value() && quiet;
      (void)self.scan_->take();
      self.scan_.reset();
      if (quiet)
        (void)self.adc_.take_buffer();
    }
    self.fault_ |= !quiet;
    return quiet ? core::quiesce_result::reusable
                 : core::quiesce_result::faulted;
  }
  S &s_;
  Native adc_;
  pool_type operations_;
  std::array<std::int16_t, 8> scratch_{};
  std::optional<typename Native::calibration_pool::transfer_type> calibration_;
  std::optional<typename Native::pool_type::transfer_type> scan_;
  std::optional<core::operation_id> id_;
  std::span<std::uint16_t> output_;
  bool ready_{}, fault_{};
};
} // namespace mcubsp::backend::nrf52840
