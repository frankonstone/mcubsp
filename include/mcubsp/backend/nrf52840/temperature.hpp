#pragma once
// @req-file REQ-0064
// PS 4413_417 v1.11 6.28; erratum 66 factory linearity coefficients.
#include <bit>
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/core/transfer.hpp>
#include <type_traits>
namespace mcubsp::backend::nrf52840 {
struct die_temperature {
  std::int32_t quarter_celsius;
  [[nodiscard]] constexpr std::int64_t milli_celsius() const noexcept {
    return std::int64_t(quarter_celsius) * 250;
  }
};
template <class Context> class temperature_controller {
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using pool_type = core::operation_pool<die_temperature, system_error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  temperature_controller(Context &c, clock_controller<Context> &clocks) noexcept
      : c_(c), clocks_(clocks), operations_(c.execution) {}
  temperature_controller(const temperature_controller &) = delete;
  temperature_controller &operator=(const temperature_controller &) = delete;
  [[nodiscard]] system_result initialize() noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(system_error::busy);
    if (fault_)
      return std::unexpected(system_error::faulted);
    auto claim = c_.resources.claim(c_.execution, peripheral::temp, this);
    if (!claim)
      return std::unexpected(system_error::busy);
    owner_ = *claim;
    if (!clocks_.acquire_hf()) {
      (void)release_owner();
      return std::unexpected(system_error::not_initialized);
    }
    hf_ = true;
    std::array<std::uint32_t, 17> calibration{};
    for (unsigned i = 0; i < 17; ++i) {
      auto value = c_.read(0x10000404U + i * 4);
      if (!value || *value == 0xFFFFFFFFU) {
        fault_ = true;
        (void)shutdown();
        return std::unexpected(value ? system_error::invalid
                                     : system_error::mmio);
      }
      calibration[i] = *value;
    }
    if (!write(0x004, 1) || !write(0x308, 1) || !write(0x100, 0))
      return fail_initialization();
    for (unsigned i = 0; i < 17; ++i) {
      const auto offset = i < 6    ? 0x520U + i * 4
                          : i < 12 ? 0x540U + (i - 6) * 4
                                   : 0x560U + (i - 12) * 4;
      const auto mask = i < 6 ? 0xFFFU : i < 12 ? 0x3FFFU : 0xFFU;
      if (!write(offset, calibration[i] & mask))
        return fail_initialization();
    }
    ready_ = true;
    work_ = true;
    c_.work_started();
    return {};
  }
  [[nodiscard]] start_result sample() noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(system_error::not_initialized);
    return c_.execution.critical([&]() -> start_result {
      auto operation =
          operations_.try_start({.quiesce = &cancel, .context = this});
      if (!operation)
        return std::unexpected(operation.error());
      id_ = operation->id();
      if (!write(0x100, 0) || !write(0x000, 1))
        fail_operation();
      return operation;
    });
  }
  [[nodiscard]] bool poll() noexcept {
    if (!id_)
      return false;
    return c_.execution.critical([&] {
      auto event = c_.read(base + 0x100);
      if (!event) {
        fail_operation();
        return true;
      }
      if (!*event)
        return false;
      auto value = c_.read(base + 0x508);
      if (!value || !write(0x004, 1) || !write(0x100, 0)) {
        fail_operation();
        return true;
      }
      auto id = *id_;
      id_.reset();
      (void)operations_.complete(
          id, die_temperature{std::bit_cast<std::int32_t>(*value)});
      return true;
    });
  }
  [[nodiscard]] system_result interrupt_delivery(bool enable) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (!write(enable ? 0x304 : 0x308, 1))
      return std::unexpected(system_error::mmio);
    return {};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (c_.execution.in_isr() || id_)
      return std::unexpected(system_error::busy);
    if (!owner_)
      return {};
    if (!write(0x308, 1) || !write(0x004, 1) || !write(0x100, 0))
      return std::unexpected(system_error::mmio);
    if (hf_ && !clocks_.release_hf())
      return std::unexpected(system_error::faulted);
    hf_ = false;
    if (!release_owner())
      return std::unexpected(system_error::busy);
    if (work_) {
      c_.work_stopped();
      work_ = false;
    }
    ready_ = false;
    return {};
  }

private:
  start_result failure(system_error e) noexcept {
    return std::unexpected(
        typename pool_type::error_type{core::transfer_error_kind::failure, e});
  }
  bool write(unsigned off, std::uint32_t v) noexcept {
    if (off == 0x100 && !v)
      return c_.clear_event(base + off).has_value();
    auto r = c_.write(base + off, v);
    arch::data_memory_barrier();
    return r.has_value();
  }
  bool release_owner() noexcept {
    if (!owner_)
      return true;
    if (!c_.resources.release(c_.execution, *owner_))
      return false;
    owner_.reset();
    return true;
  }
  system_result fail_initialization() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(system_error::mmio);
  }
  void fail_operation() noexcept {
    fault_ = true;
    (void)write(0x004, 1);
    auto id = *id_;
    id_.reset();
    (void)operations_.fail(id, system_error::mmio);
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &s = *static_cast<temperature_controller *>(p);
    if (!s.id_ || *s.id_ != id)
      return core::quiesce_result::reusable;
    const bool stopped = s.write(0x004, 1) && s.write(0x100, 0);
    s.id_.reset();
    s.fault_ |= !stopped;
    return stopped ? core::quiesce_result::reusable
                   : core::quiesce_result::faulted;
  }
  static constexpr std::uintptr_t base = 0x4000C000;
  Context &c_;
  clock_controller<Context> &clocks_;
  pool_type operations_;
  std::optional<resource_token> owner_;
  std::optional<core::operation_id> id_;
  bool ready_{}, fault_{}, hf_{}, work_{};
};
} // namespace mcubsp::backend::nrf52840
