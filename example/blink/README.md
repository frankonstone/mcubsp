# A standalone mcubsp application

Copy this directory into your own project. It builds through either a source
checkout or an installed package and does not run mcubsp's tests. Each source
includes and names its concrete C++ board, for example
`mcubsp::board::arduino::uno_r4_minima` in `arduino/uno_r4_minima.cpp`.

## Use a source checkout

From your copied application directory, with mcubsp at `/path/to/mcubsp`:

```sh
cmake -S . -B output/build/uno -G Ninja -DMCUBSP_SOURCE_DIR=/path/to/mcubsp -DCMAKE_TOOLCHAIN_FILE=/path/to/mcubsp/cmake/board-toolchain.cmake -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/uno --target blink
```

## Use an installed package

With the package installed at `/path/to/prefix`:

```sh
cmake -S . -B output/build/uno -G Ninja -DCMAKE_PREFIX_PATH=/path/to/prefix -DCMAKE_TOOLCHAIN_FILE=/path/to/prefix/share/mcubsp/cmake/board-toolchain.cmake -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/uno --target blink
```

The toolchain is selected before `project()`. It reads the chosen board's local
settings and resolves its compiler. An installed toolchain is a path supplied
at configuration time because `find_package` runs after compiler selection.
The starter selects the matching `<vendor>/<board>.cpp` source. In your own
application, include that board's header and use its type explicitly in your
`main.cpp`; CMake does not generate a C++ board alias.

## Choose a board

| `MCUBSP_BOARD` | Default revision | Blink output | Additional environment |
| --- | --- | --- | --- |
| `arduino/uno_r4_minima` | `r4` | Built-in D13 LED | Installed Arm GNU toolchain |
| `st/nucleo_h753zi` | `mb1364-rev-e` | Built-in green LED | Installed Arm GNU toolchain |
| `waveshare/esp32p4_nano` | `development-v1.0` | Expansion GPIO2: attach an LED and series resistor to ground | Activated ESP-IDF v6.1 environment and its Python/toolchain |
| `others/promicro_nrf52840` | `uf2-s140-v6` | Active-high LED on P0.15 | Arm GNU; existing UF2/S140-v6 bootloader layout |

Use a fresh build directory per board. Override `MCUBSP_BOARD_REVISION` only
with a supported exact revision. No P4 built-in GPIO LED or production-v3
startup is claimed. For P4, `blink` builds an SDK subproject; the complete
flash bundle is in `blink-idf-build/`, next to the resulting `blink.elf`.

Building never flashes. For application setup, ownership choices and an explicit
probe/port runner plan, read the
[application package guide](../../doc/package.md).

Pro Micro also generates `blink.elf.uf2`. Its GPIO-only profile selects
`.serial = false`; USB serial and Bluetooth are not supplied by the application.
See the [Pro Micro guide](../../src/board/others/promicro_nrf52840/README.md)
for memory/handoff assumptions and the explicit mounted-drive UF2 upload.
