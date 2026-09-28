# ST NUCLEO-H753ZI


The local `board.cmake` owns build selection. Public wiring remains in
`include/mcuhw/board/nucleo_h753zi.hpp` and the application profile in `include/mcuhw/io/board/nucleo_h753zi.hpp`.
The shared MCU description is `st/stm32h753`; no register description is copied here.

See [the board wiring, startup, physical procedures and observation limits](doc/README.md)
and [the board contribution guide](../../../doc/development/adding-a-board.md).
Select this board with `cmake --preset arm-cortex-m7 -DMCUHW_BOARD=st/nucleo_h753zi` for board verification.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Set `MCUHW_PROBE_SERIAL` to the intended J-Link/ST-LINK probe before using
physical fixture scripts. The shared runner uses the board flash regions
and recovery instructions; it never selects an attached probe implicitly.

## Revision and evidence scope

MB1364 revision E, received jumper/bridge configuration; supplied startup uses HSI 64 MHz.

Ethernet availability is implemented MAC/PHY wiring; a passed Ethernet protocol exchange is not claimed.

PWM/ADC/I2C/SPI electrical and external-device acceptance remains limited as documented.
