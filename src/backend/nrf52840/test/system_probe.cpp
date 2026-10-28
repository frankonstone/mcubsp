// @req-file REQ-0056 REQ-0057 REQ-0058
#include <mcubsp/backend/nrf52840/events.hpp>
#include <mcubsp/backend/nrf52840/external_endpoints.hpp>
#include <mcubsp/backend/nrf52840/rtc_watchdog.hpp>
#include <mcubsp/backend/nrf52840/timer.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
namespace {
using namespace mcubsp;
namespace n = backend::nrf52840;
using context = n::system_context<core::direct_mmio, execution::fake_policy>;
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
