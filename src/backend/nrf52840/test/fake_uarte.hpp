#pragma once
#include "fake_system.hpp"
#include <mcubsp/execution/fake_policy.hpp>
namespace nrf52840_test {
class fake_uarte : public fake_system {
public:
  std::uintptr_t base{0x40002000};
  bool tx_stop_responds{true}, rx_stop_responds{true}, flush_responds{true};
  bool fail_terminal_ack{};
  std::optional<std::uintptr_t> failed_write_address;
  unsigned fifo_bytes{}, flushes{}, starts{};
  std::uint32_t interrupts{};
  bool pending_irq() const noexcept {
    for (unsigned i = 0; i < 32; ++i)
      if ((interrupts & (1U << i)) && peek(base + 0x100 + 4 * i))
        return true;
    return false;
  }
  mcubsp::execution::fake_policy *clock{};
  template <class T>
  std::expected<T, mcubsp::core::mmio_error>
  read(std::uintptr_t address) noexcept {
    if (clock)
      clock->advance();
    return fake_system::read<T>(address);
  }
  template <class T>
  std::expected<void, mcubsp::core::mmio_error> write(std::uintptr_t address,
                                                      T value) noexcept {
    const auto before = peek(address);
    auto r = fake_system::write<T>(address, value);
    if (failed_write_address && address == *failed_write_address) {
      failed_write_address.reset();
      return std::unexpected(mcubsp::core::mmio_error::out_of_range);
    }
    if (!r && !fail_after_commit)
      return r;
    if (address == base + 0x304)
      interrupts |= value;
    if (address == base + 0x308)
      interrupts &= ~value;
    if (address == base + 0x480)
      poke(address, before & ~static_cast<std::uint32_t>(value));
    if (fail_terminal_ack && address == base + 0x158 && before && !value) {
      fail_terminal_ack = false;
      return std::unexpected(mcubsp::core::mmio_error::out_of_range);
    }
    if (!value)
      return r;
    if (address == base + 0x000) {
      ++starts;
      poke(base + 0x14C, 1);
      poke(base + 0x53C, 0);
    }
    if (address == base + 0x008) {
      ++starts;
      poke(base + 0x150, 1);
      poke(base + 0x54C, 0);
    }
    if (address == base + 0x00C && tx_stop_responds) {
      poke(base + 0x120, 1);
      poke(base + 0x158, 1);
    }
    if (address == base + 0x004 && rx_stop_responds) {
      poke(base + 0x110, 1);
      poke(base + 0x144, 1);
    }
    if (address == base + 0x02C) {
      ++flushes;
      if (flush_responds) {
        poke(base + 0x53C, fifo_bytes);
        fifo_bytes = 0;
        poke(base + 0x110, 1);
      }
    }
    return r;
  }
};
// Numeric caller spans below are simulated RAM, never dereferenced. The private
// drain buffer is translated separately, preserving production RAM validation.
struct fake_dma_address {
  std::uintptr_t operator()(const void *p, std::size_t) const noexcept {
    const auto a = reinterpret_cast<std::uintptr_t>(p);
    return a <= 0xFFFFFFFFU ? a : 0x2003FFF0U;
  }
};
} // namespace nrf52840_test
