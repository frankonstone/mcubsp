#pragma once
// @req-file REQ-0056 REQ-0064 REQ-0062
// Nordic PS v1.11 6.5/6.12. COMP/LPCOMP share one physical identity.
#include <mcubsp/backend/nrf52840/saadc.hpp>
#include <mcubsp/mcu/nrf52840/product/comp.hpp>
#include <mcubsp/mcu/nrf52840/product/lpcomp.hpp>
namespace mcubsp::backend::nrf52840 {
enum class comp_reference : std::uint8_t {
  internal_1v2,
  internal_1v8,
  internal_2v4,
  vdd = 4,
  external = 5
};
enum class comp_speed : std::uint8_t { low, normal, high };
enum class comparator_wake : std::uint8_t { cross, up, down };
struct comp_configuration {
  unsigned positive_input{}, external_input{}, threshold_down{},
      threshold_up{63};
  comp_reference reference{comp_reference::internal_1v2};
  comp_speed speed{comp_speed::normal};
  bool differential{}, hysteresis{true};
  std::uint32_t shortcuts{};
};
struct lpcomp_configuration {
  unsigned positive_input{}, reference{3}, external_input{};
  comparator_wake wake{comparator_wake::cross};
  bool hysteresis{true};
  std::uint32_t shortcuts{};
};
struct comparator_events {
  bool ready{}, down{}, up{}, cross{};
};
namespace detail {
struct no_dma_buffers {};
} // namespace detail
template <class Context, bool LowPower = false> class comparator_controller {
  using Session = detail::peripheral_session<Context, detail::no_dma_buffers>;

public:
  using configuration =
      std::conditional_t<LowPower, lpcomp_configuration, comp_configuration>;
  comparator_controller(Context &c, clock_controller<Context> &clocks) noexcept
      : s_(c, clocks, unused_) {}
  comparator_controller(const comparator_controller &) = delete;
  comparator_controller &operator=(const comparator_controller &) = delete;
  [[nodiscard]] system_result initialize(configuration config = {}) noexcept {
    if (s_.owned() || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (fault_)
      return std::unexpected(system_error::faulted);
    if (config.positive_input > 7)
      return std::unexpected(system_error::invalid);
    std::array<core::pin_id, 2> pins{*analog_input_pin(config.positive_input),
                                     detail::disconnected};
    if constexpr (LowPower) {
      if (config.reference > 15 || config.external_input > 1 ||
          static_cast<unsigned>(config.wake) > 2 || config.shortcuts & ~3U)
        return std::unexpected(system_error::invalid);
      if (config.reference == 7)
        pins[1] = *analog_input_pin(config.external_input);
    } else {
      auto ref = static_cast<unsigned>(config.reference);
      if ((ref > 2 && ref != 4 && ref != 5) || config.external_input > 7 ||
          config.threshold_up > 63 ||
          config.threshold_down > config.threshold_up ||
          static_cast<unsigned>(config.speed) > 2 || config.shortcuts & ~31U)
        return std::unexpected(system_error::invalid);
      if (config.differential || config.reference == comp_reference::external)
        pins[1] = *analog_input_pin(config.external_input);
    }
    if (pins[0] == pins[1])
      pins[1] = detail::disconnected;
    auto acquired = s_.acquire(LowPower ? peripheral::lpcomp : peripheral::comp,
                               base, pins, 0x500, !LowPower);
    if (!acquired)
      return acquired;
    for (auto pin : pins)
      if (pin != detail::disconnected && !s_.analog_pin(pin))
        return init_failed();
    config_ = config;
    if (!s_.write(0x308, 15) || !clear_events() ||
        !s_.write(0x504, config.positive_input) ||
        !s_.write(0x508, static_cast<unsigned>(config.reference)) ||
        !s_.write(0x50C, config.external_input) ||
        !s_.write(0x538, config.hysteresis ? 1 : 0) ||
        !s_.write(0x200, config.shortcuts))
      return init_failed();
    if constexpr (LowPower) {
      if (!s_.write(0x520, static_cast<unsigned>(config.wake)))
        return init_failed();
    } else if (!s_.write(0x530,
                         config.threshold_down | (config.threshold_up << 8)) ||
               !s_.write(0x534, static_cast<unsigned>(config.speed) |
                                    (config.differential ? 256U : 0U)))
      return init_failed();
    if (!s_.write(0x500, LowPower ? 1 : 2))
      return init_failed();
    ready_ = true;
    s_.publish();
    return {};
  }
  [[nodiscard]] system_result start() noexcept {
    if (!ready_ || fault_ || s_.context.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (running_)
      return std::unexpected(system_error::busy);
    if (!clear_events())
      return fail();
    seen_ready_ = wake_ready_ = false;
    running_ = true;
    s_.publish();
    if (!s_.write(0, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_value<comparator_events> poll() noexcept {
    if (!ready_)
      return std::unexpected(system_error::not_initialized);
    return s_.context.execution.critical(
        [&]() -> system_value<comparator_events> {
          std::array<bool, 4> bits{};
          for (unsigned i = 0; i < 4; ++i) {
            auto event = s_.read(0x100 + 4 * i);
            if (!event)
              return std::unexpected(system_error::mmio);
            if (*event) {
              bits[i] = true;
              if (!s_.write(0x100 + 4 * i, 0))
                return std::unexpected(system_error::mmio);
            }
          }
          if (bits[0])
            seen_ready_ = true;
          if ((bits[0] && (config_.shortcuts & 2U)) ||
              (!LowPower && ((bits[1] && (config_.shortcuts & 4U)) ||
                             (bits[2] && (config_.shortcuts & 8U)) ||
                             (bits[3] && (config_.shortcuts & 16U)))))
            running_ = false;
          return comparator_events{bits[0], bits[1], bits[2], bits[3]};
        });
  }
  [[nodiscard]] system_value<bool> sample() noexcept {
    if (!ready_ || !seen_ready_ || !running_ || fault_)
      return std::unexpected(system_error::not_initialized);
    if (!s_.write(8, 1))
      return std::unexpected(system_error::mmio);
    auto result = s_.read(0x400);
    if (!result)
      return std::unexpected(result.error());
    return bool(*result & 1U);
  }
  [[nodiscard]] system_result stop() noexcept {
    if (!s_.owned() || s_.context.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (!s_.write(4, 1))
      return fail();
    running_ = seen_ready_ = wake_ready_ = false;
    return {};
  }
  [[nodiscard]] system_result interrupt_delivery(std::uint32_t mask) noexcept {
    if (!ready_ || mask & ~15U || s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.write(0x308, 15) || !s_.write(0x304, mask))
      return fail();
    return {};
  }
  // Feed wake_ready() to the explicit power-controller System OFF request.
  // READY must already have occurred; no pending startup may cross System OFF.
  [[nodiscard]] system_result prepare_system_off() noexcept
    requires(LowPower)
  {
    if (!ready_ || !running_ || !seen_ready_ || fault_ ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!s_.write(0x308, 15))
      return fail();
    auto idle = s_.allow_low_power_idle();
    if (!idle)
      return idle;
    wake_ready_ = true;
    return {};
  }
  [[nodiscard]] bool wake_ready() const noexcept
    requires(LowPower)
  {
    return wake_ready_;
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    // No STOPPED event exists. STOP, then disable/readback, is the documented
    // lifecycle; never infer stop by reading the task register.
    if (!s_.write(0x308, 15) || !s_.write(0x200, 0) || !s_.write(4, 1) ||
        !s_.disabled())
      return fail();
    if constexpr (!LowPower)
      if (!s_.write(0xFFC, 0) || !s_.read(0xFFC) || !s_.write(0xFFC, 1))
        return fail(); // anomaly 275
    if (!clear_events())
      return fail();
    auto r = s_.release();
    if (!r)
      return r;
    ready_ = running_ = seen_ready_ = wake_ready_ = false;
    return {};
  }

private:
  bool clear_events() noexcept {
    for (unsigned off : {0x100U, 0x104U, 0x108U, 0x10CU})
      if (!s_.write(off, 0))
        return false;
    return true;
  }
  system_result fail() noexcept {
    fault_ = true;
    (void)s_.write(0x308, 15);
    return std::unexpected(system_error::mmio);
  }
  system_result init_failed() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x40013000;
  detail::no_dma_buffers unused_;
  Session s_;
  configuration config_{};
  bool ready_{}, running_{}, seen_ready_{}, fault_{}, wake_ready_{};
};
template <class Context>
using comp_controller = comparator_controller<Context, false>;
template <class Context>
using lpcomp_controller = comparator_controller<Context, true>;
} // namespace mcubsp::backend::nrf52840
