#pragma once
// @req-file REQ-0055 REQ-0061 REQ-0062
// PS v1.11 6.32: address events suspend the bus until the caller responds.
#include <mcubsp/backend/nrf52840/twim.hpp>
#include <mcubsp/mcu/nrf52840/product/twis0.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
template <unsigned I, core::pin_id Scl, core::pin_id Sda>
struct twis_binding : i2c_binding<I, Scl, Sda> {
  static constexpr auto peripheral_id =
      I == 0 ? peripheral::twis0 : peripheral::twis1;
};
struct twis_configuration {
  driver::i2c_address address0{0x42};
  std::optional<driver::i2c_address> address1{};
  std::uint8_t over_read_character{0xFF};
};
enum class twis_direction : std::uint8_t { master_write, master_read };
struct twis_request {
  twis_direction direction{};
  unsigned address_slot{};
  driver::i2c_address address{};
  driver::i2c_transfer_result previous{};
};
struct twis_result {
  std::size_t transmitted{}, received{};
  std::uint32_t error_source{};
  unsigned address_slot{}, phases{};
  twis_direction last_direction{};
};
// Each direction may be served once per listen operation, including a repeated
// START. Further requests remain stretched until explicitly stopped/cancelled.
// Buffers stay leased until STOPPED and peripheral disable have been observed.
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned StopPollBudget = 1000000>
class twis_controller {
  static_assert(Binding::instance < 2 && StopPollBudget > 0);
  using error = driver::i2c_error;
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using pool_type = core::operation_pool<twis_result, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  twis_controller(Context &c, clock_controller<Context> &clocks,
                  Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                  Address address = {}) noexcept
      : c_(c), clocks_(clocks), buffers_(buffers), address_(address),
        operations_(c.execution), stop_ticks_(stop_ticks) {}
  twis_controller(const twis_controller &) = delete;
  twis_controller &operator=(const twis_controller &) = delete;
  ~twis_controller() {
    if (tx_ || rx_)
      c_.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(twis_configuration config = {}) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::transport);
    if (!driver::make_i2c_address(config.address0.value) ||
        (config.address1 &&
         (!driver::make_i2c_address(config.address1->value) ||
          *config.address1 == config.address0)))
      return std::unexpected(error::invalid_address);
    if (!stop_ticks_ || stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(error::invalid_rate);
    auto part = c_.read(0x10000130), revision = c_.read(0x10000134);
    // Engineering A's duplicate STOPPED anomaly 83 has no reviewed protocol
    // here. Fail explicitly before enabling rather than accepting false proof.
    if (!part || !revision)
      return std::unexpected(error::mmio);
    if (!mcu::nrf52840::serial_revision{*part, *revision}.reviewed_modern())
      return std::unexpected(error::transport);
    for (auto p : pins)
      if (mcu::nrf52840::product::silicon_low_frequency_only(p))
        return std::unexpected(error::invalid_rate);
    auto owner = c_.resources.claim(c_.execution, Binding::peripheral_id, this);
    if (!owner)
      return std::unexpected(error::already_owned);
    owner_ = *owner;
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
      pins_claimed_[i] = true;
    }
    if (!clocks_.acquire_hf()) {
      release_pins();
      (void)release_owner();
      return std::unexpected(error::transport);
    }
    hf_ = true;
    for (auto p : pins)
      if (!c_.gpio.configure(
              p, {.direction = driver::gpio_direction::output,
                  .initial_level = driver::gpio_level::high,
                  .output_type = driver::gpio_output_type::open_drain}))
        return init_failed();
    config_ = config;
    if (!reg_write(0x308, all_irq_mask) || !acknowledge() ||
        !reg_write(0x4D0, 13) || !reg_write(0x200, (1U << 13) | (1U << 14)) ||
        !reg_write(0x540, 0) || !reg_write(0x550, 0) ||
        !reg_write(0x508, (pins[0].port << 5) | pins[0].index) ||
        !reg_write(0x50C, (pins[1].port << 5) | pins[1].index) ||
        !reg_write(0x588, config.address0.value) ||
        !reg_write(0x58C, config.address1 ? config.address1->value : 0) ||
        !reg_write(0x594, config.address1 ? 3 : 1) ||
        !reg_write(0x5C0, config.over_read_character))
      return init_failed();
    ready_ = work_ = true;
    c_.work_started();
    return {};
  }
  // Empty directions can be supplied later while the address is stretched.
  // The caller sets a deadline on the operation if the peer never sends STOP.
  [[nodiscard]] start_result listen(std::span<const std::uint8_t> tx = {},
                                    std::span<std::uint8_t> rx = {}) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(error::not_initialized);
    if (tx.size() > 65535 || rx.size() > 65535)
      return failure(error::excessive_length);
    return c_.execution.critical([&]() -> start_result {
      if (id_ || active_)
        return failure(error::busy);
      if (!prepare(tx_, tx, dma_access::read) ||
          !prepare(rx_, rx, dma_access::write)) {
        discard(tx_);
        discard(rx_);
        return failure(error::transport);
      }
      auto operation =
          operations_.try_start({.quiesce = &cancel, .context = this});
      if (!operation) {
        discard(tx_);
        discard(rx_);
        return std::unexpected(operation.error());
      }
      id_ = operation->id();
      tx_data_ = tx;
      rx_data_ = rx;
      counts_ = {};
      pending_.reset();
      request_.reset();
      quiet_ = stopping_ = false;
      tx_used_ = rx_used_ = false;
      if (!acknowledge() || !reg_write(0x4D0, 13) || !reg_write(0x500, 9)) {
        // ENABLE may have committed; no PREPARE has been issued, so DMA cannot
        // touch the prepared leases. Disable before allowing another listen.
        discard(tx_);
        discard(rx_);
        fault_ = true;
        finish_operation(error::mmio);
        return operation;
      }
      active_ = true;
      return operation;
    });
  }
  [[nodiscard]] std::optional<twis_request> pending_request() const noexcept {
    return c_.execution.critical([&] { return request_; });
  }
  // Called after poll exposes READ/WRITE. Repeated START may switch direction;
  // each supplied buffer is prepared at most once and never silently reused.
  [[nodiscard]] std::expected<void, error> resume() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(error::busy);
    return c_.execution.critical([&]() -> std::expected<void, error> {
      if (!id_ || !request_ || pending_ || stopping_)
        return std::unexpected(error::busy);
      bool read = request_->direction == twis_direction::master_read;
      if ((read && (tx_data_.empty() || tx_used_)) ||
          (!read && (rx_data_.empty() || rx_used_)))
        return std::unexpected(error::empty_transfer);
      auto &token = read ? tx_ : rx_;
      if (!token || !buffers_.arm(*token))
        return std::unexpected(error::transport);
      // Arm before PREPARE; it can start DMA even when MMIO reports failure.
      if (read)
        tx_used_ = true;
      else
        rx_used_ = true;
      bool ok =
          read
              ? (reg_write(0x544, address_(tx_data_.data(), tx_data_.size())) &&
                 reg_write(0x548, tx_data_.size()) && reg_write(0x034, 1))
              : (reg_write(0x534, address_(rx_data_.data(), rx_data_.size())) &&
                 reg_write(0x538, rx_data_.size()) && reg_write(0x030, 1));
      if (ok)
        ok = reg_write(0x020, 1);
      if (!ok) {
        fault_ = true;
        pending_ = error::mmio;
        return std::unexpected(error::mmio);
      }
      request_.reset();
      return {};
    });
  }
  [[nodiscard]] std::expected<void, error>
  provide_transmit(std::span<const std::uint8_t> bytes) noexcept {
    return supply(tx_, tx_data_, bytes, dma_access::read,
                  twis_direction::master_read, tx_used_);
  }
  [[nodiscard]] std::expected<void, error>
  provide_receive(std::span<std::uint8_t> bytes) noexcept {
    return supply(rx_, rx_data_, bytes, dma_access::write,
                  twis_direction::master_write, rx_used_);
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_)
      return false;
    return c_.execution.critical([&] {
      bool progress = false;
      if (!sample_error()) {
        fault_ = true;
        pending_ = error::mmio;
      }
      auto stopped = c_.read(base + 0x104);
      if (!stopped) {
        fault_ = true;
        pending_ = error::mmio;
      }
      if (!quiet_ && !pending_)
        progress = sample_request();
      if (quiet_ || pending_ || stopping_ || (stopped && *stopped)) {
        if (!stop_step()) {
          if (fault_)
            (void)reg_write(0x308, all_irq_mask);
          return false;
        }
        finish_operation(pending_);
        return true;
      }
      return progress;
    });
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool on) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!reg_write(on ? 0x304 : 0x308, irq_mask))
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
    if (!reg_write(0x308, all_irq_mask) || !reg_write(0x200, 0) || !disable())
      return std::unexpected(error::mmio);
    if (!reg_write(0x508, 0x80000000) || !reg_write(0x50C, 0x80000000))
      return std::unexpected(error::mmio);
    for (unsigned i = 0; i < 2; ++i)
      if (pins_claimed_[i] && !c_.gpio.configure(pins[i], {}))
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
  [[nodiscard]] twis_result transferred_count() const noexcept {
    return c_.execution.critical([&] { return counts_; });
  }
  [[nodiscard]] bool faulted() const noexcept {
    return c_.execution.critical([&] { return fault_; });
  }

