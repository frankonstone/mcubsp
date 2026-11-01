#pragma once
// @req-file REQ-0055 REQ-0060 REQ-0062
// PS v1.11 6.26. One granted CSN transaction per explicit buffer submission.
#include <mcubsp/backend/nrf52840/spim.hpp>
#include <mcubsp/mcu/nrf52840/product/spis0.hpp>
namespace mcubsp::backend::nrf52840 {
template <unsigned I, core::pin_id Sck, core::pin_id Mosi, core::pin_id Miso,
          core::pin_id Csn>
struct spis_binding : spim_binding<I, Sck, Mosi, Miso, Csn> {
  static_assert(I < 3, "SPIS instance must be 0 through 2");
  static_assert(Csn != spi_disconnected, "SPIS requires CSN");
  static constexpr auto peripheral_id =
      std::array{peripheral::spis0, peripheral::spis1, peripheral::spis2}[I];
};
struct spis_configuration {
  driver::spi_mode mode{driver::spi_mode::mode0};
  driver::spi_bit_order bit_order{
      driver::spi_bit_order::most_significant_first};
  std::uint8_t default_character{0xFF}, over_read_character{0xFF};
};
struct spis_result {
  std::size_t transmitted{}, received{};
  bool overread{}, overflow{};
};
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned StopPollBudget = 1000000>
class spis_controller {
  using error = driver::spi_error;
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using pool_type = core::operation_pool<spis_result, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  spis_controller(Context &c, clock_controller<Context> &clocks,
                  Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                  Address address = {}) noexcept
      : c_(c), clocks_(clocks), buffers_(buffers), address_(address),
        operations_(c.execution), stop_ticks_(stop_ticks) {}
  spis_controller(const spis_controller &) = delete;
  spis_controller &operator=(const spis_controller &) = delete;
  ~spis_controller() {
    if (tx_ || rx_)
      c_.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(spis_configuration config = {}) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::transport);
    if (!stop_ticks_ || stop_ticks_ > execution::maximum_deadline_interval ||
        static_cast<unsigned>(config.mode) > 3 ||
        static_cast<unsigned>(config.bit_order) > 1)
      return std::unexpected(error::invalid_frame);
    // Slave timing permits up to 8 MHz; NFC/low frequency pads cannot meet it.
    for (auto p : Binding::pins)
      if (p != spi_disconnected &&
          mcu::nrf52840::product::silicon_low_frequency_only(p))
        return std::unexpected(error::invalid_frame);
    auto owner = c_.resources.claim(c_.execution, Binding::peripheral_id, this);
    if (!owner)
      return std::unexpected(error::already_owned);
    owner_ = *owner;
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled) {
      (void)release_owner();
      return std::unexpected(enabled ? error::already_owned : error::mmio);
    }
    for (unsigned i = 0; i < 4; ++i)
      if (Binding::pins[i] != spi_disconnected) {
        auto direction = i == 2 ? driver::gpio_direction::output
                                : driver::gpio_direction::input;
        if (!c_.gpio.claim(Binding::pins[i], direction)) {
          release_pins();
          (void)release_owner();
          return std::unexpected(error::already_owned);
        }
        pins_[i] = true;
      }
    if (!clocks_.acquire_hf()) {
      release_pins();
      (void)release_owner();
      return std::unexpected(error::transport);
    }
    hf_ = true;
    for (unsigned i = 0; i < 4; ++i)
      if (pins_[i] &&
          !c_.gpio.configure(
              Binding::pins[i],
              {.direction = i == 2 ? driver::gpio_direction::output
                                   : driver::gpio_direction::input}))
        return init_failed();
    auto mode = static_cast<unsigned>(config.mode);
    if (!write(0x308, irq_mask) || !acknowledge() || !write(0x200, 4) ||
        !write(0x508, select(Binding::sck)) ||
        !write(0x50C, select(Binding::miso)) ||
        !write(0x510, select(Binding::mosi)) ||
        !write(0x514, select(Binding::cs)) ||
        !write(0x554, ((mode & 1U) << 1) | ((mode & 2U) << 1) |
                          static_cast<unsigned>(config.bit_order)) ||
        !write(0x55C, config.default_character) ||
        !write(0x5C0, config.over_read_character) || !write(0x540, 0) ||
        !write(0x550, 0) || !write(0x500, 2))
      return init_failed();
    ready_ = work_ = true;
    c_.work_started();
    return {};
  }
  [[nodiscard]] start_result transfer(std::span<const std::uint8_t> tx,
                                      std::span<std::uint8_t> rx) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(error::not_initialized);
    if (tx.empty() && rx.empty())
      return failure(error::empty_transfer);
    if (tx.size() > 65535 || rx.size() > 65535)
      return failure(error::excessive_length);
    if ((!tx.empty() && Binding::miso == spi_disconnected) ||
        (!rx.empty() && Binding::mosi == spi_disconnected))
      return failure(error::invalid_frame);
    return c_.execution.critical([&]() -> start_result {
      if (id_ || active_)
        return failure(error::busy);
      auto sem = c_.read(base + 0x400);
      if (!sem)
        return failure(error::mmio);
      if (*sem != 1)
        return failure(error::busy);
      if (!prepare(tx_, tx, dma_access::read) ||
          !prepare(rx_, rx, dma_access::write)) {
        discard(tx_);
        discard(rx_);
        return failure(error::dma);
      }
      auto operation =
          operations_.try_start({.quiesce = &cancel, .context = this});
      if (!operation) {
        discard(tx_);
        discard(rx_);
        return std::unexpected(operation.error());
      }
      id_ = operation->id();
      counts_ = {};
      tx_size_ = tx.size();
      rx_size_ = rx.size();
      quiet_ = false;
      pending_.reset();
      auto ta = tx.empty() ? address_(rx.data(), rx.size())
                           : address_(tx.data(), tx.size());
      auto ra = rx.empty() ? ta : address_(rx.data(), rx.size());
      if (!acknowledge() || !write(0x440, 3) || !write(0x544, ta) ||
          !write(0x548, tx.size()) || !write(0x534, ra) ||
          !write(0x538, rx.size())) {
        discard(tx_);
        discard(rx_);
        finish_operation(error::mmio);
        return operation;
      }
      if ((tx_ && !buffers_.arm(*tx_)) || (rx_ && !buffers_.arm(*rx_))) {
        (void)release(tx_);
        (void)release(rx_);
        finish_operation(error::dma);
        return operation;
      }
      active_ = true;
      if (!write(0x028, 1)) {
        fault_ = true;
        pending_ = error::mmio;
      }
      return operation;
    });
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_)
      return false;
    return c_.execution.critical([&] {
      auto end = c_.read(base + 0x104);
      if (!end) {
        fault_ = true;
        pending_ = error::mmio;
      }
      if (!pending_ && !quiet_ && (!end || !*end))
        return false;
      if (!stop_step()) {
        if (fault_)
          (void)write(0x308, irq_mask);
        return false;
      }
      finish_operation(pending_);
      return true;
    });
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool on) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!write(on ? 0x304 : 0x308, on ? 0x402U : irq_mask))
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
    if (!write(0x308, irq_mask) || !write(0x200, 0) || !write(0x500, 0))
      return std::unexpected(error::mmio);
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled)
      return std::unexpected(error::mmio);
    // Nordic anomaly 214: reset the residual TX byte after disabling SPIS.
    if (!write(0x0A4, 1) || !write(0x0AC, 1))
      return std::unexpected(error::mmio);
    for (unsigned off : {0x508U, 0x50CU, 0x510U, 0x514U})
      if (!write(off, 0x80000000))
        return std::unexpected(error::mmio);
    for (unsigned i = 0; i < 4; ++i)
      if (pins_[i] && !c_.gpio.configure(Binding::pins[i], {}))
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
  [[nodiscard]] spis_result transferred_count() const noexcept {
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
  static constexpr std::uint32_t select(core::pin_id p) noexcept {
    return p == spi_disconnected ? 0x80000000U
                                 : (std::uint32_t(p.port) << 5) | p.index;
  }
  bool write(unsigned off, std::uint32_t v) noexcept {
    if (off >= 0x100 && off < 0x200 && !v)
      return c_.clear_event(base + off).has_value();
    auto r = c_.write(base + off, v);
    arch::data_memory_barrier();
    return r.has_value();
  }
  bool acknowledge() noexcept {
    return write(0x104, 0) && write(0x110, 0) && write(0x128, 0);
  }
  template <class Span>
  bool prepare(std::optional<dma_buffer_token> &t, Span data,
               dma_access access) noexcept {
    if (data.empty())
      return true;
    auto r = buffers_.prepare(
        *owner_, mcu::nrf52840::serial_dma_constraints<Binding::peripheral_id>,
        address_(data.data(), data.size()), data.size(), data.size(), access);
    if (!r)
      return false;
    t = *r;
    return true;
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
  bool stop_step() noexcept {
    if (!quiet_) {
      if (!write(0x024, 1)) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      auto sem = c_.read(base + 0x400);
      if (!sem) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      if (*sem != 1)
        return false;
      quiet_ = true; // Latch proof before acknowledging events that may fail.
    }
    if (active_) {
      auto tx = c_.read(base + 0x54C), rx = c_.read(base + 0x53C),
           status = c_.read(base + 0x440), end = c_.read(base + 0x104);
      if (!tx || !rx || !status || !end) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      // AMOUNT belongs to this submission only when a granted transaction
      // ended.
      counts_ = {*end && tx_size_ ? *tx : 0U, *end && rx_size_ ? *rx : 0U,
                 bool(*status & 1U), bool(*status & 2U)};
      if (counts_.transmitted > tx_size_ || counts_.received > rx_size_)
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
    auto &s = *static_cast<spis_controller *>(p);
    if (!s.id_ || *s.id_ != id)
      return core::quiesce_result::reusable;
    bool quiet = s.bounded_stop();
    s.id_.reset();
    s.fault_ |= !quiet;
    if (!quiet)
      (void)s.write(0x308, irq_mask);
    return quiet ? core::quiesce_result::reusable
                 : core::quiesce_result::faulted;
  }
  void finish_operation(std::optional<error> e) noexcept {
    auto id = *id_;
    id_.reset();
    if (e)
      (void)operations_.fail(id, *e);
    else
      (void)operations_.complete(id, counts_);
  }
  void release_pins() noexcept {
    for (unsigned i = 0; i < 4; ++i)
      if (pins_[i]) {
        c_.gpio.release(Binding::pins[i]);
        pins_[i] = false;
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
  static constexpr auto irq_mask =
      mcu::nrf52840::product::spis0::intenclr::writable_mask;
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
  std::array<bool, 4> pins_{};
  spis_result counts_{};
  std::size_t tx_size_{}, rx_size_{};
  bool ready_{}, fault_{}, hf_{}, work_{}, active_{}, quiet_{};
};
} // namespace mcubsp::backend::nrf52840
