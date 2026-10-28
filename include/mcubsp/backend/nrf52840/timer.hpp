#pragma once
// @req-file REQ-0058
#include <mcubsp/backend/nrf52840/system_context.hpp>
namespace mcubsp::backend::nrf52840 {
enum class timer_mode : std::uint32_t { timer = 0, counter = 1, low_power_counter = 2 };
template <unsigned Index, class Context>
    requires(Index < 5)
class timer_controller {
  public:
    explicit timer_controller(Context &c) noexcept : c_(c) {}
    timer_controller(const timer_controller &) = delete;
    timer_controller &operator=(const timer_controller &) = delete;
    static constexpr unsigned channels = mcu::nrf52840::timer_channels[Index],
                              irq = mcu::nrf52840::timer_irqs[Index];
    [[nodiscard]] system_result initialize(timer_mode mode = timer_mode::timer, unsigned bits = 32,
                                           unsigned prescaler = 4) noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto raw = static_cast<std::uint32_t>(mode);
        if (raw > 2 || prescaler > 9 || (bits != 8 && bits != 16 && bits != 24 && bits != 32))
            return std::unexpected(system_error::invalid);
        auto r = c_.claim(Index);
        if (!r)
            return r;
        owned_ = true;
        // Stop/shutdown rather than STOP alone (Nordic anomaly 78).
        if (!c_.write(base + 4, 1) || !c_.write(base + 0x10, 1) ||
            !c_.write(base + 0x308, ((1U << channels) - 1) << 16) || !c_.write(base + 0x200, 0))
            return fail();
        for (unsigned n = 0; n < channels; ++n)
            if (!c_.clear_event(base + 0x140 + n * 4))
                return fail();
        if (!c_.write(base + 0x504, raw) ||
            !c_.write(base + 0x508, bits == 16   ? 0U
                                    : bits == 8  ? 1U
                                    : bits == 24 ? 2U
                                                 : 3U) ||
            !c_.write(base + 0x510, prescaler) || !c_.write(base + 0xC, 1))
            return fail();
        bits_ = bits;
        mode_ = mode;
        ready_ = true;
        c_.publish(Index);
        return {};
    }
    [[nodiscard]] system_result compare(unsigned n, std::uint32_t value, bool clear = false,
                                        bool stop = false, bool interrupt = false) noexcept {
        if (!ready_ || n >= channels || (bits_ < 32 && value >= (1U << bits_)))
            return std::unexpected(system_error::invalid);
        auto s = c_.read(base + 0x200);
        if (!s)
            return fail();
        auto mask = (1U << n) | (1U << (8 + n));
        if (!c_.write(base + 0x308, 1U << (16 + n)) || !c_.clear_event(base + 0x140 + n * 4) ||
            !c_.write(base + 0x540 + n * 4, value) ||
            !c_.write(base + 0x200,
                      (*s & ~mask) | (clear ? 1U << n : 0) | (stop ? 1U << (8 + n) : 0)) ||
            (interrupt && !c_.write(base + 0x304, 1U << (16 + n))))
            return fail();
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
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        if (!c_.write(base + 4, 1) || !c_.write(base + 0x10, 1))
            return fail();
        arch::data_synchronization_barrier();
        if (running_) {
            c_.work_stopped();
            running_ = false;
        }
        return {};
    }
    [[nodiscard]] system_result count() noexcept {
        if (!ready_ || mode_ == timer_mode::timer)
            return std::unexpected(system_error::invalid);
        return c_.write(base + 8, 1);
    }
    [[nodiscard]] system_result clear() noexcept {
        if (!ready_)
            return std::unexpected(system_error::not_initialized);
        return c_.write(base + 0xC, 1);
    }
    [[nodiscard]] system_value<std::uint32_t> capture(unsigned n) noexcept {
        if (!ready_ || n >= channels)
            return std::unexpected(system_error::invalid);
        if (!c_.write(base + 0x40 + n * 4, 1))
            return std::unexpected(system_error::mmio);
        return c_.read(base + 0x540 + n * 4);
    }
    [[nodiscard]] system_value<bool> consume(unsigned n) noexcept {
        if (!ready_ || n >= channels)
            return std::unexpected(system_error::invalid);
        auto r = c_.read(base + 0x140 + n * 4);
        if (!r)
            return std::unexpected(r.error());
        if (*r && !c_.clear_event(base + 0x140 + n * 4))
            return std::unexpected(system_error::mmio);
        return *r != 0;
    }
    [[nodiscard]] system_endpoint event(unsigned n) const noexcept {
        return ready_ && n < channels
                   ? c_.endpoint(Index, base + 0x140 + n * 4, endpoint_kind::event)
                   : system_endpoint{};
    }
    [[nodiscard]] system_endpoint capture_task(unsigned n) const noexcept {
        return ready_ && n < channels ? c_.endpoint(Index, base + 0x40 + n * 4, endpoint_kind::task)
                                      : system_endpoint{};
    }
    [[nodiscard]] system_endpoint clear_task() const noexcept {
        return ready_ ? c_.endpoint(Index, base + 0xC, endpoint_kind::task) : system_endpoint{};
    }
    [[nodiscard]] system_endpoint count_task() const noexcept {
        return ready_ && mode_ != timer_mode::timer
                   ? c_.endpoint(Index, base + 8, endpoint_kind::task)
                   : system_endpoint{};
    }
    [[nodiscard]] system_endpoint start_task() const noexcept {
        return ready_ ? c_.endpoint(Index, base, endpoint_kind::task) : system_endpoint{};
    }
    [[nodiscard]] system_endpoint stop_task() const noexcept {
        return ready_ ? c_.endpoint(Index, base + 4, endpoint_kind::task) : system_endpoint{};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(Index))
            return std::unexpected(system_error::busy);
        if (!stop() || !c_.write(base + 0x308, ((1U << channels) - 1) << 16) ||
            !c_.write(base + 0x200, 0))
            return fail();
        for (unsigned n = 0; n < channels; ++n)
            if (!c_.clear_event(base + 0x140 + n * 4))
                return fail();
        auto r = c_.release(Index);
        if (r) {
            owned_ = ready_ = false;
        }
        return r;
    }

  private:
    system_result fail() noexcept {
        ready_ = false;
        c_.invalidate(Index);
        return std::unexpected(system_error::mmio);
    }
    static constexpr std::uintptr_t base = mcu::nrf52840::timer_bases[Index];
    Context &c_;
    unsigned bits_{};
    timer_mode mode_{};
    bool owned_{}, ready_{}, running_{};
};
} // namespace mcubsp::backend::nrf52840
