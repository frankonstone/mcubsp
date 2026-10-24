# Arduino UNO R4 Minima


The local `CMakeLists.txt` owns the board images. Public wiring remains in
`include/mcubsp/board/arduino_uno_r4_minima.hpp` and the application profile in `include/mcubsp/io/board/uno_r4_minima.hpp`.
The shared MCU description is `renesas/ra4m1`; no register description is copied here.

See [the board wiring, startup, physical procedures and observation limits](doc/README.md)
and [the board contribution guide](../../../doc/development/adding-a-board.md).
Select this board with `cmake --preset arduino-uno-r4-minima` and build its
images with `cmake --build --preset arduino-uno-r4-minima`.
For coverage-instrumented test images, configure with
`cmake --preset arduino-uno-r4-minima -B build/arduino-uno-r4-minima-coverage -DMCUBSP_COVERAGE=ON`
and build with `cmake --build build/arduino-uno-r4-minima-coverage`.
Flashing is a separate command and requires the probe/port and recovery details
in the linked board procedure.
Set `MCUBSP_PROBE_SERIAL` to the intended J-Link/ST-LINK probe before using
physical fixture scripts. The shared runner uses the board flash regions
and recovery instructions; it never selects an attached probe implicitly.

## Revision and evidence scope

Retains the Arduino DFU bootloader below 0x4000; supplied startup owns 48 MHz HOCO and vectors.

The binding revision r4 names the documented board model; the historical records do not establish an exact PCB revision.

External TMP102 wiring is an application requirement; no physical TMP102 acceptance is claimed.
