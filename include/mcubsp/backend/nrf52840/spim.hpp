#pragma once
// @req-file REQ-0028 REQ-0060 REQ-0062
// PS 4413_417 v1.11 6.25; finite native EasyDMA transactions.
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/backend/nrf52840/dma_address.hpp>
#include <mcubsp/backend/nrf52840/dma_buffers.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/spi.hpp>
#include <mcubsp/mcu/nrf52840/product/spim0.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
#include <type_traits>
namespace mcubsp::backend::nrf52840 {
inline constexpr core::pin_id spi_disconnected{0xFFFF, 0xFFFF};
template <unsigned I, core::pin_id Sck, core::pin_id Mosi, core::pin_id Miso,
          core::pin_id Cs = spi_disconnected, unsigned DedicatedAhbSlave = 9>
struct spim_binding {
  static_assert(I < 4, "SPIM instance must be 0 through 3");
  static_assert(
      mcu::nrf52840::gpio_supported(Sck) &&
          (Mosi == spi_disconnected || mcu::nrf52840::gpio_supported(Mosi)) &&
          (Miso == spi_disconnected || mcu::nrf52840::gpio_supported(Miso)) &&
          (Cs == spi_disconnected || mcu::nrf52840::gpio_supported(Cs)),
      "SPIM pins must be available GPIOs");
  static_assert(Mosi != spi_disconnected || Miso != spi_disconnected,
                "SPIM requires a data pin");
  static constexpr std::array pins{Sck, Mosi, Miso, Cs};
  static_assert(
      [] {
        for (unsigned i = 0; i < 4; ++i) {
          for (unsigned j = 0; j < i; ++j) {
            if (pins[i] != spi_disconnected && pins[i] == pins[j])
              return false;
          }
        }
        return true;
      }(),
      "SPIM pins must be distinct");
  static constexpr bool easy_dma = true;
  static constexpr unsigned instance = I;
  static constexpr unsigned dedicated_ahb_slave = DedicatedAhbSlave;
  static constexpr auto clock_pin = Sck, mosi_pin = Mosi, miso_pin = Miso,
                        select_pin = Cs;
  static constexpr auto peripheral_id =
      std::array{peripheral::spim0, peripheral::spim1, peripheral::spim2,
                 peripheral::spim3}[I];
  static constexpr auto base = std::array<std::uintptr_t, 4>{
      0x40003000, 0x40004000, 0x40023000, 0x4002F000}[I];
  static constexpr auto sck = Sck, mosi = Mosi, miso = Miso, cs = Cs;
};
struct spim_timing : driver::spi_timing {
  std::uint32_t encoding;
};
[[nodiscard]] constexpr std::expected<spim_timing, driver::spi_error>
solve_spim_timing(unsigned instance, driver::spi_timing_request r) noexcept {
  if (instance > 3 || r.clock_hz != 16'000'000)
    return std::unexpected(driver::spi_error::invalid_clock);
  if (!r.target_hz || r.target_hz > (instance == 3 ? 32'000'000U : 8'000'000U))
    return std::unexpected(driver::spi_error::invalid_rate);
  constexpr std::array rates{125000U,  250000U,  500000U,   1000000U, 2000000U,
                             4000000U, 8000000U, 16000000U, 32000000U};
  constexpr std::array words{0x02000000U, 0x04000000U, 0x08000000U,
                             0x10000000U, 0x20000000U, 0x40000000U,
                             0x80000000U, 0x0A000000U, 0x14000000U};
  std::optional<spim_timing> best;
  for (unsigned i = 0; i < (instance == 3 ? 9U : 7U); ++i) {
    const auto d = rates[i] > r.target_hz ? rates[i] - r.target_hz
                                          : r.target_hz - rates[i];
    auto ppm = (std::uint64_t(d) * 1000000 + r.target_hz / 2) / r.target_hz;
    if (ppm <= r.maximum_error_ppm && (!best || ppm < best->error_ppm))
      best = spim_timing{
          {r.clock_hz, r.target_hz, rates[i], static_cast<std::uint32_t>(ppm)},
          words[i]};
  }
  if (!best)
    return std::unexpected(driver::spi_error::unattainable_tolerance);
  return *best;
}
struct spim_segment {
  std::span<const std::uint8_t> tx;
  std::span<std::uint8_t> rx;
  std::uint8_t over_read{0xFF};
};
struct spim_result {
  std::size_t frames{}, transmitted{}, received{};
};
// Erratum 198 requires the SPIM3 TX buffer's entire AHB RAM slave to be
// dedicated. The application must reserve it in its linker/storage layout and
// must not access it from CPU/ISR during a transaction (including its code
// alias). The DMA pool additionally excludes other DMA clients from that whole
// region.
struct spim3_tx_reservation {
  unsigned ahb_slave{9};
  [[nodiscard]] constexpr std::optional<mcu::nrf52840::memory::region>
  region() const noexcept {
    if (ahb_slave < 8)
      return mcu::nrf52840::memory::region{0x20000000U + ahb_slave * 0x2000U,
                                           0x2000};
    if (ahb_slave == 8)
      return mcu::nrf52840::memory::region{0x20010000U, 0x30000};
    return std::nullopt;
  }
};
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, std::size_t SegmentCapacity = 8,
          unsigned StopPollBudget = 1000000, bool CommonResult = false>
class spim_controller {
  static_assert(SegmentCapacity > 0 && StopPollBudget > 0);
  using error = driver::spi_error;
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using completion_type =
      std::conditional_t<CommonResult, driver::spi_transfer_result,
                         spim_result>;
  using pool_type = core::operation_pool<completion_type, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  spim_controller(Context &c, clock_controller<Context> &clocks,
                  Buffers &buffers, execution::tick stop_ticks = 12'800'000,
                  Address address = {}) noexcept
      : c_(c), clocks_(clocks), buffers_(buffers), address_(address),
        operations_(c.execution), stop_ticks_(stop_ticks) {}
  spim_controller(const spim_controller &) = delete;
  spim_controller &operator=(const spim_controller &) = delete;
  ~spim_controller() {
    if (tx_ || rx_)
      c_.execution.active_transfer_destroyed();
  }
  [[nodiscard]] std::expected<void, error>
  initialize(spim_timing timing, driver::spi_configuration config = {},
             spim3_tx_reservation dedicated = {}) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::transport);
    auto solved =
        solve_spim_timing(Binding::instance, {timing.clock_hz, timing.target_hz,
                                              timing.error_ppm});
    if (!solved || solved->encoding != timing.encoding ||
        solved->achieved_hz != timing.achieved_hz)
      return std::unexpected(error::invalid_rate);
    if (!stop_ticks_ || stop_ticks_ > execution::maximum_deadline_interval ||
        config.frame_width != driver::spi_frame_width::bits8 ||
        static_cast<unsigned>(config.mode) > 3 ||
        static_cast<unsigned>(config.bit_order) > 1 ||
        static_cast<unsigned>(config.chip_select) > 1)
      return std::unexpected(error::invalid_frame);
    for (auto pin : Binding::pins)
      if (pin != spi_disconnected &&
          mcu::nrf52840::product::silicon_low_frequency_only(pin) &&
          timing.achieved_hz > 10000)
        return std::unexpected(error::invalid_rate);
    auto part = c_.read(0x10000130), rev = c_.read(0x10000134);
    if (!part || !rev)
      return std::unexpected(error::mmio);
    revision_ = {*part, *rev};
    if constexpr (Binding::instance == 3) {
      if (!revision_.reviewed_modern() || !dedicated.region())
        return std::unexpected(error::invalid_frame);
      dedicated_ = dedicated.region();
      reservation_ = dedicated;
    }
    auto claim = c_.resources.claim(c_.execution, Binding::peripheral_id, this);
    if (!claim)
      return std::unexpected(error::already_owned);
    owner_ = *claim;
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled) {
      (void)release_owner();
      return std::unexpected(enabled ? error::already_owned : error::mmio);
    }
    for (unsigned i = 0; i < 4; ++i)
      if (Binding::pins[i] != spi_disconnected) {
        if (!c_.gpio.claim(Binding::pins[i],
                           i == 2 ? driver::gpio_direction::input
                                  : driver::gpio_direction::output)) {
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
    config_ = config;
    for (unsigned i = 0; i < 4; ++i)
      if (pins_[i]) {
        const bool high = i == 0 ? static_cast<unsigned>(config.mode) >= 2
                          : i == 3
                              ? config.chip_select ==
                                    driver::spi_chip_select_polarity::active_low
                              : false;
        if (!c_.gpio.configure(
                Binding::pins[i],
                {.direction = i == 2 ? driver::gpio_direction::input
                                     : driver::gpio_direction::output,
                 .initial_level = high ? driver::gpio_level::high
                                       : driver::gpio_level::low}))
          return init_failed();
      }
    const auto mode = static_cast<unsigned>(config.mode);
    if (!write(0x308, all_irq_mask) || !write(0x104, 0) || !write(0x118, 0) ||
        !write(0x110, 0) || !write(0x120, 0) || !write(0x14C, 0) ||
        !write(0x200, 0) || !write(0x540, 0) || !write(0x550, 0) ||
        !write(0x508, select(Binding::sck)) ||
        !write(0x50C, select(Binding::mosi)) ||
        !write(0x510, select(Binding::miso)) ||
        !write(0x524, timing.encoding) ||
        !write(0x554,
               ((mode & 1U) << 1) | ((mode & 2U) << 1) |
                   (config.bit_order ==
                            driver::spi_bit_order::least_significant_first
                        ? 1U
                        : 0U)))
      return init_failed();
    if constexpr (Binding::instance == 3)
      if (!write(0x514, 0x80000000U) || !write(0x56C, 0x80000000U))
        return init_failed();
    if (!write(0x500, 7))
      return init_failed();
    ready_ = true;
    work_ = true;
    c_.work_started();
    return {};
  }
  using transfer_type = typename pool_type::transfer_type;
  [[nodiscard]] bool busy() const noexcept {
    return id_.has_value() || active_ || selected_;
  }
  [[nodiscard]] std::expected<void, error>
  set_select_handler(bool (*set)(void *, bool) noexcept,
                     void *context) noexcept {
    if (busy())
      return std::unexpected(error::busy);
    select_handler_ = set;
    select_context_ = context;
    return {};
  }
  [[nodiscard]] std::expected<void, error>
  reconfigure(spim_timing timing,
              driver::spi_configuration config = {}) noexcept {
    if (!ready_)
      return std::unexpected(error::not_initialized);
    if (busy())
      return std::unexpected(error::busy);
    if (fault_)
      return std::unexpected(error::transport);
    auto solved =
        solve_spim_timing(Binding::instance, {timing.clock_hz, timing.target_hz,
                                              timing.error_ppm});
    if (!solved || solved->encoding != timing.encoding ||
        solved->achieved_hz != timing.achieved_hz)
      return std::unexpected(error::invalid_rate);
    if (config.frame_width != driver::spi_frame_width::bits8 ||
        static_cast<unsigned>(config.mode) > 3 ||
        static_cast<unsigned>(config.bit_order) > 1 ||
        static_cast<unsigned>(config.chip_select) > 1)
      return std::unexpected(error::invalid_frame);
    auto stopped = shutdown();
    if (!stopped)
      return stopped;
    return initialize(timing, config, reservation_);
  }
  [[nodiscard]] start_result
  transaction(std::span<const driver::spi_segment> segments) noexcept
    requires(CommonResult)
  {
    if (segments.empty())
      return failure(error::empty_transfer);
    if (segments.size() > SegmentCapacity)
      return failure(error::excessive_length);
    std::array<spim_segment, SegmentCapacity> native{};
    for (std::size_t i = 0; i < segments.size(); ++i) {
      auto s = segments[i];
      auto checked = s.validate(driver::spi_frame_width::bits8);
      if (!checked)
        return failure(checked.error());
      if (!s.transmit.empty() || !s.receive.empty())
        return failure(error::invalid_frame);
      native[i] = {s.transmit_bytes, s.receive_bytes,
                   static_cast<std::uint8_t>(s.dummy)};
    }
    return transaction(
        std::span<const spim_segment>{native.data(), segments.size()});
  }
  [[nodiscard]] start_result
  transaction(std::span<const spim_segment> segments) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(error::not_initialized);
    if (segments.empty())
      return failure(error::empty_transfer);
    if (segments.size() > SegmentCapacity)
      return failure(error::excessive_length);
    for (const auto &s : segments) {
      if (s.tx.empty() && s.rx.empty())
        return failure(error::empty_transfer);
      if ((!s.tx.empty() && (Binding::mosi == spi_disconnected ||
                             !valid(s.tx.data(), s.tx.size()))) ||
          (!s.rx.empty() && (Binding::miso == spi_disconnected ||
                             !valid(s.rx.data(), s.rx.size()))))
        return failure(error::dma);
      if (!revision_.reviewed_modern() && s.rx.size() == 1 && s.tx.size() <= 1)
        return failure(error::invalid_frame); // anomaly 58
      if constexpr (Binding::instance == 3)
        if (!s.tx.empty() &&
            !dedicated_->contains(address_(s.tx.data(), s.tx.size()),
                                  s.tx.size()))
          return failure(error::dma);
    }
    return c_.execution.critical([&]() -> start_result {
      if (id_ || tx_ || rx_)
        return failure(error::busy);
      auto operation =
          operations_.try_start({.quiesce = &cancel, .context = this});
      if (!operation)
        return std::unexpected(operation.error());
      id_ = operation->id();
      size_ = segments.size();
      index_ = 0;
      result_ = {};
      pending_.reset();
      for (std::size_t i = 0; i < size_; ++i)
        segments_[i] = segments[i];
      selected_ = true;
      if (!chip_select(true)) {
        finish_failure(error::mmio);
        return operation;
      }
      if (!start_segment())
        finish_failure(pending_.value_or(error::dma));
      return operation;
    });
  }
  [[nodiscard]] start_result transfer(std::span<const std::uint8_t> tx,
                                      std::span<std::uint8_t> rx,
                                      std::uint8_t over_read = 0xFF) noexcept {
    const spim_segment s{tx, rx, over_read};
    return transaction({&s, 1});
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_)
      return false;
    return c_.execution.critical([&] {
      if (fault_ && !pending_)
        pending_ = error::mmio;
      if (!fault_ && !stopping_) {
        auto end = c_.read(base + 0x118);
        if (!end) {
          fault_ = true;
          pending_ = error::mmio;
        } else if (!*end)
          return false;
      }
      if (!stop_step()) {
        if (fault_)
          (void)write(0x308, all_irq_mask);
        return false;
      }
      if (!pending_) {
        auto &s = segments_[index_];
        if (tx_count_ != s.tx.size() || rx_count_ != s.rx.size())
          pending_ = error::dma;
        result_.transmitted += tx_count_;
        result_.received += rx_count_;
        result_.frames += s.tx.size() > s.rx.size() ? s.tx.size() : s.rx.size();
      }
      if (!pending_ && ++index_ < size_) {
        if (start_segment())
          return true;
        if (!pending_)
          pending_ = error::dma;
      }
      if (!chip_select(false)) {
        fault_ = true;
        pending_ = error::mmio;
        return false;
      }
      selected_ = false;
      auto id = *id_;
      id_.reset();
      if (pending_)
        (void)operations_.fail(id, *pending_);
      else
        (void)operations_.complete(id, completion_result());
      return true;
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
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (c_.execution.in_isr() || id_)
      return std::unexpected(error::busy);
    if (!owner_)
      return {};
    if ((tx_ || rx_) && !bounded_stop())
      return std::unexpected(error::transport);
    if (selected_ && !chip_select(false))
      return std::unexpected(error::mmio);
    selected_ = false;
    if (!write(0x308, all_irq_mask) || !write(0x200, 0) || !write(0x500, 0))
      return std::unexpected(error::mmio);
    auto disabled = c_.read(base + 0x500);
    if (!disabled || *disabled)
      return std::unexpected(error::mmio);
    if constexpr (Binding::instance == 3)
      if (!write(0x004, 1))
        return std::unexpected(error::mmio); // anomaly 195
    for (unsigned off : {0x508U, 0x50CU, 0x510U})
      if (!write(off, 0x80000000U))
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
  [[nodiscard]] bool faulted() const noexcept {
    return c_.execution.critical([&] { return fault_; });
  }

  [[nodiscard]] bool owns_resource() const noexcept {
    return owner_.has_value();
  }
  [[nodiscard]] bool operation_pending() const noexcept {
    return id_.has_value();
  }

private:
  completion_type completion_result() const noexcept {
    if constexpr (CommonResult)
      return {result_.frames};
    else
      return result_;
  }
  start_result failure(error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  bool valid(const void *p, std::size_t n) noexcept {
    return mcu::nrf52840::serial_dma_constraints<Binding::peripheral_id>.accepts(
        address_(p, n), n, n);
  }
  static constexpr std::uint32_t select(core::pin_id p) noexcept {
    return p == spi_disconnected ? 0x80000000U
                                 : (std::uint32_t(p.port) << 5) | p.index;
  }
  bool write(unsigned offset, std::uint32_t value) noexcept {
    if (offset >= 0x100 && offset < 0x200 && !value)
      return c_.clear_event(base + offset).has_value();
    auto r = c_.write(base + offset, value);
    arch::data_memory_barrier();
    return r.has_value();
  }
  bool chip_select(bool active) noexcept {
    if (select_handler_)
      return select_handler_(select_context_, active);
    if constexpr (Binding::cs == spi_disconnected)
      return true;
    bool high = active == (config_.chip_select ==
                           driver::spi_chip_select_polarity::active_high);
    return (high ? c_.gpio.set(Binding::cs) : c_.gpio.clear(Binding::cs))
        .has_value();
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
  bool start_segment() noexcept {
    auto &s = segments_[index_];
    stopping_ = active_ = quiet_ = false;
    tx_count_ = rx_count_ = 0;
    if (!s.tx.empty()) {
      auto a = address_(s.tx.data(), s.tx.size());
      auto n = s.tx.size();
      auto access = dma_access::read;
      if constexpr (Binding::instance == 3) {
        a = dedicated_->begin;
        n = dedicated_->bytes;
        access = dma_access::read_write;
      }
      auto t = buffers_.prepare(*owner_,
                                mcu::nrf52840::dma_constraints{0x40000, 1, 1},
                                a, n, n, access);
      if (!t)
        return false;
      tx_ = *t;
    }
    if (!s.rx.empty()) {
      auto t = buffers_.prepare(
          *owner_,
          mcu::nrf52840::serial_dma_constraints<Binding::peripheral_id>,
          address_(s.rx.data(), s.rx.size()), s.rx.size(), s.rx.size(),
          dma_access::write);
      if (!t) {
        discard(tx_);
        return false;
      }
      rx_ = *t;
    }
    // A zero count still gets a valid RAM pointer, never a reset/flash PTR.
    auto tx_address = !s.tx.empty() ? address_(s.tx.data(), s.tx.size())
                                    : address_(s.rx.data(), s.rx.size());
    auto rx_address =
        !s.rx.empty() ? address_(s.rx.data(), s.rx.size()) : tx_address;
    if (!write(0x104, 0) || !write(0x118, 0) || !write(0x110, 0) ||
        !write(0x120, 0) || !write(0x544, tx_address) ||
        !write(0x548, s.tx.size()) || !write(0x534, rx_address) ||
        !write(0x538, s.rx.size()) || !write(0x5C0, s.over_read)) {
      discard(tx_);
      discard(rx_);
      pending_ = error::mmio;
      return false;
    }
    if ((tx_ && !buffers_.arm(*tx_)) || (rx_ && !buffers_.arm(*rx_))) {
      // START was not issued, so both prepared/armed leases are quiescent.
      (void)release(tx_);
      (void)release(rx_);
      return false;
    }
    active_ = true;
    if (!write(0x010, 1)) {
      fault_ = true;
      pending_ = error::mmio;
    }
    return true;
  }
  bool stop_step() noexcept {
    if (!active_)
      return true;
    if (!stopping_) {
      if (!write(0x118, 0) || !write(0x104, 0)) {
        fault_ = true;
        return false;
      }
      stopping_ = true;
      if (!write(0x014, 1)) {
        fault_ = true;
        return false;
      }
    }
    if (!quiet_) {
      auto stopped = c_.read(base + 0x104);
      if (!stopped) {
        fault_ = true;
        return false;
      }
      if (!*stopped) {
        (void)write(0x014, 1);
        return false;
      }
      auto tx = c_.read(base + 0x54C), rx = c_.read(base + 0x53C);
      if (!tx || !rx) {
        fault_ = true;
        return false;
      }
      tx_count_ = *tx;
      rx_count_ = *rx;
      quiet_ = true;
    }
    if (!write(0x104, 0) || !write(0x118, 0)) {
      fault_ = true;
      return false;
    }
    if (!release(tx_) || !release(rx_)) {
      fault_ = true;
      return false;
    }
    active_ = false;
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
  void finish_failure(error e) noexcept {
    if (active_ || tx_ || rx_) {
      fault_ = true;
      pending_ = e;
      return;
    }
    if (selected_ && !chip_select(false)) {
      fault_ = true;
      pending_ = error::mmio;
      return;
    }
    selected_ = false;
    auto id = *id_;
    id_.reset();
    (void)operations_.fail(id, e);
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &s = *static_cast<spim_controller *>(p);
    if (!s.id_ || *s.id_ != id)
      return core::quiesce_result::reusable;
    bool quiet = s.bounded_stop();
    if (quiet && s.selected_)
      quiet = s.chip_select(false);
    if (quiet)
      s.selected_ = false;
    s.id_.reset();
    s.fault_ |= !quiet;
    if (!quiet)
      (void)s.write(0x308, all_irq_mask);
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
    for (unsigned i = 0; i < 4; ++i)
      if (pins_[i]) {
        c_.gpio.release(Binding::pins[i]);
        pins_[i] = false;
      }
  }
  std::expected<void, error> init_failed() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(error::mmio);
  }
  static constexpr auto base = Binding::base;
  static constexpr std::uint32_t irq_mask = (1U << 1) | (1U << 6);
  static constexpr auto all_irq_mask =
      mcu::nrf52840::product::spim0::intenclr::writable_mask;
  bool (*select_handler_)(void *, bool) noexcept {};
  void *select_context_{};
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
  std::optional<mcu::nrf52840::memory::region> dedicated_;
  spim3_tx_reservation reservation_{};
  mcu::nrf52840::serial_revision revision_;
  driver::spi_configuration config_;
  std::array<spim_segment, SegmentCapacity> segments_{};
  std::array<bool, 4> pins_{};
  std::size_t size_{}, index_{}, tx_count_{}, rx_count_{};
  spim_result result_;
  bool ready_{}, fault_{}, hf_{}, work_{}, active_{}, stopping_{}, selected_{},
      quiet_{};
};
} // namespace mcubsp::backend::nrf52840
