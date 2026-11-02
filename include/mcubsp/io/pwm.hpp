#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0022 REQ-0045
#include <chrono>
#include <cstdint>

#include <mcubsp/driver/pwm.hpp>
#include <mcubsp/io/core.hpp>
#include <mcubsp/io/operation.hpp>

namespace mcubsp::io {

// Exact rational duty values avoid floating-point support. Out-of-range values
// remain representable so begin()/duty() can report invalid_duty normally.
[[nodiscard]] constexpr driver::pwm_duty percent(std::uint32_t value) noexcept {
    return driver::pwm_duty::percent(value);
}

template<class Context>
class pwm_output {
    using board_type = typename Context::board_type;

public:
    using backend_type = typename board_type::template pwm_backend<
        typename Context::mmio_type, typename Context::execution_type>;
    using driver_type = driver::pwm_channel<backend_type>;

    explicit pwm_output(Context& context) noexcept
        : context_(context), backend_(detail::make_backend<backend_type>(context)), driver_(backend_) {}
    pwm_output(const pwm_output&) = delete;
    pwm_output& operator=(const pwm_output&) = delete;
    pwm_output(pwm_output&&) = delete;
    pwm_output& operator=(pwm_output&&) = delete;

    ~pwm_output() {
        if (driver_.initialized() && !shutdown()) {
            context_.execution().fail({"pwm", "live resources or failed cleanup during destruction"});
            __builtin_trap();
        }
    }

    // Initialization starts the configured waveform; the default duty is 0%.
    [[nodiscard]] result<void> begin(
        std::uint32_t hz = 1'000U, driver::pwm_duty initial = percent(0U)) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!context_.started()) return failure(driver::pwm_error::not_initialized);
        if (driver_.initialized()) return failure(driver::pwm_error::already_initialized);
        const auto timing = board_type::solve_pwm(hz);
        if (!timing) return failure(timing.error());
        // Validate before claiming/configuring any physical resource.
        const auto duty = driver::solve_pwm_duty(timing->period_counts, initial);
        if (!duty) return failure(duty.error());
        const auto claimed = context_.claim(board_type::pwm_pins);
        if (!claimed) return claimed;
        const auto initialized = driver_.initialize(*timing, initial);
        if (!initialized) {
            if(driver_.initialized()){context_.retain();return failure(initialized.error());}
            context_.release(board_type::pwm_pins);
            return failure(initialized.error());
        }
        // Retain as soon as the channel owns hardware, even if start fails.
        context_.retain();
        const auto running = driver_.start();
        if (!running) {
            const auto cleanup = shutdown();
            if (!cleanup) return cleanup;
            return failure(running.error());
        }
        return {};
    }

    [[nodiscard]] result<void> shutdown() noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!driver_.initialized()) return {};
        const auto stopped = driver_.release();
        if (!stopped) return failure(stopped.error());
        context_.release(board_type::pwm_pins);
        context_.drop();
        return {};
    }

    [[nodiscard]] result<void> duty(driver::pwm_duty value) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        const auto updated = driver_.update_duty(value);
        if (!updated) return failure(updated.error());
        return {};
    }

    [[nodiscard]] result<void> start() noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        const auto started = driver_.start();
        if (!started) return failure(started.error());
        return {};
    }

    [[nodiscard]] result<void> stop() noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        const auto stopped = driver_.stop();
        if (!stopped) return failure(stopped.error());
        return {};
    }

    void poll() noexcept {
        if (driver_.initialized() && !context_.execution().in_isr())
            board_type::poll_pwm(context_.mmio(), backend_);
    }

    [[nodiscard]] bool initialized() const noexcept { return driver_.initialized(); }
    [[nodiscard]] bool running() const noexcept { return driver_.running(); }
    [[nodiscard]] driver::pwm_timing timing() const noexcept { return driver_.timing(); }
    [[nodiscard]] const backend_type& driver() const noexcept { return backend_; }
    // Borrowed same-backend access; bypasses facade lifecycle guarantees.
    [[nodiscard]] backend_type& unsafe_driver() noexcept { return backend_; }

private:
    [[nodiscard]] static result<void> failure(driver::pwm_error cause) noexcept {
        return std::unexpected(to_error(cause));
    }

    Context& context_;
    backend_type backend_;
    driver_type driver_;
};

} // namespace mcubsp::io
