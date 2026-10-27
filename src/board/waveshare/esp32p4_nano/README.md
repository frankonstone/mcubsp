# Waveshare ESP32-P4-NANO


The local `board.cmake` owns build selection, startup and runner settings.
The public board type is `mcubsp::board::waveshare::esp32p4_nano`, defined in
`include/mcubsp/board/waveshare/esp32p4_nano.hpp`. Its lightweight wiring and
connection headers are in `include/mcubsp/board/waveshare/esp32p4_nano/`.
The shared MCU description remains under `include/mcubsp/mcu/esp32p4/`.

See [the board wiring, startup, physical procedures and observation limits](doc/README.md)
and [the board contribution guide](../../../../doc/development/adding-a-board.md).
Select this board with `cmake --preset waveshare-esp32p4-nano` for board verification.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Physical fixture scripts require an explicit serial port, supplied as their port
argument or `MCUBSP_SERIAL_PORT` where documented.

## Revision and evidence scope

development-v1.0 names observed silicon, not an independently established PCB revision. Production v3.x remains outside this board observation scope.

Explicit ESP-IDF v6.1 boot path; HP CPU0 only. Existing controller-specific development I2C masks remain board-owned.

Ethernet link was down in recorded observation; no network exchange claim.
