// @req-file REQ-0068 REQ-0069 REQ-0070 REQ-0071 REQ-0062 REQ-0074
#include <mcubsp/backend/nrf52840/aar.hpp>
#include <mcubsp/backend/nrf52840/ccm.hpp>
#include <mcubsp/backend/nrf52840/cryptocell.hpp>
#include <mcubsp/backend/nrf52840/ecb.hpp>
#include <mcubsp/backend/nrf52840/nfct.hpp>
#include <mcubsp/backend/nrf52840/radio.hpp>
#include <mcubsp/backend/nrf52840/usb_device.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
namespace {
using namespace mcubsp;
namespace n = backend::nrf52840;
using C = n::system_context<core::direct_mmio, execution::fake_policy>;
using B = n::dma_buffer_pool<execution::fake_policy, 24>;
using U = n::usbd_controller<C, B>;
} // namespace
void nrf_usb_crypto_radio_probe(C &c, n::clock_controller<C> &clocks,
                                B &buffers) {
  alignas(4) std::array<std::uint8_t, 512> input{}, output{}, scratch{};
  n::ecb_controller<C, B> ecb{c, clocks, buffers};
  n::ecb_data block{};
  (void)ecb.initialize();
  (void)ecb.start(block);
  (void)ecb.interrupt_delivery(true);
  (void)ecb.poll();
  (void)ecb.cancel();
  (void)ecb.shutdown();
  n::aar_controller<C, B> aar{c, clocks, buffers};
  std::array<n::identity_resolving_key, 1> irk{};
  (void)aar.initialize();
  n::aar_packet aar_address{};
  (void)aar.resolve(irk, aar_address, scratch);
  (void)aar.interrupt_delivery(true);
  (void)aar.event_endpoint(0x100);
  (void)aar.task_endpoint(0);
  (void)aar.poll();
  (void)aar.cancel();
  (void)aar.shutdown();
  n::ccm_controller<C, B> ccm{c, clocks, buffers};
  n::ccm_nonce_configuration nonce{};
  (void)ccm.initialize();
  (void)ccm.start(nonce, input, output, scratch);
  (void)ccm.crypt();
  (void)ccm.override_rate(n::ccm_data_rate::mbit1);
  (void)ccm.interrupt_delivery(true);
  (void)ccm.event_endpoint(0x100);
  (void)ccm.task_endpoint(0);
  (void)ccm.poll();
  (void)ccm.cancel();
  (void)ccm.shutdown();
  n::cryptocell_controller<C, B> cc{c, clocks, buffers};
  (void)cc.initialize();
  (void)cc.aes({}, std::span{input}.first(16), std::span{output}.first(16));
  (void)cc.mac({.mode = n::cc_aes_mode::cmac}, {});
  (void)cc.hash(n::cc_hash_mode::sha1, input);
  (void)cc.hmac(n::cc_hash_mode::sha256, std::span{input}.first(16),
                std::span{input}.first(16), scratch);
  (void)cc.ccm({}, std::span{input}.first(13), {}, std::span{input}.first(16),
               std::span{output}.first(16), std::span{scratch}.first(8));
  (void)cc.chacha({}, input, output);
  (void)cc.poly1305_key();
  (void)cc.chacha_poly1305({}, {}, std::span{input}.first(64),
                           std::span{output}.first(64),
                           std::span{scratch}.first<16>());
  (void)cc.copy(input, output);
  (void)cc.pka_configure(256);
  std::array<std::uint32_t, 8> words{};
  (void)cc.pka_load(2, words);
  (void)cc.pka_execute({mcu::nrf52840::product::cc_pka::opcode_fields::
                            opcode_encoding::value_modexp});
  (void)cc.pka_read(4, words);
  (void)cc.entropy();
  (void)cc.rng_sram_write(0, words);
  (void)cc.rng_sram_read(0, words);
  (void)cc.interrupt_delivery(true);
  (void)cc.interrupt_status();
  (void)cc.recover();
  (void)cc.shutdown();
  n::radio_controller<C, B> radio{c, clocks, buffers};
  (void)radio.initialize();
  (void)radio.receive(output, false);
  (void)radio.transmit(input);
  (void)radio.poll();
  (void)radio.prepare_coded_turnaround(n::radio_phy::coded125);
  std::array<n::radio_device_address, 1> addresses{};
  (void)radio.device_addresses(addresses);
  (void)radio.match_mac_header(0, 0);
  (void)radio.bit_counter(32);
  (void)radio.clear_channel();
  (void)radio.energy_detect();
  (void)radio.interrupt_delivery(true);
  (void)radio.event_endpoint(0x100);
  (void)radio.task_endpoint(0);
  (void)radio.cancel();
  (void)radio.shutdown();
  n::radio_stack_reservation<C> stack{c};
  (void)stack.acquire(true);
  (void)stack.release(true);
  n::nfct_controller<C, B> nfct{c, clocks, buffers};
  (void)nfct.initialize({});
  (void)nfct.sense();
  (void)nfct.activate();
  (void)nfct.poll_events();
  (void)nfct.receive(output);
  (void)nfct.transmit(input);
  (void)nfct.poll_frame();
  (void)nfct.go_idle();
  (void)nfct.prepare_system_off();
  (void)nfct.interrupt_delivery(true);
  (void)nfct.cancel();
  (void)nfct.shutdown();
  n::readonly_type2_tag<> tag;
  (void)tag.initialize({}, {});
  (void)tag.respond(input);
  U usb{c, clocks, buffers};
  (void)usb.initialize();
  (void)usb.connect();
  (void)usb.configure_endpoint(8, true, n::usb_endpoint_type::isochronous, 512);
  (void)usb.configure_endpoint(8, false, n::usb_endpoint_type::isochronous,
                               512);
  (void)usb.transmit(8, input, 64);
  (void)usb.receive(8, output, 64);
  (void)usb.poll();
  (void)usb.take_completed(8, true);
  (void)usb.take_setup();
  (void)usb.take_events();
  (void)usb.control_status(1);
  (void)usb.control_stall(1);
  (void)usb.halt(1, true, false);
  (void)usb.halted(1, true);
  (void)usb.interrupt_delivery(true);
  (void)usb.enter_suspend();
  (void)usb.resume();
  usb.remote_wakeup_enabled(true);
  (void)usb.remote_wakeup();
  (void)usb.cancel_all();
  (void)usb.shutdown();
  (void)usb.request_bootloader({});
  n::usb_device<U, n::usb_profile::cdc_acm, 0xFFFF, 1> cdc{usb};
  (void)cdc.initialize();
  (void)cdc.poll();
  (void)cdc.write(input);
  (void)cdc.read(output);
  (void)cdc.notify_serial_state(3);
  (void)cdc.shutdown();
  n::usb_device<U, n::usb_profile::hid_keyboard, 0xFFFF, 2> hid{usb};
  (void)hid.initialize();
  (void)hid.poll();
  (void)hid.send_report({});
  (void)hid.shutdown();
}
