#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <span>

#include <mcubsp/core/mmio.hpp>

namespace mcubsp::testing {

using mmio_error = core::mmio_error;

enum class mmio_operation_kind : std::uint8_t {
    read,
    write,
};

struct mmio_operation {
    mmio_operation_kind kind;
    std::uintptr_t address;
    std::size_t width;
    std::uint64_t value;
};

template<typename Value>
concept mmio_value = std::unsigned_integral<Value>
    && (sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4);

// A fixed-capacity host policy that models an address window in local storage.
// Target addresses are only numeric keys and are never dereferenced.
template<std::size_t StorageBytes, std::size_t MaxOperations>
// @req REQ-0002 REQ-0008
class recording_mmio {
public:
    explicit constexpr recording_mmio(std::uintptr_t base_address) noexcept
        : base_address_(base_address) {}

    template<mmio_value Value>
    [[nodiscard]] std::expected<Value, mmio_error>
    read(std::uintptr_t address) noexcept {
        const auto offset = checked_offset<Value>(address);
        if (!offset) {
            return std::unexpected(offset.error());
        }
        if (operation_count_ == MaxOperations) {
            return std::unexpected(mmio_error::trace_full);
        }

        Value value{};
        std::memcpy(&value, storage_.data() + *offset, sizeof(Value));
        operations_[operation_count_++] = {
            mmio_operation_kind::read,
            address,
            sizeof(Value),
            static_cast<std::uint64_t>(value),
        };
        return value;
    }

    template<mmio_value Value>
    [[nodiscard]] std::expected<void, mmio_error>
    write(std::uintptr_t address, Value value) noexcept {
        const auto offset = checked_offset<Value>(address);
        if (!offset) {
            return std::unexpected(offset.error());
        }
        if (operation_count_ == MaxOperations) {
            return std::unexpected(mmio_error::trace_full);
        }

        std::memcpy(storage_.data() + *offset, &value, sizeof(Value));
        operations_[operation_count_++] = {
            mmio_operation_kind::write,
            address,
            sizeof(Value),
            static_cast<std::uint64_t>(value),
        };
        return {};
    }

    [[nodiscard]] constexpr std::span<const mmio_operation>
    operations() const noexcept {
        return {operations_.data(), operation_count_};
    }

    constexpr void clear_operations() noexcept {
        operation_count_ = 0;
    }

private:
    template<mmio_value Value>
    [[nodiscard]] constexpr std::expected<std::size_t, mmio_error>
    checked_offset(std::uintptr_t address) const noexcept {
        if (address % alignof(Value) != 0) {
            return std::unexpected(mmio_error::misaligned);
        }
        if (address < base_address_) {
            return std::unexpected(mmio_error::out_of_range);
        }
        const auto offset = address - base_address_;
        if constexpr (sizeof(Value) > StorageBytes) {
            return std::unexpected(mmio_error::out_of_range);
        } else if (offset > StorageBytes - sizeof(Value)) {
            return std::unexpected(mmio_error::out_of_range);
        }
        return static_cast<std::size_t>(offset);
    }

    std::uintptr_t base_address_;
    std::array<std::byte, StorageBytes> storage_{};
    std::array<mmio_operation, MaxOperations> operations_{};
    std::size_t operation_count_{};
};

} // namespace mcubsp::testing
