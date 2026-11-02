#pragma once
// @req-file REQ-0072 REQ-0062
// PS v1.11 6.13: CPU accesses only; EasyDMA does not generate these events.
#include <mcubsp/backend/nrf52840/system_context.hpp>
#include <mcubsp/mcu/nrf52840/product/mwu.hpp>
namespace mcubsp::backend::nrf52840 {
struct memory_watch_events {
  std::uint8_t reads{}, writes{};
  std::array<std::uint32_t, 2> peripheral_reads{}, peripheral_writes{};
};
template <class Context> class memory_watch_controller {
public:
  explicit memory_watch_controller(Context &c) noexcept : c_(c) {}
  memory_watch_controller(const memory_watch_controller &) = delete;
  memory_watch_controller &operator=(const memory_watch_controller &) = delete;
  [[nodiscard]] system_result initialize() noexcept {
    if (owner_ || c_.execution.in_isr())
      return std::unexpected(system_error::busy);
    auto claim = c_.resources.claim(c_.execution, peripheral::mwu, this);
    if (!claim)
      return std::unexpected(system_error::busy);
    owner_ = *claim;
    auto active = c_.read(base + 0x510);
    if (!active || *active) {
      (void)release();
      return std::unexpected(active ? system_error::busy : system_error::mmio);
    }
    if (!write(0x308, mask) || !write(0x328, mask) || !clear_events())
      return init_failed();
    ready_ = true;
    c_.work_started();
    work_ = true;
    return {};
  }
  [[nodiscard]] system_result watch(unsigned slot,
                                    mcu::nrf52840::memory::region region,
                                    bool read, bool write_access) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    constexpr mcu::nrf52840::memory::region peripherals{0x40000000, 0x20000000};
    if (slot >= 4 || !region.bytes ||
        (!mcu::nrf52840::memory::data_ram.contains(region.begin,
                                                   region.bytes) &&
         !peripherals.contains(region.begin, region.bytes)))
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      auto bits = 3U << (2 * slot);
      if (!write(0x518, bits) || !write(0x600 + 16 * slot, region.begin) ||
          !write(0x604 + 16 * slot, region.begin + region.bytes - 1) ||
          !clear_pair(slot) ||
          !write(0x514, ((write_access ? 1U : 0U) | (read ? 2U : 0U))
                            << (2 * slot)))
        return fail();
      return {};
    });
  }
  [[nodiscard]] system_result watch_peripherals(unsigned region,
                                                std::uint32_t subregions,
                                                bool read,
                                                bool write_access) noexcept {
    if (!ready_ || fault_ || c_.execution.in_isr())
      return std::unexpected(system_error::not_initialized);
    if (region > 1)
      return std::unexpected(system_error::invalid);
    return c_.execution.critical([&]() -> system_result {
      auto shift = 24 + 2 * region;
      if (!write(0x518, 3U << shift) ||
          !write(0x6C8 + 16 * region, subregions) || !clear_pair(region + 4) ||
          !write(0x514, ((write_access ? 1U : 0U) | (read ? 2U : 0U)) << shift))
        return fail();
      return {};
    });
  }
  [[nodiscard]] system_result interrupt_delivery(std::uint32_t events,
                                                 bool nmi = false) noexcept {
    if (!ready_ || events & ~mask || c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!write(0x308, mask) || !write(0x328, mask) ||
        !write(nmi ? 0x324 : 0x304, events))
      return fail();
    return {};
  }
  [[nodiscard]] system_value<memory_watch_events> poll() noexcept {
    if (!ready_)
      return std::unexpected(system_error::not_initialized);
    return c_.execution.critical([&]() -> system_value<memory_watch_events> {
      memory_watch_events result{};
      for (unsigned i = 0; i < 6; ++i)
        for (unsigned direction = 0; direction < 2; ++direction) {
          auto off =
              (i < 4 ? 0x100 + 8 * i : 0x160 + 8 * (i - 4)) + 4 * direction;
          auto event = c_.read(base + off);
          if (!event)
            return std::unexpected(system_error::mmio);
          if (!*event)
            continue;
          if (!c_.clear_event(base + off))
            return std::unexpected(system_error::mmio);
          if (direction)
            result.reads |= 1U << i;
          else
            result.writes |= 1U << i;
          if (i >= 4) {
            auto sub = 0x400 + 8 * (i - 4) + 4 * direction;
            auto bits = c_.read(base + sub);
            if (!bits || !write(sub, *bits))
              return std::unexpected(system_error::mmio);
            if (direction)
              result.peripheral_reads[i - 4] = *bits;
            else
              result.peripheral_writes[i - 4] = *bits;
          }
        }
      return result;
    });
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (c_.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!owner_)
      return {};
    if (!write(0x308, mask) || !write(0x328, mask) || !write(0x518, mask))
      return fail();
    auto enabled = c_.read(base + 0x510);
    if (!enabled || *enabled)
      return fail();
    if (!clear_events())
      return fail();
    auto released = release();
    if (!released)
      return released;
    ready_ = false;
    if (work_) {
      c_.work_stopped();
      work_ = false;
    }
    return {};
  }

private:
  bool write(unsigned off, std::uint32_t value) noexcept {
    auto r = (off >= 0x100 && off < 0x200 && !value)
                 ? c_.clear_event(base + off)
                 : c_.write(base + off, value);
    arch::data_synchronization_barrier();
    return r.has_value();
  }
  bool clear_pair(unsigned i) noexcept {
    auto off = i < 4 ? 0x100 + 8 * i : 0x160 + 8 * (i - 4);
    if (!write(off, 0) || !write(off + 4, 0))
      return false;
    return i < 4 || (write(0x400 + 8 * (i - 4), 0xFFFFFFFF) &&
                     write(0x404 + 8 * (i - 4), 0xFFFFFFFF));
  }
  bool clear_events() noexcept {
    for (unsigned i = 0; i < 6; ++i)
      if (!clear_pair(i))
        return false;
    return true;
  }
  system_result release() noexcept {
    if (!owner_)
      return {};
    if (!c_.resources.release(c_.execution, *owner_))
      return std::unexpected(system_error::busy);
    owner_.reset();
    return {};
  }
  system_result fail() noexcept {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  system_result init_failed() noexcept {
    fault_ = true;
    (void)shutdown();
    return std::unexpected(system_error::mmio);
  }
  static constexpr auto base = mcu::nrf52840::product::mwu::base;
  static constexpr auto mask =
      mcu::nrf52840::product::mwu::regionenclr::writable_mask;
  Context &c_;
  std::optional<resource_token> owner_;
  bool ready_{}, fault_{}, work_{};
};
} // namespace mcubsp::backend::nrf52840
