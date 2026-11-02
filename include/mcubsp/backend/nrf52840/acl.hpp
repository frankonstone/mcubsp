#pragma once
// @req-file REQ-0072 REQ-0062
// PS v1.11 6.3. ADDR/SIZE/PERM are write-once until a hardware reset.
#include <mcubsp/backend/nrf52840/system_context.hpp>
#include <mcubsp/mcu/nrf52840/product/acl.hpp>
namespace mcubsp::backend::nrf52840 {
enum class acl_protection : std::uint32_t {
  write = 2,
  read_execute = 4,
  all = 6
};
struct acl_readback {
  std::uintptr_t address;
  std::size_t bytes;
  std::uint32_t permissions;
};
template <class Context> class access_control_controller {
  using region = mcu::nrf52840::memory::region;

public:
  explicit access_control_controller(Context &c) noexcept : c_(c) {}
  access_control_controller(const access_control_controller &) = delete;
  access_control_controller &
  operator=(const access_control_controller &) = delete;
  [[nodiscard]] system_result
  initialize(region assigned,
             std::span<const region> protected_ranges) noexcept {
    if (owner_ || c_.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!assigned.bytes || assigned.begin % 4096 || assigned.bytes % 4096 ||
        !mcu::nrf52840::memory::flash.contains(assigned.begin,
                                               assigned.bytes) ||
        protected_ranges.size() > 8)
      return std::unexpected(system_error::invalid);
    for (auto r : protected_ranges)
      if (overlaps(assigned, r) ||
          !mcu::nrf52840::memory::flash.contains(r.begin, r.bytes))
        return std::unexpected(system_error::invalid);
    auto claim = c_.resources.claim(c_.execution, peripheral::acl, this);
    if (!claim)
      return std::unexpected(system_error::busy);
    owner_ = *claim;
    assigned_ = assigned;
    return {};
  }
  [[nodiscard]] system_value<acl_readback> read(unsigned slot) noexcept {
    if (slot >= 8)
      return std::unexpected(system_error::invalid);
    auto a = c_.read(base + 0x800 + 16 * slot),
         n = c_.read(base + 0x804 + 16 * slot),
         p = c_.read(base + 0x808 + 16 * slot);
    if (!a || !n || !p)
      return std::unexpected(system_error::mmio);
    return acl_readback{*a, *n, *p};
  }
  [[nodiscard]] system_result protect(unsigned slot, region target,
                                      acl_protection permission) noexcept {
    if (!owner_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    auto bits = static_cast<unsigned>(permission);
    if (slot >= 8 || target.begin % 4096 || !target.bytes ||
        target.bytes % 4096 || target.bytes > 0x80000 ||
        !assigned_.contains(target.begin, target.bytes) ||
        (bits != 2 && bits != 4 && bits != 6))
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      if (attempted_ & (1U << slot))
        return std::unexpected(system_error::busy);
      auto old = read(slot);
      if (!old)
        return std::unexpected(old.error());
      if (old->address || old->bytes || old->permissions)
        return std::unexpected(system_error::busy);
      auto mutation =
          c_.resources.claim(c_.execution, peripheral::flash_mutation, this);
      if (!mutation)
        return std::unexpected(system_error::busy);
      mutation_ = *mutation;
      attempted_ |= 1U << slot;
      pending_slot_ = slot;
      // PERM is last: reset PERM=0 leaves partial configuration unprotected.
      if (!c_.write(base + 0x800 + 16 * slot, target.begin) ||
          !c_.write(base + 0x804 + 16 * slot, target.bytes) ||
          !c_.write(base + 0x808 + 16 * slot, bits)) {
        fault_ = true;
        return std::unexpected(system_error::mmio);
      }
      arch::data_synchronization_barrier();
      auto actual = read(slot);
      if (!actual || actual->address != target.begin ||
          actual->bytes != target.bytes || actual->permissions != bits) {
        fault_ = true;
        return std::unexpected(system_error::mmio);
      }
      return recover();
    });
  }
  // Readback resolves an uncertain bus write; it does not undo protection.
  [[nodiscard]] system_result recover() noexcept {
    if (!mutation_)
      return {};
    if (c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto actual = read(pending_slot_);
    if (!actual)
      return std::unexpected(actual.error());
    last_ = *actual;
    arch::data_synchronization_barrier();
    if (!c_.resources.release(c_.execution, *mutation_))
      return std::unexpected(system_error::busy);
    mutation_.reset();
    return {};
  }
  [[nodiscard]] std::optional<acl_readback> last_observed() const noexcept {
    return last_;
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!owner_)
      return {};
    auto recovered = recover();
    if (!recovered)
      return recovered;
    if (!c_.resources.release(c_.execution, *owner_))
      return std::unexpected(system_error::busy);
    owner_.reset();
    return {};
  }

private:
  static constexpr bool overlaps(region a, region b) noexcept {
    return a.bytes && b.bytes && a.begin < b.begin + b.bytes &&
           b.begin < a.begin + a.bytes;
  }
  static constexpr auto base = mcu::nrf52840::product::acl::base;
  Context &c_;
  std::optional<resource_token> owner_, mutation_;
  region assigned_{};
  std::uint8_t attempted_{};
  unsigned pending_slot_{};
  std::optional<acl_readback> last_;
  bool fault_{};
};
} // namespace mcubsp::backend::nrf52840
