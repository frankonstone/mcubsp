#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

#include <mcubsp/core/mmio.hpp>

namespace mcubsp::core {

// @req REQ-0009
enum class access_width : std::uint8_t {
    bits8 = 1,
    bits16 = 2,
    bits32 = 4,
};

// @req REQ-0009
struct write_step {
    std::size_t offset;
    access_width width;
    std::uint32_t value;
};

template<typename Policy>
// @req REQ-0009
[[nodiscard]] std::expected<void, mmio_error> execute_write_sequence(
    Policy& policy, std::uintptr_t base,
    std::span<const write_step> steps) noexcept {
    for (const auto& step : steps) {
        std::expected<void, mmio_error> result;
        switch (step.width) {
            case access_width::bits8:
                if (step.value > 0xFFU) {
                    return std::unexpected(mmio_error::invalid_value);
                }
                result = policy.template write<std::uint8_t>(
                    base + step.offset, static_cast<std::uint8_t>(step.value));
                break;
            case access_width::bits16:
                if (step.value > 0xFFFFU) {
                    return std::unexpected(mmio_error::invalid_value);
                }
                result = policy.template write<std::uint16_t>(
                    base + step.offset, static_cast<std::uint16_t>(step.value));
                break;
            case access_width::bits32:
                result = policy.template write<std::uint32_t>(
                    base + step.offset, step.value);
                break;
            default:
                return std::unexpected(mmio_error::unsupported_width);
        }
        if (!result) {
            return std::unexpected(result.error());
        }
    }
    return {};
}

} // namespace mcubsp::core