private:
  start_result failure(error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  bool reg_write(unsigned off, std::uint32_t v) noexcept {
    if (off >= 0x100 && off < 0x200 && !v)
      return c_.clear_event(base + off).has_value();
    auto r = c_.write(base + off, v);
    arch::data_memory_barrier();
    return r.has_value();
  }
  bool acknowledge() noexcept {
    for (unsigned off : {0x104U, 0x124U, 0x14CU, 0x150U, 0x164U, 0x168U})
      if (!reg_write(off, 0))
        return false;
    return true;
  }
  template <class Span>
  bool prepare(std::optional<dma_buffer_token> &token, Span data,
               dma_access access) noexcept {
    if (data.empty())
      return true;
    auto t = buffers_.prepare(
        *owner_, mcu::nrf52840::serial_dma_constraints<Binding::peripheral_id>,
        address_(data.data(), data.size()), data.size(), data.size(), access);
    if (!t)
      return false;
    token = *t;
    return true;
  }
  template <class Span>
  std::expected<void, error>
  supply(std::optional<dma_buffer_token> &token, Span &storage, Span data,
         dma_access access, twis_direction direction, bool used) noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(error::busy);
    return c_.execution.critical([&]() -> std::expected<void, error> {
      if (!id_ || !request_ || request_->direction != direction || used ||
          token || !storage.empty() || pending_)
        return std::unexpected(error::busy);
      if (data.empty())
        return std::unexpected(error::empty_transfer);
      if (data.size() > 65535)
        return std::unexpected(error::excessive_length);
      if (!prepare(token, data, access))
        return std::unexpected(error::transport);
      storage = data;
      return {};
    });
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
  bool sample_request() noexcept {
    bool progress = false;
    auto write = c_.read(base + 0x164), read = c_.read(base + 0x168);
    if (!write || !read) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    if (*write || *read) {
      if (request_ || (*write && *read)) {
        pending_ = error::transport;
        return false;
      }
      auto match = c_.read(base + 0x4D4);
      auto tx = c_.read(base + 0x54C), rx = c_.read(base + 0x53C);
      if (!match || !tx || !rx || !reg_write(*read ? 0x168 : 0x164, 0)) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      unsigned slot = *match & 1U;
      if (slot && !config_.address1) {
        pending_ = error::transport;
        return false;
      }
      counts_.address_slot = slot;
      counts_.last_direction =
          *read ? twis_direction::master_read : twis_direction::master_write;
      ++counts_.phases;
      if (counts_.phases > 2) {
        pending_ = error::transport;
        return false;
      }
      request_ = twis_request{*read ? twis_direction::master_read
                                    : twis_direction::master_write,
                              slot,
                              slot ? *config_.address1 : config_.address0,
                              {tx_used_ ? *tx : 0U, rx_used_ ? *rx : 0U}};
      progress = true;
    }
    return progress;
  }
  bool sample_error() noexcept {
    auto event = c_.read(base + 0x124);
    if (!event)
      return false;
    if (!*event)
      return true;
    if (!reg_write(0x124, 0))
      return false;
    auto bits = c_.read(base + 0x4D0);
    if (!bits || !reg_write(0x4D0, *bits & 13U))
      return false;
    counts_.error_source |= *bits & 13U;
    // Master DNACK terminates its read normally. OVERREAD/OVERFLOW are
    // reported in the result, then STOP the operation without losing counts.
    if (*bits & 9U)
      stopping_ = true;
    return true;
  }
  bool disable() noexcept {
    if (!reg_write(0x500, 0))
      return false;
    auto enabled = c_.read(base + 0x500);
    return enabled && !*enabled;
  }
  bool stop_step() noexcept {
    if (!quiet_) {
      auto stopped = c_.read(base + 0x104);
      if (!stopped) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      if (!*stopped) {
        stopping_ = true;
        if (!reg_write(0x014, 1)) {
          fault_ = true;
          pending_ = error::mmio;
          return false;
        }
        stopped = c_.read(base + 0x104);
        if (!stopped || !*stopped)
          return false;
      }
      quiet_ = true;
    }
    // Prevent a new address/repeated start from using a PREPARE flag while
    // buffers are being returned. Do not clear STOPPED until proof is latched.
    if (!disable() || !sample_error()) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    if (active_) {
      auto tx = c_.read(base + 0x54C), rx = c_.read(base + 0x53C);
      if (!tx || !rx) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      counts_.transmitted = tx_used_ ? *tx : 0U;
      counts_.received = rx_used_ ? *rx : 0U;
      if (counts_.transmitted > tx_data_.size() ||
          counts_.received > rx_data_.size())
        pending_ = error::transport;
      if (!release(tx_) || !release(rx_)) {
        fault_ = true;
        return false;
      }
      active_ = false;
    }
    if (!acknowledge()) {
      fault_ = true;
      pending_ = error::mmio;
      return false;
    }
    request_.reset();
    return true;
  }
  bool bounded_stop() noexcept {
    execution::deadline until{
        static_cast<execution::tick>(c_.execution.now() + stop_ticks_)};
    auto budget = c_.execution.in_isr() ? 1U : StopPollBudget;
    for (unsigned i = 0; i < budget; ++i) {
      if (c_.execution.critical([&] { return stop_step(); }))
        return true;
      if (execution::reached(c_.execution.now(), until))
        break;
    }
    return false;
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &s = *static_cast<twis_controller *>(p);
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
  void finish_operation(std::optional<error> e) noexcept {
    auto id = *id_;
    id_.reset();
    request_.reset();
    if (e)
      (void)operations_.fail(id, *e);
    else
      (void)operations_.complete(id, counts_);
  }
  void release_pins() noexcept {
    for (unsigned i = 0; i < 2; ++i)
      if (pins_claimed_[i]) {
        c_.gpio.release(pins[i]);
        pins_claimed_[i] = false;
      }
  }
  bool release_owner() noexcept {
    if (!owner_)
      return true;
    if (!c_.resources.release(c_.execution, *owner_))
      return false;
    owner_.reset();
    return true;
  }
  std::expected<void, error> init_failed() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(error::mmio);
  }
  static constexpr auto base = Binding::base;
  static constexpr std::array pins{Binding::scl_pin, Binding::sda_pin};
  static constexpr auto all_irq_mask =
      mcu::nrf52840::product::twis0::intenclr::writable_mask;
  static constexpr auto irq_mask =
      (1U << 1) | (1U << 9) | (1U << 25) | (1U << 26);
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
  std::optional<twis_request> request_;
  twis_configuration config_{};
  twis_result counts_{};
  std::span<const std::uint8_t> tx_data_;
  std::span<std::uint8_t> rx_data_;
  std::array<bool, 2> pins_claimed_{};
  bool ready_{}, fault_{}, hf_{}, work_{}, active_{}, quiet_{}, stopping_{},
      tx_used_{}, rx_used_{};
};
} // namespace mcubsp::backend::nrf52840
