# W5100 Ethernet

Receives raw Ethernet frames through an Ethernet Shield R3 and reports reception over the physical D0/D1 UART. D10 selects the W5100; D4 keeps the shield SD card deselected. This example uses the raw device interface; IP-stack compositions live in the separate integration project.

Supported board: Arduino UNO R4 Minima. The complete source is
[arduino/uno_r4_minima.cpp](arduino/uno_r4_minima.cpp).

From the repository root:

```sh
cmake -S example/w5100 -B output/build/example-w5100 -G Ninja \
  -DMCUBSP_SOURCE_DIR="$PWD" \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/example-w5100 --target w5100
```

You can also copy this directory into your own application. For an installed
package, omit `MCUBSP_SOURCE_DIR`, set `CMAKE_PREFIX_PATH` to its prefix, and
select its `share/mcubsp/cmake/board-toolchain.cmake`. See the
[Blink guide](../blink/README.md) for complete package commands.

The repository preset exposes the same source as
`arduino.uno_r4_minima.example.w5100`. Building does not flash or run it.
