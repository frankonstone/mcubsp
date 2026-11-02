#include "fake_analog.hpp"
#include <mcubsp/backend/nrf52840/qspi.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_qspi : public nrf52840_test::fake_analog {
public:
  bool respond{true}, dpm_responds{true};
  unsigned status_busy_reads{}, commands{}, activations{};
  std::optional<std::uintptr_t> fail_write;
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto r = fake_analog::write<T>(a, v);
    if (!r)
      return r;
    if (a == 0x40029000 && v) {
      ++activations;
      if (respond)
        poke(0x40029100, 1);
    }
    if ((a == 0x40029004 || a == 0x40029008 || a == 0x4002900C) && v && respond)
      poke(0x40029100, 1);
    if (a == 0x40029634) {
      ++commands;
      if ((v & 255) == 5) {
        poke(0x40029638, status_busy_reads ? 1 : 0);
        if (status_busy_reads)
          --status_busy_reads;
      } else {
        poke(0x40029638, 0x44332211);
        poke(0x4002963C, 0x88776655);
      }
      if (respond)
        poke(0x40029100, 1);
    }
    if (a == 0x40029600 && dpm_responds)
      poke(0x40029604, (v & (1U << 24)) ? 4 : 0);
    if (fail_write == a) {
      fail_write.reset();
      return std::unexpected(core::mmio_error::invalid_value);
    }
    return {};
  }
};
using binding =
    qspi_binding<core::pin_id{0, 19}, core::pin_id{0, 17}, core::pin_id{0, 20},
                 core::pin_id{0, 21}, core::pin_id{0, 22}, core::pin_id{0, 23}>;
