#pragma once
// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0056 REQ-0062
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/backend/nrf52840/dma_address.hpp>
#include <mcubsp/backend/nrf52840/dma_buffers.hpp>
#include <mcubsp/core/transfer.hpp>
#include <type_traits>
namespace mcubsp::backend::nrf52840::detail {
inline constexpr core::pin_id disconnected{0xFFFF, 0xFFFF};
// Shared bookkeeping only. The concrete peripheral supplies its own enable,
// STOP/disable proof and event protocol; this is not a generic DMA engine.
template <class Context, class Buffers, class Address = native_dma_address,
          unsigned MaxPins = 8>
class peripheral_session {
public:
  using execution_type =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;
  Context &context;
  clock_controller<Context> &clocks;
  Buffers &buffers;
  Address address;
  peripheral_session(Context &c, clock_controller<Context> &clock, Buffers &b,
                     Address a = {}) noexcept
      : context(c), clocks(clock), buffers(b), address(a) {}
  peripheral_session(const peripheral_session &) = delete;
  peripheral_session &operator=(const peripheral_session &) = delete;
  [[nodiscard]] system_result
  acquire(peripheral p, std::uintptr_t base,
          std::span<const core::pin_id> pins = {},
          std::optional<unsigned> enable_offset = 0x500,
          bool high_frequency = true) noexcept {
    if (owner_ || context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (pins.size() > MaxPins)
      return std::unexpected(system_error::invalid);
    auto token = context.resources.claim(context.execution, p, this);
    if (!token)
      return std::unexpected(system_error::busy);
    owner_ = *token;
    base_ = base;
    if (enable_offset) {
      auto enabled = read(*enable_offset);
      if (!enabled || *enabled) {
        (void)release_owner();
        return std::unexpected(enabled ? system_error::busy
                                       : system_error::mmio);
      }
    }
    for (auto pin : pins)
      if (pin != disconnected) {
        if (!context.gpio.claim(pin, driver::gpio_direction::input)) {
          release_pins();
          (void)release_owner();
          return std::unexpected(system_error::busy);
        }
        pins_[pin_count_++] = pin;
      }
    if (high_frequency && !clocks.acquire_hf()) {
      release_pins();
      (void)release_owner();
      return std::unexpected(system_error::not_initialized);
    }
    hf_ = high_frequency;
    return {};
  }
  [[nodiscard]] system_result
  acquire_routed(peripheral p, unsigned id, endpoint_domain domain,
                 std::uintptr_t base,
                 std::optional<unsigned> inactive_offset = 0x500) noexcept {
    if (owner_)
      return std::unexpected(system_error::busy);
    auto claimed = context.claim(id, domain);
    if (!claimed)
      return claimed;
    auto token = context.coordinated_owner(id);
    if (!token || token->personality != p) {
      (void)context.release(id);
      return std::unexpected(system_error::invalid);
    }
    owner_ = *token;
    context_id_ = id;
    context.bind_dma_endpoint_state(id, &has_dma_storage_);
    base_ = base;
    if (inactive_offset) {
      auto state = read(*inactive_offset);
      if (!state || *state) {
        (void)release_owner();
        return std::unexpected(state ? system_error::busy : system_error::mmio);
      }
    }
    auto clock = require_high_frequency();
    if (!clock) {
      (void)release_owner();
      return clock;
    }
    return {};
  }
  [[nodiscard]] bool routes_referenced() noexcept {
    return context_id_ && context.referenced(*context_id_);
  }
  [[nodiscard]] system_endpoint endpoint(unsigned offset,
                                         endpoint_kind kind) const noexcept {
    return context_id_ ? context.endpoint(*context_id_, base_ + offset, kind)
                       : system_endpoint{};
  }
  [[nodiscard]] system_result require_high_frequency() noexcept {
    if (!owner_)
      return std::unexpected(system_error::not_initialized);
    if (!hf_) {
      auto r = clocks.acquire_hf();
      if (!r)
        return std::unexpected(system_error::faulted);
      hf_ = true;
    }
    publish();
    return {};
  }
  [[nodiscard]] system_result release_high_frequency() noexcept {
    if (hf_) {
      auto r = clocks.release_hf();
      if (!r)
        return std::unexpected(system_error::faulted);
      hf_ = false;
    }
    return {};
  }
  void publish() noexcept {
    if (!work_) {
      context.work_started();
      work_ = true;
    }
    if (context_id_)
      context.publish(*context_id_);
  }
  [[nodiscard]] bool analog_pin(core::pin_id pin) noexcept {
    if (!context.gpio.configure(pin, {}))
      return false;
    return context.write(mcu::nrf52840::pin_configuration(pin), 2).has_value();
  }
  // Only autonomous low-power peripherals may request this after READY.
  [[nodiscard]] system_result allow_low_power_idle() noexcept {
    if (hf_) {
      if (!clocks.release_hf())
        return std::unexpected(system_error::faulted);
      hf_ = false;
    }
    if (work_) {
      context.work_stopped();
      work_ = false;
    }
    return {};
  }
  [[nodiscard]] bool owned() const noexcept { return owner_.has_value(); }
  [[nodiscard]] std::optional<resource_token> owner() const noexcept {
    return owner_;
  }
  [[nodiscard]] bool write(unsigned offset, std::uint32_t value) noexcept {
    if (offset >= 0x100 && offset < 0x200 && !value)
      return context.clear_event(base_ + offset).has_value();
    auto r = context.write(base_ + offset, value);
    arch::data_memory_barrier();
    return r.has_value();
  }
  [[nodiscard]] auto read(unsigned offset) noexcept {
    return context.read(base_ + offset);
  }
  [[nodiscard]] bool clear(std::span<const unsigned> events) noexcept {
    for (auto off : events)
      if (!write(off, 0))
        return false;
    return true;
  }
  [[nodiscard]] bool disabled(unsigned offset = 0x500) noexcept {
    if (!write(offset, 0))
      return false;
    auto r = read(offset);
    return r && !*r;
  }
  [[nodiscard]] std::expected<dma_buffer_token, coordination_error>
  prepare(const void *data, std::size_t capacity, std::size_t count,
          mcu::nrf52840::dma_constraints constraints,
          dma_access access) noexcept {
    if (!owner_)
      return std::unexpected(coordination_error::invalid);
    auto result = buffers.prepare(*owner_, constraints, address(data, capacity),
                                  capacity, count, access);
    if (result) {
      ++dma_leases_;
      has_dma_storage_ = true;
    }
    return result;
  }
  void discard(std::optional<dma_buffer_token> &token) noexcept {
    if (token && buffers.discard_prepared(*token)) {
      token.reset();
      has_dma_storage_ = --dma_leases_ != 0;
    }
  }
  // Call only after the concrete driver's terminal event/disable proof.
  [[nodiscard]] bool
  release_quiet(std::optional<dma_buffer_token> &token) noexcept {
    if (!token)
      return true;
    if (routes_referenced())
      return false;
    if (!buffers.recover(
            *token,
            []() -> std::expected<bool, coordination_error> { return true; }))
      return false;
    token.reset();
    has_dma_storage_ = --dma_leases_ != 0;
    return true;
  }
  [[nodiscard]] system_result release() noexcept {
    if (!owner_)
      return {};
    if (context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (routes_referenced())
      return std::unexpected(system_error::busy);
    for (unsigned i = 0; i < pin_count_; ++i)
      if (!context.gpio.configure(pins_[i], {}))
        return std::unexpected(system_error::mmio);
    if (hf_ && !clocks.release_hf())
      return std::unexpected(system_error::faulted);
    hf_ = false;
    release_pins();
    if (!release_owner())
      return std::unexpected(system_error::busy);
    if (work_) {
      context.work_stopped();
      work_ = false;
    }
    return {};
  }
  template <class Step>
  [[nodiscard]] bool bounded(Step &&step, execution::tick interval,
                             unsigned budget = 1000000) noexcept {
    execution::deadline until{
        static_cast<execution::tick>(context.execution.now() + interval)};
    if (context.execution.in_isr())
      budget = 1;
    for (unsigned i = 0; i < budget; ++i) {
      if (context.execution.critical(step))
        return true;
      if (execution::reached(context.execution.now(), until))
        break;
    }
    return false;
  }
  static constexpr std::uint32_t select(core::pin_id pin) noexcept {
    return pin == disconnected ? 0x80000000U
                               : (std::uint32_t(pin.port) << 5) | pin.index;
  }

private:
  bool release_owner() noexcept {
    if (!owner_)
      return true;
    if (context_id_) {
      if (!context.release(*context_id_))
        return false;
      context_id_.reset();
    } else if (!context.resources.release(context.execution, *owner_))
      return false;
    owner_.reset();
    return true;
  }
  void release_pins() noexcept {
    for (unsigned i = 0; i < pin_count_; ++i)
      context.gpio.release(pins_[i]);
    pin_count_ = 0;
  }
  unsigned dma_leases_{};
  bool has_dma_storage_{};
  std::uintptr_t base_{};
  std::optional<resource_token> owner_;
  std::optional<unsigned> context_id_;
  std::array<core::pin_id, MaxPins> pins_{};
  unsigned pin_count_{};
  bool hf_{}, work_{};
};
} // namespace mcubsp::backend::nrf52840::detail
