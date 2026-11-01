#pragma once
// @req-file REQ-0056
#include <cstdint>
#include <mcubsp/backend/nrf52840/system_context.hpp>
namespace mcubsp::backend::nrf52840 {
enum class lf_source : std::uint32_t { rc = 0, crystal = 1, synthesized = 2 };
enum class lf_input : std::uint8_t { oscillator, external_low_swing, external_full_swing };
struct lf_configuration {
    lf_source source{lf_source::rc};
    lf_input input{lf_input::oscillator};
    constexpr bool operator==(const lf_configuration &) const noexcept = default;
};
enum class clock_event : unsigned {
    hf_started = 0,
    lf_started = 1,
    calibrated = 3,
    calibration_timeout = 4,
    calibration_timer_started = 10,
    calibration_timer_stopped = 11
};
enum class power_event : unsigned {
    power_fail = 2,
    sleep_enter = 5,
    sleep_exit = 6,
    usb_detected = 7,
    usb_removed = 8,
    usb_ready = 9
};
template <class Context> class clock_controller {
  public:
    explicit clock_controller(Context &c) noexcept : c_(c) {}
    clock_controller(const clock_controller &) = delete;
    clock_controller &operator=(const clock_controller &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(61);
        if (r) {
            owned_ = true;
            c_.publish(61);
        }
        return r;
    }
    [[nodiscard]] system_result acquire_hf(unsigned budget = 100000) noexcept {
        if (!owned_ || fault_)
            return std::unexpected(system_error::not_initialized);
        if (c_.execution.in_isr() || hf_ == 65535)
            return std::unexpected(system_error::invalid);
        if (hf_) {
            ++hf_;
            return {};
        }
        auto running = c_.read(0x40000408);
        if (!running)
            return std::unexpected(running.error());
        hf_started_ = (*running & 1) == 0;
        if (hf_started_ && (!c_.clear_event(0x40000100) || !c_.write(0x40000000, 1)))
            return fail();
        auto r = c_.wait_bits(0x4000040C, 0x10001, 0x10001, budget);
        if (!r) {
            fault_ = true;
            return r;
        }
        hf_ = 1;
        return {};
    }
    [[nodiscard]] system_result release_hf() noexcept {
        if (!owned_ || !hf_)
            return std::unexpected(system_error::not_initialized);
        if (hf_ > 1) {
            --hf_;
            return {};
        }
        if (calibrating_ || cal_timer_)
            return std::unexpected(system_error::busy);
        if (hf_started_) {
            if (!c_.write(0x40000004, 1) || !c_.wait_bits(0x40000408, 1, 0, 64))
                return fail();
        }
        hf_ = 0;
        hf_started_ = false;
        return {};
    }
    [[nodiscard]] system_result acquire_lf(lf_source source, unsigned budget = 100000) noexcept {
        return acquire_lf(lf_configuration{source}, budget);
    }
    [[nodiscard]] system_result acquire_lf(lf_configuration configuration,
                                           unsigned budget = 100000) noexcept {
        if (!owned_ || fault_)
            return std::unexpected(system_error::not_initialized);
        auto source = configuration.source;
        auto raw = static_cast<std::uint32_t>(source);
        auto input = static_cast<unsigned>(configuration.input);
        if (raw > 2 || input > 2 || (input && source != lf_source::crystal) ||
            c_.execution.in_isr() || lf_ == 65535)
            return std::unexpected(system_error::invalid);
        auto selection = raw | (input ? 1U << 17 : 0U) | (input == 2 ? 1U << 16 : 0U);
        if (lf_) {
            if (configuration != configuration_)
                return std::unexpected(system_error::busy);
            ++lf_;
            return {};
        }
        auto status = c_.read(0x40000418), run = c_.read(0x40000414);
        if (!status || !run)
            return std::unexpected(system_error::mmio);
        if ((*run & 1) && ((*status & 0x10003) != (0x10000 | raw)))
            return std::unexpected(system_error::busy);
        if (*run & 1) {
            // SRCCOPY latches SRC only. EXTERNAL/BYPASS are checked in LFCLKSRC,
            // which the specification forbids changing while LFCLK runs.
            auto inherited = c_.read(0x4000041C), selected = c_.read(0x40000518);
            if (!inherited || !selected)
                return std::unexpected(system_error::mmio);
            if ((*inherited & 3) != raw || (*selected & 0x30003) != selection)
                return std::unexpected(system_error::busy);
        }
        if (source == lf_source::synthesized) {
            auto h = acquire_hf(budget);
            if (!h)
                return h;
            synth_hf_ = true;
        }
        lf_started_ = (*run & 1) == 0;
        source_ = source;
        configuration_ = configuration;
        if (lf_started_ && (!c_.clear_event(0x40000104) || !c_.write(0x40000518, selection) ||
                            !c_.write(0x40000008, 1)))
            return fail();
        auto r = c_.wait_bits(0x40000418, 0x10003, 0x10000 | raw, budget);
        if (!r) {
            fault_ = true;
            return r;
        }
        lf_ = 1;
        return {};
    }
    [[nodiscard]] system_result release_lf(unsigned budget = 100000) noexcept {
        if (!owned_ || !lf_)
            return std::unexpected(system_error::not_initialized);
        if (lf_ > 1) {
            --lf_;
            return {};
        }
        if (calibrating_ || cal_timer_)
            return std::unexpected(system_error::busy);
        if (lf_started_) {
            auto state = c_.read(0x40000418);
            if (!state)
                return fail();
            // A previous STOP may have completed after a bounded wait expired.
            // The data sheet forbids issuing LFCLKSTOP while STATE is stopped.
            if ((*state & 0x10000) && !c_.write(0x4000000C, 1))
                return fail();
            auto stopped = c_.wait_bits(0x40000418, 0x10000, 0, budget);
            if (!stopped) {
                fault_ = true;
                return stopped;
            }
        }
        if (synth_hf_) {
            auto h = release_hf();
            if (!h)
                return h;
            synth_hf_ = false;
        }
        lf_ = 0;
        lf_started_ = false;
        return {};
    }
    [[nodiscard]] system_result synchronize_lf(unsigned budget = 100000) noexcept {
        if (!owned_ || fault_ || !lf_)
            return std::unexpected(system_error::not_initialized);
        // Erratum 20: wait for LFCLKSTARTED before touching RTC registers.
        if (!c_.clear_event(0x40000104) || !c_.write(0x40000008, 1))
            return fail();
        auto r = c_.wait_bits(0x40000104, 1, 1, budget);
        if (!r)
            fault_ = true;
        return r;
    }
    [[nodiscard]] system_result rc_mode(bool ultra_low_power) noexcept {
        if (!owned_ || fault_ || lf_ || calibrating_ || cal_timer_)
            return std::unexpected(system_error::busy);
        auto status = c_.read(0x40000418);
        if (!status)
            return std::unexpected(status.error());
        if (*status & 0x10000)
            return std::unexpected(system_error::busy);
        return c_.write(0x400005B4, ultra_low_power ? 1U : 0U);
    }
    [[nodiscard]] system_result hf_debounce(std::uint8_t units_16us) noexcept {
        if (!owned_ || fault_ || !units_16us)
            return std::unexpected(system_error::invalid);
        auto requested = c_.read(0x40000408);
        if (!requested)
            return std::unexpected(requested.error());
        if (hf_ || hf_started_ || (*requested & 1))
            return std::unexpected(system_error::busy);
        return c_.write(0x40000528, units_16us);
    }
    [[nodiscard]] system_result calibrate(unsigned budget = 100000) noexcept {
        if (!owned_ || fault_ || !lf_ || source_ != lf_source::rc || !hf_)
            return std::unexpected(system_error::invalid);
        auto mode = c_.read(0x400005B4);
        if (!mode)
            return std::unexpected(mode.error());
        if (*mode & 1)
            return std::unexpected(system_error::unsupported); // anomaly 209: no ULP calibration
        calibrating_ = true;
        if (!c_.clear_event(0x4000010C) || !c_.write(0x40000010, 1))
            return fail();
        auto r = c_.wait_bits(0x4000010C, 1, 1, budget);
        if (!r) {
            fault_ = true;
            return r;
        }
        calibrating_ = false;
        return c_.clear_event(0x4000010C);
    }
    [[nodiscard]] system_result calibration_timer(unsigned quarter_seconds,
                                                  unsigned budget = 100000) noexcept {
        if (!owned_ || (fault_ && quarter_seconds) || !lf_ || source_ != lf_source::rc ||
            quarter_seconds > 127)
            return std::unexpected(system_error::invalid);
        auto event = quarter_seconds ? 0x40000128U : 0x4000012CU;
        if (quarter_seconds) {
            if (!c_.clear_event(0x40000110))
                return fail();
            cal_timer_ = true; // START may commit before a failed acknowledgement.
        }
        if (!c_.clear_event(event) || !c_.write(0x40000538, quarter_seconds) ||
            !c_.write(quarter_seconds ? 0x40000014 : 0x40000018, 1))
            return fail();
        auto r = c_.wait_bits(event, 1, 1, budget);
        if (!r) {
            fault_ = true;
            return r;
        }
        cal_timer_ = quarter_seconds != 0;
        return {};
    }
    // Only CTTO is exposed as an asynchronously acknowledged clock interrupt.
    // Other events participate in synchronous startup/calibration handshakes;
    // they may be observed/routed, but an ISR must not consume their acknowledgements.
    [[nodiscard]] system_result timeout_interrupt(bool on) noexcept {
        if (!owned_ || fault_)
            return std::unexpected(system_error::not_initialized);
        if (on)
            timeout_irq_ = true;
        auto r = c_.write(on ? 0x40000304 : 0x40000308, 1U << 4);
        if (r && !on)
            timeout_irq_ = false;
        return r;
    }
    [[nodiscard]] system_value<bool> consume_timeout() noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        auto pending = c_.read(0x40000110);
        if (!pending)
            return std::unexpected(pending.error());
        if (*pending) {
            if (!c_.clear_event(0x40000110))
                return std::unexpected(system_error::mmio);
            cal_timer_ = false; // CTTO automatically stops the hardware timer.
        }
        return *pending != 0;
    }
    [[nodiscard]] system_endpoint event(clock_event source) const noexcept {
        auto n = static_cast<unsigned>(source);
        return owned_ && n < 12 && ((0xC1BU >> n) & 1U)
                   ? c_.endpoint(61, 0x40000100 + n * 4, endpoint_kind::event)
                   : system_endpoint{};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (hf_ || lf_ || calibrating_ || cal_timer_ || c_.referenced(61))
            return std::unexpected(system_error::busy);
        if (timeout_irq_ && (!c_.write(0x40000308, 1U << 4) || !c_.clear_event(0x40000110)))
            return fail();
        timeout_irq_ = false;
        // A failed asynchronous start retains the controller until sources are
        // quiescent.
        if (lf_started_) {
            auto s = c_.read(0x40000418);
            if (!s || !(*s & 0x10000))
                return std::unexpected(system_error::faulted);
            if (!c_.write(0x4000000C, 1) || !c_.wait_bits(0x40000418, 0x10000, 0, 100000))
                return fail();
        }
        if (hf_started_ && (!c_.write(0x40000004, 1) || !c_.wait_bits(0x40000408, 1, 0, 64)))
            return fail();
        auto r = c_.release(61);
        if (r) {
            owned_ = false;
            fault_ = false;
            lf_started_ = hf_started_ = false;
        }
        return r;
    }
    [[nodiscard]] bool has_clients() const noexcept {
        return hf_ || lf_ || calibrating_ || cal_timer_;
    }
    [[nodiscard]] unsigned hf_clients() const noexcept { return hf_; }
    [[nodiscard]] unsigned lf_clients() const noexcept { return lf_; }

