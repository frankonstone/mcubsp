#pragma once

// @req-file REQ-0014 REQ-0021 REQ-0045
#include <array>
#include <utility>
#include <mcubsp/io/core.hpp>

namespace mcubsp::io {

using pull = driver::gpio_pull;

template<class Context>
class digital_pin {
    using Board = typename Context::board_type;
    using backend_type = typename Board::template gpio_backend<typename Context::mmio_type>;
public:
    digital_pin(const digital_pin&) = delete;
    digital_pin& operator=(const digital_pin&) = delete;
    digital_pin(digital_pin&& other) noexcept
        : context_(std::exchange(other.context_, nullptr)), backend_(other.backend_),
          pin_(other.pin_), output_(other.output_), high_(other.high_) {}
    digital_pin& operator=(digital_pin&&) = delete;
    ~digital_pin() {
        if (context_ && !release()) {
            context_->execution().active_transfer_destroyed();
            __builtin_trap();
        }
    }

    [[nodiscard]] static result<digital_pin> acquire(
        Context& context, backend_type& backend, core::pin_id pin,
        driver::gpio_configuration configuration) noexcept {
        const std::array pins{pin};
        if (auto claim = context.claim(pins); !claim) return std::unexpected(claim.error());
        const auto claimed = backend.claim(pin, configuration.direction);
        if (!claimed) {
            context.release(pins);
            return std::unexpected(to_error(claimed.error()));
        }
        const auto configured = backend.configure(pin, configuration);
        if (!configured) {
            backend.release(pin);
            context.release(pins);
            return std::unexpected(to_error(configured.error()));
        }
        context.retain();
        return digital_pin{context, backend, pin, configuration};
    }
    [[nodiscard]] result<void> write(bool high) noexcept {
        if (!context_) return std::unexpected(error{errc::not_started});
        if (context_->execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!output_) return std::unexpected(error{errc::invalid_operation});
        const auto written = high ? backend_->set(pin_) : backend_->clear(pin_);
        if (!written) return std::unexpected(to_error(written.error()));
        high_ = high;
        return {};
    }
    [[nodiscard]] result<void> toggle() noexcept { return write(!high_); }
    [[nodiscard]] result<bool> read() noexcept {
        if (!context_) return std::unexpected(error{errc::not_started});
        if (context_->execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        const auto value = backend_->read(pin_);
        if (!value) return std::unexpected(to_error(value.error()));
        return *value == driver::gpio_level::high;
    }
    [[nodiscard]] result<void> release() noexcept {
        if (!context_) return {};
        if (context_->execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        const auto stopped = backend_->configure(pin_, driver::gpio_configuration{});
        if (!stopped) return std::unexpected(to_error(stopped.error()));
        backend_->release(pin_);
        const std::array pins{pin_};
        context_->release(pins);
        context_->drop();
        context_ = nullptr;
        return {};
    }
    [[nodiscard]] core::pin_id pin() const noexcept { return pin_; }
private:
    digital_pin(Context& context, backend_type& backend, core::pin_id pin,
                driver::gpio_configuration configuration) noexcept
        : context_(&context), backend_(&backend), pin_(pin),
          output_(configuration.direction == driver::gpio_direction::output),
          high_(configuration.initial_level == driver::gpio_level::high) {}
    Context* context_;
    backend_type* backend_;
    core::pin_id pin_;
    bool output_;
    bool high_;
};

} // namespace mcubsp::io
