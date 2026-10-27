#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace nrf52840_test {

// Deliberately models W1C errors and CPU-driven TWI events, not a complete MCU.
// Automatic mode supplies a small TMP102 protocol peer through the real backend.
class fake_twi {
public:
    static constexpr std::uintptr_t base = 0x4000'3000U;
    std::optional<std::uintptr_t> failed_address;
    bool fail_once{};
    bool stop_responds{true};
    bool automatic{};
    std::size_t reads{};
    std::size_t write_count{};
    std::array<mcubsp::testing::mmio_operation, 4096> trace{};

    fake_twi() {
        poke(0x5000'0510U, 0xFFFF'FFFFU);
        poke(0x5000'0810U, 0xFFFF'FFFFU);
    }
    template<class T> std::expected<T, mcubsp::core::mmio_error> read(std::uintptr_t address) noexcept {
        ++reads;
        if (fails(address)) return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        auto* value = storage(address);
        if (!value) return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        return static_cast<T>(*value);
    }
    template<class T> std::expected<void, mcubsp::core::mmio_error>
    write(std::uintptr_t address, T value) noexcept {
        if (fails(address)) return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        auto* cell = storage(address);
        if (!cell || write_count == trace.size()) return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        trace[write_count++] = {mcubsp::testing::mmio_operation_kind::write, address, sizeof(T), value};
        const auto relative = address >= base && address < base + 0x2000 ? (address - base) % 0x1000 : 0xFFFFU;
        if (relative == 0x4C4U) *cell &= ~static_cast<std::uint32_t>(value);
        else *cell = value;
        const auto instance = address >= base && address < base + 0x2000 ? (address - base) / 0x1000 : 0U;
        const auto block = base + instance * 0x1000;
        if (relative == 0x014U && value && stop_responds) {
            poke(block + 0x104, 1U);
            receiving_ = false;
        }
        if (!automatic || instance != 0U) return {};
        if (relative == 0x008U && value) { tx_count_ = 0U; transmitting_ = true; receiving_ = false; }
        if (relative == 0x51CU && transmitting_) {
            if (tx_count_++ == 0U) selected_register_ = static_cast<std::uint8_t>(value);
            poke(base + 0x11C, 1U);
        }
        if (relative == 0x01CU && value) { poke(base + 0x148, 1U); transmitting_ = false; }
        if (relative == 0x000U && value) { receiving_ = true; rx_count_ = 0; next_byte(); }
        if (relative == 0x020U && value && receiving_) next_byte();
        if (relative == 0x500U && value == 0U) { transmitting_ = false; receiving_ = false; }
        return {};
    }
    void poke(std::uintptr_t address, std::uint32_t value) { if (auto* p = storage(address)) *p = value; }
    [[nodiscard]] std::uint32_t peek(std::uintptr_t address) { auto* p = storage(address); return p ? *p : 0U; }
    [[nodiscard]] bool wrote(std::uintptr_t address, std::uint32_t value) const {
        for (std::size_t i = 0; i < write_count; ++i)
            if (trace[i].address == address && trace[i].value == value) return true;
        return false;
    }
    void clear_trace() { write_count = 0; reads = 0; }
private:
    bool fails(std::uintptr_t address) {
        if (!failed_address || *failed_address != address) return false;
        if (fail_once) failed_address.reset();
        return true;
    }
    std::uint32_t* storage(std::uintptr_t address) {
        if (address % 4) return nullptr;
        if (address >= base && address < base + registers_.size() * 4)
            return &registers_[(address - base) / 4];
        if (address >= 0x5000'0000U && address < 0x5000'0000U + gpio_.size() * 4)
            return &gpio_[(address - 0x5000'0000U) / 4];
        return nullptr;
    }
    void next_byte() {
        const std::array<std::uint8_t, 2> response = selected_register_ == 1U
            ? std::array<std::uint8_t, 2>{0x60, 0xA0} : std::array<std::uint8_t, 2>{0x19, 0x00};
        poke(base + 0x518, response[rx_count_++ % response.size()]);
        poke(base + 0x108, 1U);
        if (peek(base + 0x200) == 2U) { poke(base + 0x104, 1U); receiving_ = false; }
        else poke(base + 0x148, 1U);
    }
    std::array<std::uint32_t, 0x2000 / 4> registers_{};
    std::array<std::uint32_t, 0xB00 / 4> gpio_{};
    std::size_t tx_count_{}, rx_count_{};
    std::uint8_t selected_register_{};
    bool transmitting_{}, receiving_{};
};
} // namespace nrf52840_test
