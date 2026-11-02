#pragma once
// @req-file REQ-0072 REQ-0062
// PS 4413_417 v1.11 4.3. Explicit storage only; no implicit UICR/mass erase.
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/execution/deadline.hpp>
#include <mcubsp/mcu/nrf52840/memory.hpp>
namespace mcubsp::backend::nrf52840 {
template <class Context, class Power, unsigned ReadyPollBudget = 1000000>
class nvmc_controller {
  static_assert(ReadyPollBudget > 0);
  using region = mcu::nrf52840::memory::region;

public:
  nvmc_controller(Context &c, Power &power,
                  execution::tick ready_ticks = 12'800'000) noexcept
      : c_(c), power_(power), ready_ticks_(ready_ticks) {}
  nvmc_controller(const nvmc_controller &) = delete;
  nvmc_controller &operator=(const nvmc_controller &) = delete;
  // Caller supplies a linker-reserved storage span and all protected ranges.
  // The protected ranges are copied into fixed storage, not borrowed.
  [[nodiscard]] system_result
  initialize(region storage,
             std::span<const region> protected_ranges) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(system_error::busy);
    if (fault_)
      return std::unexpected(system_error::faulted);
    if (!ready_ticks_ || ready_ticks_ > execution::maximum_deadline_interval ||
        !storage.bytes || storage.begin % page_bytes ||
        storage.bytes % page_bytes ||
        !mcu::nrf52840::memory::flash.contains(storage.begin, storage.bytes) ||
        protected_ranges.size() > protected_.size())
      return std::unexpected(system_error::invalid);
    for (auto p : protected_ranges) {
      if (!mcu::nrf52840::memory::flash.contains(p.begin, p.bytes) ||
          overlaps(storage, p))
        return std::unexpected(system_error::invalid);
    }
    auto claim = c_.resources.claim(c_.execution, peripheral::nvmc, this);
    if (!claim)
      return std::unexpected(system_error::busy);
    owner_ = *claim;
    auto mode = c_.read(base + 0x504), ready = c_.read(base + 0x400);
    if (!mode || !ready || *mode || !(*ready & 1U)) {
      (void)release_owner();
      return std::unexpected(mode && ready ? system_error::busy
                                           : system_error::mmio);
    }
    storage_ = storage;
    protected_count_ = protected_ranges.size();
    for (std::size_t i = 0; i < protected_count_; ++i)
      protected_[i] = protected_ranges[i];
    ready_ = true;
    c_.work_started();
    work_ = true;
    return {};
  }
  // One program operation per erased word prevents violating nWRITE. Writing
  // all ones to an already erased word is an intentional no-op.
  [[nodiscard]] system_result program_word(std::uintptr_t address,
                                           std::uint32_t value) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (address % 4 || !allowed(address, 4))
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      auto old = c_.read(address);
      if (!old)
        return std::unexpected(system_error::mmio);
      if (*old != 0xFFFFFFFFU)
        return std::unexpected(system_error::invalid);
      if (value == 0xFFFFFFFFU)
        return {};
      auto begun = begin(1);
      if (!begun)
        return begun;
      // The CPU's value is committed by the store; no caller buffer is
      // borrowed after this method, including a failed READY wait.
      auto written = c_.write(address, value);
      arch::data_synchronization_barrier();
      if (!written)
        return fail(system_error::mmio);
      auto finished = finish();
      if (!finished)
        return finished;
      auto actual = c_.read(address);
      if (!actual || *actual != value)
        return fail(system_error::mmio);
      return {};
    });
  }
  [[nodiscard]] system_result erase_page(std::uintptr_t address) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (address % page_bytes || !allowed(address, page_bytes))
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      auto begun = begin(2);
      if (!begun)
        return begun;
      auto erased = c_.write(base + 0x508, address);
      arch::data_synchronization_barrier();
      if (!erased)
        return fail(system_error::mmio);
      return finish();
    });
  }
  // One bounded portion, not a claim that the page is fully erased. The caller
  // accumulates the documented erase duration before programming that page.
  [[nodiscard]] system_result
  erase_page_partial(std::uintptr_t address, unsigned milliseconds) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (address % page_bytes || !allowed(address, page_bytes) ||
        !milliseconds || milliseconds > 127)
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      auto begun = begin(2);
      if (!begun)
        return begun;
      if (!c_.write(base + 0x51C, milliseconds) ||
          !c_.write(base + 0x518, address))
        return fail(system_error::mmio);
      arch::data_synchronization_barrier();
      return finish();
    });
  }
  struct cache_statistics {
    std::uint32_t hits{}, misses{};
  };
  [[nodiscard]] system_result
  configure_instruction_cache(bool enabled, bool profile = false) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (mutation_ || power_.power_fail_suspended_by(this))
      return std::unexpected(system_error::busy);
    if (!c_.write(base + 0x540, (enabled ? 1U : 0U) | (profile ? 256U : 0U)))
      return fail(system_error::mmio);
    arch::data_synchronization_barrier();
    return {};
  }
  [[nodiscard]] system_value<cache_statistics>
  instruction_cache_statistics(bool reset = false) noexcept {
    if (!ready_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    auto hits = c_.read(base + 0x548), misses = c_.read(base + 0x54C);
    if (!hits || !misses)
      return std::unexpected(system_error::mmio);
    if (reset && (!c_.write(base + 0x548, 0) || !c_.write(base + 0x54C, 0)))
      return std::unexpected(system_error::mmio);
    return cache_statistics{*hits, *misses};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!owner_)
      return {};
    if (mutation_ || power_.power_fail_suspended_by(this)) {
      auto finished = finish();
      if (!finished)
        return finished;
    }
    if (!release_owner())
      return std::unexpected(system_error::busy);
    if (work_) {
      c_.work_stopped();
      work_ = false;
    }
    ready_ = false;
    return {};
  }
  [[nodiscard]] bool faulted() const noexcept { return fault_; }

