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
static n::watchdog_controller watchdog{context, clocks};
#ifndef MCUBSP_SYSTEM_WATCHDOG_TIMEOUT
#define MCUBSP_SYSTEM_WATCHDOG_TIMEOUT 0
#endif
// Readable through a future debugger; no USB application class is introduced.
extern "C" {
volatile std::uint32_t mcubsp_system_status{};
volatile std::uint32_t mcubsp_system_identity[7]{};
volatile std::uint32_t mcubsp_system_reset_reason{};
volatile std::uint32_t mcubsp_system_rtc_interrupts{};
volatile std::uint32_t mcubsp_system_timer_interrupts{};
}
static bool led_ready{}, route_ready{};
[[noreturn]] static void fail(unsigned code) noexcept {
    mcubsp_system_status = 0x80000000U | code;
    if (route_ready)
        // Emergency halt can originate in an ISR. Disable only our PPI0;
        // retain ownership rather than invoking foreground resource teardown.
        (void)context.write(0x4001F508, 1);
    if (led_ready)
        (void)context.write(led.set_task().address, 1); // steady on; never a success pattern
    for (;;) __asm volatile("nop"); // a started watchdog can still reset this failure
}
static void require(n::system_result r) noexcept {
    if (!r) fail(static_cast<unsigned>(r.error()));
}
static void rtc_irq(void *) noexcept {
    require(rtc.poll());
    mcubsp_system_rtc_interrupts = mcubsp_system_rtc_interrupts + 1;
}
static void timer_irq(void *) noexcept {
    auto fired = timer.consume(0);
    if (!fired) fail(0x100);
    if (*fired) mcubsp_system_timer_interrupts = mcubsp_system_timer_interrupts + 1;
}
static void wait_rtc(unsigned ticks) noexcept {
    auto now = rtc.ticks();
    if (!now) fail(0x101);
    auto before_irq = mcubsp_system_rtc_interrupts;
    auto before_guard = mcubsp_system_timer_interrupts;
    require(rtc.arm_alarm(*now + ticks));
    while (!rtc.alarm_ready()) {
        // TIMER2 is an independent 500 ms wake/timeout guard. Do not poll RTC
        // here: a successful alarm must have been serviced by its own ISR.
        if (std::uint32_t(mcubsp_system_timer_interrupts - before_guard) > 4) fail(0x102);
        auto slept = power.idle_if([] { return !rtc.alarm_ready(); });
        // An IRQ can complete the alarm between the outer check and the
        // masked sleep predicate; busy means there is no reason to sleep.
        if (!slept && slept.error() != n::system_error::busy) require(slept);
    }
    auto after = rtc.ticks();
    if (!after || *after < *now + ticks || mcubsp_system_rtc_interrupts == before_irq) fail(0x103);
}
int main() {
    if (!profile::startup_ok())
        fail(0x104);
    mcubsp::io::detail::cortex_m_cycle_hooks<64'000'000>::begin();
    auto id = context.identity();
    if (!id || !id->is_nrf52840())
        fail(0x105);
    mcubsp_system_identity[0] = id->part;
    mcubsp_system_identity[1] = id->variant;
    mcubsp_system_identity[2] = id->package;
    mcubsp_system_identity[3] = id->ram_kib;
    mcubsp_system_identity[4] = id->flash_kib;
    mcubsp_system_identity[5] = id->rom_part;
    mcubsp_system_identity[6] = id->rom_revision;
    require(clocks.initialize());
    require(power.initialize());
    auto reason = power.reset_reason();
    if (!reason) fail(0x106);
    mcubsp_system_reset_reason = *reason;
    const bool watchdog_reset = (*reason & 2U) != 0;
    require(power.clear_reset_reason(2));
    require(interrupts.initialize());
    require(interrupts.bind_target());
    require(rtc.initialize());
    auto subscription = interrupts.subscribe(rtc.irq, rtc_irq, nullptr);
    if (!subscription)
        fail(0x107);
    require(rtc.start());
    require(timer.initialize(n::timer_mode::timer, 32, 4)); // 1 MHz TIMER base
    auto timer_subscription = interrupts.subscribe(timer.irq, timer_irq, nullptr);
    if (!timer_subscription) fail(0x108);
    require(timer.compare(0, 500'000, true, false, true));
    require(led.initialize(profile::led_builtin, n::edge::toggle, true));
    led_ready = true;
    require(route.initialize(timer.event(0), led.task()));
    route_ready = true;
    require(route.enable());
    require(timer.start());
    // Four seconds of autonomous TIMER/PPI/GPIOTE toggling. Busy waiting here
    // keeps the DWT guard running even if timer interrupt delivery is broken.
    auto start = execution.now();
    while (mcubsp_system_timer_interrupts < 8) {
        if (std::uint32_t(execution.now() - start) > 6U * profile::ticks_per_second) fail(0x109);
    }
    require(route.shutdown());
    route_ready = false;
    require(context.write(led.clear_task().address, 1));
    mcubsp_system_status = 1; // timer/route phase completed

    // Every subsequent flash depends on a completed RTC alarm and its ISR.
    // A surviving watchdog reset reason changes double pulses to triple pulses.
    unsigned frames{};
    for (;;) {
        for (unsigned pulse = 0; pulse < (watchdog_reset ? 3U : 2U); ++pulse) {
            wait_rtc(102);
            require(led.trigger());
            wait_rtc(102);
            require(led.trigger());
        }
        wait_rtc(1024);
        mcubsp_system_status = 3U | (watchdog_reset ? 16U : 0U);
        if (frames == 0) {
            require(watchdog.initialize(3U * 32768U, 1));
            require(watchdog.start());
        }
        ++frames;
        if (MCUBSP_SYSTEM_WATCHDOG_TIMEOUT && frames == 3) {
            mcubsp_system_status = 7U | (watchdog_reset ? 16U : 0U);
            require(context.write(led.set_task().address, 1));
            auto guard = mcubsp_system_timer_interrupts;
            // Deliberately stop feeding. Only a hardware reset ends this wait.
            while (std::uint32_t(mcubsp_system_timer_interrupts - guard) < 10)
                require(power.idle_if([] { return true; }));
            fail(0x10A); // no reset within five nominal seconds
        }
        require(watchdog.feed(0));
        mcubsp_system_status = 15U | (watchdog_reset ? 16U : 0U);
    }
}
