#pragma once
// @req-file REQ-0055 REQ-0062
// Nordic PS 4413_417 v1.11, Table 3 and 6.1.1--6.1.2.
#include <cstddef>
#include <cstdint>
#include <mcubsp/mcu/nrf52840/memory.hpp>

namespace mcubsp::mcu::nrf52840 {
enum class peripheral : unsigned {
    power, clock, approtect, uart0, uarte0, uarte1,
    spi0, spim0, spis0, twi0, twim0, twis0,
    spi1, spim1, spis1, twi1, twim1, twis1,
    spi2, spim2, spis2, spim3, comp, lpcomp, aar, ccm,
    egu0, egu1, egu2, egu3, egu4, egu5,
    swi0, swi1, swi2, swi3, swi4, swi5,
    nvmc, acl, mwu, cryptocell, radio, usbd, qspi, saadc, pdm, i2s,
    pwm0, pwm1, pwm2, pwm3, nfct, ecb, rng, temp, qdec,
    timer0, timer1, timer2, timer3, timer4, rtc0, rtc1, rtc2, wdt,
    // Software guard for flash mutations/permission changes, not an MMIO block.
    flash_mutation, count
};
inline constexpr unsigned peripheral_count = static_cast<unsigned>(peripheral::count);
[[nodiscard]] constexpr bool valid(peripheral p) noexcept {
    return static_cast<unsigned>(p) < peripheral_count;
}
// Canonical configuration ownership. Shared IRQ subscribers still use the
// interrupt dispatcher. Sharing a register bank alone is not exclusivity:
// POWER/CLOCK/APPROTECT and NVMC/ACL have separate controls. Flash drivers must
// additionally hold flash_mutation while altering data or permissions.
[[nodiscard]] constexpr peripheral ownership_identity(peripheral p) noexcept {
    using enum peripheral;
    switch (p) {
    case uarte0: return uart0;
    case spim0: case spis0: case twi0: case twim0: case twis0: return spi0;
    case spim1: case spis1: case twi1: case twim1: case twis1: return spi1;
    case spim2: case spis2: return spi2;
    case lpcomp: return comp;
    case ccm: return aar;
    case swi0: return egu0;
    case swi1: return egu1;
    case swi2: return egu2;
    case swi3: return egu3;
    case swi4: return egu4;
    case swi5: return egu5;
    default: return p;
    }
}
[[nodiscard]] constexpr bool has_serial_easydma(peripheral p) noexcept {
    using enum peripheral;
    return p == uarte0 || p == uarte1 || p == spim0 || p == spim1 || p == spim2 || p == spim3 ||
           p == spis0 || p == spis1 || p == spis2 || p == twim0 || p == twim1 ||
           p == twis0 || p == twis1;
}
// PS current MAXCNT tables: UARTE 6.34.9.28/29, SPIM 6.25.6.20/21,
// SPIS 6.26.5.17/21, TWIM 6.31.7.22/23, TWIS 6.32.8.21/22.
template<peripheral P> requires(has_serial_easydma(P))
struct serial_dma_traits {
    static constexpr unsigned count_bits = 16;
    static constexpr std::size_t maximum_count = 0xFFFF, unit_bytes = 1, alignment = 1;
};
struct dma_constraints {
    std::size_t maximum_count, unit_bytes, alignment, minimum_count{1};
    [[nodiscard]] constexpr bool valid() const noexcept {
        return maximum_count && unit_bytes && std::has_single_bit(alignment) &&
               minimum_count && minimum_count <= maximum_count;
    }
    [[nodiscard]] constexpr bool accepts(std::uintptr_t address, std::size_t capacity,
                                          std::size_t count) const noexcept {
        return valid() && count >= minimum_count && count <= maximum_count &&
               count <= capacity / unit_bytes &&
               memory::easydma_buffer(address, count * unit_bytes, alignment);
    }
};
template<peripheral P> requires(has_serial_easydma(P))
inline constexpr dma_constraints serial_dma_constraints{serial_dma_traits<P>::maximum_count,
    serial_dma_traits<P>::unit_bytes, serial_dma_traits<P>::alignment};
// CryptoCell DIN/DOUT AHB memory paths use 30-bit BYTE sizes, not the serial
// EasyDMA MAXCNT protocol (PS 6.6.13.2.1.4 / 6.6.13.3.1.4). FIRST/LAST in these
// size words are reserved. The algorithm/engine owner supplies its reviewed
// alignment; this conservative buffer policy restricts handoff to Data RAM.
struct cryptocell_memory_dma {
    static constexpr bool uses_easydma = false;
    static constexpr unsigned count_bits = 30;
    static constexpr std::size_t maximum_count = 0x3FFFFFFF;
    [[nodiscard]] static constexpr dma_constraints constraints(std::size_t alignment) noexcept {
        return {maximum_count,1,alignment};
    }
};
} // namespace mcubsp::mcu::nrf52840
