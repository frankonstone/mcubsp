#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/nvmc.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_nvmc : public nrf52840_test::fake_system {
public:
  execution::fake_policy *clock{};
  bool busy_after_write{}, pof_during_disable{};
  unsigned flash_writes{}, erases{};
  template <class T>
  std::expected<T, core::mmio_error> read(std::uintptr_t a) noexcept {
    if (clock)
      clock->advance();
    return nrf52840_test::fake_system::read<T>(a);
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto before = peek(a);
    auto r = nrf52840_test::fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a < 0x100000 && peek(0x4001E504) == 1) {
      ++flash_writes;
      poke(a, before & static_cast<std::uint32_t>(v));
      if (busy_after_write)
        poke(0x4001E400, 0);
    }
    if (a == 0x4001E508 && peek(0x4001E504) == 2) {
      ++erases;
      for (unsigned i = 0; i < 4096; i += 4)
        poke(v + i, 0xFFFFFFFFU);
      if (busy_after_write)
        poke(0x4001E400, 0);
    }
    if (a == 0x40000510 && !(v & 1U) && pof_during_disable)
      poke(0x40000108, 1);
    return r;
  }
};
struct fixture {
  fake_nvmc mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_nvmc> gpio{mmio, resources};
  system_context<fake_nvmc, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  power_controller<decltype(context)> power{context, clocks};
  nvmc_controller<decltype(context), decltype(power), 16> flash{context, power,
                                                                100};
  fixture() {
    mmio.clock = &execution;
    mmio.poke(0x10000100, 0x52840);
    mmio.poke(0x4001E400, 1);
    mmio.poke(0x40000510, 0xF19);
  }
  ~fixture() {
    (void)flash.shutdown();
    (void)power.shutdown();
    (void)clocks.shutdown();
  }
  bool initialize() {
    const std::array protected_ranges{
        mcu::nrf52840::memory::region{0, 0x26000},
        mcu::nrf52840::memory::region{0xEC000, 0x14000}};
    return clocks.initialize().has_value() && power.initialize().has_value() &&
           flash.initialize({0xE0000, 4096}, protected_ranges).has_value();
  }
};
TEST(nrf_nvmc,
     word_programming_is_confined_to_explicit_storage_and_restores_power_config,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0xE0000, 0xFFFFFFFFU);
  ASSERT_TRUE(f.flash.program_word(0xE0000, 0x12345678).has_value());
  EXPECT_EQ(f.mmio.peek(0xE0000), 0x12345678U);
  EXPECT_EQ(f.mmio.peek(0x4001E504), 0U);
  EXPECT_EQ(f.mmio.peek(0x40000510), 0xF19U);
  EXPECT_EQ(f.mmio.flash_writes, 1U);
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::flash_mutation));
  EXPECT_FALSE(f.flash.program_word(0xE0000, 0)
                   .has_value()); // no repeated word programming
}
TEST(
    nrf_nvmc,
    reserved_boundaries_alignment_and_protected_ranges_are_checked_before_writes,
    "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  EXPECT_FALSE(f.flash.program_word(0xE0001, 0).has_value());
  EXPECT_FALSE(f.flash.program_word(0xEC000, 0).has_value());
  EXPECT_FALSE(f.flash.program_word(0x26000, 0).has_value());
  EXPECT_FALSE(f.flash.erase_page(0xE0004).has_value());
  EXPECT_FALSE(f.flash.erase_page(0xF4000).has_value());
  EXPECT_EQ(f.mmio.flash_writes, 0U);
  EXPECT_EQ(f.mmio.erases, 0U);
}
TEST(nrf_nvmc, protected_overlap_and_invalid_geometry_cannot_be_initialized,
     "REQ-0072") {
  fixture f;
  const std::array protected_ranges{
      mcu::nrf52840::memory::region{0xE0000, 4096}};
  EXPECT_FALSE(
      f.flash.initialize({0xE0000, 4096}, protected_ranges).has_value());
  EXPECT_FALSE(f.flash.initialize({0xE0001, 4096}, {}).has_value());
  EXPECT_FALSE(f.flash.initialize({0xFF000, 8192}, {}).has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::nvmc));
}
TEST(nrf_nvmc, page_erase_and_erased_all_ones_noop_respect_nwrite_limit,
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0xE0000, 0);
  ASSERT_TRUE(f.flash.erase_page(0xE0000).has_value());
  EXPECT_EQ(f.mmio.peek(0xE0000), 0xFFFFFFFFU);
  EXPECT_EQ(f.mmio.erases, 1U);
  ASSERT_TRUE(f.flash.program_word(0xE0000, 0xFFFFFFFFU).has_value());
  EXPECT_EQ(f.mmio.flash_writes, 0U);
}
TEST(nrf_nvmc, preexisting_pofwarn_or_shared_mutation_prevents_programming,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0xE0000, 0xFFFFFFFFU);
  f.mmio.poke(0x40000108, 1);
  EXPECT_FALSE(f.flash.program_word(0xE0000, 0).has_value());
  EXPECT_EQ(f.mmio.flash_writes, 0U);
  EXPECT_EQ(f.mmio.peek(0x40000510), 0xF19U);
  f.mmio.poke(0x40000108, 0);
  auto guard = f.resources.claim(f.execution, peripheral::flash_mutation, &f);
  ASSERT_TRUE(guard.has_value());
  EXPECT_FALSE(f.flash.program_word(0xE0000, 0).has_value());
  ASSERT_TRUE(f.resources.release(f.execution, *guard).has_value());
}
TEST(nrf_nvmc,
     pofwarn_race_after_disabling_blocks_nvmc_and_retains_recoverable_handoff,
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0xE0000, 0xFFFFFFFFU);
  f.mmio.pof_during_disable = true;
  EXPECT_FALSE(f.flash.program_word(0xE0000, 0).has_value());
  EXPECT_EQ(f.mmio.flash_writes, 0U);
  EXPECT_TRUE(f.power.power_fail_suspended_by(&f.flash));
  EXPECT_FALSE(f.power.shutdown().has_value());
  EXPECT_FALSE(f.power.power_fail(4, true).has_value());
  EXPECT_FALSE(f.power.consume(power_event::power_fail).has_value());
  ASSERT_TRUE(f.flash.shutdown().has_value());
  EXPECT_EQ(f.mmio.peek(0x40000510), 0xF19U);
}
TEST(nrf_nvmc,
     timeout_preserves_write_mode_power_handoff_and_mutation_until_ready,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0xE0000, 0xFFFFFFFFU);
  f.mmio.busy_after_write = true;
  f.execution.set_time(0xFFFFFFFC);
  EXPECT_FALSE(f.flash.program_word(0xE0000, 0).has_value());
  EXPECT_TRUE(f.flash.faulted());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::flash_mutation));
  EXPECT_TRUE(f.power.power_fail_suspended_by(&f.flash));
  EXPECT_FALSE(f.flash.shutdown().has_value());
  f.mmio.poke(0x4001E400, 1);
  ASSERT_TRUE(f.flash.shutdown().has_value());
  EXPECT_EQ(f.mmio.peek(0x4001E504), 0U);
  EXPECT_EQ(f.mmio.peek(0x40000510), 0xF19U);
}
TEST(nrf_nvmc,
     config_error_after_commit_is_not_success_and_shutdown_recovers_mode,
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.mmio.poke(0xE0000, 0xFFFFFFFFU);
  f.mmio.failed_address = 0x4001E504;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  EXPECT_FALSE(f.flash.program_word(0xE0000, 0).has_value());
  EXPECT_EQ(f.mmio.flash_writes, 0U);
  EXPECT_TRUE(f.flash.faulted());
  ASSERT_TRUE(f.flash.shutdown().has_value());
  EXPECT_EQ(f.mmio.peek(0x4001E504), 0U);
}
TEST(nrf_nvmc,
     isr_mutation_is_rejected_and_shared_power_owner_cannot_be_released,
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.initialize());
  f.execution.set_isr(true);
  EXPECT_FALSE(f.flash.erase_page(0xE0000).has_value());
  f.execution.set_isr(false);
  int client{};
  ASSERT_TRUE(f.power.suspend_power_fail(&client).has_value());
  EXPECT_FALSE(f.power.shutdown().has_value());
  EXPECT_FALSE(f.power.restore_power_fail(&f).has_value());
  ASSERT_TRUE(f.power.restore_power_fail(&client).has_value());
}
} // namespace
