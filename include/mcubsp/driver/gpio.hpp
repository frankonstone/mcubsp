#pragma once

#include <cstdint>
#include <expected>

#include <mcubsp/core/metadata.hpp>

namespace mcubsp::driver {

enum class gpio_level : std::uint8_t { low, high };
enum class gpio_direction : std::uint8_t { input, output };
enum class gpio_output_type : std::uint8_t { push_pull, open_drain };
enum class gpio_pull : std::uint8_t { none, up, down };
enum class gpio_drive : std::uint8_t { low, middle };

struct gpio_configuration {
    gpio_direction direction{gpio_direction::input};
    gpio_level initial_level{gpio_level::low};
    gpio_output_type output_type{gpio_output_type::push_pull};
    gpio_pull pull{gpio_pull::none};
    gpio_drive drive{gpio_drive::low};
};

enum class gpio_error : std::uint8_t {
    invalid_pin,
    unsupported_configuration,
    already_owned,
    already_initialized,
    not_initialized,
    wrong_direction,
    mmio,
};

// Functional GPIO driver. It owns no register addresses or vendor layouts;
// those remain in Backend. Backend uses fixed storage to arbitrate ownership.
template<typename Backend>
class gpio_pin {
public:
    constexpr gpio_pin(Backend& backend, core::pin_id pin) noexcept
        : backend_(backend), pin_(pin) {}

    gpio_pin(const gpio_pin&) = delete;
    gpio_pin& operator=(const gpio_pin&) = delete;
    gpio_pin(gpio_pin&&) = delete;
    gpio_pin& operator=(gpio_pin&&) = delete;

    // @req REQ-0003 REQ-0014 REQ-0021
    [[nodiscard]] std::expected<void, gpio_error>
    initialize(gpio_configuration configuration) noexcept {
        if (initialized_) {
            return std::unexpected(gpio_error::already_initialized);
        }
        const auto claimed = backend_.claim(pin_, configuration.direction);
        if (!claimed) {
            return claimed;
        }
        const auto configured = backend_.configure(pin_, configuration);
        if (!configured) {
            backend_.release(pin_);
            return configured;
        }
        configuration_ = configuration;
        initialized_ = true;
        return {};
    }

    [[nodiscard]] std::expected<void, gpio_error> release() noexcept {
        if (!initialized_) {
            return std::unexpected(gpio_error::not_initialized);
        }
        const auto safe = backend_.configure(pin_, gpio_configuration{});
        if (!safe) {
            return safe;
        }
        backend_.release(pin_);
        initialized_ = false;
        return {};
    }

    [[nodiscard]] std::expected<void, gpio_error>
    write(gpio_level level) noexcept {
        if (!initialized_) {
            return std::unexpected(gpio_error::not_initialized);
        }
        if (configuration_.direction != gpio_direction::output) {
            return std::unexpected(gpio_error::wrong_direction);
        }
        return level == gpio_level::high ? backend_.set(pin_)
                                         : backend_.clear(pin_);
    }

    [[nodiscard]] std::expected<void, gpio_error> set() noexcept {
        return write(gpio_level::high);
    }

    [[nodiscard]] std::expected<void, gpio_error> clear() noexcept {
        return write(gpio_level::low);
    }

    [[nodiscard]] std::expected<gpio_level, gpio_error> read() noexcept {
        if (!initialized_) {
            return std::unexpected(gpio_error::not_initialized);
        }
        return backend_.read(pin_);
    }

    [[nodiscard]] constexpr bool initialized() const noexcept {
        return initialized_;
    }

private:
    Backend& backend_;
    core::pin_id pin_;
    gpio_configuration configuration_{};
    bool initialized_{};
};

} // namespace mcubsp::driver
