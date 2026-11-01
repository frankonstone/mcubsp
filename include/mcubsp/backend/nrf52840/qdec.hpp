#pragma once
// @req-file REQ-0065 REQ-0057
// PS 4413_417 v1.11 6.18. Atomic task snapshots retain signed movement.
#include <bit>
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/execution/deadline.hpp>
namespace mcubsp::backend::nrf52840 {
inline constexpr core::pin_id qdec_disconnected{0xFFFF, 0xFFFF};
template <core::pin_id A, core::pin_id B, core::pin_id Led = qdec_disconnected>
struct qdec_binding {
  static_assert(mcu::nrf52840::gpio_supported(A) &&
                    mcu::nrf52840::gpio_supported(B) &&
                    (Led == qdec_disconnected ||
                     mcu::nrf52840::gpio_supported(Led)),
                "QDEC pins must be available GPIOs");
  static_assert(A != B && (Led == qdec_disconnected || (Led != A && Led != B)),
                "QDEC pins must be distinct");
  static constexpr std::array pins{A, B, Led};
};
struct qdec_configuration {
  unsigned sample_period{0}; // 0..10 => 128 us * 2^n
  unsigned report_period{0}; // 0..7 => 10,40,80,...,280; 8 => 1 sample
  unsigned led_pre_microseconds{16};
  bool debounce{true}, led_active_high{true};
};
struct qdec_snapshot {
  std::int32_t movement;
  std::uint32_t double_transitions;
  bool overflow, report_ready, double_ready;
};
enum class qdec_event : unsigned {
  sample,
  report,
  overflow,
  double_transition,
  stopped
};
enum class qdec_task : unsigned {
  start,
  stop,
  read_clear_both,
  read_clear_movement,
  read_clear_double
};
template <class Binding, class Context, unsigned StopPollBudget = 1000000>
class qdec_controller {
  static_assert(StopPollBudget > 0);

public:
  qdec_controller(Context &c, clock_controller<Context> &clocks,
                  execution::tick stop_ticks = 12'800'000) noexcept
      : c_(c), clocks_(clocks), stop_ticks_(stop_ticks) {}
  qdec_controller(const qdec_controller &) = delete;
  qdec_controller &operator=(const qdec_controller &) = delete;
  [[nodiscard]] system_result
  initialize(qdec_configuration config = {}) noexcept {
    if (owned_ || c_.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (fault_)
      return std::unexpected(system_error::faulted);
    if (config.sample_period > 10 || config.report_period > 8 ||
        config.led_pre_microseconds > 511 || !stop_ticks_ ||
        stop_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto claim = c_.claim(resource);
    if (!claim)
      return claim;
    owned_ = true;
    auto enabled = c_.read(base + 0x500);
    if (!enabled || *enabled) {
      (void)c_.release(resource);
      owned_ = false;
      return std::unexpected(enabled ? system_error::busy : system_error::mmio);
    }
    for (unsigned i = 0; i < 3; ++i)
      if (Binding::pins[i] != qdec_disconnected) {
        if (!c_.gpio.claim(Binding::pins[i],
                           i == 2 ? driver::gpio_direction::output
                                  : driver::gpio_direction::input)) {
          release_pins();
          (void)c_.release(resource);
          owned_ = false;
          return std::unexpected(system_error::busy);
        }
        pins_[i] = true;
      }
    if (!clocks_.acquire_hf()) {
      release_pins();
      (void)c_.release(resource);
      owned_ = false;
      return std::unexpected(system_error::not_initialized);
    }
    hf_ = true;
    for (unsigned i = 0; i < 3; ++i)
      if (pins_[i] && !c_.gpio.configure(
                          Binding::pins[i],
                          {.direction = i == 2 ? driver::gpio_direction::output
                                               : driver::gpio_direction::input,
                           .initial_level = config.led_active_high
                                                ? driver::gpio_level::low
                                                : driver::gpio_level::high}))
        return fail_init();
    if (!write(0x308, 31) || !write(0x200, 0) || !clear_events() ||
        !write(0x504, config.led_active_high ? 1U : 0U) ||
        !write(0x508, config.sample_period) ||
        !write(0x510, config.report_period) ||
        !write(0x51C, select(Binding::pins[2])) ||
        !write(0x520, select(Binding::pins[0])) ||
        !write(0x524, select(Binding::pins[1])) ||
        !write(0x528, config.debounce ? 1U : 0U) ||
        !write(0x540, config.led_pre_microseconds) || !write(0x500, 1) ||
        !write(0x008, 1))
      return fail_init();
    ready_ = true;
    c_.publish(resource);
    c_.work_started();
    work_ = true;
    return {};
  }
  [[nodiscard]] system_result start() noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (running_ || c_.referenced(resource))
      return std::unexpected(system_error::busy);
    if (!write(0x110, 0))
      return fail();
    running_ = true;
    if (!write(0x000, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_result stop() noexcept {
    if (!owned_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (c_.referenced(resource))
      return std::unexpected(system_error::busy);
    // A PPI START task may have run even when the CPU never called start().
    // STOP always generates STOPPED; retain the previous request's ack.
    if (!stopping_) {
      if (!write(0x110, 0))
        return fail();
      stopping_ = true;
    }
    auto done = c_.read(base + 0x110);
    if (!done)
      return fail();
    if (!*done && !write(0x004, 1))
      return fail();
    execution::deadline limit{
        static_cast<execution::tick>(c_.execution.now() + stop_ticks_)};
    for (unsigned i = 0; i < StopPollBudget; ++i) {
      done = c_.read(base + 0x110);
      if (!done)
        return fail();
      if (*done) {
        running_ = stopping_ = false;
        return {};
      }
      if (execution::reached(c_.execution.now(), limit))
        break;
    }
    fault_ = true;
    return std::unexpected(system_error::timeout);
  }
  [[nodiscard]] system_value<qdec_snapshot> read_clear() noexcept {
    if (!ready_ || fault_)
      return std::unexpected(system_error::not_initialized);
    return c_.execution.critical([&]() -> system_value<qdec_snapshot> {
      if (c_.referenced(resource))
        return std::unexpected(system_error::busy);
      auto report = c_.read(base + 0x104), overflow = c_.read(base + 0x108),
           doubled = c_.read(base + 0x10C);
      if (!report || !overflow || !doubled) {
        fault_ = true;
        return std::unexpected(system_error::mmio);
      }
      // Clear flags before the atomic snapshot task. New flags remain
      // pending rather than being erased after the data was captured.
      if ((*report && !write(0x104, 0)) || (*overflow && !write(0x108, 0)) ||
          (*doubled && !write(0x10C, 0)) || !write(0x008, 1)) {
        fault_ = true;
        return std::unexpected(system_error::mmio);
      }
      return snapshot_values(*overflow != 0, *report != 0, *doubled != 0);
    });
  }
  // For a caller-controlled PPI snapshot protocol; CPU never issues a new
  // snapshot task here. The application coordinates snapshot/register timing.
  [[nodiscard]] system_value<qdec_snapshot> snapshot() noexcept {
    if (!ready_ || fault_)
      return std::unexpected(system_error::not_initialized);
    return c_.execution.critical(
        [&] { return snapshot_values(false, false, false); });
  }
  [[nodiscard]] system_value<std::int32_t> last_sample() noexcept {
    if (!ready_ || fault_)
      return std::unexpected(system_error::not_initialized);
    auto sample = c_.read(base + 0x50C);
    if (!sample)
      return std::unexpected(system_error::mmio);
    return std::bit_cast<std::int32_t>(*sample);
  }
  [[nodiscard]] system_value<bool> consume(qdec_event event) noexcept {
    auto index = static_cast<unsigned>(event);
    if (!ready_ || index >= 5)
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_value<bool> {
      auto address = base + 0x100 + 4 * index;
      auto value = c_.read(address);
      if (!value)
        return std::unexpected(system_error::mmio);
      if (*value && !c_.clear_event(address))
        return std::unexpected(system_error::mmio);
      return *value != 0;
    });
  }
  [[nodiscard]] system_result interrupt_delivery(std::uint32_t mask,
                                                 bool enable) noexcept {
    if (!ready_ || c_.execution.in_isr() || mask & ~31U)
      return std::unexpected(system_error::invalid);
    if (!write(enable ? 0x304 : 0x308, mask))
      return fail();
    return {};
  }
  [[nodiscard]] system_endpoint event(qdec_event e) const noexcept {
    return c_.endpoint(resource, base + 0x100 + 4 * static_cast<unsigned>(e),
                       endpoint_kind::event);
  }
  [[nodiscard]] system_endpoint task(qdec_task t) const noexcept {
    return c_.endpoint(resource, base + 4 * static_cast<unsigned>(t),
                       endpoint_kind::task);
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!owned_)
      return {};
    if (c_.referenced(resource))
      return std::unexpected(system_error::busy);
    if (ready_ || running_ || stopping_) {
      auto quiet = stop();
      if (!quiet)
        return quiet;
    }
    if (!write(0x308, 31) || !write(0x200, 0) || !write(0x500, 0))
      return fail();
    auto disabled = c_.read(base + 0x500);
    if (!disabled || *disabled)
      return fail();
    for (unsigned off : {0x51CU, 0x520U, 0x524U})
      if (!write(off, 0x80000000U))
        return fail();
    for (unsigned i = 0; i < 3; ++i)
      if (pins_[i] && !c_.gpio.configure(Binding::pins[i], {}))
        return fail();
    if (hf_ && !clocks_.release_hf())
      return std::unexpected(system_error::faulted);
    hf_ = false;
    release_pins();
    auto released = c_.release(resource);
    if (!released)
      return released;
    owned_ = ready_ = false;
    if (work_) {
      c_.work_stopped();
      work_ = false;
    }
    return {};
  }

private:
  bool write(unsigned off, std::uint32_t v) noexcept {
    if (off >= 0x100 && off < 0x200 && !v)
      return c_.clear_event(base + off).has_value();
    auto r = c_.write(base + off, v);
    arch::data_memory_barrier();
    return r.has_value();
  }
  bool clear_events() noexcept {
    for (unsigned off = 0x100; off <= 0x110; off += 4)
      if (!write(off, 0))
        return false;
    return true;
  }
  static constexpr std::uint32_t select(core::pin_id p) noexcept {
    return p == qdec_disconnected ? 0x80000000U
                                  : (std::uint32_t(p.port) << 5) | p.index;
  }
  system_value<qdec_snapshot> snapshot_values(bool overflow, bool report,
                                              bool doubled) noexcept {
    auto acc = c_.read(base + 0x518), dbl = c_.read(base + 0x548);
    if (!acc || !dbl) {
      fault_ = true;
      return std::unexpected(system_error::mmio);
    }
    return qdec_snapshot{std::bit_cast<std::int32_t>(*acc), *dbl, overflow,
                         report, doubled};
  }
  system_result fail() noexcept {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  system_result fail_init() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(system_error::mmio);
  }
  void release_pins() noexcept {
    for (unsigned i = 0; i < 3; ++i)
      if (pins_[i]) {
        c_.gpio.release(Binding::pins[i]);
        pins_[i] = false;
      }
  }
  static constexpr unsigned resource = 78;
  static constexpr std::uintptr_t base = 0x40012000;
  Context &c_;
  clock_controller<Context> &clocks_;
  execution::tick stop_ticks_;
  std::array<bool, 3> pins_{};
  bool owned_{}, ready_{}, running_{}, stopping_{}, hf_{}, work_{}, fault_{};
};
} // namespace mcubsp::backend::nrf52840