private:
  static constexpr bool overlaps(region a, region b) noexcept {
    return a.bytes && b.bytes && a.begin < b.begin + b.bytes &&
           b.begin < a.begin + a.bytes;
  }
  bool allowed(std::uintptr_t a, std::size_t n) const noexcept {
    if (!storage_.contains(a, n))
      return false;
    for (std::size_t i = 0; i < protected_count_; ++i)
      if (overlaps({a, n}, protected_[i]))
        return false;
    return true;
  }
  system_result wait_ready() noexcept {
    execution::deadline until{
        static_cast<execution::tick>(c_.execution.now() + ready_ticks_)};
    for (unsigned i = 0; i < ReadyPollBudget; ++i) {
      auto ready = c_.read(base + 0x400);
      if (!ready)
        return fail(system_error::mmio);
      if (*ready & 1U)
        return {};
      if (execution::reached(c_.execution.now(), until))
        break;
    }
    return fail(system_error::timeout);
  }
  system_result begin(std::uint32_t mode) noexcept {
    if (mutation_ || power_.power_fail_suspended_by(this))
      return std::unexpected(system_error::busy);
    auto claim =
        c_.resources.claim(c_.execution, peripheral::flash_mutation, this);
    if (!claim)
      return std::unexpected(system_error::busy);
    mutation_ = *claim;
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
    if (!c_.write(base + 0x504, mode))
      return fail(system_error::mmio);
    arch::data_synchronization_barrier();
    return {};
  }
  system_result finish() noexcept {
    auto ready = wait_ready();
    if (!ready)
      return ready;
    if (!c_.write(base + 0x504, 0))
      return fail(system_error::mmio);
    arch::data_synchronization_barrier();
    auto mode = c_.read(base + 0x504);
    if (!mode || *mode)
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
  bool release_owner() noexcept {
    if (!owner_)
      return true;
    if (!c_.resources.release(c_.execution, *owner_))
      return false;
    owner_.reset();
    return true;
  }
  system_result fail(system_error e) noexcept {
    fault_ = true;
    return std::unexpected(e);
  }
  static constexpr std::uintptr_t base = 0x4001E000;
  static constexpr std::size_t page_bytes =
      mcu::nrf52840::memory::flash_page_bytes;
  Context &c_;
  Power &power_;
  execution::tick ready_ticks_;
  std::optional<resource_token> owner_, mutation_;
  region storage_{};
  std::array<region, 8> protected_{};
  std::size_t protected_count_{};
  bool ready_{}, fault_{}, work_{};
};
} // namespace mcubsp::backend::nrf52840
