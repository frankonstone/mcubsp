#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace mcuhw::driver {

enum class adc_error : std::uint8_t {
    invalid_channel,
    invalid_sample_time,
    empty_sequence,
    duplicate_channel,
    result_size_mismatch,
    excessive_length,
    busy,
    conversion_overrun,
    not_initialized,
    already_owned,
    mmio,
};

enum class adc_resolution : std::uint8_t { bits12 = 12, bits14 = 14 };
enum class adc_reference : std::uint8_t { vrefh0 };
enum class adc_result_format : std::uint8_t { right_aligned_unsigned };

struct adc_configuration {
    adc_resolution resolution{adc_resolution::bits14};
    adc_reference reference{adc_reference::vrefh0};
    adc_result_format result_format{adc_result_format::right_aligned_unsigned};
    std::uint8_t sample_states{13U};
};

struct adc_channel {
    std::uint8_t value;

    friend constexpr bool operator==(adc_channel, adc_channel) = default;
};

[[nodiscard]] constexpr std::expected<adc_channel, adc_error>
make_adc_channel(std::uint8_t channel) noexcept {
    if (channel > 19U) {
        return std::unexpected(adc_error::invalid_channel);
    }
    return adc_channel{channel};
}

struct adc_sequence_result {
    std::size_t samples;

    friend constexpr bool operator==(adc_sequence_result,
                                     adc_sequence_result) = default;
};

template<typename Backend>
class adc_controller {
public:
    explicit adc_controller(Backend& backend) noexcept : backend_(backend) {}

    [[nodiscard]] std::expected<void, adc_error>
    initialize(adc_configuration configuration = {}) noexcept {
        return backend_.initialize(configuration);
    }

    [[nodiscard]] auto convert(std::span<const adc_channel> channels,
                               std::span<std::uint16_t> results) noexcept {
        return backend_.convert(channels, results);
    }

    [[nodiscard]] std::expected<void, adc_error> shutdown() noexcept {
        return backend_.shutdown();
    }

private:
    Backend& backend_;
};

} // namespace mcuhw::driver
