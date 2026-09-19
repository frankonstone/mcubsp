#pragma once

#include <concepts>
#include <cstdint>
#include <expected>

#include <mcuhw/core/mmio.hpp>

namespace mcuhw::core {

// Target policy for memory-mapped registers. Host tests use recording_mmio
// instead, so numeric target addresses are never dereferenced on the host.
// @req REQ-0003 REQ-0008
class direct_mmio {
public:
    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<Value, mmio_error>
    read(std::uintptr_t address) noexcept {
        if ((address % alignof(Value)) != 0U) {
            return std::unexpected(mmio_error::misaligned);
        }
        const auto* register_address =
            reinterpret_cast<volatile const Value*>(address);
        return *register_address;
    }

    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<void, mmio_error>
    write(std::uintptr_t address, Value value) noexcept {
        if ((address % alignof(Value)) != 0U) {
            return std::unexpected(mmio_error::misaligned);
        }
        auto* register_address = reinterpret_cast<volatile Value*>(address);
        *register_address = value;
        return {};
    }
};

} // namespace mcuhw::core
