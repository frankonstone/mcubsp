#pragma once
// @req-file REQ-0027 REQ-0059 REQ-0062
// Legacy CPU-driven UART0: PS v1.11 6.33. UARTE remains a separate DMA backend.
#include <optional>
#include <type_traits>
#include <mcubsp/backend/nrf52840/uart_config.hpp>
#include <mcubsp/backend/nrf52840/clock_power.hpp>
#include <mcubsp/core/transfer.hpp>

namespace mcubsp::backend::nrf52840 {
template<class Binding, class Context, unsigned StopPollBudget=1000000>
    requires(Binding::instance==peripheral::uart0 && StopPollBudget>0)
class uart_controller {
    static_assert(Binding::base==mcu::nrf52840::uart_traits<peripheral::uart0>::base,
                  "UART0 backend requires the UART0 register block");
    using E=std::remove_reference_t<decltype(std::declval<Context&>().execution)>;
    using error=driver::uart_error;
  public:
    using tx_pool=core::operation_pool<void,error,1,E>;
    using rx_pool=core::operation_pool<std::size_t,error,1,E>;
    using tx_result=std::expected<typename tx_pool::transfer_type,typename tx_pool::error_type>;
    using rx_result=std::expected<typename rx_pool::transfer_type,typename rx_pool::error_type>;
    // Default cleanup deadline is 200 ms in the Pro Micro's 64 MHz cycle clock.
    // A borrowed execution clock with different units supplies its own interval.
    uart_controller(Context& c,clock_controller<Context>& clocks,
                    execution::tick stop_timeout_ticks=12'800'000) noexcept
        : c_(c),clocks_(clocks),tx_(c.execution),rx_(c.execution),stop_timeout_(stop_timeout_ticks) {}
    uart_controller(const uart_controller&)=delete;
    uart_controller& operator=(const uart_controller&)=delete;
    [[nodiscard]] std::expected<void,error> initialize(uart_baud_timing timing,driver::uart_format format={}) noexcept {
        if (c_.execution.in_isr() || owner_) return std::unexpected(error::already_owned);
        if (fault_) return std::unexpected(error::transport);
        if (!stop_timeout_ || stop_timeout_>execution::maximum_deadline_interval ||
            !uart_format_supported(Binding::instance,format) || !Binding::supports_rate(timing.achieved_baud))
            return std::unexpected(error::invalid_format);
        auto solved=solve_uart_baud(Binding::instance,{timing.clock_hz,timing.target_baud,timing.error_ppm});
        if (!solved || timing.instance!=Binding::instance || solved->encoding!=timing.encoding ||
            solved->achieved_baud!=timing.achieved_baud) return std::unexpected(error::invalid_baud);
        auto claim=c_.resources.claim(c_.execution,Binding::instance,this);
        if (!claim) return std::unexpected(error::already_owned);
        owner_=*claim;
        auto enabled=c_.read(base+0x500);
        if (!enabled || *enabled) { (void)release_claim();return std::unexpected(enabled ? error::already_owned : error::mmio); }
        // Claim every pin before configuring any of them.
        for (unsigned i=0;i<pins.size();++i) if (pins[i]!=uart_disconnected) {
            if (!c_.gpio.claim(pins[i],i<2 ? driver::gpio_direction::output : driver::gpio_direction::input)) {
                release_unconfigured_pins();(void)release_claim();return std::unexpected(error::already_owned);
            }
            pin_claimed_[i]=true;
        }
        if (!clocks_.acquire_hf()) { release_unconfigured_pins();(void)release_claim();return std::unexpected(error::transport); }
        hf_=true;
        for (unsigned i=0;i<pins.size();++i) if (pin_claimed_[i]) {
            if (!c_.gpio.configure(pins[i],{.direction=i<2 ? driver::gpio_direction::output : driver::gpio_direction::input,
                                               .initial_level=driver::gpio_level::high})) return init_failed();
        }
        if (!write(0x308,irq_mask) || !write(0x200,0) || !clear_rx() || !write(0x11C,0) ||
            !write(0x508,select(Binding::rts)) || !write(0x50C,select(Binding::tx)) ||
            !write(0x510,select(Binding::cts)) || !write(0x514,select(Binding::rx)) ||
            !write(0x524,timing.encoding) || !write(0x56C,(Binding::flow_control ? 1U:0U) |
                    (format.parity==driver::uart_parity::even ? 14U:0U)) || !write(0x500,4)) return init_failed();
        ready_=true;work_=true;c_.work_started();return {};
    }
    [[nodiscard]] tx_result transmit(std::span<const std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure<tx_result>(error::empty_transfer);
        if (!ready_ || fault_ || c_.execution.in_isr()) return failure<tx_result>(error::not_initialized);
        return c_.execution.critical([&]() -> tx_result {
            auto operation=tx_.try_start({.quiesce=&cancel_tx,.context=this});
            if (!operation) return std::unexpected(operation.error());
            tx_id_=operation->id();output_=bytes;sent_=0;
            if (!write(0x11C,0) || !write(0x008,1) || !write(0x51C,bytes[0])) fail_io();
            return operation;
        });
    }
    [[nodiscard]] rx_result receive(std::span<std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure<rx_result>(error::empty_transfer);
        if (!ready_ || fault_ || c_.execution.in_isr()) return failure<rx_result>(error::not_initialized);
        return c_.execution.critical([&]() -> rx_result {
            if (rx_id_) return failure<rx_result>(error::busy);
            auto stale=drain();
            if (!stale) { fail_io();return failure<rx_result>(error::mmio); }
            if (*stale) return failure<rx_result>(error::overrun); // discarded bytes are observable
            auto operation=rx_.try_start({.quiesce=&cancel_rx,.context=this});
            if (!operation) return std::unexpected(operation.error());
            rx_id_=operation->id();input_=bytes;received_=0;error_bits_=0;rx_stopping_=false;rx_error_.reset();
            if (!clear_rx()) fail_io();
            else {
                rx_hardware_active_=true; // START may commit before reporting a bus error
                if (!write(0x000,1)) fail_io();
            }
            return operation;
        });
    }
    // Foreground polling or a caller-owned IRQ subscription may invoke service.
    // One TX byte and one RX byte per call; the final FIFO drain is bounded at 6.
    [[nodiscard]] bool poll() noexcept {
        if (!ready_ || fault_) return false;
        return c_.execution.critical([&] {
            bool progress{};
            if (tx_id_) {
                auto ready=c_.read(base+0x11C);
                if (!ready) { fail_io();return true; }
                if (*ready) {
                    if (!write(0x11C,0)) { fail_io();return true; }
                    ++sent_;progress=true;
                    if (sent_<output_.size()) {
                        if (!write(0x51C,output_[sent_])) { fail_io();return true; }
                    } else {
                        if (!write(0x00C,1)) { fail_io();return true; }
                        auto id=*tx_id_;tx_id_.reset();output_={};(void)tx_.complete(id);
                    }
                }
            }
            if (rx_id_) {
                auto failed=c_.read(base+0x124);
                if (!failed) { fail_io();return true; }
                if (*failed) {
                    // Clear the event first so a new cause can reassert it
                    // while ERRORSRC is sampled and acknowledged with W1C.
                    if (!write(0x124,0)) { fail_io();return true; }
                    auto source=c_.read(base+0x480);
                    if (!source || !write(0x480,*source&15U)) { fail_io();return true; }
                    error_bits_|=*source&15U;
                    if (!rx_error_) rx_error_=(*source&1U) ? error::overrun : (*source&2U) ? error::parity :
                                              (*source&12U) ? error::framing : error::transport;
                    if (!stop_receive()) { fail_io();return true; }
                }
                auto ready=c_.read(base+0x108);
                if (!ready) { fail_io();return true; }
                if (*ready && received_<input_.size()) {
                    // Clear BEFORE RXD read: reading RXD can immediately assert a new RXDRDY.
                    if (!write(0x108,0)) { fail_io();return true; }
                    auto byte=c_.read(base+0x518);
                    if (!byte) { fail_io();return true; }
                    input_[received_++]=static_cast<std::uint8_t>(*byte);progress=true;
                    if (received_==input_.size() && !stop_receive()) { fail_io();return true; }
                }
                if (rx_stopping_) {
                    auto stopped=c_.read(base+0x144);
                    if (!stopped) { fail_io();return true; }
                    if (*stopped) {
                        rx_hardware_active_=false;
                        auto extra=drain();
                        if (!extra || !clear_rx()) { fail_io();return true; }
                        if (*extra && !rx_error_) rx_error_=error::overrun;
                        auto id=*rx_id_;rx_id_.reset();input_={};rx_stopping_=false;
                        if (rx_error_) (void)rx_.fail(id,*rx_error_);else (void)rx_.complete(id,received_);
                        progress=true;
                    }
                }
            }
            return progress;
        });
    }
    [[nodiscard]] std::expected<void,error> interrupt_delivery(bool enable) noexcept {
        if (!ready_ || c_.execution.in_isr()) return std::unexpected(error::not_initialized);
        if (!write(enable ? 0x304:0x308,irq_mask)) return std::unexpected(error::mmio);
        return {};
    }
    [[nodiscard]] std::expected<void,error> shutdown() noexcept {
        if (c_.execution.in_isr() || tx_id_ || rx_id_) return std::unexpected(error::busy);
        if (!owner_) return {};
        if (rx_hardware_active_ && !stop_receive_bounded()) return std::unexpected(error::transport);
        if (!write(0x308,irq_mask) || !write(0x00C,1) || !write(0x500,0)) return std::unexpected(error::mmio);
        auto enabled=c_.read(base+0x500);
        if (!enabled || *enabled) return std::unexpected(error::mmio);
        for (unsigned offset:{0x508U,0x50CU,0x510U,0x514U})
            if (!write(offset,0x80000000)) return std::unexpected(error::mmio);
        for (unsigned i=0;i<pins.size();++i) if (pin_claimed_[i] && !c_.gpio.configure(pins[i],{})) return std::unexpected(error::mmio);
        if (hf_ && !clocks_.release_hf()) return std::unexpected(error::transport);
        hf_=false;release_unconfigured_pins();
        if (!release_claim()) return std::unexpected(error::busy);
        if (work_) {c_.work_stopped();work_=false;}
        ready_=false;return {};
    }
    [[nodiscard]] std::size_t received_count() const noexcept {
        return c_.execution.critical([&] {return received_;});
    }
    [[nodiscard]] std::uint32_t error_source() const noexcept {
        return c_.execution.critical([&] {return error_bits_;});
    }
    [[nodiscard]] bool faulted() const noexcept {
        return c_.execution.critical([&] {return fault_;});
    }
  private:
    template<class Result> static Result failure(error e) noexcept {
        return std::unexpected(typename Result::error_type{core::transfer_error_kind::failure,e});
    }
    bool write(unsigned offset,std::uint32_t value) noexcept {
        if (offset>=0x100 && offset<0x200 && value==0)
            return c_.clear_event(base+offset).has_value();
        auto r=c_.write(base+offset,value);arch::data_memory_barrier();return r.has_value();
    }
    static constexpr std::uint32_t select(core::pin_id p) noexcept {
        return p==uart_disconnected ? 0x80000000U : (std::uint32_t(p.port)<<5)|p.index;
    }
    bool release_claim() noexcept {
        if (!owner_) return true;
        auto r=c_.resources.release(c_.execution,*owner_);if (r) owner_.reset();return r.has_value();
    }
    void release_unconfigured_pins() noexcept {
        for (unsigned i=0;i<pins.size();++i) if (pin_claimed_[i]) {c_.gpio.release(pins[i]);pin_claimed_[i]=false;}
    }
    std::expected<void,error> init_failed() noexcept {
        fault_=true;(void)shutdown();return std::unexpected(error::mmio);
    }
    bool clear_rx() noexcept {return write(0x108,0) && write(0x124,0) && write(0x144,0) && write(0x480,15);}
    std::expected<unsigned,error> drain() noexcept {
        unsigned count{};
        for (;count<6;++count) {
            auto ready=c_.read(base+0x108);if (!ready) return std::unexpected(error::mmio);
            if (!*ready) return count;
            if (!write(0x108,0) || !c_.read(base+0x518)) return std::unexpected(error::mmio);
        }
        auto pending=c_.read(base+0x108);
        if (!pending || *pending) return std::unexpected(error::overrun);
        return count;
    }
    bool stop_receive() noexcept {
        if (rx_stopping_) return true;
        if (!write(0x144,0)) return false;
        rx_stopping_=true;return write(0x004,1);
    }
    bool stop_receive_bounded() noexcept {
        // A late acknowledgment from the previous bounded stop is still proof
        // of quiescence. Never erase it and wait for a nonexistent second stop.
        auto pending=c_.read(base+0x144);
        if (!pending) return false;
        if (*pending) {rx_hardware_active_=false;return true;}
        if (rx_stopping_ ? !write(0x004,1) : !stop_receive()) return false;
        execution::deadline deadline{static_cast<execution::tick>(c_.execution.now()+stop_timeout_)};
        // ISR fault/cancel paths observe once and retain ownership on a
        // pending stop. A foreground shutdown performs deadline-based recovery.
        const auto budget=c_.execution.in_isr() ? 1U:StopPollBudget;
        for (unsigned i=0;i<budget;++i) {
            auto stopped=c_.read(base+0x144);if (!stopped) return false;
            if (*stopped) {rx_hardware_active_=false;return true;}
            if (execution::reached(c_.execution.now(),deadline)) break;
        }
        return false;
    }
    static core::quiesce_result cancel_tx(void *state,core::operation_id id) noexcept {
        auto &s=*static_cast<uart_controller*>(state);
        if (!s.tx_id_ || *s.tx_id_!=id) return core::quiesce_result::reusable;
        bool stopped=s.write(0x00C,1) && s.write(0x11C,0);
        s.tx_id_.reset();s.output_={};s.fault_|=!stopped;
        if (!stopped) s.fail_io();
        return stopped ? core::quiesce_result::reusable:core::quiesce_result::faulted;
    }
    static core::quiesce_result cancel_rx(void *state,core::operation_id id) noexcept {
        auto &s=*static_cast<uart_controller*>(state);
        if (!s.rx_id_ || *s.rx_id_!=id) return core::quiesce_result::reusable;
        bool stopped=s.stop_receive_bounded();
        if (stopped) stopped=s.drain().has_value() && s.clear_rx();
        s.rx_id_.reset();s.input_={};if (stopped) s.rx_stopping_=false;s.fault_|=!stopped;
        if (!stopped) s.fail_io();
        return stopped ? core::quiesce_result::reusable:core::quiesce_result::faulted;
    }
    void fail_io() noexcept {
        fault_=true;
        // UART0 has no DMA. Detaching CPU spans makes memory quiescent even
        // when hardware STOP fails; retain pins/clock/instance until shutdown.
        (void)write(0x308,irq_mask);(void)write(0x00C,1);
        if (rx_hardware_active_) (void)stop_receive_bounded();
        if (tx_id_) {auto id=*tx_id_;tx_id_.reset();output_={};(void)tx_.fail(id,error::mmio);}
        if (rx_id_) {auto id=*rx_id_;rx_id_.reset();input_={};(void)rx_.fail(id,error::mmio);}
    }
    static constexpr std::uintptr_t base=Binding::base;
    static constexpr std::uint32_t irq_mask=(1U<<2)|(1U<<7)|(1U<<9)|(1U<<17);
    static constexpr std::array pins{Binding::tx,Binding::rts,Binding::rx,Binding::cts};
    Context& c_;clock_controller<Context>& clocks_;tx_pool tx_;rx_pool rx_;
    execution::tick stop_timeout_;std::optional<resource_token> owner_;
    std::optional<core::operation_id> tx_id_,rx_id_;
    std::span<const std::uint8_t> output_;std::span<std::uint8_t> input_;
    std::optional<error> rx_error_;std::array<bool,4> pin_claimed_{};
    std::size_t sent_{},received_{};
    std::uint32_t error_bits_{};
    bool ready_{},fault_{},hf_{},work_{},rx_stopping_{},rx_hardware_active_{};
};
} // namespace mcubsp::backend::nrf52840
