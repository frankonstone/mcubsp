#pragma once
// @req-file REQ-0054 REQ-0061 REQ-0062
// PS 4413_417 v1.11 6.31. STOPPED, not LASTTX/LASTRX, releases RAM.
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/backend/nrf52840/dma_address.hpp>
#include <mcubsp/backend/nrf52840/dma_buffers.hpp>
#include <mcubsp/backend/nrf52840/i2c.hpp>
#include <mcubsp/mcu/nrf52840/product/twim0.hpp>
#include <type_traits>
namespace mcubsp::backend::nrf52840 {
struct twim_timing : driver::i2c_timing {
  std::uint32_t frequency;
};
// Apply erratum 219 conservatively at every stepping: use the documented
// 390 kHz encoding instead of promising compliant 400 kHz wire timing.
[[nodiscard]] constexpr std::expected<twim_timing, driver::i2c_error>
solve_twim_timing(driver::i2c_timing_request r) noexcept {
  using enum driver::i2c_error;
  if (r.clock_hz != 16000000)
    return std::unexpected(invalid_clock);
  if (!r.target_hz || r.target_hz > 400000)
    return std::unexpected(invalid_rate);
  constexpr std::array rates{100000U, 250000U, 390000U};
  constexpr std::array words{0x01980000U, 0x04000000U, 0x06200000U};
  std::optional<twim_timing> best;
  bool electrical{};
  for (unsigned i = 0; i < 3; ++i) {
    if (r.rise_time_ns > (rates[i] <= 100000 ? 1000U : 300U) ||
        r.fall_time_ns > 300)
      continue;
    electrical = true;
    auto d = rates[i] > r.target_hz ? rates[i] - r.target_hz
                                    : r.target_hz - rates[i];
    auto ppm = (std::uint64_t(d) * 1000000 + r.target_hz / 2) / r.target_hz;
    if (ppm <= r.maximum_error_ppm && (!best || ppm < best->error_ppm))
      best = twim_timing{{r.clock_hz, r.target_hz, rates[i],
                          static_cast<std::uint32_t>(ppm), r.rise_time_ns,
                          r.fall_time_ns},
                         words[i]};
  }
  if (!best)
    return std::unexpected(electrical ? unattainable_tolerance
                                      : invalid_rise_fall);
  return *best;
}
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned StopPollBudget = 1000000>
class twim_controller {
  static_assert(Binding::instance < 2 && StopPollBudget > 0);
  using error = driver::i2c_error;
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using pool_type =
      core::operation_pool<driver::i2c_transfer_result, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  twim_controller(Context &c, clock_controller<Context> &clocks,
                  Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                  Address address = {}) noexcept
      : c_(c), clocks_(clocks), buffers_(buffers), address_(address),
        operations_(c.execution), stop_ticks_(stop_ticks) {}
  twim_controller(const twim_controller &) = delete;
  twim_controller &operator=(const twim_controller &) = delete;
  ~twim_controller() {
    if (tx_ || rx_)
      c_.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(twim_timing timing) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::transport);
    auto solved =
        solve_twim_timing({timing.clock_hz, timing.target_hz, timing.error_ppm,
                           timing.rise_time_ns, timing.fall_time_ns});
    if (!solved || solved->frequency != timing.frequency ||
        solved->achieved_hz != timing.achieved_hz || !stop_ticks_ ||
        stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(error::invalid_rate);
    for (auto pin : pins)
      if (mcu::nrf52840::product::silicon_low_frequency_only(pin) &&
          timing.achieved_hz > 10000)
        return std::unexpected(error::invalid_rate);
    auto claim = c_.resources.claim(c_.execution, personality, this);
    if (!claim)
      return std::unexpected(error::already_owned);
    owner_ = *claim;
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled) {
      (void)release_owner();
      return std::unexpected(enabled ? error::already_owned : error::mmio);
    }
    for (unsigned i = 0; i < 2; ++i) {
      if (!c_.gpio.claim(pins[i], driver::gpio_direction::output)) {
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
    for (auto pin : pins)
      if (!c_.gpio.configure(
              pin, {.direction = driver::gpio_direction::output,
                    .initial_level = driver::gpio_level::high,
                    .output_type = driver::gpio_output_type::open_drain}))
        return init_failed();
    if (!reg_write(0x308, all_irq_mask) || !reg_write(0x104, 0) ||
        !reg_write(0x124, 0) || !reg_write(0x148, 0) || !reg_write(0x4C4, 7) ||
        !reg_write(0x200, 0) || !reg_write(0x540, 0) || !reg_write(0x550, 0) ||
        !reg_write(0x508, (pins[0].port << 5) | pins[0].index) ||
        !reg_write(0x50C, (pins[1].port << 5) | pins[1].index) ||
        !reg_write(0x524, timing.frequency) || !reg_write(0x500, 6))
      return init_failed();
    ready_ = true;
    work_ = true;
    c_.work_started();
    return {};
  }
  [[nodiscard]] start_result
  write(driver::i2c_address address,
        std::span<const std::uint8_t> bytes) noexcept {
    if (bytes.empty())
      return failure(error::empty_transfer);
    return start(address, bytes, {}, false);
  }
  [[nodiscard]] start_result read(driver::i2c_address address,
                                  std::span<std::uint8_t> bytes) noexcept {
    if (bytes.empty())
      return failure(error::empty_transfer);
    return start(address, {}, bytes, false);
  }
  [[nodiscard]] start_result write_read(driver::i2c_address address,
                                        std::span<const std::uint8_t> tx,
                                        std::span<std::uint8_t> rx) noexcept {
    if (tx.empty() || rx.empty())
      return failure(error::empty_transfer);
    return start(address, tx, rx, false);
  }
  [[nodiscard]] start_result
  read_write(driver::i2c_address address, std::span<std::uint8_t> rx,
             std::span<const std::uint8_t> tx) noexcept {
    if (tx.empty() || rx.empty())
      return failure(error::empty_transfer);
    return start(address, tx, rx, true);
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_)
      return false;
    return c_.execution.critical([&] {
      if (!sample_errors()) {
        fault_ = true;
        pending_ = error::mmio;
      }
      auto stopped = c_.read(base + 0x104);
      if (!stopped) {
        fault_ = true;
        pending_ = error::mmio;
      }
      if (!fault_ && !pending_ && (!stopped || !*stopped))
        return false;
      if (!stop_step()) {
        if (fault_)
          (void)reg_write(0x308, all_irq_mask);
        return false;
      }
      if (!reg_write(0x200, 0)) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      auto id = *id_;
      id_.reset();
      if (pending_)
        (void)operations_.fail(id, *pending_);
      else
        (void)operations_.complete(id, counts_);
      return true;
    });
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool enable) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!reg_write(enable ? 0x304 : 0x308, irq_mask))
      return std::unexpected(error::mmio);
    return {};
  }
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (c_.execution.in_isr() || id_)
      return std::unexpected(error::busy);
    if (!owner_)
      return {};
    if (active_ && !bounded_stop())
      return std::unexpected(error::transport);
    if (!reg_write(0x308, all_irq_mask) || !reg_write(0x200, 0) ||
        !reg_write(0x500, 0))
      return std::unexpected(error::mmio);
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled)
      return std::unexpected(error::mmio);
    if (!reg_write(0x508, 0x80000000U) || !reg_write(0x50C, 0x80000000U))
      return std::unexpected(error::mmio);
    for (auto pin : pins)
      if (!c_.gpio.configure(pin, {}))
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
  [[nodiscard]] driver::i2c_transfer_result transferred_count() const noexcept {
    return c_.execution.critical([&] { return counts_; });
  }
  [[nodiscard]] std::uint32_t error_source() const noexcept {
    return c_.execution.critical([&] { return error_bits_; });
  }
  [[nodiscard]] bool faulted() const noexcept {
    return c_.execution.critical([&] { return fault_; });
  }

