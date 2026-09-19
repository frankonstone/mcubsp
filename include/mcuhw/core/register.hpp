#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <type_traits>
#include <utility>

#include <mcuhw/core/mmio.hpp>

namespace mcuhw::core {

// @req REQ-0007 REQ-0008
enum class access_semantics : std::uint8_t {
    read_only,
    write_only,
    read_write,
    write_one_to_clear,
    write_zero_to_clear,
    read_clear,
    command,
    set_alias,
    clear_alias,
};

// @req REQ-0007 REQ-0008
enum class reserved_bit_policy : std::uint8_t {
    write_zero,
    preserve,
};

template<
    std::unsigned_integral Value,
    std::size_t Offset,
    access_semantics Access,
    Value WritableMask = std::numeric_limits<Value>::max(),
    bool ReadModifyWriteSafe = false,
    reserved_bit_policy ReservedBits = reserved_bit_policy::write_zero,
    Value RequiredOneMask = 0,
    Value ResetValue = 0,
    Value ResetKnownMask = 0,
    typename CanonicalRegister = void>
// @req REQ-0007 REQ-0008
struct register_descriptor {
    static_assert(Offset % alignof(Value) == 0,
                  "register offset violates its access alignment");
    static_assert((RequiredOneMask & WritableMask) == 0,
                  "fixed-one and writable masks overlap");

    using value_type = Value;
    using canonical_register = CanonicalRegister;

    static constexpr std::size_t offset = Offset;
    static constexpr std::size_t width = sizeof(Value);
    static constexpr access_semantics access = Access;
    static constexpr Value writable_mask = WritableMask;
    static constexpr bool read_modify_write_safe = ReadModifyWriteSafe;
    static constexpr reserved_bit_policy reserved_bits = ReservedBits;
    static constexpr Value required_one_mask = RequiredOneMask;
    static constexpr Value reset_value = ResetValue;
    static constexpr Value reset_known_mask = ResetKnownMask;
    static constexpr bool reset_fully_known =
        ResetKnownMask == std::numeric_limits<Value>::max();
};

template<typename Register, unsigned Position, unsigned Width,
         access_semantics Access = Register::access>
// @req REQ-0007 REQ-0008
struct field_descriptor {
    using register_type = Register;
    using value_type = typename Register::value_type;

    static constexpr unsigned register_bits =
        std::numeric_limits<value_type>::digits;
    static_assert(Width > 0, "field width must be nonzero");
    static_assert(Position < register_bits,
                  "field position is outside the register");
    static_assert(Width <= register_bits - Position,
                  "field extends past the register width");

    static constexpr unsigned position = Position;
    static constexpr unsigned width = Width;
    static constexpr access_semantics access = Access;
    static constexpr value_type unshifted_mask = [] {
        if constexpr (Width == register_bits) {
            return std::numeric_limits<value_type>::max();
        } else {
            return static_cast<value_type>((value_type{1} << Width) - 1U);
        }
    }();
    static constexpr value_type mask =
        static_cast<value_type>(unshifted_mask << Position);

    [[nodiscard]] static constexpr bool fits(std::uintmax_t value) noexcept {
        return value <= unshifted_mask;
    }

    [[nodiscard]] static constexpr std::expected<value_type, mmio_error>
    try_encode(std::uintmax_t value) noexcept {
        if (!fits(value)) {
            return std::unexpected(mmio_error::invalid_value);
        }
        return static_cast<value_type>(static_cast<value_type>(value) << Position);
    }

    template<std::uintmax_t Value>
        requires(fits(Value))
    [[nodiscard]] static consteval value_type encode_constant() noexcept {
        return static_cast<value_type>(static_cast<value_type>(Value) << Position);
    }

    [[nodiscard]] static constexpr value_type extract(value_type value) noexcept {
        return static_cast<value_type>((value & mask) >> Position);
    }
};

template<typename Field, typename Enum, Enum... Allowed>
    requires std::is_enum_v<Enum>
// @req REQ-0007 REQ-0008
struct enum_field_descriptor {
    using field_type = Field;
    using value_type = typename Field::value_type;
    using enum_type = Enum;

    [[nodiscard]] static constexpr bool contains(Enum value) noexcept {
        return ((value == Allowed) || ...);
    }

    [[nodiscard]] static constexpr bool fits(Enum value) noexcept {
        const auto raw = std::to_underlying(value);
        if constexpr (std::is_signed_v<decltype(raw)>) {
            if (raw < 0) {
                return false;
            }
        }
        return contains(value)
            && Field::fits(static_cast<std::uintmax_t>(raw));
    }

