# UART recovery

Echoes bytes on D0/D1 at 115200 baud, with explicit timeout, cancellation and recovery handling. Send `q` to shut down the peripherals. Use a voltage-compatible UART peer and common ground; this uses the physical UART, not USB CDC.

Supported board: Arduino UNO R4 Minima. The complete source is
[arduino/uno_r4_minima.cpp](arduino/uno_r4_minima.cpp).

From the repository root:

```sh
cmake -S example/recovery -B output/build/example-recovery -G Ninja \
  -DMCUBSP_SOURCE_DIR="$PWD" \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/example-recovery --target recovery
```

You can also copy this directory into your own application. For an installed
package, omit `MCUBSP_SOURCE_DIR`, set `CMAKE_PREFIX_PATH` to its prefix, and
select its `share/mcubsp/cmake/board-toolchain.cmake`. See the
[Blink guide](../blink/README.md) for complete package commands.

The repository preset exposes the same source as
`arduino.uno_r4_minima.example.recovery`. Building does not flash or run it.