struct fixture {
  fake_qspi mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_qspi> gpio{mmio, resources};
  system_context<fake_qspi, execution::fake_policy> context{mmio, execution,
                                                            gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy> buffers{resources, execution};
  qspi_controller<binding, decltype(context), decltype(buffers),
                  nrf52840_test::analog_address, 64>
      qspi{context, clocks, buffers, 100};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x10000130, 8);
    mmio.poke(0x10000134, 5);
  }
  ~fixture() {
    mmio.respond = true;
    (void)qspi.shutdown();
    (void)clocks.shutdown();
  }
  qspi_device device() {
    return {.capacity_bytes = 0x100000,
            .maximum_clock_hz = 32'000'000,
            .deep_power_down = true,
            .dpm_enter = 160,
            .dpm_exit = 320,
            .operation_timeout = 1000};
  }
  bool init() {
    return clocks.initialize() &&
           qspi.initialize(device(), {}, {0x80000, 0x80000});
  }
  auto span() {
    return std::span<std::uint32_t>{
        reinterpret_cast<std::uint32_t *>(0x20001000), 4};
  }
};
TEST(nrf_qspi, configuration_geometry_and_mutation_bounds, "REQ-0066",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  EXPECT_EQ(f.mmio.peek(0x40029600) >> 28, 3U);
  EXPECT_EQ(f.mmio.peek(0x40029614), 0x14000A0U);
  EXPECT_FALSE(f.qspi.program(0, f.span()).has_value());
  EXPECT_FALSE(f.qspi.erase(0x80004, 4096).has_value());
  EXPECT_FALSE(f.qspi.read(3, f.span()).has_value());
  EXPECT_FALSE(f.qspi.read(0xFFFFC, f.span()).has_value());
  ASSERT_TRUE(f.qspi.program(0x80000, f.span()).has_value());
  auto p = f.qspi.poll();
  ASSERT_TRUE(p.has_value());
  EXPECT_FALSE(p->has_value());
  auto done = f.qspi.poll();
  ASSERT_TRUE(done.has_value());
  ASSERT_TRUE(done->has_value());
  EXPECT_EQ((**done).bytes, 16U);
  ASSERT_TRUE(f.qspi.shutdown().has_value());
  EXPECT_TRUE(f.mmio.wrote(0x40029010, 1));
  EXPECT_TRUE(f.mmio.wrote(0x40029054, 1));
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::qspi));
}
TEST(nrf_qspi, erase_ready_is_not_external_flash_completion, "REQ-0066") {
  fixture f;
  ASSERT_TRUE(f.init());
  f.mmio.status_busy_reads = 2;
  ASSERT_TRUE(f.qspi.erase(0x80000, 4096).has_value());
  for (unsigned i = 0; i < 3; ++i) {
    auto p = f.qspi.poll();
    ASSERT_TRUE(p.has_value());
    EXPECT_FALSE(p->has_value());
    EXPECT_TRUE(f.qspi.busy());
  }
  auto p = f.qspi.poll();
  ASSERT_TRUE(p.has_value());
  EXPECT_TRUE(p->has_value());
}
TEST(nrf_qspi, timeout_and_failed_start_keep_dma_until_real_ready, "REQ-0066",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  f.mmio.respond = false;
  f.mmio.fail_write = 0x40029004;
  EXPECT_FALSE(f.qspi.read(0, f.span()).has_value());
  EXPECT_FALSE(f.qspi.cancel().has_value());
  EXPECT_TRUE(f.qspi.busy());
  auto owner = f.resources.claim(f.execution, peripheral::pdm, &f);
  ASSERT_TRUE(owner.has_value());
  EXPECT_FALSE(f.buffers
                   .prepare(*owner, mcu::nrf52840::dma_constraints{100, 4, 4},
                            0x20001000, 16, 4, dma_access::write)
                   .has_value());
  f.mmio.poke(0x40029100, 1);
  ASSERT_TRUE(f.qspi.cancel().has_value());
  auto lease =
      f.buffers.prepare(*owner, mcu::nrf52840::dma_constraints{100, 4, 4},
                        0x20001000, 16, 4, dma_access::write);
  ASSERT_TRUE(lease.has_value());
  ASSERT_TRUE(f.buffers.discard_prepared(*lease).has_value());
  ASSERT_TRUE(f.resources.release(f.execution, *owner).has_value());
}
TEST(nrf_qspi, custom_encodes_bytes_and_long_frame_closes_explicitly,
     "REQ-0066") {
  fixture f;
  ASSERT_TRUE(f.init());
  ASSERT_TRUE(f.qspi.custom({.opcode = 0x9F, .size = 8}).has_value());
  auto p = f.qspi.poll();
  ASSERT_TRUE(p && p->has_value());
  EXPECT_EQ((**p).response[0], 0x11U);
  EXPECT_EQ((**p).response[7], 0x88U);
  ASSERT_TRUE(f.qspi
                  .custom({.opcode = 0xA5,
                           .data = {1, 2, 3},
                           .size = 3,
                           .long_frame = true})
                  .has_value());
  EXPECT_EQ(f.mmio.peek(0x40029634), 0x134A5U);
  ASSERT_TRUE(f.qspi.poll().has_value());
  EXPECT_FALSE(f.qspi.read(0, f.span()).has_value());
  ASSERT_TRUE(f.qspi.cancel().has_value());
  EXPECT_TRUE(f.mmio.peek(0x40029634) & (1U << 17));
  ASSERT_TRUE(f.qspi.deep_power_down(true).has_value());
  EXPECT_FALSE(f.qspi.read(0, f.span()).has_value());
  ASSERT_TRUE(f.qspi.deep_power_down(false).has_value());
}
TEST(nrf_qspi, xip_lifetime_excludes_mutation_and_requires_execution_handoff,
     "REQ-0066", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  EXPECT_FALSE(f.qspi.borrow_xip(0, 16, {}).has_value());
  auto mapping = f.qspi.borrow_xip(0, 16, {true, true});
  ASSERT_TRUE(mapping.has_value());
  EXPECT_FALSE(f.qspi.program(0x80000, f.span()).has_value());
  EXPECT_FALSE(f.qspi.custom({.opcode = 0x9F, .size = 3}).has_value());
  EXPECT_FALSE(f.qspi.shutdown().has_value());
  EXPECT_FALSE(f.qspi.return_xip(*mapping).has_value());
  std::array<std::uint32_t, 4> copied{};
  f.mmio.poke(0x12000000, 0x12345678);
  ASSERT_TRUE(f.qspi.copy_xip_words(*mapping, 0, copied).has_value());
  EXPECT_EQ(copied[0], 0x12345678U);
  ASSERT_TRUE(f.qspi.return_xip(*mapping, true).has_value());
  EXPECT_FALSE(f.qspi.return_xip(*mapping, true).has_value());
  auto before = f.mmio.activations;
  ASSERT_TRUE(f.qspi.custom({.opcode = 0x9F, .size = 3}).has_value());
  EXPECT_EQ(f.mmio.activations, before + 1);
  ASSERT_TRUE(f.qspi.cancel().has_value());
}
TEST(nrf_qspi, unreviewed_silicon_and_missing_quad_pins_are_rejected,
     "REQ-0066") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  f.mmio.poke(0x10000134, 0);
  EXPECT_FALSE(f.qspi.initialize(f.device(), {}, {}).has_value());
  EXPECT_FALSE(f.resources.claimed(f.execution, peripheral::qspi));
  f.mmio.poke(0x10000134, 5);
  auto device = f.device();
  device.read = qspi_read_protocol::quad_io;
  EXPECT_FALSE(f.qspi.initialize(device, {}, {}).has_value());
}

TEST(nrf_qspi, ambiguous_power_down_blocks_commands_until_status_recovery,
     "REQ-0066", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.init());
  f.mmio.dpm_responds = false;
  EXPECT_FALSE(f.qspi.deep_power_down(true).has_value());
  EXPECT_TRUE(f.qspi.busy());
  EXPECT_FALSE(
      f.qspi.borrow_xip(0, 16, {.reviewed_linker_and_read_accesses = true})
          .has_value());
  EXPECT_FALSE(f.qspi.cancel().has_value());
  f.mmio.poke(0x40029604, 4);
  ASSERT_TRUE(f.qspi.cancel().has_value());
  EXPECT_FALSE(f.qspi.busy());
  f.mmio.dpm_responds = true;
  ASSERT_TRUE(f.qspi.deep_power_down(false).has_value());
}
} // namespace
