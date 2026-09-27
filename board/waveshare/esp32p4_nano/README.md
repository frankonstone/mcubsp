# Waveshare ESP32-P4-NANO

Catalog identity: `board:waveshare/esp32p4_nano@development-v1.0`.

The local manifest owns discovery and build selection. Public wiring remains in
`include/mcuhw/board/esp32p4_nano.hpp` and the application profile in `include/mcuhw/io/boards/esp32p4_nano.hpp`.
The shared MCU description is `espressif/esp32p4`; no register description is copied here.

See [the board wiring, startup, physical procedures and observation limits](docs/README.md)
and [the component workflow](../../../doc/development/catalog.md).
Select this board with `python3 tools/mcuhw.py new blink --board waveshare/esp32p4_nano`.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Physical fixture scripts require an explicit serial port, supplied as their port
argument or `MCUHW_SERIAL_PORT` where documented.

## Revision and evidence scope

development-v1.0 names observed silicon, not an independently established PCB revision. Production v3.x remains outside this board observation scope.

Explicit ESP-IDF v6.1 boot path; HP CPU0 only. Existing controller-specific development I2C masks remain board-owned.

Ethernet link was down in recorded observation; no network exchange claim.
