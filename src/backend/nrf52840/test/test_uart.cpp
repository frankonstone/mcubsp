#include "fake_uart.hpp"
#include <mcubsp/backend/nrf52840/uart.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
using binding=uart_binding<peripheral::uart0,{0,6},{0,8}>;
using mmio_type=nrf52840_test::fake_uart;
using context_type=system_context<mmio_type,execution::fake_policy>;
struct fixture {
    mmio_type mmio;execution::fake_policy execution;resource_registry resources;
    gpio_backend<mmio_type> gpio{mmio,resources};context_type context{mmio,execution,gpio};
    clock_controller<context_type> clocks{context};uart_controller<binding,context_type,16> serial{context,clocks,10};
    fixture() {mmio.clock=&execution;}
    ~fixture() {mmio.failed_address.reset();mmio.stop_responds=true;(void)serial.shutdown();(void)clocks.shutdown();}
    bool initialize() {
        auto timing=solve_uart_baud(peripheral::uart0,{16'000'000,115200,10000});
        return clocks.initialize().has_value() && serial.initialize(*timing).has_value();
    }
};
TEST(nrf_uart, initializes_hfxo_native_pins_and_registers_without_enabling_a_shared_personality,"REQ-0059","REQ-0062") {
    fixture f;ASSERT_TRUE(f.initialize());
    EXPECT_EQ(f.mmio.peek(binding::base+0x500),4U);
    EXPECT_EQ(f.mmio.peek(binding::base+0x524),0x01D7E000U);
    EXPECT_EQ(f.mmio.peek(binding::base+0x50C),6U);EXPECT_EQ(f.mmio.peek(binding::base+0x514),8U);
    EXPECT_EQ(f.clocks.hf_clients(),1U);EXPECT_TRUE(f.context.active_work());
    EXPECT_FALSE(f.resources.claim(f.execution,peripheral::uarte0,&f).has_value());
    ASSERT_TRUE(f.serial.shutdown().has_value());
    EXPECT_EQ(f.clocks.hf_clients(),0U);EXPECT_FALSE(f.context.active_work());
}
TEST(nrf_uart, transmit_waits_for_the_last_txdrdy_then_stops_before_publishing,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());const std::array<std::uint8_t,2> bytes{0x41,0x42};
    auto tx=f.serial.transmit(bytes);ASSERT_TRUE(tx.has_value());EXPECT_FALSE(tx->ready());
    f.mmio.poke(binding::base+0x11C,1);ASSERT_TRUE(f.serial.poll());
    EXPECT_FALSE(tx->ready());EXPECT_EQ(f.mmio.peek(binding::base+0x51C),0x42U);
    f.mmio.poke(binding::base+0x11C,1);ASSERT_TRUE(f.serial.poll());
    ASSERT_TRUE(tx->ready());EXPECT_TRUE(f.mmio.wrote(binding::base+0x00C,1));ASSERT_TRUE(tx->take().has_value());
}
TEST(nrf_uart, inherited_enable_and_foreign_pins_are_rejected_before_reconfiguration,"REQ-0059","REQ-0062") {
    fixture f;ASSERT_TRUE(f.clocks.initialize().has_value());
    auto timing=solve_uart_baud(peripheral::uart0,{16'000'000,115200,10000});
    f.mmio.poke(binding::base+0x500,8);f.mmio.trace_count=0;
    EXPECT_FALSE(f.serial.initialize(*timing).has_value());EXPECT_EQ(f.mmio.trace_count,0U);
    EXPECT_FALSE(f.resources.claimed(f.execution,peripheral::uart0));
    f.mmio.poke(binding::base+0x500,0);
    ASSERT_TRUE(f.gpio.claim(binding::rx,driver::gpio_direction::input).has_value());
    EXPECT_FALSE(f.serial.initialize(*timing).has_value());EXPECT_EQ(f.mmio.trace_count,0U);
    EXPECT_FALSE(f.gpio.claim(binding::rx,driver::gpio_direction::input).has_value());
    f.gpio.release(binding::rx);
    ASSERT_TRUE(f.serial.initialize(*timing).has_value());
}
TEST(nrf_uart, partial_pin_configuration_failure_quiesces_before_releasing_claims,"REQ-0059","REQ-0062") {
    fixture f;ASSERT_TRUE(f.clocks.initialize().has_value());
    f.mmio.failed_address=mcu::nrf52840::pin_configuration(binding::tx);f.mmio.fail_once=true;
    auto timing=solve_uart_baud(peripheral::uart0,{16'000'000,115200,10000});
    EXPECT_FALSE(f.serial.initialize(*timing).has_value());EXPECT_TRUE(f.serial.faulted());
    EXPECT_FALSE(f.resources.claimed(f.execution,peripheral::uarte0));
    EXPECT_EQ(f.clocks.hf_clients(),0U);
    auto next=f.resources.claim(f.execution,peripheral::uarte0,&f);ASSERT_TRUE(next.has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*next).has_value());
    ASSERT_TRUE(f.gpio.claim(binding::tx,driver::gpio_direction::output).has_value());f.gpio.release(binding::tx);
}
TEST(nrf_uart, receive_clears_ready_before_read_and_waits_for_receiver_timeout,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,2> bytes{};
    f.mmio.stop_responds=false;auto rx=f.serial.receive(bytes);ASSERT_TRUE(rx.has_value());
    f.mmio.inject(std::uint8_t{0x51});f.mmio.inject(std::uint8_t{0x52});
    ASSERT_TRUE(f.serial.poll());EXPECT_FALSE(rx->ready());ASSERT_TRUE(f.serial.poll());EXPECT_FALSE(rx->ready());
    EXPECT_FALSE(f.mmio.bad_rx_read_order);EXPECT_EQ(bytes[0],0x51U);EXPECT_EQ(bytes[1],0x52U);
    f.mmio.poke(binding::base+0x144,1);ASSERT_TRUE(f.serial.poll());ASSERT_TRUE(rx->ready());
    EXPECT_EQ(*rx->take(),2U);
}
TEST(nrf_uart, simultaneous_tx_rx_loopback_works_with_an_irq_owned_service_callback,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());f.mmio.automatic_tx=f.mmio.loopback=true;
    const std::array<std::uint8_t,3> out{1,2,3};std::array<std::uint8_t,3> in{};
    auto rx=f.serial.receive(in);ASSERT_TRUE(rx.has_value());auto tx=f.serial.transmit(out);ASSERT_TRUE(tx.has_value());
    ASSERT_TRUE(f.serial.interrupt_delivery(true).has_value());f.execution.set_isr(true);
    for(unsigned i=0;i<4;++i) (void)f.serial.poll();
    f.execution.set_isr(false);
    ASSERT_TRUE(rx->ready());ASSERT_TRUE(tx->ready());ASSERT_TRUE(rx->take().has_value());ASSERT_TRUE(tx->take().has_value());
    EXPECT_TRUE(in==out);EXPECT_FALSE(f.mmio.bad_rx_read_order);
}
TEST(nrf_uart, errors_stop_reception_and_report_the_copied_prefix_without_overwriting,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,4> bytes{};
    auto rx=f.serial.receive(bytes);ASSERT_TRUE(rx.has_value());f.mmio.inject(std::uint8_t{9});ASSERT_TRUE(f.serial.poll());
    f.mmio.poke(binding::base+0x480,2);f.mmio.poke(binding::base+0x124,1);ASSERT_TRUE(f.serial.poll());
    ASSERT_TRUE(rx->ready());auto result=rx->take();ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().cause==driver::uart_error::parity);EXPECT_EQ(f.serial.received_count(),1U);
    EXPECT_EQ(bytes[0],9U);EXPECT_EQ(bytes[1],0U);
}
TEST(nrf_uart, cancellation_has_wrapping_clock_deadline_and_retains_resources_on_failed_stop,"REQ-0059","REQ-0062") {
    fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,2> bytes{};
    auto rx=f.serial.receive(bytes);ASSERT_TRUE(rx.has_value());f.execution.set_time(0xFFFFFFFC);f.mmio.stop_responds=false;
    auto cancelled=rx->cancel();EXPECT_FALSE(cancelled.has_value());
    EXPECT_TRUE(rx->take().error().kind==core::transfer_error_kind::quiesce_failed);
    EXPECT_TRUE(f.serial.faulted());EXPECT_FALSE(f.serial.shutdown().has_value());
    EXPECT_FALSE(f.resources.claim(f.execution,peripheral::uarte0,&f).has_value());
    // The late original RXTO must be consumed without clearing/requiring a second acknowledgment.
    f.mmio.poke(binding::base+0x144,1);ASSERT_TRUE(f.serial.shutdown().has_value());
}
TEST(nrf_uart, observable_receive_errors_remain_distinct,"REQ-0059") {
    const std::array sources{1U,4U,8U,0U};
    const std::array expected{driver::uart_error::overrun,driver::uart_error::framing,
                              driver::uart_error::framing,driver::uart_error::transport};
    for (unsigned i=0;i<sources.size();++i) {
        fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,1> bytes{};
        auto rx=f.serial.receive(bytes);ASSERT_TRUE(rx.has_value());
        f.mmio.poke(binding::base+0x480,sources[i]);f.mmio.poke(binding::base+0x124,1);
        ASSERT_TRUE(f.serial.poll());auto result=rx->take();ASSERT_FALSE(result.has_value());
        EXPECT_TRUE(result.error().cause==expected[i]);
        EXPECT_EQ(f.serial.error_source(),sources[i]);
    }
}
TEST(nrf_uart, an_error_arriving_during_acknowledgment_is_preserved_in_native_status,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,2> bytes{};
    auto rx=f.serial.receive(bytes);ASSERT_TRUE(rx.has_value());
    f.mmio.poke(binding::base+0x480,2);f.mmio.poke(binding::base+0x124,1);f.mmio.error_during_ack=true;
    ASSERT_TRUE(f.serial.poll());auto result=rx->take();ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().cause==driver::uart_error::parity);EXPECT_EQ(f.serial.error_source(),6U);
}
TEST(nrf_uart, a_fifo_tail_beyond_the_finite_buffer_is_reported_as_overrun,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,1> bytes{};
    auto rx=f.serial.receive(bytes);ASSERT_TRUE(rx.has_value());f.mmio.inject(std::uint8_t{1});f.mmio.inject(std::uint8_t{2});
    ASSERT_TRUE(f.serial.poll());auto result=rx->take();ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().cause==driver::uart_error::overrun);EXPECT_EQ(bytes[0],1U);
}
TEST(nrf_uart, hardware_flow_control_stall_times_out_and_can_be_reused_after_cancellation,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.clocks.initialize().has_value());
    using flow=uart_binding<peripheral::uart0,{0,6},{0,8},{0,7},{0,12}>;
    uart_controller<flow,context_type,16> serial{f.context,f.clocks,10};
    auto timing=solve_uart_baud(peripheral::uart0,{16'000'000,115200,10000});
    ASSERT_TRUE(serial.initialize(*timing).has_value());EXPECT_EQ(f.mmio.peek(binding::base+0x56C),1U);
    f.mmio.automatic_tx=true;f.mmio.cts_active=false;const std::array<std::uint8_t,1> bytes{7};
    auto first=serial.transmit(bytes);ASSERT_TRUE(first.has_value());
    auto result=first->wait_until(execution::deadline{static_cast<execution::tick>(f.execution.now()+5)});
    ASSERT_FALSE(result.has_value());EXPECT_TRUE(result.error().kind==core::transfer_error_kind::timeout);
    (void)first->take();f.mmio.cts_active=true;
    auto next=serial.transmit(bytes);ASSERT_TRUE(next.has_value());ASSERT_TRUE(serial.poll());
    ASSERT_TRUE(next->take().has_value());ASSERT_TRUE(serial.shutdown().has_value());
}
TEST(nrf_uart, mmio_failure_detaches_cpu_buffers_and_late_events_cannot_finish_a_new_transfer,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());const std::array<std::uint8_t,1> bytes{7};
    auto tx=f.serial.transmit(bytes);ASSERT_TRUE(tx.has_value());f.mmio.failed_address=binding::base+0x11C;
    ASSERT_TRUE(f.serial.poll());EXPECT_TRUE(tx->take().error().cause==driver::uart_error::mmio);
    f.mmio.failed_address.reset();f.mmio.poke(binding::base+0x11C,1);
    EXPECT_FALSE(f.serial.poll());EXPECT_FALSE(f.serial.transmit(bytes).has_value());
    ASSERT_TRUE(f.serial.shutdown().has_value());
}
TEST(nrf_uart, irq_mmio_fault_does_not_wait_for_a_stuck_receiver_stop,"REQ-0059") {
    fixture f;ASSERT_TRUE(f.initialize());std::array<std::uint8_t,2> input{};
    const std::array<std::uint8_t,1> output{7};
    auto rx=f.serial.receive(input);ASSERT_TRUE(rx.has_value());
    auto tx=f.serial.transmit(output);ASSERT_TRUE(tx.has_value());
    f.mmio.stop_responds=false;f.mmio.failed_address=binding::base+0x11C;f.mmio.fail_once=true;
    const auto reads=f.mmio.reads;f.execution.set_isr(true);
    ASSERT_TRUE(f.serial.poll());f.execution.set_isr(false);
    EXPECT_TRUE(f.mmio.reads-reads<=8U);
    ASSERT_TRUE(rx->ready());ASSERT_TRUE(tx->ready());
    EXPECT_FALSE(rx->take().has_value());EXPECT_FALSE(tx->take().has_value());
    EXPECT_TRUE(f.serial.faulted());f.mmio.stop_responds=true;
    ASSERT_TRUE(f.serial.shutdown().has_value());
}
} // namespace
