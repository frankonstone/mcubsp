#pragma once
// @req-file REQ-0055 REQ-0059
// PS 4413_417 v1.11 6.33.10.23 and 6.34.9.27. The documented ACTUAL
// rates, not a fabricated fractional-divider formula, determine tolerance.
#include <array>
#include <mcubsp/mcu/nrf52840/resources.hpp>
namespace mcubsp::mcu::nrf52840 {
struct uart_baud_entry { std::uint32_t nominal, encoding, actual; };
inline constexpr std::array<uart_baud_entry,18> uart_bauds{{
    uart_baud_entry{1200,0x0004F000,1205}, {2400,0x0009D000,2396}, {4800,0x0013B000,4808},
    {9600,0x00275000,9598}, {14400,0x003B0000,14414}, {19200,0x004EA000,19208},
    {28800,0x0075F000,28829}, {31250,0x00800000,31250}, {38400,0x009D5000,38462},
    {56000,0x00E50000,55944}, {57600,0x00EBF000,57762}, {76800,0x013A9000,76923},
    {115200,0x01D7E000,115942}, {230400,0x03AFB000,231884}, {250000,0x04000000,250000},
    {460800,0x075F7000,470588}, {921600,0x0EBED000,941176}, {1000000,0x10000000,1000000}}};
inline constexpr std::array<uart_baud_entry,18> uarte_bauds{{
    {1200,0x0004F000,1205}, {2400,0x0009D000,2396}, {4800,0x0013B000,4808},
    {9600,0x00275000,9598}, {14400,0x003AF000,14401}, {19200,0x004EA000,19208},
    {28800,0x0075C000,28777}, {31250,0x00800000,31250}, {38400,0x009D0000,38369},
    {56000,0x00E50000,55944}, {57600,0x00EB0000,57554}, {76800,0x013A9000,76923},
    {115200,0x01D60000,115108}, {230400,0x03B00000,231884}, {250000,0x04000000,250000},
    {460800,0x07400000,457143}, {921600,0x0F000000,941176}, {1000000,0x10000000,1000000}}};
[[nodiscard]] constexpr bool uart_instance(peripheral p) noexcept {
    return p==peripheral::uart0 || p==peripheral::uarte0 || p==peripheral::uarte1;
}
template<peripheral P> requires(uart_instance(P)) struct uart_traits {
    static constexpr bool easydma = P!=peripheral::uart0;
    static constexpr std::uintptr_t base = P==peripheral::uarte1 ? 0x40028000U : 0x40002000U;
    static constexpr unsigned irq = P==peripheral::uarte1 ? 40 : 2;
    static constexpr std::uint32_t enable = easydma ? 8U : 4U;
    static constexpr bool two_stop_bits = easydma;
    static constexpr unsigned data_bits=8;
};
} // namespace mcubsp::mcu::nrf52840
