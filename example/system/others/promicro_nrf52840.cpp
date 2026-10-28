// @req-file REQ-0055 REQ-0056 REQ-0057 REQ-0058 REQ-0073
#include <mcubsp/backend/nrf52840/core.hpp>
#include <mcubsp/backend/nrf52840/events.hpp>
#include <mcubsp/backend/nrf52840/rtc_watchdog.hpp>
#include <mcubsp/backend/nrf52840/timer.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/core/direct_mmio.hpp>

namespace n = mcubsp::backend::nrf52840;
using profile = mcubsp::board::others::promicro_nrf52840;
using execution_type = profile::default_execution;
static mcubsp::core::direct_mmio mmio;
static execution_type execution;
static n::gpio_backend gpio{mmio};
static n::system_context context{mmio, execution, gpio};
using context_type = decltype(context);
static n::clock_controller clocks{context};
static n::power_controller power{context, clocks};
static n::interrupt_dispatcher<context_type, 4> interrupts{context};
static n::rtc_controller<1, context_type> rtc{context, clocks};
static n::timer_controller<2, context_type> timer{context};
static n::gpiote_channel<0, context_type> led{context};
static n::ppi_channel<0, context_type> route{context};
// Readable through a future debugger; no USB application class is introduced.
extern "C" {
volatile std::uint32_t mcubsp_system_status{};
volatile std::uint32_t mcubsp_system_identity[7]{};
}
static void require(n::system_result r) noexcept {
    if (!r) {
        mcubsp_system_status = 0x80000000U | static_cast<unsigned>(r.error());
        __builtin_trap();
    }
}
static void rtc_irq(void *) noexcept { require(rtc.poll()); }
int main() {
    if (!profile::startup_ok())
        __builtin_trap();
    mcubsp::io::detail::cortex_m_cycle_hooks<64'000'000>::begin();
    auto id = context.identity();
    if (!id || !id->is_nrf52840())
        __builtin_trap();
    mcubsp_system_identity[0] = id->part;
    mcubsp_system_identity[1] = id->variant;
    mcubsp_system_identity[2] = id->package;
    mcubsp_system_identity[3] = id->ram_kib;
    mcubsp_system_identity[4] = id->flash_kib;
    mcubsp_system_identity[5] = id->rom_part;
    mcubsp_system_identity[6] = id->rom_revision;
    require(clocks.initialize());
    require(power.initialize());
    require(interrupts.initialize());
    require(interrupts.bind_target());
    require(rtc.initialize());
    auto subscription = interrupts.subscribe(rtc.irq, rtc_irq, nullptr);
    if (!subscription)
        __builtin_trap();
    require(rtc.start());
    require(timer.initialize(n::timer_mode::timer, 32, 4)); // 1 MHz TIMER base
    require(timer.compare(0, 500'000, true));
    require(led.initialize(profile::led_builtin, n::edge::toggle, true));
    require(route.initialize(timer.event(0), led.task()));
    require(route.enable());
    require(timer.start());
    mcubsp_system_status = 1;
    // TIMER -> PPI -> GPIOTE toggles the LED without foreground GPIO writes.
    // RTC wakes the foreground periodically; no watchdog or System OFF reset.
    for (;;) {
        auto now = rtc.ticks();
        if (!now)
            __builtin_trap();
        require(rtc.arm_alarm(*now + 1024));
        while (!rtc.alarm_ready())
            require(power.idle_if([] { return !rtc.alarm_ready(); }));
    }
}
