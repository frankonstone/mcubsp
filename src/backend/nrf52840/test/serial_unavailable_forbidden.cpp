#include <mcubsp/io/board.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
// Initial GPIO-only profile must reject the default serial selection clearly.
mcubsp::io::board<mcubsp::board::others::promicro_nrf52840> hw;
