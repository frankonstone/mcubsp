# TMP102 temperature sensor

Reads I2C address `0x48` and lights the built-in LED above 25 °C. Power the sensor
from 3.3 V, connect common ground and use suitable external bus pull-ups to 3.3 V;
tie ADD0 to ground for address `0x48`. Choose the matching board wiring:

| Board | SDA | SCL | Complete source |
| --- | --- | --- | --- |
| Arduino UNO R4 Minima | A4 | A5 | [UNO](arduino/uno_r4_minima.cpp) |
| Pro Micro nRF52840 | P1.00, label 100 | P0.11, label 011 | [Pro Micro](others/promicro_nrf52840.cpp) |

The Pro Micro route follows the Zephyr board definition; it is not an Arduino
pin-number mapping. Its I2C/sensor path has software verification only and has
not yet been observed on a physical sensor fixture.

From the repository root:

```sh
cmake -S example/tmp102 -B output/build/example-tmp102 -G Ninja \
  -DMCUBSP_SOURCE_DIR="$PWD" \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/example-tmp102 --target tmp102
```

For Pro Micro, choose `-DMCUBSP_BOARD=others/promicro_nrf52840` and a separate
build directory. It emits `tmp102.elf.uf2` and the firmware/input records.

You can also copy this directory into your own application. For an installed
package, omit `MCUBSP_SOURCE_DIR`, set `CMAKE_PREFIX_PATH` to its prefix, and
select its `share/mcubsp/cmake/board-toolchain.cmake`. See the
[Blink guide](../blink/README.md) for complete package commands.

The repository presets expose `arduino.uno_r4_minima.example.tmp102` and
`others.promicro_nrf52840.example.tmp102`. Building does not flash or run them.
