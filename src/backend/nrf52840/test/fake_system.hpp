#pragma once
#include <array>
#include <cstdint>
#include <expected>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/testing/recording_mmio.hpp>
#include <optional>
namespace nrf52840_test {
// Sparse keys, never native MMIO. Side effects are behavioral models, not a
// hardware acceptance claim. Task/status separation catches wrong polling.
class fake_system {
  public:
    std::optional<std::uintptr_t> failed_address;
    bool fail_once{}, fail_after_commit{}, clock_responds{true};
    unsigned reads{}, writes{};
    std::array<mcubsp::testing::mmio_operation, 4096> trace{};
    unsigned trace_count{};
    template <class T>
    std::expected<T, mcubsp::core::mmio_error> read(std::uintptr_t address) noexcept {
        ++reads;
        if (fails(address))
            return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        auto a = address & ~std::uintptr_t{3};
        return static_cast<T>(peek(a) >> ((address & 3) * 8));
    }
    template <class T>
    std::expected<void, mcubsp::core::mmio_error> write(std::uintptr_t address, T value) noexcept {
        ++writes;
        bool failed = fails(address);
        if (failed && !fail_after_commit)
            return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        if (trace_count < trace.size())
            trace[trace_count++] = {mcubsp::testing::mmio_operation_kind::write, address, sizeof(T),
                                    value};
        if constexpr (sizeof(T) == 1) {
            auto a = address & ~std::uintptr_t{3};
            auto shift = (address & 3) * 8;
            poke(a, (peek(a) & ~(255U << shift)) | (static_cast<std::uint32_t>(value) << shift));
        } else
            poke(address, static_cast<std::uint32_t>(value));
        auto v = static_cast<std::uint32_t>(value);
        if (clock_responds && v) {
            if (address == 0x40000000) {
                poke(0x40000408, 1);
                poke(0x4000040C, 0x10001);
                poke(0x40000100, 1);
            }
            if (address == 0x40000004) {
                poke(0x40000408, 0);
                poke(0x4000040C, 0x10000);
            }
            if (address == 0x40000008) {
                poke(0x40000414, 1);
                poke(0x40000418, 0x10000 | (peek(0x40000518) & 3));
                poke(0x4000041C, peek(0x40000518) & 3);
                poke(0x40000104, 1);
            }
            if (address == 0x4000000C) {
                poke(0x40000414, 0);
                poke(0x40000418, 0);
            }
            if (address == 0x40000010)
                poke(0x4000010C, 1);
            if (address == 0x40000014)
                poke(0x40000128, 1);
            if (address == 0x40000018)
                poke(0x4000012C, 1);
        }
        if (address == 0x4001F504)
            poke(0x4001F500, peek(0x4001F500) | v);
        if (address == 0x4001F508)
            poke(0x4001F500, peek(0x4001F500) & ~v);
        if (address >= 0x4001F000 && address < 0x4001F030 && v) {
            auto group = (address - 0x4001F000) / 8;
            auto mask = peek(0x4001F800 + group * 4);
            poke(0x4001F500, (address & 4) ? peek(0x4001F500) & ~mask : peek(0x4001F500) | mask);
        }
        for (auto b : std::array<std::uintptr_t, 3>{0x4000B000, 0x40011000, 0x40024000})
            if (address == b + 8 && v)
                poke(b + 0x504, 0);
        for (auto b : std::array<std::uintptr_t, 5>{0x40008000, 0x40009000, 0x4000A000, 0x4001A000,
                                                    0x4001B000}) {
            if (address == b + 8 && v)
                poke(b + 0xFFC, peek(b + 0xFFC) + 1);
            if (address == b + 12 && v)
                poke(b + 0xFFC, 0);
            if (address >= b + 0x40 && address < b + 0x58 && v)
                poke(b + 0x540 + (address - b - 0x40), peek(b + 0xFFC));
        }
        if (address == 0x40010000 && v)
            poke(0x40010400, 1);
        for (unsigned n = 0; n < 6; ++n) {
            auto b = 0x40014000 + n * 0x1000;
            if (address >= b && address < b + 0x40 && v)
                poke(b + 0x100 + (address - b), 1);
        }
        if (failed)
            return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        return {};
    }
    void poke(std::uintptr_t address, std::uint32_t value) noexcept {
        for (unsigned n = 0; n < count_; ++n)
            if (cells_[n].address == address) {
                cells_[n].value = value;
                return;
            }
        if (count_ < cells_.size())
            cells_[count_++] = {address, value};
    }
    [[nodiscard]] std::uint32_t peek(std::uintptr_t address) const noexcept {
        for (unsigned n = 0; n < count_; ++n)
            if (cells_[n].address == address)
                return cells_[n].value;
        return 0;
    }
    [[nodiscard]] bool wrote(std::uintptr_t a, std::uint32_t v) const noexcept {
        for (unsigned n = 0; n < trace_count; ++n)
            if (trace[n].address == a && trace[n].value == v)
                return true;
        return false;
    }

  private:
    bool fails(std::uintptr_t a) noexcept {
        if (!failed_address || *failed_address != a)
            return false;
        if (fail_once)
            failed_address.reset();
        return true;
    }
    struct cell {
        std::uintptr_t address;
        std::uint32_t value;
    };
    std::array<cell, 2048> cells_{};
    unsigned count_{};
};
} // namespace nrf52840_test
