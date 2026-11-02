#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/acl.hpp>
#include <mcubsp/backend/nrf52840/factory.hpp>
#include <mcubsp/backend/nrf52840/mwu.hpp>
#include <mcubsp/backend/nrf52840/nvmc.hpp>
#include <mcubsp/backend/nrf52840/provisioning.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_memory : public nrf52840_test::fake_system {
public:
  execution::fake_policy *clock{};
  bool busy_after_program{};
  std::optional<std::uintptr_t> failed_write;
  unsigned uicr_writes{}, uicr_erases{}, partial_erases{};
  std::array<bool, 24> acl_written{};
  template <class T>
  std::expected<T, core::mmio_error> read(std::uintptr_t a) noexcept {
    if (clock)
      clock->advance();
    return nrf52840_test::fake_system::read<T>(a);
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    bool write_fault = failed_write == a;
    if (write_fault)
      failed_write.reset();
    auto previous = peek(a);
    auto r = nrf52840_test::fake_system::write<T>(a, v);
    if (!r && !fail_after_commit)
      return r;
    if (a == 0x40020514)
      poke(0x40020510, peek(0x40020510) | v);
    if (a == 0x40020518)
      poke(0x40020510, peek(0x40020510) & ~v);
    if (a >= 0x40020400 && a <= 0x4002040C)
      poke(a, previous & ~v);
    if (a >= 0x4001E800 && a <= 0x4001E878 && (a & 15U) < 12) {
      auto slot = (a - 0x4001E800) / 16 * 3 + (a & 15U) / 4;
      if (acl_written[slot])
        poke(a, previous);
      acl_written[slot] = true;
    }
    if (a >= 0x10001000 && a < 0x10002000 && peek(0x4001E504) == 1) {
      ++uicr_writes;
      poke(a, previous & v);
      if (busy_after_program)
        poke(0x4001E400, 0);
    }
    if (a == 0x4001E514 && v && peek(0x4001E504) == 2) {
      ++uicr_erases;
      for (unsigned o = 0; o < 4096; o += 4)
        poke(0x10001000 + o, 0xFFFFFFFF);
    }
    if (a == 0x4001E518 && peek(0x4001E504) == 2)
      ++partial_erases;
    if (write_fault)
      return std::unexpected(core::mmio_error::invalid_value);
    return r;
  }
};
struct fixture {
  fake_memory mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_memory> gpio{mmio, resources};
  system_context<fake_memory, execution::fake_policy> context{mmio, execution,
                                                              gpio};
  clock_controller<decltype(context)> clocks{context};
  power_controller<decltype(context)> power{context, clocks};
  nvmc_controller<decltype(context), decltype(power), 16> flash{context, power,
                                                                100};
  uicr_provisioner<decltype(context), decltype(power), 16> uicr{context, power,
                                                                100};
  access_control_controller<decltype(context)> acl{context};
  memory_watch_controller<decltype(context)> watch{context};
  factory_information<decltype(context)> factory{context};
  access_port_controller<decltype(context)> access{context};
  fixture() {
    mmio.clock = &execution;
    mmio.poke(0x10000100, 0x52840);
    mmio.poke(0x10000104, 0x41414630);
    mmio.poke(0x40000550, 255);
    mmio.poke(0x4001E400, 1);
    mmio.poke(0x40000510, 0xF19);
    mmio.poke(0x10001208, 0xFFFFFFFF);
    mmio.poke(0x10001080, 0xFFFFFFFF);
  }
  ~fixture() {
    (void)uicr.shutdown();
    (void)flash.shutdown();
    (void)acl.shutdown();
    (void)watch.shutdown();
    (void)power.shutdown();
    (void)clocks.shutdown();
  }
  bool power_on() {
    return clocks.initialize().has_value() && power.initialize().has_value();
  }
};
TEST(nrf_memory, factory_queries_are_read_only_and_preserve_native_word_layout,
     "REQ-0072") {
  fixture f;
  f.mmio.poke(0x10000010, 4096);
  f.mmio.poke(0x10000014, 256);
  f.mmio.poke(0x1000010C, 256);
  f.mmio.poke(0x10000110, 1024);
  f.mmio.poke(0x100000A0, 1);
  f.mmio.poke(0x100000A4, 0x44332211);
  f.mmio.poke(0x100000A8, 0x6655);
  auto before = f.mmio.writes;
  auto geometry = f.factory.geometry();
  ASSERT_TRUE(geometry.has_value());
  EXPECT_EQ(geometry->page_bytes, 4096U);
  EXPECT_EQ(geometry->flash_kib, 1024U);
  auto address = f.factory.device_address();
  ASSERT_TRUE(address.has_value());
  EXPECT_EQ(address->bytes[0], 0x11U);
  EXPECT_EQ(address->bytes[5], 0x66U);
  EXPECT_TRUE(address->random);
  EXPECT_EQ(f.mmio.writes, before);
}
TEST(nrf_memory,
     mwu_inclusive_ranges_and_peripheral_subregions_report_cpu_events,
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.watch.initialize().has_value());
  ASSERT_TRUE(f.watch.watch(0, {0x20001000, 16}, true, true).has_value());
  EXPECT_EQ(f.mmio.peek(0x40020604), 0x2000100FU);
  ASSERT_TRUE(
      f.watch.watch_peripherals(1, 0x12345678, true, false).has_value());
  EXPECT_EQ(f.mmio.peek(0x400206D8), 0x12345678U);
  f.mmio.poke(0x40020100, 1);
  f.mmio.poke(0x4002016C, 1);
  f.mmio.poke(0x4002040C, 8);
  auto events = f.watch.poll();
  ASSERT_TRUE(events.has_value());
  EXPECT_EQ(events->writes, 1U);
  EXPECT_EQ(events->reads, 1U << 5);
  EXPECT_EQ(events->peripheral_reads[1], 8U);
  EXPECT_EQ(f.mmio.peek(0x4002040C), 0U);
  ASSERT_TRUE(f.watch.shutdown().has_value());
  EXPECT_EQ(f.mmio.peek(0x40020510), 0U);
}
TEST(nrf_memory,
     mwu_rejects_flash_code_alias_and_retains_ownership_after_disable_fault,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.watch.initialize().has_value());
  EXPECT_FALSE(f.watch.watch(0, {0x800000, 16}, true, true).has_value());
  EXPECT_FALSE(f.watch.watch(0, {0x1000, 16}, true, true).has_value());
  ASSERT_TRUE(f.watch.watch(3, {0x20001000, 16}, true, false).has_value());
  f.mmio.failed_address = 0x40020518;
  f.mmio.fail_once = f.mmio.fail_after_commit = true;
  EXPECT_FALSE(f.watch.shutdown().has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::mwu));
  ASSERT_TRUE(f.watch.shutdown().has_value());
}
TEST(nrf_memory,
     acl_write_once_protection_is_confined_to_explicit_reserved_storage,
     "REQ-0072", "REQ-0062") {
  fixture f;
  const std::array protected_ranges{mcu::nrf52840::memory::region{0, 0x26000}};
  ASSERT_TRUE(f.acl.initialize({0xE0000, 8192}, protected_ranges).has_value());
  EXPECT_FALSE(
      f.acl.protect(0, {0xF4000, 4096}, acl_protection::all).has_value());
  ASSERT_TRUE(
      f.acl.protect(0, {0xE0000, 4096}, acl_protection::write).has_value());
  auto status = f.acl.read(0);
  ASSERT_TRUE(status.has_value());
  EXPECT_EQ(status->permissions, 2U);
  EXPECT_FALSE(
      f.acl.protect(0, {0xE1000, 4096}, acl_protection::all).has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::flash_mutation));
  ASSERT_TRUE(f.acl.shutdown().has_value());
  EXPECT_EQ(f.mmio.peek(0x4001E804), 4096U);
}
TEST(nrf_memory, acl_ambiguous_final_write_keeps_mutation_guard_until_readback,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.acl.initialize({0xE0000, 4096}, {}).has_value());
  f.mmio.failed_write = 0x4001E808;
  EXPECT_FALSE(
      f.acl.protect(0, {0xE0000, 4096}, acl_protection::all).has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::flash_mutation));
  ASSERT_TRUE(f.acl.recover().has_value());
  ASSERT_TRUE(f.acl.last_observed().has_value());
  EXPECT_EQ(f.acl.last_observed()->permissions, 6U);
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::flash_mutation));
}
TEST(nrf_memory, uicr_scope_preserves_bootloader_pin_roles_and_debug_by_default,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.power_on());
  ASSERT_TRUE(f.uicr.initialize({.customer_words = true}).has_value());
  EXPECT_FALSE(f.uicr.program_word(0x14, 0).has_value());
  EXPECT_FALSE(f.uicr.program_word(0x208, 0).has_value());
  EXPECT_FALSE(f.uicr.program_word(0x20C, 0).has_value());
  EXPECT_FALSE(f.uicr.erase_uicr().has_value());
  ASSERT_TRUE(f.uicr.program_word(0x80, 0x12345678).has_value());
  EXPECT_EQ(f.mmio.uicr_writes, 1U);
  EXPECT_TRUE(f.uicr.reset_required());
  EXPECT_EQ(f.mmio.peek(0x4001E504), 0U);
  EXPECT_EQ(f.mmio.peek(0x40000510), 0xF19U);
  EXPECT_FALSE(f.uicr.program_word(0x80, 0).has_value());
}
TEST(nrf_memory, uicr_timeout_keeps_power_and_flash_guard_until_ready,
     "REQ-0072", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.power_on());
  ASSERT_TRUE(f.uicr.initialize({.customer_words = true}).has_value());
  f.mmio.busy_after_program = true;
  EXPECT_FALSE(f.uicr.program_word(0x80, 0x1234).has_value());
  EXPECT_TRUE(f.resources.claimed(f.execution, peripheral::flash_mutation));
  EXPECT_EQ(f.mmio.peek(0x4001E504), 1U);
  EXPECT_FALSE(f.uicr.shutdown().has_value());
  f.mmio.poke(0x4001E400, 1);
  ASSERT_TRUE(f.uicr.shutdown().has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::flash_mutation));
  EXPECT_EQ(f.mmio.peek(0x4001E504), 0U);
}
TEST(nrf_memory,
     access_port_build_gate_distinguishes_hardware_and_software_controls,
     "REQ-0072") {
  fixture f;
  f.mmio.poke(0x10001208, 0xFFFFFF5A);
  auto state = f.access.state();
  ASSERT_TRUE(state.has_value());
  EXPECT_TRUE(state->revision == access_port_revision::hardware_and_software);
  ASSERT_TRUE(f.access.disable_software_protection().has_value());
  EXPECT_EQ(f.mmio.peek(0x40000558), 0x5AU);
  EXPECT_FALSE(f.access.force_protection(false).has_value());
  ASSERT_TRUE(f.access.force_protection(true).has_value());
  EXPECT_FALSE(f.access.disable_software_protection().has_value());
  f.mmio.poke(0x10000104, 0x41414430);
  EXPECT_FALSE(f.access.state()->software_disable.has_value());
  EXPECT_FALSE(f.access.disable_software_protection().has_value());
  f.mmio.poke(0x10000104, 0xFFFFFFFF);
  EXPECT_FALSE(f.access.state().has_value());
}
TEST(nrf_memory,
     partial_erase_and_instruction_cache_have_bounded_explicit_interfaces,
     "REQ-0072") {
  fixture f;
  ASSERT_TRUE(f.power_on());
  ASSERT_TRUE(f.flash.initialize({0xE0000, 4096}, {}).has_value());
  EXPECT_FALSE(f.flash.erase_page_partial(0xE0000, 0).has_value());
  EXPECT_FALSE(f.flash.erase_page_partial(0xF4000, 10).has_value());
  ASSERT_TRUE(f.flash.erase_page_partial(0xE0000, 10).has_value());
  EXPECT_EQ(f.mmio.partial_erases, 1U);
  EXPECT_EQ(f.mmio.peek(0x4001E51C), 10U);
  ASSERT_TRUE(f.flash.configure_instruction_cache(true, true).has_value());
  EXPECT_EQ(f.mmio.peek(0x4001E540), 0x101U);
  f.mmio.poke(0x4001E548, 42);
  f.mmio.poke(0x4001E54C, 5);
  auto stats = f.flash.instruction_cache_statistics(true);
  ASSERT_TRUE(stats.has_value());
  EXPECT_EQ(stats->hits, 42U);
  EXPECT_EQ(stats->misses, 5U);
  EXPECT_EQ(f.mmio.peek(0x4001E548), 0U);
}
} // namespace
