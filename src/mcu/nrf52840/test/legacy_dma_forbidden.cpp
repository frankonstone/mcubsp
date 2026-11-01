// @req-file REQ-0062
#include <mcubsp/mcu/nrf52840/resources.hpp>
using invalid = mcubsp::mcu::nrf52840::serial_dma_traits<mcubsp::mcu::nrf52840::peripheral::uart0>;
static_assert(invalid::count_bits > 0);