private:
  start_result failure(error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  bool reg_write(unsigned offset, std::uint32_t v) noexcept {
    if (offset >= 0x100 && offset < 0x200 && !v)
      return c_.clear_event(base + offset).has_value();
    auto r = c_.write(base + offset, v);
    arch::data_memory_barrier();
    return r.has_value();
  }
  void discard(std::optional<dma_buffer_token> &t) noexcept {
    if (t && buffers_.discard_prepared(*t))
      t.reset();
  }
  bool release(std::optional<dma_buffer_token> &t) noexcept {
    if (!t)
      return true;
    if (!buffers_.recover(*t, []() -> std::expected<bool, coordination_error> {
          return true;
        }))
      return false;
    t.reset();
    return true;
  }
  start_result start(driver::i2c_address address,
                     std::span<const std::uint8_t> tx,
                     std::span<std::uint8_t> rx, bool read_first) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(error::not_initialized);
    if (!driver::make_i2c_address(address.value))
      return failure(error::invalid_address);
    if (tx.size() > 65535 || rx.size() > 65535)
      return failure(error::excessive_length);
    return c_.execution.critical([&]() -> start_result {
      if (id_ || active_)
        return failure(error::busy);
      if (!tx.empty()) {
        auto t = buffers_.prepare(
            *owner_, mcu::nrf52840::serial_dma_constraints<personality>,
            address_(tx.data(), tx.size()), tx.size(), tx.size(),
            dma_access::read);
        if (!t)
          return failure(error::transport);
        tx_ = *t;
      }
      if (!rx.empty()) {
        auto t = buffers_.prepare(
            *owner_, mcu::nrf52840::serial_dma_constraints<personality>,
            address_(rx.data(), rx.size()), rx.size(), rx.size(),
            dma_access::write);
        if (!t) {
          discard(tx_);
          return failure(error::transport);
        }
        rx_ = *t;
      }
      auto operation =
          operations_.try_start({.quiesce = &cancel, .context = this});
      if (!operation) {
        discard(tx_);
        discard(rx_);
        return std::unexpected(operation.error());
      }
      id_ = operation->id();
      tx_size_ = tx.size();
      rx_size_ = rx.size();
      counts_ = {};
      error_bits_ = 0;
      pending_.reset();
      stopping_ = false;
      const auto tx_address = !tx.empty() ? address_(tx.data(), tx.size())
                                          : address_(rx.data(), rx.size());
      const auto rx_address =
          !rx.empty() ? address_(rx.data(), rx.size()) : tx_address;
      const auto shorts = tx.empty()   ? 1U << 12
                          : rx.empty() ? 1U << 9
                          : read_first ? (1U << 10) | (1U << 9)
                                       : (1U << 7) | (1U << 12);
      if (!reg_write(0x104, 0) || !reg_write(0x124, 0) ||
          !reg_write(0x148, 0) || !reg_write(0x4C4, 7) ||
          !reg_write(0x588, address.value) || !reg_write(0x544, tx_address) ||
          !reg_write(0x548, tx.size()) || !reg_write(0x534, rx_address) ||
          !reg_write(0x538, rx.size()) || !reg_write(0x200, shorts)) {
        discard(tx_);
        discard(rx_);
        auto id = *id_;
        id_.reset();
        (void)operations_.fail(id, error::mmio);
        return operation;
      }
      if ((tx_ && !buffers_.arm(*tx_)) || (rx_ && !buffers_.arm(*rx_))) {
        (void)release(tx_);
        (void)release(rx_);
        auto id = *id_;
        id_.reset();
        (void)operations_.fail(id, error::transport);
        return operation;
      }
      active_ = true;
      if (!reg_write((tx.empty() || read_first) ? 0x000 : 0x008, 1)) {
        fault_ = true;
        pending_ = error::mmio;
      }
      return operation;
    });
  }
  bool sample_errors() noexcept {
    auto event = c_.read(base + 0x124);
    if (!event)
      return error_sample_failed();
    if (!*event)
      return true;
    if (!reg_write(0x124, 0))
      return error_sample_failed();
    auto bits = c_.read(base + 0x4C4);
    if (!bits || !reg_write(0x4C4, *bits & 7U))
      return error_sample_failed();
    error_bits_ |= *bits & 7U;
    if (!pending_)
      pending_ = (*bits & 2U)   ? error::address_nack
                 : (*bits & 4U) ? error::data_nack
                                : error::transport;
    return true;
  }
  bool error_sample_failed() noexcept {
    fault_ = true;
    pending_ = error::mmio;
    return false;
  }
  bool finish_buffers() noexcept {
    if (!active_)
      return true;
    auto tx = c_.read(base + 0x54C), rx = c_.read(base + 0x53C);
    if (!tx || !rx) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    counts_ = {tx_size_ ? *tx : 0U, rx_size_ ? *rx : 0U};
    if (!pending_ &&
        (counts_.bytes_written != tx_size_ || counts_.bytes_read != rx_size_))
      pending_ = error::transport;
    if (!release(tx_) || !release(rx_)) {
      fault_ = true;
      return false;
    }
    active_ = false;
    return true;
  }
  bool stop_step() noexcept {
    if (!active_)
      return acknowledge_stop();
    auto stopped = c_.read(base + 0x104);
    if (!stopped) {
      fault_ = true;
      return false;
    }
    if (*stopped)
      return sample_errors() && finish_buffers() && acknowledge_stop();
    if (!stopping_) {
      if (!reg_write(0x200, 0))
        return false;
      stopping_ = true;
    }
    // This driver never requests suspension. RESUME also handles an
    // inherited/late suspended indication before the required STOP task.
    if (!reg_write(0x020, 1) || !reg_write(0x014, 1)) {
      fault_ = true;
      return false;
    }
    stopped = c_.read(base + 0x104);
    if (!stopped || !*stopped)
      return false;
    return sample_errors() && finish_buffers() && acknowledge_stop();
  }
  bool acknowledge_stop() noexcept {
    if (!reg_write(0x104, 0) || !reg_write(0x124, 0)) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    return true;
  }
  bool bounded_stop() noexcept {
    execution::deadline limit{
        static_cast<execution::tick>(c_.execution.now() + stop_ticks_)};
    auto budget = c_.execution.in_isr() ? 1U : StopPollBudget;
    for (unsigned i = 0; i < budget; ++i) {
      if (c_.execution.critical([&] { return stop_step(); }))
        return true;
      if (execution::reached(c_.execution.now(), limit))
        break;
    }
    return false;
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &s = *static_cast<twim_controller *>(p);
    if (!s.id_ || *s.id_ != id)
      return core::quiesce_result::reusable;
    bool quiet = s.bounded_stop();
    s.id_.reset();
    s.fault_ |= !quiet;
    if (!quiet)
      (void)s.reg_write(0x308, all_irq_mask);
    return quiet ? core::quiesce_result::reusable
                 : core::quiesce_result::faulted;
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
    for (unsigned i = 0; i < 2; ++i)
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
  static constexpr auto base = Binding::base;
  static constexpr auto personality =
      Binding::instance == 0 ? peripheral::twim0 : peripheral::twim1;
  static constexpr std::uint32_t irq_mask = (1U << 1) | (1U << 9);
  static constexpr auto all_irq_mask =
      mcu::nrf52840::product::twim0::intenclr::writable_mask;
  static constexpr std::array pins{Binding::scl_pin, Binding::sda_pin};
  Context &c_;
  clock_controller<Context> &clocks_;
  Buffers &buffers_;
  Address address_;
  pool_type operations_;
  execution::tick stop_ticks_;
  std::optional<resource_token> owner_;
  std::optional<core::operation_id> id_;
  std::optional<dma_buffer_token> tx_, rx_;
  std::optional<error> pending_;
  std::array<bool, 2> pin_claimed_{};
  std::size_t tx_size_{}, rx_size_{};
  driver::i2c_transfer_result counts_{};
  std::uint32_t error_bits_{};
  bool ready_{}, fault_{}, hf_{}, work_{}, active_{}, stopping_{};
};
} // namespace mcubsp::backend::nrf52840
