// @req-file REQ-0056 REQ-0057 REQ-0058
#include <mcubsp/backend/nrf52840/events.hpp>
#include <mcubsp/backend/nrf52840/external_endpoints.hpp>
#include <mcubsp/backend/nrf52840/rtc_watchdog.hpp>
#include <mcubsp/backend/nrf52840/timer.hpp>
#include <mcubsp/backend/nrf52840/dma_buffers.hpp>
#include <mcubsp/backend/nrf52840/uart_config.hpp>
#include <mcubsp/backend/nrf52840/uart.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
namespace {
using namespace mcubsp;
namespace n = backend::nrf52840;
using context = n::system_context<core::direct_mmio, execution::fake_policy>;
using serial_pins=n::uart_binding<mcu::nrf52840::peripheral::uarte1,{0,6},{0,8},{0,7},{0,12}>;
static_assert(serial_pins::flow_control && serial_pins::supports_rate(1'000'000));
static_assert(n::solve_uart_baud(mcu::nrf52840::peripheral::uarte0,{16'000'000,115200,1000})->encoding==0x01D60000);
} // namespace
// Compile actual service bodies with every target compiler, without executing MMIO.
void nrf_fixed_route_probe(context &c, n::clock_controller<context> &clock) {
    (void)clock.event(n::clock_event::calibration_timeout);
    n::timer_controller<0, context> timer{c};
    n::rtc_controller<0, context> rtc{c, clock};
    n::fixed_ppi_channel<31, context> route{c};
    n::ppi_group<5, context> group{c};
    (void)timer.initialize();
    (void)rtc.initialize();
    (void)route.initialize(rtc.event(0), timer.start_task());
    (void)group.initialize(1U << 31);
    (void)group.enable(true);
    (void)group.clear_members();
    (void)group.shutdown();
    (void)route.shutdown();
    (void)timer.shutdown();
    (void)rtc.shutdown();
}
void nrf_dma_ownership_probe(n::resource_registry &resources, execution::fake_policy &execution) {
    using p = mcu::nrf52840::peripheral;
    n::dma_buffer_pool<execution::fake_policy,2> buffers{resources,execution};
    auto owner=resources.claim(execution,p::uarte0,&buffers);
    if (!owner) return;
    auto token=buffers.prepare(*owner,mcu::nrf52840::serial_dma_constraints<p::uarte0>,
                               0x20000000,16,16,n::dma_access::write);
    if (token) {
        (void)buffers.arm(*token);
        (void)buffers.finish(*token,16,[]()->std::expected<bool,n::coordination_error>{return true;});
    }
    (void)resources.release(execution,*owner);
}
void nrf_uart_probe(context &c,n::clock_controller<context> &clock) {
    using b=n::uart_binding<mcu::nrf52840::peripheral::uart0,{0,6},{0,8}>;
    n::uart_controller<b,context,16> serial{c,clock,10};
    auto timing=n::solve_uart_baud(b::instance,{16'000'000,115200,10000});
    if (!serial.initialize(*timing)) return;
    std::array<std::uint8_t,2> bytes{};
    auto tx=serial.transmit(bytes);
    if (tx) {(void)serial.poll();(void)tx->cancel();(void)tx->take();}
    auto rx=serial.receive(bytes);
    if (rx) {(void)serial.poll();(void)rx->cancel();(void)rx->take();}
    (void)serial.shutdown();
}
