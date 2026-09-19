#pragma once

#include <compare>
#include <cstdint>
#include <expected>
#include <limits>

namespace mcuhw::core {

// @req REQ-0011 REQ-0013
enum class arithmetic_error : std::uint8_t {
    overflow,
    divide_by_zero,
};

template<typename Tag>
// @req REQ-0011 REQ-0013
struct quantity {
    std::uint64_t count{};

    friend constexpr bool operator==(quantity, quantity) = default;
    friend constexpr auto operator<=>(quantity, quantity) = default;
};

struct hertz_tag;
struct bytes_tag;
using hertz = quantity<hertz_tag>;
using bytes = quantity<bytes_tag>;

template<typename Tag>
[[nodiscard]] constexpr std::expected<quantity<Tag>, arithmetic_error>
checked_add(quantity<Tag> left, quantity<Tag> right) noexcept {
    if (right.count > std::numeric_limits<std::uint64_t>::max() - left.count) {
        return std::unexpected(arithmetic_error::overflow);
    }
    return quantity<Tag>{left.count + right.count};
}

template<typename Tag>
[[nodiscard]] constexpr std::expected<quantity<Tag>, arithmetic_error>
checked_multiply(quantity<Tag> value, std::uint64_t multiplier) noexcept {
    if (multiplier != 0
        && value.count > std::numeric_limits<std::uint64_t>::max() / multiplier) {
        return std::unexpected(arithmetic_error::overflow);
    }
    return quantity<Tag>{value.count * multiplier};
}

template<typename Tag>
[[nodiscard]] constexpr std::expected<quantity<Tag>, arithmetic_error>
checked_divide(quantity<Tag> value, std::uint64_t divisor) noexcept {
    if (divisor == 0) {
        return std::unexpected(arithmetic_error::divide_by_zero);
    }
    return quantity<Tag>{value.count / divisor};
}

} // namespace mcuhw::core
