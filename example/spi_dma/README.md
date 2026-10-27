# SPI with DMA

Transfers four bytes through SPI1 using DMA channels 0 and 1. Connect D11 (MOSI) to D12 (MISO) for loopback without a slave. D10 is chip select and D13 is the clock. Caller-owned RAM buffers remain alive until completion.

Supported board: Arduino UNO R4 Minima. The complete source is
[arduino/uno_r4_minima.cpp](arduino/uno_r4_minima.cpp).

From the repository root:

```sh
cmake -S example/spi_dma -B output/build/example-spi_dma -G Ninja \
  -DMCUBSP_SOURCE_DIR="$PWD" \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/example-spi_dma --target spi_dma
```

You can also copy this directory into your own application. For an installed
package, omit `MCUBSP_SOURCE_DIR`, set `CMAKE_PREFIX_PATH` to its prefix, and
select its `share/mcubsp/cmake/board-toolchain.cmake`. See the
[Blink guide](../blink/README.md) for complete package commands.

The repository preset exposes the same source as
`arduino.uno_r4_minima.example.spi_dma`. Building does not flash or run it.
