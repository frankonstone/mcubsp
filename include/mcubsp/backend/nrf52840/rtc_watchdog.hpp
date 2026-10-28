#pragma once
// @req-file REQ-0058
#include <limits>
#include <mcubsp/backend/nrf52840/clock_power.hpp>
namespace mcubsp::backend::nrf52840 {
template <unsigned Index, class Context, unsigned Prescaler = 31>
    requires(Index < 3 && Prescaler < 4096)
class rtc_controller {
  public:
    rtc_controller(Context &c, clock_controller<Context> &clocks) noexcept
        : c_(c), clocks_(clocks) {}
    rtc_controller(const rtc_controller &) = delete;
    rtc_controller &operator=(const rtc_controller &) = delete;
    static constexpr unsigned irq = mcu::nrf52840::rtc_irqs[Index];
    static constexpr std::uint32_t ticks_per_second = 32768 / (Prescaler + 1);
    static constexpr bool exact_frequency = 32768 % (Prescaler + 1) == 0;
    [[nodiscard]] system_result initialize(lf_source source = lf_source::rc) noexcept {
        return initialize(lf_configuration{source});
    }
    [[nodiscard]] system_result initialize(lf_configuration configuration) noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(resource);
        if (!r)
            return r;
        owned_ = true;
        auto clock = clocks_.acquire_lf(configuration);
        if (!clock)
            return clock;
        clock_ = true;
        if (!clocks_.synchronize_lf())
            return fail();
        if (!c_.write(base + 4, 1) || !c_.write(base + 0x308, 0xF0003) ||
            !c_.write(base + 0x348, 0xF0003) || !c_.write(base + 0x508, Prescaler) ||
            !c_.write(base + 8, 1) || !c_.clear_event(base + 0x104))
            return fail();
        for (unsigned n = 0; n < 4; ++n)
            if (!c_.clear_event(base + 0x140 + n * 4) || !c_.write(base + 0x540 + n * 4, 0))
                return fail();
        if (!c_.clear_event(base + 0x100) || !c_.write(base + 0x304, 2))
            return fail();
        high_ = 0;
        ready_ = true;
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_result start() noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        if (running_)
            return {};
        if (!c_.write(base, 1))
            return fail();
        running_ = true;
        c_.work_started();
        return {};
    }
    [[nodiscard]] system_result stop() noexcept {
        if (!owned_ || c_.referenced(resource))
            return std::unexpected(system_error::not_initialized);
        if (!c_.write(base + 4, 1))
            return fail();
        if (running_) {
            c_.work_stopped();
            running_ = false;
        }
        return {};
    }
    // Must service overflow at least once every 2^24 ticks (~4.55 hours). With
    // OVERFLOW IRQ enabled, idle/sleep is safe; System OFF resets this clock.
    [[nodiscard]] system_value<std::uint64_t> ticks() noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy); // anomaly 179
        return c_.execution.critical([&]() -> system_value<std::uint64_t> {
            for (unsigned attempt = 0; attempt < 3; ++attempt) {
                auto pending = c_.read(base + 0x104);
                if (!pending)
                    return time_fault(pending.error());
                if (*pending) {
                    if (!c_.clear_event(base + 0x104))
                        return time_fault(system_error::mmio);
                    high_ += 1ULL << 24;
                }
                auto low = c_.read(base + 0x504), again = c_.read(base + 0x104);
                if (!low || !again)
                    return time_fault(system_error::mmio);
                if (!*again)
                    return high_ + (*low & 0xFFFFFF);
            }
            return time_fault(system_error::faulted);
        });
    }
    // Execution-policy adapter: a failed time read must not freeze deadlines.
    [[nodiscard]] std::uint32_t now() noexcept
        requires(exact_frequency)
    {
        auto r = ticks();
        if (!r) {
            c_.execution.active_transfer_destroyed();
            return 0;
        }
        return static_cast<std::uint32_t>(*r);
    }
    [[nodiscard]] system_result compare(unsigned n, std::uint32_t value, bool interrupt = true,
                                        bool event = true) noexcept {
        if (!ready_ || c_.referenced(resource) || n >= 4 || value > 0xFFFFFF ||
            (n == 0 && alarm_armed_ && !programming_alarm_))
            return std::unexpected(system_error::invalid);
        if (!c_.write(base + 0x308, 1U << (16 + n)) || !c_.write(base + 0x348, 1U << (16 + n)) ||
            !c_.clear_event(base + 0x140 + n * 4) || !c_.write(base + 0x540 + n * 4, value) ||
            (event && !c_.write(base + 0x344, 1U << (16 + n))) ||
            (interrupt && !c_.write(base + 0x304, 1U << (16 + n))))
            return fail();
        return {};
    }
    [[nodiscard]] system_result arm_alarm(std::uint64_t target) noexcept {
        auto current = ticks();
        if (!current)
            return std::unexpected(current.error());
        auto cancelled = cancel_alarm();
        if (!cancelled)
            return cancelled;
        alarm_target_ = target;
        alarm_armed_ = true;
        alarm_fired_ = target <= *current;
        if (alarm_fired_)
            return {};
        return program_alarm(*current);
    }
    [[nodiscard]] system_result poll() noexcept {
        auto current = ticks();
        if (!current)
            return std::unexpected(current.error());
        if (!alarm_armed_ || alarm_fired_)
            return {};
        if (*current >= alarm_target_) {
            alarm_fired_ = true;
            return c_.write(base + 0x308, 1U << 16);
        }
        auto fired = c_.read(base + 0x140);
        if (!fired)
            return std::unexpected(fired.error());
        if (*fired)
            return program_alarm(*current);
        return {};
    }
    [[nodiscard]] bool alarm_ready() const noexcept { return alarm_armed_ && alarm_fired_; }
    [[nodiscard]] system_result cancel_alarm() noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        if (!c_.write(base + 0x308, 1U << 16) || !c_.clear_event(base + 0x140))
            return fail();
        alarm_armed_ = alarm_fired_ = false;
        return {};
    }
    [[nodiscard]] system_endpoint event(unsigned n) const noexcept {
        return ready_ && n < 4 ? c_.endpoint(resource, base + 0x140 + n * 4, endpoint_kind::event)
                               : system_endpoint{};
    }
    [[nodiscard]] system_result tick_event(bool on) noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        return c_.write(base + (on ? 0x344 : 0x348), 1);
    }
    [[nodiscard]] system_endpoint overflow_event() const noexcept {
        return ready_ ? c_.endpoint(resource, base + 0x104, endpoint_kind::event)
                      : system_endpoint{};
    }
    [[nodiscard]] system_endpoint tick_endpoint() const noexcept {
        return ready_ ? c_.endpoint(resource, base + 0x100, endpoint_kind::event)
                      : system_endpoint{};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        if (!stop() || !c_.write(base + 0x308, 0xF0003) || !c_.write(base + 0x348, 0xF0003))
            return fail();
        if (clock_) {
            auto r = clocks_.release_lf();
            if (!r)
                return r;
            clock_ = false;
        }
        auto r = c_.release(resource);
        if (r) {
            owned_ = ready_ = false;
            alarm_armed_ = alarm_fired_ = false;
        }
        return r;
    }

  private:
    system_value<std::uint64_t> time_fault(system_error error) noexcept {
        ready_ = false;
        c_.invalidate(resource);
        return std::unexpected(error);
    }
    system_result program_alarm(std::uint64_t current) noexcept {
        auto delta = alarm_target_ - current;
        // Reserve CC0 for software alarm. Other channels remain native users'.
        auto step = delta > 0x7FFFFF ? 0x7FFFFFU : static_cast<std::uint32_t>(delta);
        if (step < 3)
            step = 3; // RTC compare programming needs LFCLK synchronization margin.
        programming_alarm_ = true;
        auto r = compare(0, static_cast<std::uint32_t>((current + step) & 0xFFFFFF));
        programming_alarm_ = false;
        if (!r)
            return r;
        auto after = ticks();
        if (!after)
            return std::unexpected(after.error());
        if (*after >= alarm_target_) {
            alarm_fired_ = true;
            return c_.write(base + 0x308, 1U << 16);
        }
        return {};
    }
    system_result fail() noexcept {
        ready_ = false;
        c_.invalidate(resource);
        return std::unexpected(system_error::mmio);
    }
    static constexpr std::uintptr_t base = mcu::nrf52840::rtc_bases[Index];
    static constexpr unsigned resource = 5 + Index;
    Context &c_;
    clock_controller<Context> &clocks_;
    std::uint64_t high_{}, alarm_target_{};
    bool owned_{}, ready_{}, clock_{}, running_{}, alarm_armed_{}, alarm_fired_{},
        programming_alarm_{};
};

