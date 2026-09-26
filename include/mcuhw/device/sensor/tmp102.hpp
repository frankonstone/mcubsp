#pragma once

// @req-file REQ-0045 REQ-0046
#include <array>
#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>

namespace mcuhw::device {

enum class tmp102_errc : std::uint8_t { not_started, transport, invalid_response };

template<class TransportError>
struct tmp102_error {
    tmp102_errc code;
    std::optional<TransportError> transport{};
};

struct tmp102_temperature {
    std::int16_t celsius_sixteenths{};
    [[nodiscard]] constexpr std::int32_t micro_celsius() const noexcept {
        return static_cast<std::int32_t>(celsius_sixteenths) * 62'500;
    }
    friend constexpr bool operator==(tmp102_temperature, tmp102_temperature) = default;
};

// TI TMP102 datasheet SBOS397I, sections 6.3.1 and 6.5:
// https://www.ti.com/lit/ds/symlink/tmp102.pdf
// Endpoint owns the I2C address and supplies deadline_after(timeout) and
// write_read_until(output, input, deadline). It may be an io endpoint or a fake;
// the sensor has no board, controller, MMIO or execution-policy dependency.
// begin() checks the readable configuration without changing it. read() returns
// the latest converted sample; it does not trigger a fresh conversion or wake a
// shutdown device. After reset the hardware reports zero until first conversion.
template<class Endpoint>
class tmp102 {
public:
    using error_type = tmp102_error<typename Endpoint::error_type>;
    using deadline_type = typename Endpoint::deadline_type;
    template<class T> using result = std::expected<T, error_type>;

    explicit tmp102(Endpoint&& endpoint) noexcept : endpoint_(std::move(endpoint)) {}
    tmp102(const tmp102&) = delete;
    tmp102& operator=(const tmp102&) = delete;
    tmp102(tmp102&& other) noexcept
        : endpoint_(std::move(other.endpoint_)),
          initialized_(std::exchange(other.initialized_, false)) {}
    tmp102& operator=(tmp102&&) = delete;

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<void> begin(
        Duration timeout = std::chrono::milliseconds{100}) noexcept {
        const auto limit = endpoint_.deadline_after(timeout);
        if (!limit) return transport_failure(limit.error());
        return begin_until(*limit);
    }
    [[nodiscard]] result<void> begin_until(deadline_type limit) noexcept {
        initialized_ = false;
        const auto configuration = read_register(1U, limit);
        if (!configuration) return std::unexpected(configuration.error());
        // Resolution is fixed at 11; unused configuration bits read zero. This
        // is a consistency check, not a unique chip-identification register.
        if ((*configuration & 0x600FU) != 0x6000U)
            return std::unexpected(error_type{tmp102_errc::invalid_response});
        initialized_ = true;
        return {};
    }

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<tmp102_temperature> read(
        Duration timeout = std::chrono::milliseconds{100}) noexcept {
        if (!initialized_) return std::unexpected(error_type{tmp102_errc::not_started});
        const auto limit = endpoint_.deadline_after(timeout);
        if (!limit) return transport_failure(limit.error());
        return read_until(*limit);
    }
    [[nodiscard]] result<tmp102_temperature> read_until(deadline_type limit) noexcept {
        if (!initialized_) return std::unexpected(error_type{tmp102_errc::not_started});
        const auto raw = read_register(0U, limit);
        if (!raw) return std::unexpected(raw.error());
        // D0 reports the format of this sample, avoiding a cached EM setting.
        const bool extended = (*raw & 1U) != 0U;
        if ((*raw & (extended ? 0x0006U : 0x000FU)) != 0U)
            return std::unexpected(error_type{tmp102_errc::invalid_response});
        const unsigned shift = extended ? 3U : 4U;
        auto counts = static_cast<std::int32_t>(*raw >> shift);
        if ((*raw & 0x8000U) != 0U) counts -= extended ? 8192 : 4096;
        return tmp102_temperature{static_cast<std::int16_t>(counts)};
    }
    [[nodiscard]] bool initialized() const noexcept { return initialized_; }

private:
    [[nodiscard]] static auto transport_failure(
        const typename Endpoint::error_type& error) noexcept {
        return std::unexpected(error_type{tmp102_errc::transport, error});
    }
    [[nodiscard]] result<std::uint16_t> read_register(
        std::uint8_t address, deadline_type limit) noexcept {
        const std::array<std::uint8_t, 1U> pointer{address};
        std::array<std::uint8_t, 2U> bytes{};
        const auto completed = endpoint_.write_read_until(pointer, bytes, limit);
        if (!completed) return transport_failure(completed.error());
        if (completed->bytes_written != pointer.size()
            || completed->bytes_read != bytes.size())
            return std::unexpected(error_type{tmp102_errc::invalid_response});
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(bytes[0]) << 8U) | bytes[1]);
    }
    Endpoint endpoint_;
    bool initialized_{};
};

} // namespace mcuhw::device
