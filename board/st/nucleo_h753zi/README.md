# ST NUCLEO-H753ZI

Catalog identity: `board:st/nucleo_h753zi@mb1364-rev-e`.

The local manifest owns discovery and build selection. Public wiring remains in
`include/mcuhw/board/nucleo_h753zi.hpp` and the application profile in `include/mcuhw/io/boards/nucleo_h753zi.hpp`.
The shared MCU description is `st/stm32h753`; no register description is copied here.

See [the board wiring, startup, physical procedures and observation limits](docs/README.md)
and [the component workflow](../../../doc/development/catalog.md).
Select this board with `python3 tools/mcuhw.py new blink --board st/nucleo_h753zi`.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Set `MCUHW_PROBE_SERIAL` to the intended J-Link/ST-LINK probe before using
physical fixture scripts. The shared runner uses the component flash regions
and recovery instructions; it never selects an attached probe implicitly.

## Revision and evidence scope

MB1364 revision E, received jumper/bridge configuration; supplied startup uses HSI 64 MHz.

Ethernet availability is implemented MAC/PHY wiring; a passed Ethernet protocol exchange is not claimed.

PWM/ADC/I2C/SPI electrical and external-device acceptance remains limited as documented.
