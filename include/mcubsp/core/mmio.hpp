#pragma once

#include <concepts>
#include <cstdint>
#include <expected>

namespace mcubsp::core {

// @req REQ-0008
enum class mmio_error : std::uint8_t {
    out_of_range,
    misaligned,
    trace_full,
    invalid_value,
    unsupported_width,
};

template<typename Policy, typename Value>
concept mmio_policy_for = std::unsigned_integral<Value>
    && requires(Policy& policy, std::uintptr_t address, Value value) {
        { policy.template read<Value>(address) }
            -> std::same_as<std::expected<Value, mmio_error>>;
        { policy.template write<Value>(address, value) }
            -> std::same_as<std::expected<void, mmio_error>>;
    };

} // namespace mcubsp::core
