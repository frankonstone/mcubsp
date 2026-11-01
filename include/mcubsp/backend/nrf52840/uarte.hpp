#pragma once
// @req-file REQ-0027 REQ-0059 REQ-0062
// Nordic PS 4413_417 v1.11 6.34.1--6.34.9. Separate from CPU UART0.
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/backend/nrf52840/detail/rx_queue.hpp>
#include <mcubsp/backend/nrf52840/dma_address.hpp>
#include <mcubsp/backend/nrf52840/dma_buffers.hpp>
#include <mcubsp/backend/nrf52840/uart_config.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/mcu/nrf52840/product/uarte0.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
#include <type_traits>

namespace mcubsp::backend::nrf52840 {
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned StopPollBudget = 1000000,
          std::size_t StreamCapacity = 4>
  requires((Binding::instance == peripheral::uarte0 ||
            Binding::instance == peripheral::uarte1) &&
           StopPollBudget > 0)
class uarte_controller {
  using error = driver::uart_error;
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using tx_pool = core::operation_pool<void, error, 1, E>;
  using rx_pool = core::operation_pool<std::size_t, error, 1, E>;
  using tx_result = std::expected<typename tx_pool::transfer_type,
                                  typename tx_pool::error_type>;
  using rx_result = std::expected<typename rx_pool::transfer_type,
                                  typename rx_pool::error_type>;
  uarte_controller(Context &c, clock_controller<Context> &clocks,
                   Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                   Address address = {}) noexcept
      : c_(c), clocks_(clocks), buffers_(buffers), address_(address),
        tx_(c.execution), rx_(c.execution), stop_ticks_(stop_ticks) {}
  uarte_controller(const uarte_controller &) = delete;
  uarte_controller &operator=(const uarte_controller &) = delete;
  ~uarte_controller() {
    bool queued = false;
    for (auto t : stream_tokens_)
      queued |= t.has_value();
    if (tx_buffer_ || rx_buffer_ || tail_buffer_ || queued)
      c_.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(uart_baud_timing timing,
             driver::uart_format format = {}) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::transport);
    auto part = c_.read(0x10000130), revision = c_.read(0x10000134);
    if (!part || !revision)
      return std::unexpected(error::mmio);
    revision_ = {*part, *revision};
    // Unknown/Engineering A retains conservative anomaly 127/131 limits.
    if (!stop_ticks_ || stop_ticks_ > execution::maximum_deadline_interval ||
        !uart_format_supported(Binding::instance, format) ||
        (format.stop_bits == driver::uart_stop_bits::two &&
         !revision_.reviewed_modern()) ||
        !Binding::supports_rate(timing.achieved_baud))
      return std::unexpected(error::invalid_format);
    auto solved =
        solve_uart_baud(Binding::instance, {timing.clock_hz, timing.target_baud,
                                            timing.error_ppm});
    if (!solved || timing.instance != Binding::instance ||
        solved->encoding != timing.encoding ||
        solved->achieved_baud != timing.achieved_baud)
      return std::unexpected(error::invalid_baud);
    auto claim = c_.resources.claim(c_.execution, Binding::instance, this);
    if (!claim)
      return std::unexpected(error::already_owned);
    owner_ = *claim;
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled) {
      (void)release_owner();
      return std::unexpected(enabled ? error::already_owned : error::mmio);
    }
    for (unsigned i = 0; i < pins.size(); ++i)
      if (pins[i] != uart_disconnected) {
        if (!c_.gpio.claim(pins[i], i < 2 ? driver::gpio_direction::output
                                          : driver::gpio_direction::input)) {
          release_pins();
          (void)release_owner();
          return std::unexpected(error::already_owned);
        }
        pin_claimed_[i] = true;
      }
    if (!clocks_.acquire_hf()) {
      release_pins();
      (void)release_owner();
      return std::unexpected(error::transport);
    }
    hf_ = true;
    for (unsigned i = 0; i < pins.size(); ++i)
      if (pin_claimed_[i] &&
          !c_.gpio.configure(
              pins[i], {.direction = i < 2 ? driver::gpio_direction::output
                                           : driver::gpio_direction::input,
                        .initial_level = driver::gpio_level::high}))
        return init_failed();
    if (!write(0x308, all_irq_mask) || !write(0x200, 0) || !clear_events() ||
        !write(0x508, select(Binding::rts)) ||
        !write(0x50C, select(Binding::tx)) ||
        !write(0x510, select(Binding::cts)) ||
        !write(0x514, select(Binding::rx)) || !write(0x524, timing.encoding) ||
        !write(
            0x56C,
            (Binding::flow_control ? 1U : 0U) |
                (format.parity == driver::uart_parity::even ? 14U : 0U) |
                (format.stop_bits == driver::uart_stop_bits::two ? 16U : 0U)) ||
        !write(0x500, 8))
      return init_failed();
    ready_ = true;
    c_.work_started();
    work_ = true;
    return {};
  }
  [[nodiscard]] tx_result
  transmit(std::span<const std::uint8_t> bytes) noexcept {
    if (bytes.empty())
      return failure<tx_result>(error::empty_transfer);
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure<tx_result>(error::not_initialized);
    return c_.execution.critical([&]() -> tx_result {
      if (tx_id_ || tx_buffer_)
        return failure<tx_result>(error::busy);
      auto buffer = prepare(bytes.data(), bytes.size(), dma_access::read);
      if (!buffer)
        return failure<tx_result>(error::dma);
      auto operation = tx_.try_start({.quiesce = &cancel_tx, .context = this});
      if (!operation) {
        (void)buffers_.discard_prepared(*buffer);
        return std::unexpected(operation.error());
      }
      tx_id_ = operation->id();
      tx_buffer_ = *buffer;
      sent_ = 0;
      tx_count_ = bytes.size();
      tx_stopping_ = false;
      tx_quiet_ = false;
      tx_error_.reset();
      if (!write(0x120, 0) || !write(0x158, 0) ||
          !write(0x544, static_cast<std::uint32_t>(
                            address_(bytes.data(), bytes.size()))) ||
          !write(0x548, bytes.size())) {
        discard(tx_buffer_);
        auto id = *tx_id_;
        tx_id_.reset();
        (void)tx_.fail(id, error::mmio);
        return operation;
      }
      if (!buffers_.arm(*tx_buffer_)) {
        discard(tx_buffer_);
        auto id = *tx_id_;
        tx_id_.reset();
        (void)tx_.fail(id, error::dma);
        return operation;
      }
      // START can commit before an MMIO error is returned.
      if (!write(0x008, 1))
        fault_ = true;
      return operation;
    });
  }
  [[nodiscard]] rx_result receive(std::span<std::uint8_t> bytes) noexcept {
    if (bytes.empty())
      return failure<rx_result>(error::empty_transfer);
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure<rx_result>(error::not_initialized);
    return c_.execution.critical([&]() -> rx_result {
      if (rx_id_ || rx_buffer_ || tail_buffer_ ||
          (!setting_up_stream_ && !stream_queue_.empty()))
        return failure<rx_result>(error::busy);
      auto buffer = prepare(bytes.data(), bytes.size(), dma_access::write);
      if (!buffer)
        return failure<rx_result>(error::dma);
      auto tail = prepare(tail_.data(), tail_.size(), dma_access::write);
      if (!tail) {
        (void)buffers_.discard_prepared(*buffer);
        return failure<rx_result>(error::dma);
      }
      auto operation = rx_.try_start({.quiesce = &cancel_rx, .context = this});
      if (!operation) {
        (void)buffers_.discard_prepared(*tail);
        (void)buffers_.discard_prepared(*buffer);
        return std::unexpected(operation.error());
      }
      rx_id_ = operation->id();
      rx_buffer_ = *buffer;
      tail_buffer_ = *tail;
      received_ = error_bits_ = 0;
      rx_error_.reset();
      rx_count_ = bytes.size();
      rx_phase_ = phase::idle;
      streaming_ = false;
      if (!write(0x110, 0) || !write(0x144, 0) || !write(0x124, 0) ||
          !write(0x480, 15) ||
          !write(0x534, static_cast<std::uint32_t>(
                            address_(bytes.data(), bytes.size()))) ||
          !write(0x538, bytes.size())) {
        discard(rx_buffer_);
        discard(tail_buffer_);
        auto id = *rx_id_;
        rx_id_.reset();
        (void)rx_.fail(id, error::mmio);
        return operation;
      }
      if (!buffers_.arm(*rx_buffer_)) {
        discard(rx_buffer_);
        discard(tail_buffer_);
        auto id = *rx_id_;
        rx_id_.reset();
        (void)rx_.fail(id, error::dma);
        return operation;
      }
      rx_phase_ = phase::receiving;
      if (!write(0x000, 1))
        fault_ = true;
      return operation;
    });
  }
  // CPU/ISR-assisted continuous reception, with hardware SHORTS kept disabled:
  // each ENDRX hands a completed buffer back before explicitly starting the
  // next prepared buffer. No late ISR can cause automatic buffer reuse.
  // HWFC bounds FIFO backpressure; without it the application must service
  // the documented FIFO in time and handle observable overrun/errors.
  [[nodiscard]] rx_result
  receive_stream(std::span<const std::span<std::uint8_t>> initial,
                 std::size_t maximum_bytes) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure<rx_result>(error::not_initialized);
    if (initial.empty() || initial.size() > StreamCapacity || !maximum_bytes)
      return failure<rx_result>(error::empty_transfer);
    return c_.execution.critical([&]() -> rx_result {
      if (rx_id_ || rx_buffer_ || tail_buffer_ || !stream_queue_.empty())
        return failure<rx_result>(error::busy);
      for (std::size_t i = 0; i < initial.size(); ++i) {
        auto slot = stream_queue_.push(initial[i]);
        if (!slot) {
          (void)discard_stream_queued();
          return failure<rx_result>(slot.error());
        }
        if (i) {
          auto token =
              prepare(initial[i].data(), initial[i].size(), dma_access::write);
          if (!token) {
            (void)discard_stream_queued();
            return failure<rx_result>(error::dma);
          }
          stream_tokens_[*slot] = *token;
        }
      }
      auto first = initial.front().first(initial.front().size() < maximum_bytes
                                             ? initial.front().size()
                                             : maximum_bytes);
      if (!write(0x14C, 0)) {
        (void)discard_stream_queued();
        return failure<rx_result>(error::mmio);
      }
      setting_up_stream_ = true;
      auto operation = receive(first);
      setting_up_stream_ = false;
      if (!operation || !rx_id_) {
        (void)discard_stream_queued();
        return operation;
      }
      stream_active_ = stream_queue_.activate();
      stream_limit_ = maximum_bytes;
      stream_received_ = 0;
      streaming_ = true;
      stream_start_issued_ = true;
      return operation;
    });
  }
  [[nodiscard]] std::expected<void, error>
  queue_receive_buffer(std::span<std::uint8_t> bytes) noexcept {
    return c_.execution.critical([&]() -> std::expected<void, error> {
      if (!streaming_ || !rx_id_ || fault_ || rx_phase_ != phase::receiving)
        return std::unexpected(error::not_initialized);
      auto slot = stream_queue_.push(bytes);
      if (!slot)
        return std::unexpected(slot.error());
      auto token = prepare(bytes.data(), bytes.size(), dma_access::write);
      if (!token) {
        stream_queue_.discard(*slot);
        return std::unexpected(error::dma);
      }
      stream_tokens_[*slot] = *token;
      return {};
    });
  }
  [[nodiscard]] std::optional<received_block> take_received_block() noexcept {
    return c_.execution.critical([&] { return stream_queue_.take(); });
  }
  // RXTO is a stop acknowledgment, not a receive-idle detector. A protocol
  // deadline may request a partial result explicitly through this method.
  [[nodiscard]] std::expected<void, error> stop_receive() noexcept {
    return c_.execution.critical([&]() -> std::expected<void, error> {
      if (!rx_id_)
        return std::unexpected(error::not_initialized);
      if (!request_rx_stop()) {
        fault_ = true;
        return std::unexpected(error::mmio);
      }
      return {};
    });
  }
  [[nodiscard]] bool poll() noexcept {
    if (!ready_)
      return false;
    return c_.execution.critical([&] {
      bool progress{};
      if (tx_id_) {
        auto end = c_.read(base + 0x120);
        if (!end)
          fault_ = true;
        if (fault_ || tx_stopping_ || (end && *end)) {
          if (fault_)
            tx_error_ = error::mmio;
          if (tx_step()) {
            auto id = *tx_id_;
            tx_id_.reset();
            if (tx_error_)
              (void)tx_.fail(id, *tx_error_);
            else
              (void)tx_.complete(id);
            progress = true;
          }
        }
      }
      if (rx_id_) {
        if (fault_ && !rx_error_)
          rx_error_ = error::mmio;
        if (rx_step(fault_)) {
          auto id = *rx_id_;
          rx_id_.reset();
          if (rx_error_)
            (void)rx_.fail(id, *rx_error_);
          else
            (void)rx_.complete(id, streaming_ ? stream_received_ : received_);
          progress = true;
        }
      }
      if (fault_)
        (void)write(0x308, all_irq_mask);
      return progress;
    });
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool enable) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!write(enable ? 0x304 : 0x308, irq_mask))
      return std::unexpected(error::mmio);
    return {};
  }
  // After a failed cancellation caller memory remains borrowed until shutdown
  // succeeds. Preserve the original stop events for late-ack recovery.
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (c_.execution.in_isr() || tx_id_ || rx_id_)
      return std::unexpected(error::busy);
    if (!owner_)
      return {};
    if ((tx_buffer_ && !bounded([&] { return tx_step(); })) ||
        ((rx_buffer_ || tail_buffer_) &&
         !bounded([&] { return rx_step(true); })))
      return std::unexpected(error::transport);
    if (!discard_stream_queued())
      return std::unexpected(error::dma);
    if (!write(0x308, all_irq_mask) || !write(0x200, 0) || !write(0x500, 0))
      return std::unexpected(error::mmio);
    auto disabled = c_.read(base + 0x500);
    if (!disabled || *disabled)
      return std::unexpected(error::mmio);
    for (unsigned offset : {0x508U, 0x50CU, 0x510U, 0x514U})
      if (!write(offset, 0x80000000U))
        return std::unexpected(error::mmio);
    for (unsigned i = 0; i < pins.size(); ++i)
      if (pin_claimed_[i] && !c_.gpio.configure(pins[i], {}))
        return std::unexpected(error::mmio);
    if (hf_ && !clocks_.release_hf())
      return std::unexpected(error::transport);
    hf_ = false;
    release_pins();
    if (!release_owner())
      return std::unexpected(error::busy);
    if (work_) {
      c_.work_stopped();
      work_ = false;
    }
    ready_ = false;
    return {};
  }
  [[nodiscard]] std::size_t received_count() const noexcept {
    return c_.execution.critical(
        [&] { return streaming_ ? stream_received_ : received_; });
  }
  [[nodiscard]] std::size_t transmitted_count() const noexcept {
    return c_.execution.critical([&] { return sent_; });
  }
  [[nodiscard]] std::uint32_t error_source() const noexcept {
    return c_.execution.critical([&] { return error_bits_; });
  }
  [[nodiscard]] bool faulted() const noexcept {
    return c_.execution.critical([&] { return fault_; });
  }

  [[nodiscard]] bool owns_resource() const noexcept {
    return owner_.has_value();
  }