template <class Context> class watchdog_controller {
  public:
    watchdog_controller(Context &c, clock_controller<Context> &clocks) noexcept
        : c_(c), clocks_(clocks) {}
    watchdog_controller(const watchdog_controller &) = delete;
    watchdog_controller &operator=(const watchdog_controller &) = delete;
    [[nodiscard]] system_result initialize(std::uint32_t reload_ticks, std::uint8_t channels,
                                           bool run_in_sleep = true, bool run_in_debug = false,
                                           lf_configuration configuration = {}) noexcept {
        if (owned_ || reload_ticks < 15 || !channels)
            return std::unexpected(system_error::invalid);
        auto status = c_.read(base + 0x400);
        if (!status)
            return std::unexpected(status.error());
        if (*status & 1)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(8);
        if (!r)
            return r;
        owned_ = true;
        auto lf = clocks_.acquire_lf(configuration);
        if (!lf)
            return lf;
        clock_ = true;
        if (!c_.write(base + 0x308, 1) || !c_.clear_event(base + 0x100) ||
            !c_.write(base + 0x504, reload_ticks) || !c_.write(base + 0x508, channels) ||
            !c_.write(base + 0x50C, (run_in_sleep ? 1U : 0U) | (run_in_debug ? 8U : 0U)))
            return std::unexpected(system_error::mmio);
        mask_ = channels;
        ready_ = true;
        c_.publish(8);
        return {};
    }
    [[nodiscard]] system_endpoint timeout_event() const noexcept {
        return ready_ ? c_.endpoint(8, base + 0x100, endpoint_kind::event) : system_endpoint{};
    }
    [[nodiscard]] system_value<bool> timeout_pending() noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        auto r = c_.read(base + 0x100);
        if (!r)
            return std::unexpected(r.error());
        return *r != 0; // TIMEOUT cannot cancel/postpone the impending reset.
    }
    [[nodiscard]] system_result start() noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        if (started_)
            return {};
        // Mark irreversible before issuing START: a bus error may follow a commit.
        started_ = true;
        c_.work_started();
        return c_.write(base, 1);
    }
    [[nodiscard]] system_result feed(unsigned channel) noexcept {
        if (!ready_ || !started_ || channel >= 8 || !(mask_ & (1U << channel)))
            return std::unexpected(system_error::invalid);
        return c_.write(base + 0x600 + channel * 4, 0x6E524635);
    }
    [[nodiscard]] system_value<std::uint32_t> requests() noexcept { return c_.read(base + 0x404); }
    [[nodiscard]] system_result interrupt(bool on) noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        return c_.write(base + (on ? 0x304 : 0x308), 1);
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        auto running = c_.read(base + 0x400);
        if (!running)
            return std::unexpected(running.error());
        if (started_ || (*running & 1) || c_.referenced(8))
            return std::unexpected(system_error::busy); // reset is the only stop
        if (!c_.write(base + 0x308, 1))
            return std::unexpected(system_error::mmio);
        if (clock_) {
            auto r = clocks_.release_lf();
            if (!r)
                return r;
            clock_ = false;
        }
        auto r = c_.release(8);
        if (r)
            owned_ = ready_ = false;
        return r;
    }

  private:
    static constexpr std::uintptr_t base = 0x40010000;
    Context &c_;
    clock_controller<Context> &clocks_;
    std::uint8_t mask_{};
    bool owned_{}, ready_{}, clock_{}, started_{};
};
} // namespace mcubsp::backend::nrf52840
