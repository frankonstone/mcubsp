#pragma once

// @req-file REQ-0002 REQ-0014 REQ-0015 REQ-0018 REQ-0045
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/io/detail/context.hpp>
#include <mcuhw/io/digital.hpp>
#include <mcuhw/io/analog.hpp>
#include <mcuhw/io/dma.hpp>
#include <mcuhw/io/i2c.hpp>
#include <mcuhw/io/serial.hpp>
#include <mcuhw/io/spi.hpp>
#include <mcuhw/io/pwm.hpp>

namespace mcuhw::io {
namespace detail {
template<template<class> class Feature, class Context, bool Enabled>
struct peripheral_storage {
    explicit peripheral_storage(Context&) noexcept {}
    [[nodiscard]] result<void> shutdown() noexcept { return {}; }
    void poll() noexcept {}
    template<class... Args> void begin(Args&&...) = delete;
};
template<template<class> class Feature, class Context>
struct peripheral_storage<Feature, Context, true> : Feature<Context> {
    using Feature<Context>::Feature;
};
} // namespace detail

// No construction touches hardware. Objects are deliberately immovable:
// completion state and progress callbacks always refer to stable storage.
template<class Board, board_config Configuration = {},
         class Policy = typename Board::default_execution,
         class Mmio = core::direct_mmio,
         bool BorrowExecution = false, bool BorrowMmio = false>
class board {
    static_assert((Configuration.dma & ~Board::dma_available) == 0U,
                  "selected DMA channels have no supported board backend");
    static_assert(!Configuration.adc || Board::adc_available,
                  "this board profile has no supported ADC backend");
    static_assert(!Configuration.pwm || Board::pwm_available,
                  "this board profile has no supported PWM backend");
public:
    using board_type = Board;
    using context_type = detail::context<Board, Mmio, Policy>;
    using execution_type = typename context_type::execution_type;
    using gpio_backend_type = typename Board::template gpio_backend<Mmio>;
    static constexpr auto configuration = Configuration;
    static constexpr typename Board::pins_type pins{};

private:
    [[no_unique_address]] detail::storage<Policy, BorrowExecution> policy_;
    [[no_unique_address]] detail::storage<Mmio, BorrowMmio> mmio_;
    context_type context_;
    gpio_backend_type gpio_;

public:
    [[no_unique_address]] detail::peripheral_storage<serial_port, context_type, Configuration.serial> serial;
    [[no_unique_address]] detail::peripheral_storage<i2c_bus, context_type, Configuration.i2c> i2c;
    [[no_unique_address]] detail::peripheral_storage<spi_bus, context_type, Configuration.spi> spi;
    [[no_unique_address]] dma_bank<context_type, Configuration.dma> dma;
    [[no_unique_address]] detail::peripheral_storage<analog_input, context_type, Configuration.adc> analog;
    [[no_unique_address]] detail::peripheral_storage<pwm_output, context_type, Configuration.pwm> pwm;

    board() noexcept requires (!BorrowExecution && !BorrowMmio)
        : policy_{}, mmio_{}, context_(mmio_.get(), policy_.get(), Board::ticks_per_second),
          gpio_(mmio_.get()), serial(context_), i2c(context_), spi(context_), dma(context_), analog(context_), pwm(context_) {
        install_progress();
    }
    explicit board(Policy& policy, std::uint32_t frequency = Board::ticks_per_second) noexcept
        requires (BorrowExecution && !BorrowMmio)
        : policy_(policy), mmio_{}, context_(mmio_.get(), policy_.get(), frequency),
          gpio_(mmio_.get()), serial(context_), i2c(context_), spi(context_), dma(context_), analog(context_), pwm(context_) {
        install_progress();
    }
    board(Policy& policy, Mmio& mmio, std::uint32_t frequency = Board::ticks_per_second) noexcept
        requires (BorrowExecution && BorrowMmio)
        : policy_(policy), mmio_(mmio), context_(mmio_.get(), policy_.get(), frequency),
          gpio_(mmio_.get()), serial(context_), i2c(context_), spi(context_), dma(context_), analog(context_), pwm(context_) {
        install_progress();
    }
    board(const board&) = delete;
    board& operator=(const board&) = delete;
    board(board&&) = delete;
    board& operator=(board&&) = delete;
    ~board() {
        if (context_.started() && !shutdown()) {
            context_.execution().active_transfer_destroyed();
            __builtin_trap();
        }
    }

