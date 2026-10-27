# Examples

Each topic is a standalone, copyable CMake application. Sources are grouped by
board inside the topic, for example `blink/arduino/uno_r4_minima.cpp`. Repository
build targets use these same files.

| Example | Boards | Behavior |
| --- | --- | --- |
| [Blink](blink/README.md) | UNO R4 Minima, NUCLEO-H753ZI, ESP32-P4-NANO, Pro Micro nRF52840 | Toggle an LED every 500 ms |
| [Recovery](recovery/README.md) | UNO R4 Minima | UART echo with timeout, cancellation and recovery |
| [Serial echo](serial_echo/README.md) | UNO R4 Minima | Echo bytes over the physical UART |
| [SPI DMA](spi_dma/README.md) | UNO R4 Minima | Transfer over SPI with explicit DMA channels |
| [TMP102](tmp102/README.md) | UNO R4 Minima | Read a temperature sensor and indicate a threshold |
| [W5100](w5100/README.md) | UNO R4 Minima | Receive raw Ethernet frames through an Ethernet Shield R3 |

Build one application directly from this checkout:

```sh
cmake -S example/recovery -B output/build/example-recovery -G Ninja \
  -DMCUBSP_SOURCE_DIR="$PWD" \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/example-recovery --target recovery
```

Alternatively, configure a repository board preset and build its examples:

```sh
cmake --preset arduino-uno-r4-minima
cmake --build --preset arduino-uno-r4-minima --target examples
```

Individual repository targets retain the form
`arduino.uno_r4_minima.example.recovery`. The default `firmware` target uses
the matching source in `blink/`, unless `MCUBSP_APPLICATION` selects another
application. Only examples with a source for the selected board are configured.

The [Blink guide](blink/README.md) also explains installed-package consumption.
The [application package guide](../doc/package.md) describes package use,
supported boards and the runner interface. Building does not flash or run an image.
