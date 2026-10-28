#include "fake_system.hpp"
#include <mcubsp/backend/nrf52840/core.hpp>
#include <mcubsp/backend/nrf52840/events.hpp>
#include <mcubsp/backend/nrf52840/external_endpoints.hpp>
#include <mcubsp/backend/nrf52840/rtc_watchdog.hpp>
#include <mcubsp/backend/nrf52840/timer.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
using context_type = system_context<nrf52840_test::fake_system, execution::fake_policy>;
struct fixture {
    fixture() { mmio.poke(0x10000100, 0x52840); }
    nrf52840_test::fake_system mmio;
    execution::fake_policy execution;
    gpio_backend<nrf52840_test::fake_system> gpio{mmio};
    context_type context{mmio, execution, gpio};
};
TEST(nrf_system, identifies_factory_part_without_interpreting_board_marking, "REQ-0055") {
    fixture f;
    f.mmio.poke(0x10000100, 0x52840);
    f.mmio.poke(0x10000104, 0x41414630);
    f.mmio.poke(0x1000010C, 256);
    f.mmio.poke(0x10000110, 1024);
    f.mmio.poke(0xF0000FE0, 0x108);
    f.mmio.poke(0xF0000FE8, 0x23);
    auto r = f.context.identity();
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(r->is_nrf52840());
    EXPECT_EQ(r->variant, 0x41414630U);
    EXPECT_EQ(r->ram_kib, 256U);
    EXPECT_EQ(r->rom_part, 8U);
    EXPECT_EQ(r->rom_revision, 2U);
    f.mmio.failed_address = 0xF0000FE8;
    EXPECT_FALSE(f.context.identity().has_value());
}
TEST(nrf_clock, reference_counts_prevent_premature_hf_stop, "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    ASSERT_TRUE(c.acquire_hf().has_value());
    ASSERT_TRUE(c.acquire_hf().has_value());
    EXPECT_EQ(c.hf_clients(), 2U);
    ASSERT_TRUE(c.release_hf().has_value());
    EXPECT_EQ(f.mmio.peek(0x40000408), 1U);
    EXPECT_FALSE(c.shutdown().has_value());
    ASSERT_TRUE(c.release_hf().has_value());
    EXPECT_EQ(f.mmio.peek(0x40000408), 0U);
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_clock, preserves_inherited_clock_and_refuses_conflicting_lf_source, "REQ-0056") {
    fixture f;
    f.mmio.poke(0x40000408, 1);
    f.mmio.poke(0x4000040C, 0x10001);
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    ASSERT_TRUE(c.acquire_hf().has_value());
    ASSERT_TRUE(c.release_hf().has_value());
    EXPECT_FALSE(f.mmio.wrote(0x40000004, 1));
    ASSERT_TRUE(c.acquire_lf(lf_source::rc).has_value());
    EXPECT_FALSE(c.acquire_lf(lf_source::crystal).has_value());
    ASSERT_TRUE(c.release_lf().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_clock, synthesized_source_holds_hf_and_calibration_requires_rc, "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    ASSERT_TRUE(c.acquire_lf(lf_source::synthesized).has_value());
    EXPECT_EQ(c.hf_clients(), 1U);
    EXPECT_FALSE(c.calibrate().has_value());
    ASSERT_TRUE(c.release_lf().has_value());
    EXPECT_EQ(c.hf_clients(), 0U);
    ASSERT_TRUE(c.acquire_hf().has_value());
    ASSERT_TRUE(c.acquire_lf(lf_source::rc).has_value());
    ASSERT_TRUE(c.calibrate().has_value());
    ASSERT_TRUE(c.calibration_timer(4).has_value());
    EXPECT_FALSE(c.shutdown().has_value());
    ASSERT_TRUE(c.calibration_timer(0).has_value());
    ASSERT_TRUE(c.release_lf().has_value());
    ASSERT_TRUE(c.release_hf().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_clock, failed_start_has_bounded_polling_and_keeps_controller_owned, "REQ-0056") {
    fixture f;
    f.mmio.clock_responds = false;
    clock_controller c{f.context}, other{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    auto r = c.acquire_hf(7);
    ASSERT_FALSE(r.has_value());
    EXPECT_TRUE(r.error() == system_error::timeout);
    EXPECT_TRUE(f.mmio.reads < 12U);
    EXPECT_FALSE(other.initialize().has_value());
    f.mmio.clock_responds = true;
    ASSERT_TRUE(c.shutdown().has_value());
    ASSERT_TRUE(other.initialize().has_value());
    ASSERT_TRUE(other.shutdown().has_value());
}
TEST(nrf_clock, external_lf_configuration_is_validated_and_shared_without_reprogramming,
     "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    EXPECT_FALSE(c.acquire_lf({lf_source::rc, lf_input::external_low_swing}).has_value());
    EXPECT_FALSE(c.acquire_lf({lf_source::synthesized, lf_input::external_full_swing}).has_value());
    ASSERT_TRUE(c.hf_debounce(64).has_value());
    ASSERT_TRUE(c.acquire_hf().has_value());
    EXPECT_FALSE(c.hf_debounce(16).has_value());
    ASSERT_TRUE(c.release_hf().has_value());
    ASSERT_TRUE(c.acquire_lf({lf_source::crystal, lf_input::external_full_swing}).has_value());
    EXPECT_EQ(f.mmio.peek(0x40000518), 0x30001U);
    auto writes = f.mmio.writes;
    ASSERT_TRUE(c.acquire_lf({lf_source::crystal, lf_input::external_full_swing}).has_value());
    EXPECT_FALSE(c.acquire_lf(lf_source::crystal).has_value());
    EXPECT_FALSE(c.acquire_lf({lf_source::crystal, lf_input::external_low_swing}).has_value());
    EXPECT_EQ(f.mmio.writes, writes);
    ASSERT_TRUE(c.release_lf().has_value());
    ASSERT_TRUE(c.release_lf().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_clock, inherited_external_lf_is_checked_against_latched_source_and_preserved, "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    f.mmio.poke(0x40000414, 1);
    f.mmio.poke(0x40000418, 0x10001);
    f.mmio.poke(0x4000041C, 1);
    f.mmio.poke(0x40000518, 0x20001);
    ASSERT_TRUE(c.initialize().has_value());
    EXPECT_FALSE(c.acquire_lf(lf_source::crystal).has_value());
    ASSERT_TRUE(c.acquire_lf({lf_source::crystal, lf_input::external_low_swing}).has_value());
    ASSERT_TRUE(c.release_lf().has_value());
    EXPECT_FALSE(f.mmio.wrote(0x40000518, 1));
    EXPECT_FALSE(f.mmio.wrote(0x4000000C, 1));
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_clock, mmio_failure_never_releases_an_owned_controller_implicitly, "REQ-0056") {
    fixture f;
    clock_controller c{f.context}, other{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    f.mmio.failed_address = 0x40000100;
    EXPECT_FALSE(c.acquire_hf().has_value());
    EXPECT_FALSE(other.initialize().has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_clock, calibration_expiry_acknowledges_one_shot_and_failed_start_keeps_clocks_owned,
     "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    ASSERT_TRUE(c.acquire_lf(lf_source::rc).has_value());
    ASSERT_TRUE(c.acquire_hf().has_value());
    f.mmio.poke(0x40000110, 1); // stale expiry must be cleared before rearming.
    ASSERT_TRUE(c.calibration_timer(4).has_value());
    auto stale = c.consume_timeout();
    ASSERT_TRUE(stale.has_value());
    EXPECT_FALSE(*stale);
    EXPECT_FALSE(c.release_lf().has_value());
    ASSERT_TRUE(c.timeout_interrupt(true).has_value());
    f.mmio.poke(0x40000110, 1);
    auto expired = c.consume_timeout();
    ASSERT_TRUE(expired.has_value());
    EXPECT_TRUE(*expired);
    ASSERT_TRUE(c.release_hf().has_value());
    ASSERT_TRUE(c.acquire_hf().has_value());
    f.mmio.failed_address = 0x40000128;
    EXPECT_FALSE(c.calibration_timer(4).has_value());
    EXPECT_FALSE(c.release_lf().has_value());
    EXPECT_FALSE(c.release_hf().has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(c.calibration_timer(0).has_value());
    ASSERT_TRUE(c.release_lf().has_value());
    ASSERT_TRUE(c.release_hf().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
    EXPECT_TRUE(f.mmio.wrote(0x40000308, 1U << 4));
}
TEST(nrf_power, power_and_clock_event_routes_share_irq_zero_without_sharing_source_ownership,
     "REQ-0056", "REQ-0057") {
    fixture f;
    clock_controller clock{f.context};
    power_controller power{f.context, clock};
    event_generator<0, context_type> egu{f.context};
    ppi_channel<0, context_type> power_route{f.context};
    ppi_channel<1, context_type> clock_route{f.context};
    ASSERT_TRUE(clock.initialize().has_value());
    ASSERT_TRUE(power.initialize().has_value());
    ASSERT_TRUE(egu.initialize().has_value());
    EXPECT_FALSE(f.context.valid(clock.event(static_cast<clock_event>(32)), endpoint_kind::event));
    EXPECT_FALSE(power.interrupt(static_cast<power_event>(0), true).has_value());
    ASSERT_TRUE(
        power_route.initialize(power.event(power_event::usb_ready), egu.task(0)).has_value());
    ASSERT_TRUE(clock_route.initialize(clock.event(clock_event::calibration_timeout), egu.task(1))
                    .has_value());
    ASSERT_TRUE(power.interrupt(power_event::usb_ready, true).has_value());
    ASSERT_TRUE(clock.timeout_interrupt(true).has_value());
    f.mmio.poke(0x40000124, 1);
    auto event = power.consume(power_event::usb_ready);
    ASSERT_TRUE(event.has_value());
    EXPECT_TRUE(*event);
    EXPECT_EQ(f.mmio.peek(0x40000124), 0U);
    EXPECT_FALSE(power.shutdown().has_value());
    EXPECT_FALSE(clock.shutdown().has_value());
    ASSERT_TRUE(power_route.shutdown().has_value());
    ASSERT_TRUE(clock_route.shutdown().has_value());
    auto begin = f.mmio.trace_count;
    ASSERT_TRUE(power.shutdown().has_value());
    for (auto n = begin; n < f.mmio.trace_count; ++n)
        if (f.mmio.trace[n].address == 0x40000308)
            EXPECT_EQ(f.mmio.trace[n].value, 1U << 9);
    ASSERT_TRUE(clock.shutdown().has_value());
    ASSERT_TRUE(egu.shutdown().has_value());
}
struct fake_sleep {
    static inline unsigned sleeps{};
    static void idle() noexcept { ++sleeps; }
};
TEST(nrf_power, sleep_predicate_is_rechecked_under_mask_and_system_off_rejects_work, "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    power_controller<context_type, fake_sleep> p{f.context, c};
    ASSERT_TRUE(p.initialize().has_value());
    fake_sleep::sleeps = 0;
    EXPECT_FALSE(p.idle_if([] { return false; }).has_value());
    EXPECT_EQ(fake_sleep::sleeps, 0U);
    ASSERT_TRUE(p.idle_if([] { return true; }).has_value());
    EXPECT_EQ(fake_sleep::sleeps, 1U);
    f.context.work_started();
    EXPECT_FALSE(p.system_off(true).has_value());
    f.context.work_stopped();
    EXPECT_FALSE(p.system_off(false).has_value());
    ASSERT_TRUE(p.system_off(true).has_value());
    EXPECT_TRUE(f.mmio.wrote(0x40000500, 1));
    ASSERT_TRUE(p.shutdown().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_power, ram_geometry_and_memory_owner_permissions_prevent_live_ram_shutdown, "REQ-0056") {
    fixture f;
    clock_controller c{f.context};
    power_controller p{f.context, c};
    ASSERT_TRUE(p.initialize().has_value());
    EXPECT_FALSE(p.ram(9, 3, 3).has_value());
    EXPECT_FALSE(p.ram(0, 7, 3).has_value());
    EXPECT_FALSE(p.ram(0, 1, 1).has_value());
    ASSERT_TRUE(p.ram(0, 1, 1, 2).has_value());
    ASSERT_TRUE(p.ram(8, 63, 63).has_value());
    EXPECT_EQ(f.mmio.peek(0x40000980), 0x3F003FU);
    EXPECT_FALSE(p.regulator(true, false, false).has_value());
    ASSERT_TRUE(p.retention(1, 0x57).has_value());
    ASSERT_TRUE(p.shutdown().has_value());
}
TEST(nrf_timer, validates_instance_channels_and_counter_capture, "REQ-0058") {
    fixture f;
    timer_controller<0, context_type> t{f.context};
    ASSERT_TRUE(t.initialize(timer_mode::counter, 16).has_value());
    EXPECT_FALSE(t.compare(4, 1).has_value());
    EXPECT_FALSE(t.compare(0, 65536).has_value());
    ASSERT_TRUE(t.compare(0, 200, true, false, true).has_value());
    ASSERT_TRUE(t.start().has_value());
    ASSERT_TRUE(t.count().has_value());
    ASSERT_TRUE(t.count().has_value());
    auto v = t.capture(1);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 2U);
    ASSERT_TRUE(t.shutdown().has_value());
    EXPECT_TRUE(f.mmio.wrote(0x40008010, 1));
    timer_controller<4, context_type> big{f.context};
    ASSERT_TRUE(big.initialize().has_value());
    ASSERT_TRUE(big.compare(5, 0xFFFFFFFF).has_value());
    ASSERT_TRUE(big.shutdown().has_value());
}
TEST(nrf_timer, errors_retain_instance_until_explicit_shutdown, "REQ-0058") {
    fixture f;
    timer_controller<2, context_type> t{f.context}, other{f.context};
    f.mmio.failed_address = 0x4000A504;
    EXPECT_FALSE(t.initialize().has_value());
    EXPECT_FALSE(t.start().has_value());
    EXPECT_FALSE(other.initialize().has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(t.shutdown().has_value());
    ASSERT_TRUE(other.initialize().has_value());
    ASSERT_TRUE(other.shutdown().has_value());
}
TEST(nrf_events, ppi_keeps_pin_and_timer_alive_until_route_is_disabled, "REQ-0057") {
    fixture f;
    timer_controller<0, context_type> t{f.context};
    gpiote_channel<0, context_type> pin{f.context};
    ppi_channel<0, context_type> p{f.context};
    ASSERT_TRUE(t.initialize().has_value());
    ASSERT_TRUE(pin.initialize({0, 15}, edge::toggle, true).has_value());
    ASSERT_TRUE(p.initialize(t.event(0), pin.task()).has_value());
    ASSERT_TRUE(p.enable().has_value());
    EXPECT_EQ(f.mmio.peek(0x4001F500), 1U);
    EXPECT_FALSE(t.shutdown().has_value());
    EXPECT_FALSE(pin.shutdown().has_value());
    ASSERT_TRUE(p.shutdown().has_value());
    ASSERT_TRUE(pin.shutdown().has_value());
    ASSERT_TRUE(t.shutdown().has_value());
}
TEST(nrf_events, stale_and_cross_context_endpoints_are_rejected_without_writes, "REQ-0057") {
    fixture f, other;
    timer_controller<0, context_type> t{f.context};
    gpiote_channel<0, context_type> pin{f.context};
    ppi_channel<0, context_type> p{f.context}, foreign{other.context};
    ASSERT_TRUE(t.initialize().has_value());
    auto stale = t.event(0);
    ASSERT_TRUE(t.shutdown().has_value());
    ASSERT_TRUE(t.initialize().has_value());
    ASSERT_TRUE(pin.initialize({0, 15}, edge::rising, true).has_value());
    auto writes = f.mmio.writes;
    EXPECT_FALSE(p.initialize(stale, pin.task()).has_value());
    EXPECT_EQ(f.mmio.writes, writes);
    EXPECT_FALSE(foreign.initialize(t.event(0), pin.task()).has_value());
    ASSERT_TRUE(pin.shutdown().has_value());
    ASSERT_TRUE(t.shutdown().has_value());
}
TEST(nrf_events, shared_gpio_owner_blocks_channel_reclaim_and_preserves_reserved_pads, "REQ-0057") {
    fixture f;
    gpiote_channel<1, context_type> p{f.context};
    ASSERT_TRUE(f.gpio.claim({0, 15}, driver::gpio_direction::input).has_value());
    EXPECT_FALSE(p.initialize({0, 15}, edge::rising).has_value());
    EXPECT_FALSE(p.initialize({0, 9}, edge::rising).has_value());
    f.gpio.release({0, 15});
    ASSERT_TRUE(p.initialize({0, 15}, edge::falling).has_value());
    ASSERT_TRUE(p.interrupt(true).has_value());
    f.mmio.poke(0x40006104, 1);
    auto event = p.consume();
    ASSERT_TRUE(event.has_value());
    EXPECT_TRUE(*event);
    EXPECT_EQ(f.mmio.peek(0x40006104), 0U);
    ASSERT_TRUE(p.shutdown().has_value());
}
TEST(nrf_events, failed_ppi_disable_retains_all_endpoint_ownership, "REQ-0057") {
    fixture f;
    event_generator<0, context_type> e{f.context};
    ppi_channel<2, context_type> p{f.context};
    ASSERT_TRUE(e.initialize().has_value());
    ASSERT_TRUE(p.initialize(e.event(0), e.task(1)).has_value());
    f.mmio.failed_address = 0x4001F508;
    EXPECT_FALSE(p.shutdown().has_value());
    EXPECT_FALSE(e.shutdown().has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(p.shutdown().has_value());
    ASSERT_TRUE(e.shutdown().has_value());
}
TEST(nrf_events, egu_events_and_group_owned_channel_teardown, "REQ-0057") {
    fixture f;
    event_generator<5, context_type> e{f.context};
    ppi_channel<19, context_type> p{f.context};
    ppi_group<5, context_type> group{f.context};
    ASSERT_TRUE(e.initialize().has_value());
    ASSERT_TRUE(e.trigger(15).has_value());
    auto r = e.consume(15);
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(*r);
    ASSERT_TRUE(p.initialize(e.event(0), e.task(1)).has_value());
    ASSERT_TRUE(group.initialize(1U << 19).has_value());
    ASSERT_TRUE(group.enable(true).has_value());
    EXPECT_FALSE(p.shutdown().has_value());
    ASSERT_TRUE(group.shutdown().has_value());
    ASSERT_TRUE(p.shutdown().has_value());
    ASSERT_TRUE(e.shutdown().has_value());
}
TEST(nrf_events, port_sense_latch_and_shutdown_keep_pin_ownership_consistent, "REQ-0057") {
    fixture f;
    gpio_port_events p{f.context};
    event_generator<0, context_type> egu{f.context};
    ppi_channel<0, context_type> route{f.context};
    ASSERT_TRUE(p.initialize().has_value());
    ASSERT_TRUE(egu.initialize().has_value());
    ASSERT_TRUE(p.sense({1, 15}, pin_sense::low, driver::gpio_pull::up).has_value());
    ASSERT_TRUE(route.initialize(p.event(), egu.task(0)).has_value());
    EXPECT_EQ(f.mmio.peek(0x50000A3C), 0x3000CU);
    EXPECT_FALSE(p.clear_latch(1, 1).has_value());
    ASSERT_TRUE(p.clear_latch(1, 1U << 15).has_value());
    EXPECT_FALSE(p.shutdown().has_value());
    ASSERT_TRUE(route.shutdown().has_value());
    ASSERT_TRUE(egu.shutdown().has_value());
    ASSERT_TRUE(p.shutdown().has_value());
    ASSERT_TRUE(f.gpio.claim({1, 15}, driver::gpio_direction::input).has_value());
    f.gpio.release({1, 15});
}
system_result quiesce_external(void *state) noexcept {
    auto &allowed = *static_cast<bool *>(state);
    return allowed ? system_result{} : std::unexpected(system_error::busy);
}
template <unsigned Index> void check_fixed_route() {
    fixture f;
    bool stopped = true;
    clock_controller clock{f.context};
    timer_controller<0, context_type> timer{f.context};
    rtc_controller<0, context_type> rtc{f.context, clock};
    event_generator<0, context_type> fork{f.context};
    external_endpoint_owner<endpoint_domain::radio, context_type> radio{f.context, quiesce_external,
                                                                        &stopped};
    constexpr auto binding = mcu::nrf52840::fixed_ppi_bindings[Index - 20];
    constexpr auto crypto_domain = Index == 23 ? endpoint_domain::aar : endpoint_domain::ccm;
    external_endpoint_owner<crypto_domain, context_type> crypto{f.context, quiesce_external,
                                                                &stopped};
    ASSERT_TRUE(clock.initialize().has_value());
    ASSERT_TRUE(timer.initialize().has_value());
    ASSERT_TRUE(rtc.initialize().has_value());
    ASSERT_TRUE(radio.initialize().has_value());
    ASSERT_TRUE(crypto.initialize().has_value());
    ASSERT_TRUE(fork.initialize().has_value());
    auto event = [&] {
        if constexpr (Index <= 22)
            return timer.event(Index == 22 ? 1 : 0);
        else if constexpr (Index >= 28)
            return rtc.event(0);
        else
            return radio.event(static_cast<unsigned>(binding.event - 0x40001000));
    }();
    auto task = [&] {
        if constexpr (Index == 23 || Index == 24 || Index == 25)
            return crypto.task(static_cast<unsigned>(binding.task - 0x4000F000));
        else if constexpr (Index == 26 || Index == 27)
            return timer.capture_task(Index == 26 ? 1 : 2);
        else if constexpr (Index == 30)
            return timer.clear_task();
        else if constexpr (Index == 31)
            return timer.start_task();
        else
            return radio.task(static_cast<unsigned>(binding.task - 0x40001000));
    }();
    fixed_ppi_channel<Index, context_type> route{f.context};
    ppi_group<5, context_type> group{f.context};
    auto begin = f.mmio.trace_count;
    ASSERT_TRUE(route.initialize(event, task, fork.task(15)).has_value());
    ASSERT_TRUE(route.enable().has_value());
    EXPECT_EQ(f.mmio.peek(0x4001F500), 1U << Index);
    EXPECT_EQ(f.mmio.peek(0x4001F910 + 4 * Index), 0x4001403CU);
    // Fixed routes must never touch the programmable EEP/TEP register bank.
    for (auto n = begin; n < f.mmio.trace_count; ++n) {
        auto address = f.mmio.trace[n].address;
        EXPECT_FALSE(address >= 0x4001F510 && address < 0x4001F610);
    }
    ASSERT_TRUE(group.initialize(1U << Index).has_value());
    ASSERT_TRUE(group.enable(true).has_value());
    EXPECT_FALSE(route.shutdown().has_value());
    EXPECT_FALSE(fork.shutdown().has_value());
    ASSERT_TRUE(group.shutdown().has_value());
    ASSERT_TRUE(route.shutdown().has_value());
    ASSERT_TRUE(fork.shutdown().has_value());
    ASSERT_TRUE(crypto.shutdown().has_value());
    ASSERT_TRUE(radio.shutdown().has_value());
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(timer.shutdown().has_value());
    ASSERT_TRUE(clock.shutdown().has_value());
}
TEST(nrf_events, all_twelve_fixed_routes_preserve_immutable_bindings_and_group_membership,
     "REQ-0057") {
    check_fixed_route<20>();
    check_fixed_route<21>();
    check_fixed_route<22>();
    check_fixed_route<23>();
    check_fixed_route<24>();
    check_fixed_route<25>();
    check_fixed_route<26>();
    check_fixed_route<27>();
    check_fixed_route<28>();
    check_fixed_route<29>();
    check_fixed_route<30>();
    check_fixed_route<31>();
}
TEST(nrf_events, fixed_route_rejects_wrong_crypto_personality_and_forged_domain, "REQ-0057") {
    fixture f;
    bool stopped = true;
    external_endpoint_owner<endpoint_domain::radio, context_type> radio{f.context, quiesce_external,
                                                                        &stopped};
    external_endpoint_owner<endpoint_domain::ccm, context_type> ccm{f.context, quiesce_external,
                                                                    &stopped};
    external_endpoint_owner<endpoint_domain::aar, context_type> aar{f.context, quiesce_external,
                                                                    &stopped};
    fixed_ppi_channel<23, context_type> route{f.context};
    ASSERT_TRUE(radio.initialize().has_value());
    ASSERT_TRUE(ccm.initialize().has_value());
    EXPECT_FALSE(aar.initialize().has_value());
    auto writes = f.mmio.writes;
    EXPECT_FALSE(route.initialize(radio.event(0x128), ccm.task(0)).has_value());
    auto forged = ccm.task(0);
    forged.domain = endpoint_domain::aar;
    EXPECT_FALSE(route.initialize(radio.event(0x128), forged).has_value());
    EXPECT_EQ(f.mmio.writes, writes);
    ASSERT_TRUE(ccm.shutdown().has_value());
    ASSERT_TRUE(aar.initialize().has_value());
    ASSERT_TRUE(route.initialize(radio.event(0x128), aar.task(0)).has_value());
    EXPECT_FALSE(aar.shutdown().has_value());
    ASSERT_TRUE(route.shutdown().has_value());
    stopped = false;
    EXPECT_FALSE(aar.shutdown().has_value());
    EXPECT_FALSE(ccm.initialize().has_value());
    stopped = true;
    ASSERT_TRUE(aar.shutdown().has_value());
    ASSERT_TRUE(radio.shutdown().has_value());
}
TEST(nrf_events, failed_fixed_disable_holds_primary_and_fork_until_readback, "REQ-0057") {
    fixture f;
    clock_controller clock{f.context};
    timer_controller<0, context_type> timer{f.context};
    rtc_controller<0, context_type> rtc{f.context, clock};
    event_generator<0, context_type> fork{f.context};
    fixed_ppi_channel<31, context_type> route{f.context};
    ASSERT_TRUE(clock.initialize().has_value());
    ASSERT_TRUE(timer.initialize().has_value());
    ASSERT_TRUE(rtc.initialize().has_value());
    ASSERT_TRUE(fork.initialize().has_value());
    ASSERT_TRUE(route.initialize(rtc.event(0), timer.start_task(), fork.task(1)).has_value());
    ASSERT_TRUE(route.enable().has_value());
    f.mmio.failed_address = 0x4001F500;
    EXPECT_FALSE(route.shutdown().has_value());
    EXPECT_FALSE(timer.shutdown().has_value());
    EXPECT_FALSE(rtc.shutdown().has_value());
    EXPECT_FALSE(fork.shutdown().has_value());
    EXPECT_FALSE(route.enable().has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(route.shutdown().has_value());
    ASSERT_TRUE(fork.shutdown().has_value());
    ASSERT_TRUE(timer.shutdown().has_value());
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(clock.shutdown().has_value());
}
TEST(nrf_rtc, extends_overflow_and_releases_lf_only_after_stop, "REQ-0058") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    rtc_controller<2, context_type> rtc{f.context, c};
    ASSERT_TRUE(rtc.initialize().has_value());
    ASSERT_TRUE(rtc.start().has_value());
    f.mmio.poke(0x40024504, 0xFFFFFE);
    auto a = rtc.ticks();
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(*a, 0xFFFFFEULL);
    f.mmio.poke(0x40024104, 1);
    f.mmio.poke(0x40024504, 2);
    auto b = rtc.ticks();
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(*b, 0x1000002ULL);
    auto d = rtc.ticks();
    ASSERT_TRUE(d.has_value());
    EXPECT_EQ(*d, *b);
    EXPECT_FALSE(c.shutdown().has_value());
    ASSERT_TRUE(rtc.shutdown().has_value());
    EXPECT_FALSE(f.context.active_work());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_rtc, rtc_and_watchdog_share_explicit_external_clock_and_retain_timeout_source, "REQ-0056",
     "REQ-0057", "REQ-0058") {
    fixture f;
    clock_controller clock{f.context};
    rtc_controller<2, context_type> rtc{f.context, clock};
    watchdog_controller watchdog{f.context, clock};
    event_generator<0, context_type> egu{f.context};
    ppi_channel<0, context_type> route{f.context};
    constexpr lf_configuration source{lf_source::crystal, lf_input::external_full_swing};
    ASSERT_TRUE(clock.initialize().has_value());
    ASSERT_TRUE(rtc.initialize(source).has_value());
    ASSERT_TRUE(watchdog.initialize(32767, 1, true, false, source).has_value());
    EXPECT_EQ(clock.lf_clients(), 2U);
    ASSERT_TRUE(egu.initialize().has_value());
    ASSERT_TRUE(route.initialize(watchdog.timeout_event(), egu.task(0)).has_value());
    f.mmio.poke(0x40010100, 1);
    auto timeout = watchdog.timeout_pending();
    ASSERT_TRUE(timeout.has_value());
    EXPECT_TRUE(*timeout);
    EXPECT_EQ(f.mmio.peek(0x40010100), 1U); // observation does not acknowledge/cancel reset
    EXPECT_FALSE(watchdog.shutdown().has_value());
    ASSERT_TRUE(route.shutdown().has_value());
    ASSERT_TRUE(watchdog.shutdown().has_value());
    EXPECT_EQ(clock.lf_clients(), 1U);
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(egu.shutdown().has_value());
    ASSERT_TRUE(clock.shutdown().has_value());
}
TEST(nrf_rtc, alarm_handles_past_deadlines_and_compare_delivery, "REQ-0058") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    rtc_controller<0, context_type> rtc{f.context, c};
    ASSERT_TRUE(rtc.initialize().has_value());
    f.mmio.poke(0x4000B504, 100);
    ASSERT_TRUE(rtc.arm_alarm(90).has_value());
    EXPECT_TRUE(rtc.alarm_ready());
    ASSERT_TRUE(rtc.arm_alarm(200).has_value());
    EXPECT_FALSE(rtc.alarm_ready());
    EXPECT_EQ(f.mmio.peek(0x4000B540), 200U);
    f.mmio.poke(0x4000B504, 200);
    ASSERT_TRUE(rtc.poll().has_value());
    EXPECT_TRUE(rtc.alarm_ready());
    ASSERT_TRUE(rtc.cancel_alarm().has_value());
    EXPECT_FALSE(rtc.alarm_ready());
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_rtc, long_alarm_programs_intermediate_compare_with_bounded_distance, "REQ-0058") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    rtc_controller<1, context_type> rtc{f.context, c};
    ASSERT_TRUE(rtc.initialize().has_value());
    ASSERT_TRUE(rtc.arm_alarm(1ULL << 32).has_value());
    EXPECT_EQ(f.mmio.peek(0x40011540), 0x7FFFFFU);
    EXPECT_FALSE(rtc.compare(4, 1).has_value());
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_watchdog, reload_channels_are_validated_and_running_hardware_cannot_stop, "REQ-0058") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    watchdog_controller w{f.context, c};
    EXPECT_FALSE(w.initialize(10, 1).has_value());
    ASSERT_TRUE(w.initialize(32768, 5).has_value());
    ASSERT_TRUE(w.start().has_value());
    EXPECT_FALSE(w.feed(1).has_value());
    ASSERT_TRUE(w.feed(2).has_value());
    EXPECT_EQ(f.mmio.peek(0x40010608), 0x6E524635U);
    EXPECT_FALSE(w.shutdown().has_value());
    EXPECT_FALSE(c.shutdown().has_value());
}
TEST(nrf_watchdog, configured_unstarted_watchdog_can_release_its_resources, "REQ-0058") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    watchdog_controller w{f.context, c};
    ASSERT_TRUE(w.initialize(32768, 1).has_value());
    ASSERT_TRUE(w.shutdown().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
void count_callback(void *p) noexcept { ++*static_cast<unsigned *>(p); }
TEST(nrf_core, shared_irq_callbacks_have_bounded_capacity_and_matching_priorities, "REQ-0073") {
    fixture f;
    interrupt_dispatcher<context_type, 2> d{f.context};
    ASSERT_TRUE(d.initialize().has_value());
    unsigned a{}, b{};
    auto one = d.subscribe(0, count_callback, &a, 5);
    ASSERT_TRUE(one.has_value());
    EXPECT_FALSE(d.subscribe(0, count_callback, &b, 6).has_value());
    auto two = d.subscribe(0, count_callback, &b, 5);
    ASSERT_TRUE(two.has_value());
    EXPECT_FALSE(d.subscribe(1, count_callback, &b).has_value());
    d.dispatch(0);
    EXPECT_EQ(a, 1U);
    EXPECT_EQ(b, 1U);
    EXPECT_FALSE(d.shutdown().has_value());
    ASSERT_TRUE(d.unsubscribe(*one).has_value());
    d.dispatch(0);
    EXPECT_EQ(a, 1U);
    EXPECT_EQ(b, 2U);
    EXPECT_FALSE(d.unsubscribe(*one).has_value());
    ASSERT_TRUE(d.unsubscribe(*two).has_value());
    ASSERT_TRUE(d.shutdown().has_value());
}
TEST(nrf_core, mpu_alignment_and_live_reconfiguration_are_rejected, "REQ-0073") {
    fixture f;
    f.mmio.poke(0xE000ED90, 8U << 8);
    core_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    EXPECT_FALSE(c.mpu_region(0, 0x20000001, 256, mpu_access::read_write, false).has_value());
    EXPECT_FALSE(c.mpu_region(8, 0x20000000, 256, mpu_access::read_write, false).has_value());
    ASSERT_TRUE(c.mpu_region(0, 0x20000000, 256, mpu_access::read_write, false).has_value());
    EXPECT_EQ(f.mmio.peek(0xE000EDA0), 0x1300000FU);
    ASSERT_TRUE(c.mpu_enable(true).has_value());
    EXPECT_FALSE(c.mpu_region(1, 0x20001000, 256, mpu_access::read_only, false).has_value());
    ASSERT_TRUE(c.mpu_enable(false).has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_core, vectors_fpu_fault_queries_and_trace_use_explicit_core_owner, "REQ-0073") {
    fixture f;
    core_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    EXPECT_FALSE(c.vectors(0x26004).has_value());
    ASSERT_TRUE(c.vectors(0x26000).has_value());
    ASSERT_TRUE(c.fpu(true).has_value());
    EXPECT_EQ(f.mmio.peek(0xE000ED88), 0xF00000U);
    ASSERT_TRUE(c.trace(true).has_value());
    EXPECT_EQ(f.mmio.peek(0xE000EDFC), 1U << 24);
    auto fault = c.faults();
    ASSERT_TRUE(fault.has_value());
    EXPECT_FALSE(c.systick(0x1000000).has_value());
    ASSERT_TRUE(c.systick(63999).has_value());
    EXPECT_FALSE(c.shutdown().has_value());
    ASSERT_TRUE(c.stop_systick().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_events, errata_fast_edges_and_sense_configuration_have_explicit_cleanup, "REQ-0057") {
    fixture f;
    gpiote_channel<3, context_type> edge_pin{f.context};
    ASSERT_TRUE(edge_pin.initialize({0, 6}, edge::toggle).has_value());
    EXPECT_EQ(f.mmio.peek(0x4000660C), 1U);
    ASSERT_TRUE(edge_pin.shutdown().has_value());
    EXPECT_EQ(f.mmio.peek(0x4000660C), 0U);
    gpio_port_events port{f.context};
    ASSERT_TRUE(port.initialize().has_value());
    ASSERT_TRUE(port.sense({0, 6}, pin_sense::low).has_value());
    EXPECT_EQ(f.mmio.peek(0x50000524), 1U);
    ASSERT_TRUE(port.shutdown().has_value());
    EXPECT_EQ(f.mmio.peek(0x50000524), 0U);
}
TEST(nrf_rtc, ppi_wait_excludes_cpu_register_reads_for_erratum_179, "REQ-0058") {
    fixture f;
    clock_controller c{f.context};
    ASSERT_TRUE(c.initialize().has_value());
    rtc_controller<0, context_type> rtc{f.context, c};
    gpiote_channel<0, context_type> pin{f.context};
    ppi_channel<0, context_type> route{f.context};
    ASSERT_TRUE(rtc.initialize().has_value());
    ASSERT_TRUE(rtc.compare(1, 100, false, true).has_value());
    ASSERT_TRUE(pin.initialize({0, 15}, edge::toggle, true).has_value());
    ASSERT_TRUE(route.initialize(rtc.event(1), pin.task()).has_value());
    ASSERT_TRUE(route.enable().has_value());
    ASSERT_TRUE(rtc.start().has_value());
    auto reads = f.mmio.reads;
    EXPECT_FALSE(rtc.ticks().has_value());
    EXPECT_EQ(reads, f.mmio.reads);
    EXPECT_FALSE(rtc.stop().has_value());
    ASSERT_TRUE(route.shutdown().has_value());
    ASSERT_TRUE(pin.shutdown().has_value());
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(c.shutdown().has_value());
}
TEST(nrf_events, self_disabling_group_cycles_have_an_explicit_teardown_path, "REQ-0057") {
    fixture f;
    event_generator<0, context_type> event{f.context};
    ppi_channel<0, context_type> route{f.context};
    ppi_group<0, context_type> group{f.context};
    ASSERT_TRUE(event.initialize().has_value());
    ASSERT_TRUE(group.initialize(0).has_value());
    ASSERT_TRUE(route.initialize(event.event(0), group.disable_task()).has_value());
    ASSERT_TRUE(group.set_members(1).has_value());
    EXPECT_FALSE(route.shutdown().has_value());
    EXPECT_FALSE(group.shutdown().has_value());
    ASSERT_TRUE(group.clear_members().has_value());
    ASSERT_TRUE(route.shutdown().has_value());
    ASSERT_TRUE(group.shutdown().has_value());
    ASSERT_TRUE(event.shutdown().has_value());
}
TEST(nrf_core, ambiguous_irq_enable_keeps_handler_live_until_explicit_clear, "REQ-0073") {
    fixture f;
    interrupt_dispatcher<context_type, 1> irq{f.context};
    ASSERT_TRUE(irq.initialize().has_value());
    unsigned calls{};
    f.mmio.failed_address = 0xE000E100;
    f.mmio.fail_after_commit = true;
    EXPECT_FALSE(irq.subscribe(6, count_callback, &calls).has_value());
    irq.dispatch(6);
    EXPECT_EQ(calls, 1U);
    EXPECT_FALSE(irq.shutdown().has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(irq.clear().has_value());
    ASSERT_TRUE(irq.shutdown().has_value());
}
TEST(nrf_events, malformed_endpoint_addresses_and_unready_resources_are_rejected, "REQ-0057") {
    fixture f;
    event_generator<0, context_type> event{f.context};
    ppi_channel<0, context_type> route{f.context};
    ASSERT_TRUE(event.initialize().has_value());
    auto bad = event.event(0);
    bad.address = 0x40000000;
    EXPECT_FALSE(route.initialize(bad, event.task(1)).has_value());
    ASSERT_TRUE(event.shutdown().has_value());
    f.mmio.failed_address = 0x40014308;
    EXPECT_FALSE(event.initialize().has_value());
    EXPECT_FALSE(event.trigger(0).has_value());
    f.mmio.failed_address.reset();
    ASSERT_TRUE(event.shutdown().has_value());
}

TEST(nrf_core, borrowed_clock_and_external_core_services_cannot_be_taken_over, "REQ-0073") {
    fixture f;
    context_type reserved{
        f.mmio,
        f.execution,
        f.gpio,
        {.cycle_counter = true, .systick = true, .mpu = true, .vectors = true, .fpu = true}};
    core_controller core{reserved};
    ASSERT_TRUE(core.initialize().has_value());
    EXPECT_FALSE(core.trace(false).has_value());
    EXPECT_FALSE(core.systick(100).has_value());
    EXPECT_FALSE(core.vectors(0x26000).has_value());
    EXPECT_FALSE(core.mpu_enable(true).has_value());
    EXPECT_FALSE(core.fpu(false).has_value());
    ASSERT_TRUE(core.shutdown().has_value());
}
TEST(nrf_rtc, ambiguous_overflow_acknowledgment_poisoning_prevents_time_reversal, "REQ-0058") {
    fixture f;
    clock_controller clocks{f.context};
    ASSERT_TRUE(clocks.initialize().has_value());
    rtc_controller<0, context_type> rtc{f.context, clocks};
    ASSERT_TRUE(rtc.initialize().has_value());
    f.mmio.poke(0x4000B104, 1);
    f.mmio.failed_address = 0x4000B104;
    EXPECT_FALSE(rtc.ticks().has_value());
    f.mmio.failed_address.reset();
    EXPECT_FALSE(rtc.ticks().has_value());
    ASSERT_TRUE(rtc.shutdown().has_value());
    ASSERT_TRUE(clocks.shutdown().has_value());
}
} // namespace
