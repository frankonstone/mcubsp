#pragma once

#include <cstdint>
#include <expected>
#include <limits>

namespace mcuhw::execution {

using tick = std::uint32_t;

struct deadline {
    tick value;

    friend constexpr bool operator==(deadline, deadline) = default;
};

enum class deadline_error : std::uint8_t { interval_too_large };

inline constexpr tick maximum_deadline_interval =
    static_cast<tick>(std::numeric_limits<std::int32_t>::max());

// Modular deadline ordering is valid while compared points are separated by
// less than half the 32-bit tick range. This includes wrap through zero.
// @req REQ-0015 REQ-0019
[[nodiscard]] constexpr bool reached(tick now, deadline limit) noexcept {
    return static_cast<tick>(now - limit.value) <= maximum_deadline_interval;
}

// @req REQ-0015 REQ-0019
[[nodiscard]] constexpr std::expected<deadline, deadline_error>
make_deadline(tick now, tick interval) noexcept {
    if (interval > maximum_deadline_interval) {
        return std::unexpected(deadline_error::interval_too_large);
    }
    return deadline{static_cast<tick>(now + interval)};
}

} // namespace mcuhw::execution