    [[nodiscard]] static constexpr std::expected<value_type, mmio_error>
    try_encode(Enum value) noexcept {
        if (!fits(value)) {
            return std::unexpected(mmio_error::invalid_value);
        }
        return Field::try_encode(
            static_cast<std::uintmax_t>(std::to_underlying(value)));
    }

    template<Enum Value>
        requires(fits(Value))
    [[nodiscard]] static consteval value_type encode_constant() noexcept {
        return Field::template encode_constant<
            static_cast<std::uintmax_t>(std::to_underlying(Value))>();
    }
};

template<typename Register, typename Policy>
    requires mmio_policy_for<Policy, typename Register::value_type>
// @req REQ-0008
class register_ref {
public:
    using value_type = typename Register::value_type;

    constexpr register_ref(Policy& policy, std::uintptr_t block_base) noexcept
        : policy_(policy), block_base_(block_base) {}

    [[nodiscard]] constexpr std::uintptr_t address() const noexcept {
        return block_base_ + Register::offset;
    }

    [[nodiscard]] std::expected<value_type, mmio_error> read() noexcept
        requires(Register::access == access_semantics::read_only
                 || Register::access == access_semantics::read_write
                 || Register::access == access_semantics::write_one_to_clear
                 || Register::access == access_semantics::write_zero_to_clear)
    {
        return policy_.template read<value_type>(address());
    }

    [[nodiscard]] std::expected<value_type, mmio_error>
    read_and_clear() noexcept
        requires(Register::access == access_semantics::read_clear)
    {
        return policy_.template read<value_type>(address());
    }

    [[nodiscard]] std::expected<void, mmio_error> write(value_type value) noexcept
        requires(Register::access == access_semantics::write_only
                     || Register::access == access_semantics::read_write)
              && (Register::reserved_bits == reserved_bit_policy::write_zero
                  || static_cast<value_type>(Register::writable_mask
                                             | Register::required_one_mask)
                         == std::numeric_limits<value_type>::max())
    {
        if (!direct_value_is_valid(value)) {
            return std::unexpected(mmio_error::invalid_value);
        }
        return policy_.template write<value_type>(address(), value);
    }

    [[nodiscard]] std::expected<void, mmio_error>
    issue(value_type value) noexcept
        requires(Register::access == access_semantics::command
                 || Register::access == access_semantics::set_alias
                 || Register::access == access_semantics::clear_alias)
    {
        if (!direct_value_is_valid(value)) {
            return std::unexpected(mmio_error::invalid_value);
        }
        return policy_.template write<value_type>(address(), value);
    }

    [[nodiscard]] std::expected<void, mmio_error>
    clear_bits(value_type bits) noexcept
        requires(Register::access == access_semantics::write_one_to_clear
                 || (Register::access == access_semantics::write_zero_to_clear
                     && Register::reserved_bits == reserved_bit_policy::write_zero))
    {
        if ((bits & static_cast<value_type>(~Register::writable_mask)) != 0) {
            return std::unexpected(mmio_error::invalid_value);
        }
        if constexpr (Register::access == access_semantics::write_one_to_clear) {
            return policy_.template write<value_type>(address(), bits);
        } else {
            const auto value = static_cast<value_type>(
                (static_cast<value_type>(~bits) & Register::writable_mask)
                | Register::required_one_mask);
            return policy_.template write<value_type>(address(), value);
        }
    }

    [[nodiscard]] std::expected<void, mmio_error>
    modify(value_type clear_mask, value_type set_mask) noexcept
        requires(Register::access == access_semantics::read_write
                 && Register::read_modify_write_safe)
    {
        const auto changed = static_cast<value_type>(clear_mask | set_mask);
        if ((changed & static_cast<value_type>(~Register::writable_mask)) != 0) {
            return std::unexpected(mmio_error::invalid_value);
        }
        const auto current = policy_.template read<value_type>(address());
        if (!current) {
            return std::unexpected(current.error());
        }
        auto next = static_cast<value_type>(
            (*current & static_cast<value_type>(~clear_mask)) | set_mask);
        if constexpr (Register::reserved_bits == reserved_bit_policy::write_zero) {
            next = static_cast<value_type>(
                (next & Register::writable_mask) | Register::required_one_mask);
        }
        return policy_.template write<value_type>(address(), next);
    }

private:
    [[nodiscard]] static constexpr bool
    direct_value_is_valid(value_type value) noexcept {
        const auto permitted = static_cast<value_type>(
            Register::writable_mask | Register::required_one_mask);
        const bool no_unknown_bits =
            (value & static_cast<value_type>(~permitted)) == 0;
        const bool required_bits_present =
            (value & Register::required_one_mask) == Register::required_one_mask;
        return no_unknown_bits && required_bits_present;
    }

    Policy& policy_;
    std::uintptr_t block_base_;
};

} // namespace mcuhw::core
