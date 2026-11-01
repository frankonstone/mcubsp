#pragma once
// @req-file REQ-0071
// PS 4413_417 v1.11 6.21. CPU-driven peripheral, distinct from CC_RNG.
#include <mcubsp/backend/nrf52840/system_context.hpp>
#include <mcubsp/core/transfer.hpp>
#include <type_traits>
namespace mcubsp::backend::nrf52840 {
template <class Context> class rng_controller {
  using E =
      std::remove_reference_t<decltype(std::declval<Context &>().execution)>;

public:
  using pool_type = core::operation_pool<std::size_t, system_error, 1, E>;
  using start_result = std::expected<typename pool_type::transfer_type,
                                     typename pool_type::error_type>;
  explicit rng_controller(Context &c) noexcept
      : c_(c), operations_(c.execution) {}
  rng_controller(const rng_controller &) = delete;
  rng_controller &operator=(const rng_controller &) = delete;
  [[nodiscard]] system_result initialize(bool bias_correction = true) noexcept {
    if (c_.execution.in_isr() || owner_)
      return std::unexpected(system_error::busy);
    if (fault_)
      return std::unexpected(system_error::faulted);
    auto claim = c_.resources.claim(c_.execution, peripheral::rng, this);
    if (!claim)
      return std::unexpected(system_error::busy);
    owner_ = *claim;
    // VALRDY_STOP provides stable one-byte handoffs even if foreground
    // service is delayed. Each subsequent byte is explicitly restarted.
    if (!write(0x308, 1) || !write(0x004, 1) ||
        !write(0x504, bias_correction ? 1U : 0U) || !write(0x200, 1) ||
        !write(0x100, 0)) {
      fault_ = true;
      (void)shutdown();
      return std::unexpected(system_error::mmio);
    }
    ready_ = true;
    work_ = true;
    c_.work_started();
    return {};
  }
  [[nodiscard]] start_result generate(std::span<std::uint8_t> bytes) noexcept {
    if (bytes.empty())
      return failure(system_error::invalid);
    if (!ready_ || fault_ || c_.execution.in_isr())
      return failure(system_error::not_initialized);
    return c_.execution.critical([&]() -> start_result {
      auto operation =
          operations_.try_start({.quiesce = &cancel, .context = this});
      if (!operation)
        return std::unexpected(operation.error());
      id_ = operation->id();
      output_ = bytes;
      generated_ = 0;
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
      if (!value || !write(0x100, 0)) {
        fail_operation();
        return true;
      }
      output_[generated_++] = static_cast<std::uint8_t>(*value);
      if (generated_ == output_.size()) {
        if (!write(0x004, 1)) {
          fail_operation();
          return true;
        }
        auto id = *id_;
        id_.reset();
        output_ = {};
        (void)operations_.complete(id, generated_);
      } else if (!write(0x000, 1))
        fail_operation();
      return true;
    });
  }
  [[nodiscard]] std::size_t generated_count() const noexcept {
    return c_.execution.critical([&] { return generated_; });
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
    if (!write(0x308, 1) || !write(0x004, 1) || !write(0x200, 0) ||
        !write(0x100, 0))
      return std::unexpected(system_error::mmio);
    if (!c_.resources.release(c_.execution, *owner_))
      return std::unexpected(system_error::busy);
    owner_.reset();
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
  void fail_operation() noexcept {
    fault_ = true;
    (void)write(0x004, 1);
    // Only the CPU writes the caller span. Detach it even if STOP failed;
    // keep peripheral ownership until a successful foreground shutdown.
    output_ = {};
    auto id = *id_;
    id_.reset();
    (void)operations_.fail(id, system_error::mmio);
  }
  static core::quiesce_result cancel(void *p, core::operation_id id) noexcept {
    auto &s = *static_cast<rng_controller *>(p);
    if (!s.id_ || *s.id_ != id)
      return core::quiesce_result::reusable;
    const bool stopped = s.write(0x004, 1) && s.write(0x100, 0);
    s.output_ = {};
    s.id_.reset();
    s.fault_ |= !stopped;
    return stopped ? core::quiesce_result::reusable
                   : core::quiesce_result::faulted;
  }
  static constexpr std::uintptr_t base = 0x4000D000;
  Context &c_;
  pool_type operations_;
  std::optional<resource_token> owner_;
  std::optional<core::operation_id> id_;
  std::span<std::uint8_t> output_;
  std::size_t generated_{};
  bool ready_{}, fault_{}, work_{};
};
} // namespace mcubsp::backend::nrf52840
