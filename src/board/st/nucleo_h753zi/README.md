# ST NUCLEO-H753ZI

[Requirements](requirements.md) and [implementation plan](implementation-plan.md)
record this board's scope, work and acceptance boundaries.


The local `board.cmake` owns build selection, startup and runner settings.
The public board type is `mcubsp::board::st::nucleo_h753zi`, defined in
`include/mcubsp/board/st/nucleo_h753zi.hpp`. Its lightweight wiring and
connection headers are in `include/mcubsp/board/st/nucleo_h753zi/`.
The shared MCU description remains under `include/mcubsp/mcu/stm32h753/`.

See [the board wiring, startup, physical procedures and observation limits](doc/README.md)
and [the board contribution guide](../../../../doc/development/adding-a-board.md).
Select this board with `cmake --preset st-nucleo-h753zi` for board verification.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Set `MCUBSP_PROBE_SERIAL` to the intended J-Link/ST-LINK probe before using
physical fixture scripts. The shared runner uses the board flash regions
and recovery instructions; it never selects an attached probe implicitly.

## Revision and evidence scope

MB1364 revision E, received jumper/bridge configuration; supplied startup uses HSI 64 MHz.

Ethernet availability is implemented MAC/PHY wiring; a passed Ethernet protocol exchange is not claimed.

PWM/ADC/I2C/SPI electrical and external-device acceptance remains limited as documented.
