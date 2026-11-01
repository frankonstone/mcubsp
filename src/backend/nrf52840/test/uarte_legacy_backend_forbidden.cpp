// @req-file REQ-0059 REQ-0062
#include <mcubsp/backend/nrf52840/uart.hpp>
using binding=mcubsp::backend::nrf52840::uart_binding<mcubsp::mcu::nrf52840::peripheral::uarte0,{0,6},{0,8}>;
struct context {};
using invalid=mcubsp::backend::nrf52840::uart_controller<binding,context>;
static_assert(sizeof(invalid)>0);
