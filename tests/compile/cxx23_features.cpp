#include <bit>
#include <concepts>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

enum class sample_register : std::uint8_t { control };

constexpr std::uint32_t checked_mask(unsigned width) {
    if consteval {
        return width == 32U ? 0xFFFF'FFFFU : ((1U << width) - 1U);
    } else {
        return 0U;
    }
}

static_assert(std::byteswap(std::uint32_t{0x1122'3344U}) == 0x4433'2211U);
static_assert(std::to_underlying(sample_register::control) == 0U);
static_assert(checked_mask(4U) == 0xFU);
static_assert(std::same_as<std::span<const std::byte>::size_type, std::size_t>);

std::expected<std::uint32_t, std::uint8_t> cxx23_probe(bool valid) {
    if (!valid) {
        return std::unexpected(std::uint8_t{1});
    }
    return checked_mask(8U);
}
