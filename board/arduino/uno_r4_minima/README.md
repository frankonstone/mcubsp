# Arduino UNO R4 Minima


The local `board.cmake` owns build selection. Public wiring remains in
`include/mcuhw/board/arduino_uno_r4_minima.hpp` and the application profile in `include/mcuhw/io/boards/uno_r4_minima.hpp`.
The shared MCU description is `renesas/ra4m1`; no register description is copied here.

See [the board wiring, startup, physical procedures and observation limits](docs/README.md)
and [the board contribution guide](../../../doc/development/adding-a-board.md).
Select this board with `python3 tools/mcuhw.py new ../blink --board arduino/uno_r4_minima`.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Set `MCUHW_PROBE_SERIAL` to the intended J-Link/ST-LINK probe before using
physical fixture scripts. The shared runner uses the board flash regions
and recovery instructions; it never selects an attached probe implicitly.

## Revision and evidence scope

Retains the Arduino DFU bootloader below 0x4000; supplied startup owns 48 MHz HOCO and vectors.

The binding revision r4 names the documented board model; the historical records do not establish an exact PCB revision.

External TMP102 wiring is an application requirement; no physical TMP102 acceptance is claimed.
