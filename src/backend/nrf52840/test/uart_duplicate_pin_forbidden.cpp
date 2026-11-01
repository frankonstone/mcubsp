// @req-file REQ-0059
#include <mcubsp/backend/nrf52840/uart_config.hpp>
using invalid=mcubsp::backend::nrf52840::uart_binding<mcubsp::mcu::nrf52840::peripheral::uarte0,{0,6},{0,6}>;
static_assert(invalid::base!=0);