  private:
    system_result fail() noexcept {
        fault_ = true;
        c_.invalidate(61);
        return std::unexpected(system_error::mmio);
    }
    Context &c_;
    unsigned hf_{}, lf_{};
    lf_source source_{};
    lf_configuration configuration_{};
    bool owned_{}, fault_{}, hf_started_{}, lf_started_{}, synth_hf_{}, calibrating_{},
        cal_timer_{}, timeout_irq_{};
};

// Hardware hooks deliberately take no responsibility for the application's
// ready predicate, external DMA clients or oscillator/board provisioning.
struct cortex_m_sleep {
    template <class Context> static system_result prepare(Context &c) noexcept {
#if defined(__arm__) || defined(__thumb__)
        auto enabled = c.read(0xE000ED88);
        if (!enabled)
            return std::unexpected(enabled.error());
        if ((*enabled & 0xF00000U) == 0xF00000U) {
            std::uint32_t status{};
            asm volatile("vmrs %0, fpscr" : "=r"(status));
            status &= ~0x9FU;
            asm volatile("vmsr fpscr, %0" ::"r"(status) : "memory");
            asm volatile("vmrs %0, fpscr" : "=r"(status));
            (void)status;
            if (!c.write(0xE000E284, 1U << 6))
                return std::unexpected(system_error::mmio); // anomaly 87, FPU IRQ 38
        }
#else
        (void)c;
#endif
        return {};
    }
    static void idle() noexcept {
#if defined(__arm__) || defined(__thumb__)
        asm volatile("dsb sy\n\twfi\n\tisb sy" ::: "memory");
#endif
    }
};
template <class Context, class Sleep = cortex_m_sleep> class power_controller {
  public:
    power_controller(Context &c, clock_controller<Context> &clocks) noexcept
        : c_(c), clocks_(clocks) {}
    power_controller(const power_controller &) = delete;
    power_controller &operator=(const power_controller &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(62);
        if (!r)
            return r;
        owned_ = true;
        auto part = c_.read(0x10000100);
        if (!part)
            return std::unexpected(part.error());
        if (*part != 0x52840)
            return std::unexpected(system_error::unsupported);
        // Nordic anomaly 246, all known nRF52840 revisions in pinned MDK predicates.
        auto applied = c_.write(0x4007AC84, 2);
        if (applied) {
            ready_ = true;
            c_.publish(62);
        }
        return applied;
    }
    [[nodiscard]] system_result interrupt(power_event source, bool on) noexcept {
        auto bit = event_bit(source);
        if (!owned_ || !ready_ || !bit)
            return std::unexpected(system_error::invalid);
        if (on)
            interrupts_ |= bit;
        auto r = c_.write(on ? 0x40000304 : 0x40000308, bit);
        if (r && !on)
            interrupts_ &= ~bit;
        return r;
    }
    [[nodiscard]] system_endpoint event(power_event source) const noexcept {
        return owned_ && ready_ && event_bit(source)
                   ? c_.endpoint(62, 0x40000100 + static_cast<unsigned>(source) * 4,
                                 endpoint_kind::event)
                   : system_endpoint{};
    }
    [[nodiscard]] system_value<bool> consume(power_event source) noexcept {
        if (source == power_event::power_fail && pof_owner_)
            return std::unexpected(system_error::busy);
        if (!owned_ || !ready_ || !event_bit(source))
            return std::unexpected(system_error::invalid);
        auto address = 0x40000100 + static_cast<unsigned>(source) * 4;
        auto r = c_.read(address);
        if (!r)
            return std::unexpected(r.error());
        if (*r && !c_.clear_event(address))
            return std::unexpected(system_error::mmio);
        return *r != 0;
    }
    [[nodiscard]] system_value<std::uint32_t> reset_reason() noexcept {
        return c_.read(0x40000400);
    }
    [[nodiscard]] system_result clear_reset_reason(std::uint32_t mask) noexcept {
        if (mask & ~0x1F000FU)
            return std::unexpected(system_error::invalid);
        return checked_write(0x40000400, mask);
    }
    [[nodiscard]] system_result retention(unsigned index, std::uint8_t value) noexcept {
        if (index > 1)
            return std::unexpected(system_error::invalid);
        return checked_write(0x4000051C + index * 4, value);
    }
    [[nodiscard]] system_result constant_latency(bool on) noexcept {
        return checked_write(on ? 0x40000078 : 0x4000007C, 1);
    }
    [[nodiscard]] system_value<std::uint32_t> usb_supply() noexcept { return c_.read(0x40000438); }
    // allowed_off is supplied by the memory owner for sections containing no live
    // code, data, vectors, stack or DMA buffers. Default forbids powering any
    // off.
    [[nodiscard]] system_result ram(unsigned block, unsigned powered, unsigned retained,
                                    unsigned allowed_off = 0) noexcept {
        unsigned count = mcu::nrf52840::ram_section_count(block);
        if (!count)
            return std::unexpected(system_error::invalid);
        auto mask = (1U << count) - 1;
        if ((powered | retained | allowed_off) & ~mask || ((mask & ~powered) & ~allowed_off))
            return std::unexpected(system_error::invalid);
        if (c_.active_work() && powered != mask)
            return std::unexpected(system_error::busy);
        return checked_write(0x40000900 + block * 0x10, powered | (retained << 16));
    }
    [[nodiscard]] system_result regulator(bool dc_dc, bool high_voltage,
                                          bool board_allows) noexcept {
        if (!board_allows)
            return std::unexpected(system_error::unsupported);
        return checked_write(high_voltage ? 0x40000580 : 0x40000578, dc_dc ? 1 : 0);
    }
    [[nodiscard]] system_result power_fail(unsigned threshold, bool enabled,
                                           unsigned high_voltage_threshold = 0) noexcept {
        if (pof_owner_)
            return std::unexpected(system_error::busy);
        if (threshold < 4 || threshold > 15 || high_voltage_threshold > 15)
            return std::unexpected(system_error::invalid);
        return checked_write(0x40000510, (enabled ? 1U : 0U) | (threshold << 1) |
                                             (high_voltage_threshold << 8));
    }
    // Erratum 242: a flash owner temporarily disables POFWARN and restores
    // the exact writable configuration after NVMC is ready and read-only.
    // A failed write retains this handoff for explicit recovery.
    [[nodiscard]] system_result suspend_power_fail(const void *owner) noexcept {
        if (!owner || !owned_ || !ready_ || c_.execution.in_isr())
            return std::unexpected(system_error::invalid);
        return c_.execution.critical([&]() -> system_result {
            if (pof_owner_ || c_.referenced(62))
                return std::unexpected(system_error::busy);
            auto warning = c_.read(0x40000108), config = c_.read(0x40000510);
            if (!warning || !config)
                return std::unexpected(system_error::mmio);
            if (*warning)
                return std::unexpected(system_error::busy);
            pof_saved_ = *config & 0xF1FU;
            pof_owner_ = owner;
            if (!c_.write(0x40000510, pof_saved_ & ~1U))
                return std::unexpected(system_error::mmio);
            arch::data_synchronization_barrier();
            warning = c_.read(0x40000108);
            if (!warning)
                return std::unexpected(system_error::mmio);
            if (*warning)
                return std::unexpected(system_error::busy);
            return {};
        });
    }
    [[nodiscard]] bool power_fail_suspended_by(const void *owner) const noexcept {
        return owner && pof_owner_ == owner;
    }
    [[nodiscard]] system_result restore_power_fail(const void *owner) noexcept {
        if (!owner || pof_owner_ != owner || c_.execution.in_isr())
            return std::unexpected(system_error::invalid);
        return c_.execution.critical([&]() -> system_result {
            if (!c_.write(0x40000510, pof_saved_))
                return std::unexpected(system_error::mmio);
            pof_owner_ = nullptr;
            return {};
        });
    }
    template <class Ready> [[nodiscard]] system_result idle_if(Ready &&safe_to_sleep) noexcept {
        if (!owned_ || !ready_ || c_.execution.in_isr())
            return std::unexpected(system_error::not_initialized);
        return c_.execution.critical([&]() -> system_result {
            if (!safe_to_sleep())
                return std::unexpected(system_error::busy);
            // WFI executes with the saved mask held: a newly pending interrupt wakes
            // the core before the mask is restored. Never clear a wake event here.
            if constexpr (requires { Sleep::prepare(c_); }) {
                auto p = Sleep::prepare(c_);
                if (!p)
                    return p;
            }
            Sleep::idle();
            return {};
        });
    }
    [[nodiscard]] system_result system_off(bool configured_wake_source) noexcept {
        if (!owned_ || !ready_)
            return std::unexpected(system_error::not_initialized);
        return c_.execution.critical([&]() -> system_result {
            if (!configured_wake_source || c_.system_off_blocked() || clocks_.has_clients())
                return std::unexpected(system_error::busy);
            arch::data_synchronization_barrier();
            return c_.write(0x40000500, 1);
        }); // Real System OFF wakes by reset; only debug emulation may return.
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(62) || pof_owner_)
            return std::unexpected(system_error::busy);
        if (interrupts_) {
            if (!c_.write(0x40000308, interrupts_))
                return std::unexpected(system_error::mmio);
            for (unsigned n = 0; n < 10; ++n)
                if ((interrupts_ & (1U << n)) && !c_.clear_event(0x40000100 + n * 4))
                    return std::unexpected(system_error::mmio);
            interrupts_ = 0;
        }
        auto r = c_.release(62);
        if (r) {
            owned_ = false;
            ready_ = false;
        }
        return r;
    }

  private:
    static constexpr std::uint32_t event_bit(power_event source) noexcept {
        auto n = static_cast<unsigned>(source);
        return n < 10 && ((0x3E4U >> n) & 1U) ? 1U << n : 0;
    }
    system_result checked_write(std::uintptr_t a, std::uint32_t v) noexcept {
        if (!owned_ || !ready_)
            return std::unexpected(system_error::not_initialized);
        return c_.write(a, v);
    }
    Context &c_;
    clock_controller<Context> &clocks_;
    std::uint32_t interrupts_{};
    std::uint32_t pof_saved_{};
    const void *pof_owner_{};
    bool owned_{}, ready_{};
};
} // namespace mcubsp::backend::nrf52840
