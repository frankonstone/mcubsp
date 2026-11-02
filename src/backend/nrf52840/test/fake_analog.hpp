#pragma once
#include "fake_system.hpp"
#include <mcubsp/execution/fake_policy.hpp>
namespace nrf52840_test {
struct analog_address {
  std::uintptr_t operator()(const void *p, std::size_t) const noexcept {
    auto a = reinterpret_cast<std::uintptr_t>(p);
    return a <= 0xFFFFFFFFU ? a : 0x20000000U + (a & 0xFFFFU);
  }
};
class fake_analog : public fake_system {
public:
  using dma_address_type = analog_address;
  mcubsp::execution::fake_policy *execution{};
  std::uintptr_t pwm_base{0x4001C000};
  bool stop_responds{true}, calibration_responds{true};
  unsigned pwm_starts{}, adc_starts{}, adc_samples{}, calibrations{};
  std::optional<std::uintptr_t> fail_event_ack;
  template <class T>
  std::expected<T, mcubsp::core::mmio_error> read(std::uintptr_t a) noexcept {
    if (execution)
      execution->advance();
    return fake_system::read<T>(a);
  }
  template <class T>
  std::expected<void, mcubsp::core::mmio_error> write(std::uintptr_t a,
                                                      T v) noexcept {
    auto before = peek(a);
    auto r = fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a == pwm_base + 4 && v && stop_responds)
      poke(pwm_base + 0x104, 1);
    if ((a == pwm_base + 8 || a == pwm_base + 12) && v) {
      ++pwm_starts;
      poke(pwm_base + 0x108 + (a - pwm_base - 8), 1);
    }
    if (a == 0x40007000 && v) {
      ++adc_starts;
      poke(0x40007100, 1);
      poke(0x40007634, 0);
    }
    if (a == 0x40007004 && v)
      ++adc_samples;
    if (a == 0x40007008 && v && stop_responds)
      poke(0x40007114, 1);
    if (a == 0x4000700C && v) {
      ++calibrations;
      if (calibration_responds)
        poke(0x40007110, 1);
    }
    if (a == 0x40007FFC && !v) {
      for (unsigned i = 0; i < 8; ++i)
        for (unsigned j = 0; j < 4; ++j)
          poke(0x40007510 + 16 * i + 4 * j, 0);
      for (unsigned i = 0; i < 3; ++i)
        poke(0x40007640 + 4 * i, 0);
    }
    if (a == 0x40013000 && v)
      poke(0x40013100, 1);
    if (fail_event_ack && a == *fail_event_ack && before && !v) {
      fail_event_ack.reset();
      return std::unexpected(mcubsp::core::mmio_error::out_of_range);
    }
    return r;
  }
  void adc_end(unsigned amount) {
    poke(0x40007634, amount);
    poke(0x40007104, 1);
  }
};
} // namespace nrf52840_test
