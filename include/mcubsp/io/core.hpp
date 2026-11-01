#pragma once

// @req-file REQ-0002 REQ-0014 REQ-0045
#include <cstdint>
#include <expected>
#include <source_location>
#include <type_traits>
#include <utility>

#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/driver/uart.hpp>
#include <mcubsp/driver/i2c.hpp>
#include <mcubsp/driver/spi.hpp>
#include <mcubsp/driver/dma.hpp>
#include <mcubsp/driver/adc.hpp>
#include <mcubsp/driver/pwm.hpp>

namespace mcubsp::io {

inline constexpr bool enabled = true;
inline constexpr bool disabled = false;

template<unsigned... Channel>
inline constexpr std::uint8_t channels = [] {
    static_assert(((Channel < 8U) && ...), "DMA channel mask supports channels 0..7");
    return static_cast<std::uint8_t>((0U | ... | (1U << Channel)));
}();

struct board_config {
    bool serial{true};
    bool i2c{};
    bool spi{};
    std::uint8_t dma{};
    bool adc{};
    bool pwm{};
    friend constexpr bool operator==(board_config, board_config) = default;
};

enum class errc : std::uint8_t {
    hardware, not_started, already_started, pin_in_use, busy,
    invalid_pin, invalid_timeout, invalid_context, timeout, cancelled,
    invalid_operation, exhausted, unsupported, quiesce_failed,
};
enum class error_source : std::uint8_t { io, gpio, uart, i2c, spi, dma, adc, pwm };
struct error {
    errc code{errc::hardware};
    error_source source{error_source::io};
    std::uint32_t native{};
    friend constexpr bool operator==(error, error) = default;
};
template<class T> using result = std::expected<T, error>;

[[nodiscard]] constexpr error to_error(driver::gpio_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::gpio_error::already_owned) code = errc::pin_in_use;
    if (value == driver::gpio_error::invalid_pin) code = errc::invalid_pin;
    if (value == driver::gpio_error::not_initialized) code = errc::not_started;
    return {code, error_source::gpio, static_cast<std::uint32_t>(value)};
}
[[nodiscard]] constexpr error to_error(driver::uart_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::uart_error::busy || value == driver::uart_error::already_owned) code = errc::busy;
    if (value == driver::uart_error::not_initialized) code = errc::not_started;
    return {code, error_source::uart, static_cast<std::uint32_t>(value)};
}
[[nodiscard]] constexpr error to_error(driver::i2c_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::i2c_error::busy || value == driver::i2c_error::already_owned) code = errc::busy;
    if (value == driver::i2c_error::not_initialized) code = errc::not_started;
    if (value == driver::i2c_error::timeout) code = errc::timeout;
    return {code, error_source::i2c, static_cast<std::uint32_t>(value)};
}
[[nodiscard]] constexpr error to_error(driver::spi_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::spi_error::busy || value == driver::spi_error::already_owned) code = errc::busy;
    if (value == driver::spi_error::not_initialized) code = errc::not_started;
    if (value == driver::spi_error::timeout) code = errc::timeout;
    return {code, error_source::spi, static_cast<std::uint32_t>(value)};
}
[[nodiscard]] constexpr error to_error(driver::dma_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::dma_error::busy || value == driver::dma_error::already_owned) code = errc::busy;
    if (value == driver::dma_error::not_initialized) code = errc::not_started;
    return {code, error_source::dma, static_cast<std::uint32_t>(value)};
}
[[nodiscard]] constexpr error to_error(driver::adc_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::adc_error::busy || value == driver::adc_error::already_owned) code = errc::busy;
    if (value == driver::adc_error::not_initialized) code = errc::not_started;
    return {code, error_source::adc, static_cast<std::uint32_t>(value)};
}
[[nodiscard]] constexpr error to_error(driver::pwm_error value) noexcept {
    auto code = errc::hardware;
    if (value == driver::pwm_error::already_owned || value == driver::pwm_error::already_initialized) code = errc::busy;
    if (value == driver::pwm_error::not_initialized) code = errc::not_started;
    return {code, error_source::pwm, static_cast<std::uint32_t>(value)};
}
template<class E>
[[nodiscard]] constexpr error to_error(const core::transfer_error<E>& value) noexcept {
    auto result = value.cause ? to_error(*value.cause) : error{};
    using enum core::transfer_error_kind;
    switch (value.kind) {
    case failure: break;
    case timeout: result.code = errc::timeout; break;
    case cancelled: result.code = errc::cancelled; break;
    case wait_from_isr: result.code = errc::invalid_context; break;
    case resource_exhausted: result.code = errc::exhausted; break;
    case pending: result.code = errc::busy; break;
    case quiesce_failed: result.code = errc::quiesce_failed; break;
    default: result.code = errc::invalid_operation; break;
    }
    return result;
}

// Passed to an optional execution-policy contract_violation(diagnostic) hook
// before the mandatory trap. Strings and source location have static storage.
struct contract_failure {
    const char* component;
    const char* reason;
    core::operation_id operation{};
    std::source_location location = std::source_location::current();
};

struct trap_failure {
    [[noreturn]] static void fail(error) noexcept { __builtin_trap(); }
};

// Failure policy is a type, not a stored callback. It must not return.
template<class Failure = trap_failure, class T>
[[nodiscard]] T require(result<T>&& value) noexcept {
    if (!value) { Failure::fail(value.error()); __builtin_trap(); }
    return std::move(*value);
}
template<class Failure = trap_failure>
void require(result<void> value) noexcept {
    if (!value) { Failure::fail(value.error()); __builtin_trap(); }
}

} // namespace mcubsp::io

namespace mcubsp::io::detail {
template<class Backend,class Context> Backend make_backend(Context& c) noexcept {
    if constexpr(requires{Backend(c.native_services());})return Backend(c.native_services());
    else return Backend(c.mmio(),c.execution());
}
}
