# TMP102 temperature sensor

Reads I2C address `0x48` and lights the built-in LED above 25 °C. Connect SDA to A4 and SCL to A5 with a common ground. Power the sensor from 3.3 V and use pull-ups and level shifting appropriate to its supply; tie ADD0 to ground for address `0x48`.

Supported board: Arduino UNO R4 Minima. The complete source is
[arduino/uno_r4_minima.cpp](arduino/uno_r4_minima.cpp).

From the repository root:

```sh
cmake -S example/tmp102 -B output/build/example-tmp102 -G Ninja \
  -DMCUBSP_SOURCE_DIR="$PWD" \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/example-tmp102 --target tmp102
```

You can also copy this directory into your own application. For an installed
package, omit `MCUBSP_SOURCE_DIR`, set `CMAKE_PREFIX_PATH` to its prefix, and
select its `share/mcubsp/cmake/board-toolchain.cmake`. See the
[Blink guide](../blink/README.md) for complete package commands.

The repository preset exposes the same source as
`arduino.uno_r4_minima.example.tmp102`. Building does not flash or run it.