private:
  enum class phase : std::uint8_t {
    idle,
    receiving,
    stopping,
    stopped,
    flushing,
    drained
  };
  template <class Result> static Result failure(error e) noexcept {
    return std::unexpected(
        typename Result::error_type{core::transfer_error_kind::failure, e});
  }
  std::expected<dma_buffer_token, coordination_error>
  prepare(const void *p, std::size_t n, dma_access access) noexcept {
    return buffers_.prepare(
        *owner_,
        mcu::nrf52840::dma_constraints{revision_.uarte_maximum_count(), 1, 1},
        address_(p, n), n, n, access);
  }
  bool write(unsigned offset, std::uint32_t value) noexcept {
    if (offset >= 0x100 && offset < 0x200 && !value)
      return c_.clear_event(base + offset).has_value();
    auto r = c_.write(base + offset, value);
    arch::data_memory_barrier();
    return r.has_value();
  }
  bool clear_events() noexcept {
    for (unsigned offset :
         {0x110U, 0x120U, 0x124U, 0x144U, 0x158U, 0x14CU, 0x150U})
      if (!write(offset, 0))
        return false;
    return write(0x480, 15);
  }
  static constexpr std::uint32_t select(core::pin_id p) noexcept {
    return p == uart_disconnected ? 0x80000000U
                                  : (std::uint32_t(p.port) << 5) | p.index;
  }
  void discard(std::optional<dma_buffer_token> &token) noexcept {
    if (token && buffers_.discard_prepared(*token))
      token.reset();
  }
  bool release(std::optional<dma_buffer_token> &token) noexcept {
    if (!token)
      return true;
    if (!buffers_.recover(
            *token,
            []() -> std::expected<bool, coordination_error> { return true; }))
      return false;
    token.reset();
    return true;
  }
  bool release_owner() noexcept {
    if (!owner_)
      return true;
    if (!c_.resources.release(c_.execution, *owner_))
      return false;
    owner_.reset();
    return true;
  }
  void release_pins() noexcept {
    for (unsigned i = 0; i < pins.size(); ++i)
      if (pin_claimed_[i]) {
        c_.gpio.release(pins[i]);
        pin_claimed_[i] = false;
      }
  }
  std::expected<void, error> init_failed() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(error::mmio);
  }
  bool tx_step() noexcept {
    if (!tx_buffer_)
      return true;
    if (!tx_stopping_) {
      if (!write(0x120, 0) || !write(0x158, 0)) {
        fault_ = true;
        return false;
      }
      tx_stopping_ = true;
      if (!write(0x00C, 1)) {
        fault_ = true;
        return false;
      }
    }
    if (!tx_quiet_) {
      auto stopped = c_.read(base + 0x158);
      if (!stopped) {
        fault_ = true;
        return false;
      }
      if (!*stopped) {
        (void)write(0x00C, 1);
        return false;
      }
      auto count = c_.read(base + 0x54C);
      if (!count) {
        fault_ = true;
        return false;
      }
      sent_ = *count;
      if (sent_ != tx_count_ && !tx_error_)
        tx_error_ = error::transport;
      tx_quiet_ = true;
    }
    // Latch quiescence before W0 acknowledgments so an MMIO failure after
    // clearing the event cannot force recovery to wait for a second stop.
    if (!write(0x120, 0) || !write(0x158, 0)) {
      fault_ = true;
      return false;
    }
    if (!release(tx_buffer_)) {
      fault_ = true;
      return false;
    }
    return true;
  }
  bool request_rx_stop() noexcept {
    if (rx_phase_ == phase::stopping || rx_phase_ == phase::stopped ||
        rx_phase_ == phase::flushing || rx_phase_ == phase::drained)
      return true;
    if (!write(0x144, 0) || !write(0x110, 0))
      return false;
    rx_phase_ = phase::stopping;
    return write(0x004, 1);
  }
  bool sample_rx_errors() noexcept {
    auto event = c_.read(base + 0x124);
    if (!event)
      return false;
    if (!*event)
      return true;
    if (!write(0x124, 0))
      return false;
    auto bits = c_.read(base + 0x480);
    if (!bits || !write(0x480, *bits & 15U))
      return false;
    error_bits_ |= *bits & 15U;
    if (!rx_error_)
      rx_error_ = (*bits & 1U)    ? error::overrun
                  : (*bits & 2U)  ? error::parity
                  : (*bits & 12U) ? error::framing
                                  : error::transport;
    return true;
  }
  bool rx_step(bool force_stop) noexcept {
    if (!rx_buffer_ && !tail_buffer_)
      return true;
    if (rx_phase_ == phase::receiving) {
      auto end = c_.read(base + 0x110);
      if (!end || !sample_rx_errors()) {
        fault_ = true;
        return false;
      }
      if (!force_stop && !*end && !rx_error_)
        return false;
      if (streaming_ && *end && !force_stop && !rx_error_) {
        auto count = c_.read(base + 0x53C);
        if (!count) {
          fault_ = true;
          return false;
        }
        if (*count != rx_count_)
          rx_error_ = error::dma;
        if (!publish_stream_block(*count)) {
          fault_ = true;
          return false;
        }
        if (!rx_error_ && stream_received_ < stream_limit_) {
          auto next = stream_queue_.activate();
          if (next) {
            stream_active_ = *next;
            rx_buffer_ = stream_tokens_[*next];
            stream_tokens_[*next].reset();
            const auto bytes = stream_queue_.buffer(*next);
            const auto remaining = stream_limit_ - stream_received_;
            rx_count_ = bytes.size() < remaining ? bytes.size() : remaining;
            stream_start_issued_ = false;
            if (!write(0x110, 0) || !write(0x14C, 0) ||
                !write(0x534, address_(bytes.data(), bytes.size())) ||
                !write(0x538, rx_count_) || !buffers_.arm(*rx_buffer_)) {
              fault_ = true;
              rx_error_ = error::mmio;
            } else {
              stream_start_issued_ = true;
              if (!write(0x000, 1)) {
                fault_ = true;
                rx_error_ = error::mmio;
              }
            }
            return false;
          }
          rx_error_ = error::overrun; // explicit buffer exhaustion, never reuse
        }
      }
      if (!request_rx_stop()) {
        fault_ = true;
        return false;
      }
    }
    if (rx_phase_ == phase::stopping) {
      if (!sample_rx_errors()) {
        fault_ = true;
        return false;
      }
      auto stopped = c_.read(base + 0x144);
      if (!stopped) {
        fault_ = true;
        return false;
      }
      if (!*stopped) {
        (void)write(0x110, 0);
        (void)write(0x004, 1);
        return false;
      }
      if (!sample_rx_errors()) {
        fault_ = true;
        return false;
      }
      auto count = c_.read(base + 0x53C);
      if (!count) {
        fault_ = true;
        return false;
      }
      received_ = *count;
      if (streaming_ && stream_active_) {
        auto started = c_.read(base + 0x14C);
        if (!started) {
          fault_ = true;
          return false;
        }
        if (!stream_start_issued_ || !*started)
          received_ = 0;
      }
      if (received_ > rx_count_)
        rx_error_ = error::dma;
      rx_phase_ = phase::stopped;
    }
    if (rx_phase_ == phase::stopped) {
      // RXTO precedes a separate FLUSHRX to a private >4-byte buffer.
      // This drains residual FIFO bytes without overwriting caller RAM.
      if (!write(0x144, 0) || !write(0x110, 0) ||
          !write(0x534, static_cast<std::uint32_t>(
                            address_(tail_.data(), tail_.size()))) ||
          !write(0x538, tail_.size())) {
        fault_ = true;
        return false;
      }
      if (!buffers_.arm(*tail_buffer_)) {
        fault_ = true;
        return false;
      }
      rx_phase_ = phase::flushing;
      if (!write(0x02C, 1)) {
        fault_ = true;
        return false;
      }
    }
    if (rx_phase_ == phase::flushing) {
      auto end = c_.read(base + 0x110);
      if (!end) {
        fault_ = true;
        return false;
      }
      if (!*end)
        return false;
      auto count = c_.read(base + 0x53C);
      if (!count) {
        fault_ = true;
        return false;
      }
      if (*count && !rx_error_)
        rx_error_ = error::overrun;
      rx_phase_ = phase::drained;
    }
    if (rx_phase_ == phase::drained) {
      if (!write(0x110, 0) || !write(0x144, 0) || !write(0x124, 0)) {
        fault_ = true;
        return false;
      }
      if (streaming_ && stream_active_ && !publish_stream_block(received_)) {
        fault_ = true;
        return false;
      }
      if (!release(tail_buffer_) || !release(rx_buffer_) ||
          !discard_stream_queued()) {
        fault_ = true;
        return false;
      }
      rx_phase_ = phase::idle;
      return true;
    }
    return false;
  }
  bool publish_stream_block(std::size_t count) noexcept {
    if (!stream_active_)
      return true;
    if (count > rx_count_) {
      rx_error_ = error::dma;
      count = rx_count_;
    }
    if (!release(rx_buffer_))
      return false;
    if (!stream_queue_.publish(*stream_active_, count))
      return false;
    stream_received_ += count;
    stream_active_.reset();
    return true;
  }
  bool discard_stream_queued() noexcept {
    for (unsigned i = 0; i < StreamCapacity; ++i)
      if (stream_queue_.status(i) ==
          detail::rx_queue<StreamCapacity>::state::queued) {
        discard(stream_tokens_[i]);
        if (stream_tokens_[i])
          return false;
        stream_queue_.discard(i);
      }
    return true;
  }
  template <class Step> bool bounded(Step &&step) noexcept {
    execution::deadline until{
        static_cast<execution::tick>(c_.execution.now() + stop_ticks_)};
    const unsigned budget = c_.execution.in_isr() ? 1U : StopPollBudget;
    for (unsigned i = 0; i < budget; ++i) {
      if (c_.execution.critical(step))
        return true;
      if (execution::reached(c_.execution.now(), until))
        break;
    }
    return false;
  }
  static core::quiesce_result cancel_tx(void *p,
                                        core::operation_id id) noexcept {
    auto &s = *static_cast<uarte_controller *>(p);
    if (!s.tx_id_ || *s.tx_id_ != id)
      return core::quiesce_result::reusable;
    bool stopped = s.bounded([&] { return s.tx_step(); });
    s.tx_id_.reset();
    s.fault_ |= !stopped;
    if (!stopped)
      (void)s.write(0x308, all_irq_mask);
    return stopped ? core::quiesce_result::reusable
                   : core::quiesce_result::faulted;
  }
  static core::quiesce_result cancel_rx(void *p,
                                        core::operation_id id) noexcept {
    auto &s = *static_cast<uarte_controller *>(p);
    if (!s.rx_id_ || *s.rx_id_ != id)
      return core::quiesce_result::reusable;
    bool stopped = s.bounded([&] { return s.rx_step(true); });
    s.rx_id_.reset();
    s.fault_ |= !stopped;
    if (!stopped)
      (void)s.write(0x308, all_irq_mask);
    return stopped ? core::quiesce_result::reusable
                   : core::quiesce_result::faulted;
  }
  static constexpr std::uintptr_t base = Binding::base;
  static constexpr std::uint32_t irq_mask =
      (1U << 4) | (1U << 8) | (1U << 9) | (1U << 17) | (1U << 22);
  static constexpr auto all_irq_mask =
      mcu::nrf52840::product::uarte0::intenclr::writable_mask;
  static constexpr std::array pins{Binding::tx, Binding::rts, Binding::rx,
                                   Binding::cts};
  Context &c_;
  clock_controller<Context> &clocks_;
  Buffers &buffers_;
  Address address_;
  tx_pool tx_;
  rx_pool rx_;
  execution::tick stop_ticks_;
  std::optional<resource_token> owner_;
  std::optional<core::operation_id> tx_id_, rx_id_;
  std::optional<dma_buffer_token> tx_buffer_, rx_buffer_, tail_buffer_;
  std::optional<error> tx_error_, rx_error_;
  std::array<bool, 4> pin_claimed_{};
  alignas(4) std::array<std::uint8_t, 5> tail_{};
  std::size_t tx_count_{}, rx_count_{}, sent_{}, received_{};
  std::uint32_t error_bits_{};
  mcu::nrf52840::serial_revision revision_{};
  phase rx_phase_{phase::idle};
  detail::rx_queue<StreamCapacity> stream_queue_;
  std::array<std::optional<dma_buffer_token>, StreamCapacity> stream_tokens_{};
  std::optional<unsigned> stream_active_;
  std::size_t stream_limit_{}, stream_received_{};
  bool streaming_{}, setting_up_stream_{}, stream_start_issued_{};
  bool ready_{}, fault_{}, hf_{}, work_{}, tx_stopping_{}, tx_quiet_{};
};
} // namespace mcubsp::backend::nrf52840
