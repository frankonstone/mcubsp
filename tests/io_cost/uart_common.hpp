#pragma once

#include "common.hpp"

#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/driver/uart.hpp>

namespace cost {
inline constexpr std::array<std::uint8_t, 16> message{
    'm', 'c', 'u', 'h', 'w', '-', 'i', 'o', '-', 'p', 'r', 'o', 'b', 'e', '\r', '\n',
};

#if MCUHW_IO_COST_FACADE
inline auto hardware = io::make_board<board_type>(execution, mmio, 48'000'000U);
void progress() noexcept {}
#else
using binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2, mcuhw::board::arduino_uno_r4_minima::d0_rx,
    mcuhw::board::arduino_uno_r4_minima::d1_tx>;
inline mcuhw::backend::ra4m1::sci_uart_backend<binding, mmio_type,
                                             execution_type>
    backend{mmio, execution};
inline mcuhw::driver::uart serial{backend};
void progress() noexcept { board_type::poll_serial(mmio, backend); }
#endif
} // namespace cost
