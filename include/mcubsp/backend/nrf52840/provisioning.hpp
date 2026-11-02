#pragma once
#include <mcubsp/execution/deadline.hpp>
// @req-file REQ-0072 REQ-0062
// Nordic PS v1.11 4.3, 4.5, 4.8. No provisioning runs during ordinary begin.
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/mcu/nrf52840/product/approtect.hpp>
#include <mcubsp/mcu/nrf52840/product/uicr.hpp>
namespace mcubsp::backend::nrf52840 {
enum class access_port_revision : std::uint8_t {
  hardware_only,
  hardware_and_software
};
template <class Context>
[[nodiscard]] system_value<access_port_revision>
access_port_build(Context &c) noexcept {
  auto part = c.read(0x10000100), variant = c.read(0x10000104);
  if (!part || !variant)
    return std::unexpected(system_error::mmio);
  if (*part != 0x52840)
    return std::unexpected(system_error::unsupported);
  auto build = (*variant >> 8) & 255U;
  if (build >= 'A' && build <= 'D')
    return access_port_revision::hardware_only;
  if (build == 'F')
    return access_port_revision::hardware_and_software;
  return std::unexpected(system_error::unsupported);
}
struct access_port_state {
  access_port_revision revision;
  std::uint8_t uicr;
  std::optional<std::uint8_t> software_disable, force_protect;
};
template <class Context> class access_port_controller {
public:
  explicit access_port_controller(Context &c) noexcept : c_(c) {}
  [[nodiscard]] system_value<access_port_state> state() noexcept {
    auto build = access_port_build(c_);
    if (!build)
      return std::unexpected(build.error());
    auto uicr = c_.read(0x10001208);
    if (!uicr)
      return std::unexpected(uicr.error());
    access_port_state result{*build, static_cast<std::uint8_t>(*uicr), {}, {}};
    if (*build == access_port_revision::hardware_and_software) {
      auto disabled = c_.read(0x40000558), force = c_.read(0x40000550);
      if (!disabled || !force)
        return std::unexpected(system_error::mmio);
      result.software_disable = *disabled & 255U;
      result.force_protect = *force & 255U;
    }
    return result;
  }
  [[nodiscard]] system_result disable_software_protection() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto facts = state();
    if (!facts)
      return std::unexpected(facts.error());
    if (facts->revision != access_port_revision::hardware_and_software)
      return std::unexpected(system_error::unsupported);
    if (facts->uicr != 0x5A || facts->force_protect == 0)
      return std::unexpected(system_error::busy);
    return modify(0x40000558, 0x5A);
  }
  [[nodiscard]] system_result
  force_protection(bool explicit_debug_lock_request) noexcept {
    if (!explicit_debug_lock_request || c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto facts = state();
    if (!facts)
      return std::unexpected(facts.error());
    if (facts->revision != access_port_revision::hardware_and_software)
      return std::unexpected(system_error::unsupported);
    if (facts->force_protect == 0)
      return std::unexpected(system_error::busy);
    return modify(0x40000550, 0);
  }

private:
  system_result modify(std::uintptr_t address, std::uint32_t value) noexcept {
    auto owner = c_.resources.claim(c_.execution, peripheral::approtect, this);
    if (!owner)
      return std::unexpected(system_error::busy);
    auto result = c_.write(address, value);
    arch::data_synchronization_barrier();
    auto actual = c_.read(address);
    auto released = c_.resources.release(c_.execution, *owner);
    if (!result || !actual || (*actual & 255U) != value)
      return std::unexpected(system_error::mmio);
    if (!released)
      return std::unexpected(system_error::busy);
    return {};
  }
  Context &c_;
};
struct uicr_provisioning_scope {
  bool customer_words{}, pin_roles{}, debug_configuration{}, regulator{},
      bootloader_words{}, erase_all_words{};
};
template <class Context, class Power, unsigned ReadyPollBudget = 1000000>
class uicr_provisioner {
public:
  uicr_provisioner(Context &c, Power &power,
                   execution::tick ready_ticks = 12'800'000) noexcept
      : c_(c), power_(power), ready_ticks_(ready_ticks) {}
  uicr_provisioner(const uicr_provisioner &) = delete;
  uicr_provisioner &operator=(const uicr_provisioner &) = delete;
  [[nodiscard]] system_result
  initialize(uicr_provisioning_scope scope) noexcept {
    if (owner_ || c_.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (fault_)
      return std::unexpected(system_error::faulted);
    if ((!scope.customer_words && !scope.pin_roles &&
         !scope.debug_configuration && !scope.regulator &&
         !scope.bootloader_words && !scope.erase_all_words) ||
        !ready_ticks_ || ready_ticks_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto owner = c_.resources.claim(c_.execution, peripheral::nvmc, this);
    if (!owner)
      return std::unexpected(system_error::busy);
    owner_ = *owner;
    auto config = c_.read(nvmc + 0x504), ready = c_.read(nvmc + 0x400);
    if (!config || !ready || *config || !(*ready & 1U)) {
      (void)c_.resources.release(c_.execution, *owner_);
      owner_.reset();
      return std::unexpected(config && ready ? system_error::busy
                                             : system_error::mmio);
    }
    scope_ = scope;
    c_.work_started();
    work_ = true;
    return {};
  }
  [[nodiscard]] system_result program_word(unsigned offset,
                                           std::uint32_t value) noexcept {
    if (!owner_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (!permitted(offset, value))
      return std::unexpected(system_error::invalid);
    if (offset == 0x208) {
      auto build = access_port_build(c_);
      if (!build)
        return std::unexpected(build.error());
      auto low = value & 255U;
      if (low &&
          low != (*build == access_port_revision::hardware_only ? 255U : 0x5AU))
        return std::unexpected(system_error::invalid);
    }
    return c_.execution.critical([&]() -> system_result {
      auto old = c_.read(uicr + offset);
      if (!old)
        return std::unexpected(old.error());
      if (*old != 0xFFFFFFFFU)
        return std::unexpected(system_error::invalid);
      if (value == 0xFFFFFFFFU)
        return {};
      auto begun = begin(1);
      if (!begun)
        return begun;
      reset_required_ = true;
      if (!c_.write(uicr + offset, value))
        return fail(system_error::mmio);
      arch::data_synchronization_barrier();
      auto finished = recover();
      if (!finished)
        return finished;
      auto actual = c_.read(uicr + offset);
      if (!actual || *actual != value)
        return fail(system_error::mmio);
      return {};
    });
  }
  // Erases ALL UICR words, including firmware/bootloader information and pin,
  // regulator and debug settings. Never a prerequisite for program_word().
  [[nodiscard]] system_result erase_uicr() noexcept {
    if (!owner_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (!scope_.erase_all_words || !scope_.customer_words ||
        !scope_.pin_roles || !scope_.debug_configuration || !scope_.regulator ||
        !scope_.bootloader_words)
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      auto begun = begin(2);
      if (!begun)
        return begun;
      reset_required_ = true;
      if (!c_.write(nvmc + 0x514, 1))
        return fail(system_error::mmio);
      arch::data_synchronization_barrier();
      return recover();
    });
  }
  [[nodiscard]] bool reset_required() const noexcept { return reset_required_; }
  [[nodiscard]] system_result recover() noexcept {
    if (!mutation_ && !power_.power_fail_suspended_by(this))
      return {};
    if (c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto quiet = wait_ready();
    if (!quiet)
      return quiet;
    if (!c_.write(nvmc + 0x504, 0))
      return fail(system_error::mmio);
    arch::data_synchronization_barrier();
    auto config = c_.read(nvmc + 0x504);
    if (!config || *config)
      return fail(system_error::mmio);
    if (power_.power_fail_suspended_by(this)) {
      auto restored = power_.restore_power_fail(this);
      if (!restored)
        return fail(restored.error());
    }
    if (mutation_) {
      if (!c_.resources.release(c_.execution, *mutation_))
        return fail(system_error::busy);
      mutation_.reset();
    }
    return {};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!owner_)
      return {};
    auto quiet = recover();
    if (!quiet)
      return quiet;
    if (!c_.resources.release(c_.execution, *owner_))
      return std::unexpected(system_error::busy);
    owner_.reset();
    if (work_) {
      c_.work_stopped();
      work_ = false;
    }
    return {};
  }

private:
  static constexpr bool reserved(std::uint32_t value,
                                 std::uint32_t mask) noexcept {
    auto bits = value & ~mask;
    return bits == 0 || bits == ~mask;
  }
  bool permitted(unsigned offset, std::uint32_t value) const noexcept {
    if (offset % 4)
      return false;
    if (offset >= 0x80 && offset <= 0xFC)
      return scope_.customer_words;
    if (offset >= 0x14 && offset <= 0x44)
      return scope_.bootloader_words;
    if (offset == 0x200 || offset == 0x204)
      return scope_.pin_roles && reserved(value, 0x8000003FU);
    if (offset == 0x20C)
      return scope_.pin_roles && reserved(value, 1U);
    if (offset == 0x208)
      return scope_.debug_configuration && reserved(value, 255U);
    if (offset == 0x210) {
      auto a = value & 255U, b = (value >> 8) & 255U;
      return scope_.debug_configuration && reserved(value, 0xFFFFU) &&
             (a == 0 || a == 255) && (b == 0 || b == 255);
    }
    if (offset == 0x304)
      return scope_.regulator && reserved(value, 7U) && (value & 7U) != 6;
    return false;
  }
  system_result begin(std::uint32_t mode) noexcept {
    if (mutation_ || power_.power_fail_suspended_by(this))
      return std::unexpected(system_error::busy);
    auto mutation =
        c_.resources.claim(c_.execution, peripheral::flash_mutation, this);
    if (!mutation)
      return std::unexpected(system_error::busy);
    mutation_ = *mutation;
    auto paused = power_.suspend_power_fail(this);
    if (!paused) {
      if (power_.power_fail_suspended_by(this))
        fault_ = true;
      else {
        (void)c_.resources.release(c_.execution, *mutation_);
        mutation_.reset();
      }
      return paused;
    }
    auto ready = wait_ready();
    if (!ready)
      return ready;
    if (!c_.write(nvmc + 0x504, mode))
      return fail(system_error::mmio);
    arch::data_synchronization_barrier();
    return {};
  }
  system_result wait_ready() noexcept {
    execution::deadline until{
        static_cast<execution::tick>(c_.execution.now() + ready_ticks_)};
    for (unsigned i = 0; i < ReadyPollBudget; ++i) {
      auto ready = c_.read(nvmc + 0x400);
      if (!ready)
        return fail(system_error::mmio);
      if (*ready & 1U)
        return {};
      if (execution::reached(c_.execution.now(), until))
        break;
    }
    return fail(system_error::timeout);
  }
  system_result fail(system_error error) noexcept {
    fault_ = true;
    return std::unexpected(error);
  }
  static constexpr std::uintptr_t nvmc = 0x4001E000, uicr = 0x10001000;
  Context &c_;
  Power &power_;
  execution::tick ready_ticks_;
  std::optional<resource_token> owner_, mutation_;
  uicr_provisioning_scope scope_{};
  bool work_{}, fault_{}, reset_required_{};
};
} // namespace mcubsp::backend::nrf52840