    [[nodiscard]] result<void> begin() noexcept {
        const auto activated = context_.activate();
        if (!activated) return activated;
        Board::start(mmio_.get());
        if constexpr (!BorrowExecution && requires { policy_.get().begin(); }) policy_.get().begin();
        return {};
    }
    [[nodiscard]] result<void> shutdown() noexcept {
        if (!context_.started()) return {};
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (auto stopped = serial.shutdown(); !stopped) return stopped;
        if (auto stopped = i2c.shutdown(); !stopped) return stopped;
        if (auto stopped = spi.shutdown(); !stopped) return stopped;
        if (auto stopped = analog.shutdown(); !stopped) return stopped;
        if (auto stopped = pwm.shutdown(); !stopped) return stopped;
        if (auto stopped = context_.deactivate(); !stopped) return stopped;
        Board::stop(mmio_.get());
        return {};
    }
    [[nodiscard]] bool started() const noexcept { return context_.started(); }
    [[nodiscard]] bool startup_ok() const noexcept { return Board::startup_ok(); }

    [[nodiscard]] result<digital_pin<context_type>> output(
        core::pin_id pin, bool initial_high = false) noexcept {
        return digital_pin<context_type>::acquire(context_, gpio_, pin, {
            .direction = driver::gpio_direction::output,
            .initial_level = initial_high ? driver::gpio_level::high : driver::gpio_level::low,
        });
    }
    [[nodiscard]] result<digital_pin<context_type>> input(
        core::pin_id pin, pull resistor = pull::none) noexcept {
        return digital_pin<context_type>::acquire(context_, gpio_, pin, {
            .direction = driver::gpio_direction::input, .pull = resistor,
        });
    }
    // Explicit electrical settings still use the same registry and handle.
    [[nodiscard]] result<digital_pin<context_type>> pin(
        core::pin_id pin, driver::gpio_configuration configuration) noexcept {
        return digital_pin<context_type>::acquire(context_, gpio_, pin, configuration);
    }

    void poll() noexcept {
        if (!context_.started() || context_.execution().in_isr()) return;
        // DMA completion must be visible before the peripheral consumes it.
        dma.poll();
        serial.poll();
        i2c.poll();
        spi.poll();
        analog.poll();
        pwm.poll();
    }
    template<class Rep, class Period>
    [[nodiscard]] result<void> delay(std::chrono::duration<Rep, Period> interval) noexcept {
        const auto limit = context_.deadline_after(interval);
        if (!limit) return std::unexpected(limit.error());
        auto& execution = context_.execution();
        for (;;) {
            const auto token = execution.prepare_wait();
            if (mcuhw::execution::reached(execution.now(), *limit)) return {};
            execution.wait_once(token, *limit);
        }
    }
    template<class Rep, class Period>
    [[nodiscard]] auto deadline_after(std::chrono::duration<Rep, Period> interval) noexcept {
        return context_.deadline_after(interval);
    }
    [[nodiscard]] auto now() const noexcept {
        return context_.execution().now();
    }
    execution_type& execution() noexcept { return context_.execution(); }

private:
    void install_progress() noexcept {
        context_.execution().set_progress([](void* value) noexcept {
            static_cast<board*>(value)->poll();
        }, this);
    }
};

template<class Board, board_config Configuration = {}, class Policy>
[[nodiscard]] auto make_board(Policy& policy,
                             std::uint32_t frequency = Board::ticks_per_second) noexcept {
    return board<Board, Configuration, Policy, core::direct_mmio, true, false>{policy, frequency};
}
template<class Board, board_config Configuration = {}, class Policy, class Mmio>
[[nodiscard]] auto make_board(Policy& policy, Mmio& mmio,
                             std::uint32_t frequency = Board::ticks_per_second) noexcept {
    return board<Board, Configuration, Policy, Mmio, true, true>{policy, mmio, frequency};
}

} // namespace mcuhw::io
