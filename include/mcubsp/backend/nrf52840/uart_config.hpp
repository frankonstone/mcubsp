#pragma once
// @req-file REQ-0027 REQ-0059
#include <optional>
#include <mcubsp/backend/nrf52840/resources.hpp>
#include <mcubsp/driver/uart.hpp>
#include <mcubsp/mcu/nrf52840/uart.hpp>
#include <mcubsp/mcu/nrf52840/peripherals.hpp>
#include <mcubsp/mcu/nrf52840/product/pins.hpp>
namespace mcubsp::backend::nrf52840 {
struct uart_baud_timing : driver::uart_baud_timing {
    std::uint32_t encoding;
    peripheral instance;
};
// HF source accuracy is additional to the documented hardware quantization.
[[nodiscard]] constexpr std::expected<uart_baud_timing,driver::uart_error>
solve_uart_baud(peripheral instance, driver::uart_baud_request request) noexcept {
    if (!mcu::nrf52840::uart_instance(instance)) return std::unexpected(driver::uart_error::invalid_format);
    if (request.clock_hz!=16'000'000) return std::unexpected(driver::uart_error::invalid_clock);
    if (!request.target_baud || request.target_baud>1'000'000) return std::unexpected(driver::uart_error::invalid_baud);
    const auto &rates = instance==peripheral::uart0 ? mcu::nrf52840::uart_bauds : mcu::nrf52840::uarte_bauds;
    std::optional<uart_baud_timing> best;
    for (const auto &rate:rates) {
        auto delta=rate.actual>request.target_baud ? rate.actual-request.target_baud : request.target_baud-rate.actual;
        auto error=(std::uint64_t(delta)*1'000'000+request.target_baud/2)/request.target_baud;
        if (error<=request.maximum_error_ppm && (!best || error<best->error_ppm))
            best=uart_baud_timing{{request.clock_hz,request.target_baud,rate.actual,static_cast<std::uint32_t>(error)},rate.encoding,instance};
    }
    if (!best) return std::unexpected(driver::uart_error::unattainable_tolerance);
    return *best;
}
[[nodiscard]] constexpr bool uart_format_supported(peripheral p, driver::uart_format f) noexcept {
    return mcu::nrf52840::uart_instance(p) && f.data_bits==driver::uart_data_bits::eight &&
        (f.parity==driver::uart_parity::none || f.parity==driver::uart_parity::even) &&
        (f.stop_bits==driver::uart_stop_bits::one ||
         (p!=peripheral::uart0 && f.stop_bits==driver::uart_stop_bits::two));
}
inline constexpr core::pin_id uart_disconnected{0xFFFF,0xFFFF};
template<peripheral P, core::pin_id Tx, core::pin_id Rx,
         core::pin_id Cts=uart_disconnected, core::pin_id Rts=uart_disconnected>
    requires(mcu::nrf52840::uart_instance(P))
struct uart_binding {
    static_assert(mcu::nrf52840::gpio_supported(Tx) && mcu::nrf52840::gpio_supported(Rx),"UART requires available GPIOs");
    static constexpr bool flow_control=Cts!=uart_disconnected || Rts!=uart_disconnected;
    static_assert(!flow_control || (mcu::nrf52840::gpio_supported(Cts) && mcu::nrf52840::gpio_supported(Rts)),
                  "UART hardware flow control requires both CTS and RTS");
    static_assert(Tx!=Rx && (!flow_control || (Cts!=Rts && Cts!=Tx && Cts!=Rx && Rts!=Tx && Rts!=Rx)),
                  "UART pins must be distinct");
    static constexpr auto instance=P;
    static constexpr auto tx=Tx, rx=Rx, cts=Cts, rts=Rts;
    static constexpr auto base=mcu::nrf52840::uart_traits<P>::base;
    [[nodiscard]] static constexpr bool supports_rate(std::uint32_t rate) noexcept {
        for(auto pin:std::array{Tx,Rx,Cts,Rts})
            if (pin!=uart_disconnected && mcu::nrf52840::product::silicon_low_frequency_only(pin) && rate>10000)
                return false;
        return true;
    }
};
} // namespace mcubsp::backend::nrf52840
