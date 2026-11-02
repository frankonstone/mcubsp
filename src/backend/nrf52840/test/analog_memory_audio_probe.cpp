// @req-file REQ-0063 REQ-0064 REQ-0066 REQ-0067 REQ-0072 REQ-0062
#include <mcubsp/backend/nrf52840/acl.hpp>
#include <mcubsp/backend/nrf52840/comparator.hpp>
#include <mcubsp/backend/nrf52840/factory.hpp>
#include <mcubsp/backend/nrf52840/i2s.hpp>
#include <mcubsp/backend/nrf52840/io_analog.hpp>
#include <mcubsp/backend/nrf52840/mwu.hpp>
#include <mcubsp/backend/nrf52840/nvmc.hpp>
#include <mcubsp/backend/nrf52840/pdm.hpp>
#include <mcubsp/backend/nrf52840/provisioning.hpp>
#include <mcubsp/backend/nrf52840/qspi.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
namespace {
using namespace mcubsp;
namespace n = backend::nrf52840;
using C = n::system_context<core::direct_mmio, execution::fake_policy>;
using B = n::dma_buffer_pool<execution::fake_policy, 16>;
template <unsigned I>
void pwm(C &c, n::clock_controller<C> &clocks, B &buffers) {
  using Binding = n::pwm_binding<I, {0, 15}, {0, 20}, {0, 21}, {0, 22}>;
  n::pwm_controller<Binding, C, B> p{c, clocks, buffers};
  auto timing = n::solve_pwm_timing({16'000'000, 1000, 0,15});
  if (timing) {
    (void)p.initialize(*timing);
    std::array<std::uint16_t, 4> samples{1, 2, 3, 4};
    auto op = p.play({samples});
    if (op) {
      (void)p.interrupt_delivery(true);
      (void)p.poll();
      (void)p.next_step();
      (void)op->cancel();
      (void)op->take();
    }
  }
  (void)p.shutdown();
}
} // namespace
void nrf_analog_memory_audio_probe(C &c, n::clock_controller<C> &clocks,
                                   B &buffers, n::power_controller<C> &power) {
  pwm<0>(c, clocks, buffers);
  pwm<1>(c, clocks, buffers);
  pwm<2>(c, clocks, buffers);
  pwm<3>(c, clocks, buffers);
  n::saadc_controller<C, B> adc{c, clocks, buffers};
  std::array<n::saadc_channel_configuration, 1> channels{{{}}};
  std::array<std::int16_t, 8> samples{};
  (void)adc.initialize(channels);
  auto cal = adc.calibrate();
  if (cal) {
    (void)adc.poll();
    (void)cal->cancel();
    (void)cal->take();
  }
  auto scan = adc.sample(samples);
  if (scan) {
    (void)adc.poll();
    (void)scan->cancel();
    (void)scan->take();
  }
  (void)adc.take_buffer();
  (void)adc.take_limits();
  (void)adc.shutdown();
  n::comp_controller<C> comp{c, clocks};
  (void)comp.initialize({});
  (void)comp.start();
  (void)comp.poll();
  (void)comp.sample();
  (void)comp.shutdown();
  n::lpcomp_controller<C> lpcomp{c, clocks};
  (void)lpcomp.initialize({});
  (void)lpcomp.start();
  (void)lpcomp.poll();
  (void)lpcomp.prepare_system_off();
  (void)lpcomp.shutdown();
  using QB =
      n::qspi_binding<{0, 19}, {0, 17}, {0, 20}, {0, 21}, {0, 22}, {0, 23}>;
  n::qspi_controller<QB, C, B> qspi{c, clocks, buffers};
  (void)qspi.initialize({.capacity_bytes = 0x100000,
                         .maximum_clock_hz = 8000000,
                         .operation_timeout = 64000000},
                        {}, {0x80000, 0x80000});
  std::array<std::uint32_t, 32> tx{}, rx{}, tailtx{}, tailrx{};
  (void)qspi.read(0, rx);
  (void)qspi.poll();
  (void)qspi.cancel();
  (void)qspi.program(0x80000, tx);
  (void)qspi.cancel();
  (void)qspi.erase(0x80000, 4096);
  (void)qspi.cancel();
  (void)qspi.custom({.opcode = 0x9F, .size = 3});
  (void)qspi.cancel();
  auto map = qspi.borrow_xip(0, 128, {true, false});
  if (map) {
    (void)qspi.copy_xip_words(*map, 0, rx);
    (void)qspi.return_xip(*map);
  }
  (void)qspi.shutdown();
  using IB = n::i2s_binding<{0, 20}, {0, 21}, {0, 22}, {0, 23}, {0, 24}>;
  n::i2s_controller<IB, C, B> i2s{c, clocks, buffers};
  (void)i2s.initialize();
  (void)i2s.start({tx, rx}, {tailtx, tailrx});
  (void)i2s.poll();
  (void)i2s.interrupt_mode(true);
  (void)i2s.take_completed();
  (void)i2s.cancel();
  (void)i2s.shutdown();
  using PB = n::pdm_binding<{0, 17}, {0, 19}>;
  n::pdm_controller<PB, C, B> pdm{c, clocks, buffers};
  (void)pdm.initialize();
  std::array<std::int16_t, 32> audio{}, tail{};
  (void)pdm.start(audio, tail);
  (void)pdm.poll();
  (void)pdm.interrupt_mode(true);
  (void)pdm.take_completed();
  (void)pdm.cancel();
  (void)pdm.shutdown();
  n::factory_information<C> factory{c};
  (void)factory.geometry();
  (void)factory.identity();
  (void)factory.device_address();
  (void)factory.temperature_coefficients();
  (void)factory.nfc_header();
  n::memory_watch_controller<C> watch{c};
  (void)watch.initialize();
  (void)watch.watch(0, {0x20001000, 16}, true, true);
  (void)watch.watch_peripherals(0, 1, true, true);
  (void)watch.poll();
  (void)watch.shutdown();
  n::access_control_controller<C> acl{c};
  (void)acl.initialize({0xE0000, 4096}, {});
  (void)acl.protect(0, {0xE0000, 4096}, n::acl_protection::write);
  (void)acl.recover();
  (void)acl.shutdown();
  n::uicr_provisioner<C, decltype(power)> uicr{c, power};
  (void)uicr.initialize({.customer_words = true});
  (void)uicr.program_word(0x80, 0x12345678);
  (void)uicr.shutdown();
  n::access_port_controller<C> access{c};
  (void)access.state();
  (void)access.disable_software_protection();
}
