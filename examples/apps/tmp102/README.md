# TMP102

This complete application reads a TMP102 at I2C address `0x48` and lights the
selected LED when the sample exceeds 25 °C. Copy this directory outside the
repository and use the same selected-board CMake commands as the Blink example.

```sh
cmake -S . -B build-uno -G Ninja \
  -DMCUHW_SOURCE_DIR=/path/to/mcuhw \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/mcuhw/cmake/McuhwToolchain.cmake \
  -DMCUHW_BOARD=arduino/uno_r4_minima
cmake --build build-uno
```

| Board ID | SDA | SCL | Indicator |
| --- | --- | --- | --- |
| `arduino/uno_r4_minima` | A4 | A5 | Built-in D13 LED |
| `st/nucleo_h753zi` | Arduino D14 / PB9 | Arduino D15 / PB8 | Yellow user LED |
| `waveshare/esp32p4_nano` | GPIO7 | GPIO8 | External LED + series resistor on GPIO2 |

Connect common ground and power the sensor from a suitable supply. TMP102 is
not a 5 V device: use 3.3 V sensor supply and pull-ups and a level shifter where
the board's logic requirements demand one. Ensure breakout-board pull-ups do not
pull above the sensor's supply. Tie ADD0 to GND for address `0x48`. The chosen
bus profile runs at the board's default I2C rate. Keep board wiring authoritative
in the public board headers.

ESP32-P4 requires `MCUHW_IDF_PATH` and the SDK's `PYTHON` (and
`MCUHW_IDF_TOOLS_PATH` for a nondefault tools cache); its profile is limited
to development silicon v1.0, HP CPU0 at 360 MHz, without a scheduler. Use a new
build directory per board/revision. See the board component documentation for
tool setup, flash regions and recovery. This example has cross-build coverage;
that does not claim physical TMP102 acceptance on every board.
