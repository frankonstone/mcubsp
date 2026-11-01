#pragma once
// @req-file REQ-0028 REQ-0046 REQ-0060 REQ-0062
// PS v1.11 6.24: CPU-driven SPI0--2, one byte in flight, READY is wire
// completion.
#include <mcubsp/backend/nrf52840/spim.hpp>
#include <mcubsp/mcu/nrf52840/product/spi0.hpp>
namespace mcubsp::backend::nrf52840 {
template <unsigned I, core::pin_id Sck, core::pin_id Mosi, core::pin_id Miso,
          core::pin_id Cs = spi_disconnected>
struct spi_binding : spim_binding<I, Sck, Mosi, Miso, Cs> {
  static constexpr bool easy_dma = false;
  static_assert(I < 3, "legacy SPI instance must be 0 through 2");
  static constexpr auto peripheral_id =
      std::array{peripheral::spi0, peripheral::spi1, peripheral::spi2}[I];
};
using spi_timing = spim_timing;
[[nodiscard]] constexpr std::expected<spi_timing, driver::spi_error>
solve_spi_timing(unsigned instance,
                 driver::spi_timing_request request) noexcept {
  if (instance >= 3)
    return std::unexpected(driver::spi_error::invalid_rate);
  return solve_spim_timing(instance, request);
}
template <class Binding, class Context, std::size_t SegmentCapacity = 8,
          unsigned StopPollBudget = 1000000>
class spi_controller {
  static_assert(Binding::instance < 3 && SegmentCapacity > 0 &&
                StopPollBudget > 0);
  using error = driver::spi_error;
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using pool_type =
      core::operation_pool<driver::spi_transfer_result, error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  spi_controller(Context &c, clock_controller<Context> &clocks,
                 execution::tick stop_ticks = 12'800'000) noexcept
      : c_(c), clocks_(clocks), operations_(c.execution),
        stop_ticks_(stop_ticks) {}
  spi_controller(const spi_controller &) = delete;
  spi_controller &operator=(const spi_controller &) = delete;
  [[nodiscard]] std::expected<void, error>
  initialize(spi_timing timing,
             driver::spi_configuration config = {}) noexcept {
    if (owner_ || c_.execution.in_isr())
      return std::unexpected(error::already_owned);
    if (fault_)
      return std::unexpected(error::transport);
    auto solved =
        solve_spi_timing(Binding::instance,
                         {timing.clock_hz, timing.target_hz, timing.error_ppm});
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
        bool high = i == 0   ? static_cast<unsigned>(config.mode) >= 2
                    : i == 3 ? config.chip_select ==
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
    auto mode = static_cast<unsigned>(config.mode);
    if (!write(0x308, 4) || !write(0x108, 0) ||
        !write(0x508, select(Binding::sck)) ||
        !write(0x50C, select(Binding::mosi)) ||
        !write(0x510, select(Binding::miso)) ||
        !write(0x524, timing.encoding) ||
        !write(0x554,
               ((mode & 1U) << 1) | ((mode & 2U) << 1) |
                   (config.bit_order ==
                            driver::spi_bit_order::least_significant_first
                        ? 1U
                        : 0U)) ||
        !write(0x500, 1))
      return init_failed();
    ready_ = true;
    work_ = true;
    c_.work_started();
    return {};
  }
  using transfer_type = typename pool_type::transfer_type;
  [[nodiscard]] bool busy() const noexcept {
    return id_.has_value() || in_flight_ || selected_;
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
  reconfigure(spi_timing timing,
              driver::spi_configuration config = {}) noexcept {
    if (!ready_)
      return std::unexpected(error::not_initialized);
    if (busy())
      return std::unexpected(error::busy);
    if (fault_)
      return std::unexpected(error::transport);
    auto solved =
        solve_spi_timing(Binding::instance,
                         {timing.clock_hz, timing.target_hz, timing.error_ppm});
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
    return initialize(timing, config);
  }
  [[nodiscard]] start_result
  transaction(std::span<const driver::spi_segment> segments) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(error::not_initialized);
    if (segments.empty())
      return failure(error::empty_transfer);
    if (segments.size() > SegmentCapacity)
      return failure(error::excessive_length);
    std::size_t total{};
    for (auto s : segments) {
      auto checked = s.validate(config_.frame_width);
      if (!checked)
        return failure(checked.error());
      if ((s.transmit_size() && Binding::mosi == spi_disconnected) ||
          (s.receive_size() && Binding::miso == spi_disconnected))
        return failure(error::invalid_frame);
      if (*checked > std::numeric_limits<std::size_t>::max() - total)
        return failure(error::excessive_length);
      total += *checked;
    }
    return c_.execution.critical([&]() -> start_result {
      if (id_ || in_flight_)
        return failure(error::busy);
      auto op = operations_.try_start({.quiesce = &cancel, .context = this});
      if (!op)
        return std::unexpected(op.error());
      id_ = op->id();
      count_ = segments.size();
      index_ = offset_ = frames_ = 0;
      for (std::size_t i = 0; i < count_; ++i)
        segments_[i] = segments[i];
      selected_ = true;
      if (!chip_select(true) || !send())
        fail_io();
      return op;
    });
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_ || fault_)
      return false;
    return c_.execution.critical([&] {
      auto event = c_.read(base + 0x108);
      if (!event) {
        fail_io();
        return true;
      }
      if (!*event)
        return false;
      byte_quiet_ = true;
      if (!write(0x108, 0)) {
        fail_io();
        return true;
      }
      auto byte = c_.read(base + 0x518);
      if (!byte) {
        fail_io();
        return true;
      }
      in_flight_ = false;
      auto &segment = segments_[index_];
      segment.store(offset_, static_cast<std::uint8_t>(*byte));
      ++frames_;
      ++offset_;
      if (offset_ == segment.size()) {
        offset_ = 0;
        ++index_;
      }
      if (index_ < count_) {
        if (!send())
          fail_io();
        return true;
      }
      if (!chip_select(false)) {
        fail_io();
        return true;
      }
      selected_ = false;
      auto id = *id_;
      id_.reset();
      (void)operations_.complete(id, driver::spi_transfer_result{frames_});
      return true;
    });
  }
  [[nodiscard]] std::expected<void, error>
  interrupt_delivery(bool on) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(error::not_initialized);
    if (!write(on ? 0x304 : 0x308, 4))
      return std::unexpected(error::mmio);
    return {};
  }
  [[nodiscard]] std::expected<void, error> shutdown() noexcept {
    if (c_.execution.in_isr() || id_)
      return std::unexpected(error::busy);
    if (!owner_)
      return {};
    if (!quiesce())
      return std::unexpected(error::transport);
    if (!write(0x308, 4) || !write(0x500, 0))
      return std::unexpected(error::mmio);
    auto disabled = c_.read(base + 0x500);
    if (!disabled || *disabled)
      return std::unexpected(error::mmio);
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
  [[nodiscard]] std::size_t transferred_count() const noexcept {
    return c_.execution.critical([&] { return frames_; });
  }

  [[nodiscard]] bool owns_resource() const noexcept {
    return owner_.has_value();
  }
  [[nodiscard]] bool operation_pending() const noexcept {
    return id_.has_value();
  }

private:
  start_result failure(error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  bool write(unsigned off, std::uint32_t v) noexcept {
    if (off == 0x108 && !v)
      return c_.clear_event(base + off).has_value();
    auto r = c_.write(base + off, v);
    arch::data_memory_barrier();
    return r.has_value();
  }
  static constexpr std::uint32_t select(core::pin_id p) noexcept {
    return p == spi_disconnected ? 0x80000000U
                                 : (std::uint32_t(p.port) << 5) | p.index;
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
  bool send() noexcept {
    if (!write(0x108, 0))
      return false;
    byte_quiet_ = false;
    in_flight_ = true;
    return write(0x51C, segments_[index_].frame(offset_));
  }
  bool quiesce() noexcept {
    execution::deadline until{
        static_cast<execution::tick>(c_.execution.now() + stop_ticks_)};
    unsigned budget = c_.execution.in_isr() ? 1U : StopPollBudget;
    for (unsigned i = 0; in_flight_ && i < budget; ++i) {
      if (!byte_quiet_) {
        auto event = c_.read(base + 0x108);
        if (!event)
          return false;
        byte_quiet_ = *event != 0;
      }
      if (byte_quiet_) {
        if (!write(0x108, 0) || !c_.read(base + 0x518))
          return false;
        in_flight_ = false;
        break;
      }
      if (execution::reached(c_.execution.now(), until))
        break;
    }
    if (in_flight_)
      return false;
    if (selected_) {
      if (!chip_select(false))
        return false;
      selected_ = false;
    }
    return true;
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &s = *static_cast<spi_controller *>(p);
    if (!s.id_ || *s.id_ != id)
      return core::quiesce_result::reusable;
    bool quiet = s.quiesce();
    s.id_.reset();
    s.fault_ |= !quiet;
    if (!quiet)
      (void)s.write(0x308, 4);
    return quiet ? core::quiesce_result::reusable
                 : core::quiesce_result::faulted;
  }
  void fail_io() noexcept {
    fault_ = true;
    (void)write(0x308, 4);
    if (id_) {
      auto id = *id_;
      id_.reset();
      (void)operations_.fail(id, error::mmio);
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
  bool (*select_handler_)(void *, bool) noexcept {};
  void *select_context_{};
  Context &c_;
  clock_controller<Context> &clocks_;
  pool_type operations_;
  execution::tick stop_ticks_;
  std::optional<resource_token> owner_;
  std::optional<core::operation_id> id_;
  std::array<driver::spi_segment, SegmentCapacity> segments_{};
  std::array<bool, 4> pins_{};
  driver::spi_configuration config_{};
  std::size_t count_{}, index_{}, offset_{}, frames_{};
  bool ready_{}, fault_{}, hf_{}, work_{}, in_flight_{}, selected_{},
      byte_quiet_{};
};
} // namespace mcubsp::backend::nrf52840
