# Build a board application

For a project you own, start with the [application guide](application.md) and
[standalone starter](../../example/blink/README.md). The preset commands below
build demos from the mcubsp repository itself.

Use CMake 3.25+, Ninja, Python 3.10+ and a complete C++23 cross compiler with
matching libstdc++ headers. Python records selected firmware inputs after linking.
Build and flash are separate operations. The former Python
new/setup/doctor/flash frontend was removed; CMake is the supported entry point.
No command on this page downloads tools or touches hardware.

## Build Blink from the checkout

```sh
cmake --preset arduino-uno-r4-minima
cmake --build --preset arduino-uno-r4-minima --target firmware
```

| Preset | Binding revision | Compiler | Output |
| --- | --- | --- | --- |
| `arduino-uno-r4-minima` | `r4` | Arm GNU | `output/build/arduino-uno-r4-minima/src/board/firmware.elf` |
| `st-nucleo-h753zi` | `mb1364-rev-e` | Arm GNU | `output/build/st-nucleo-h753zi/src/board/firmware.elf` |
| `waveshare-esp32p4-nano` | `development-v1.0` | ESP-IDF RISC-V GCC | `output/build/waveshare-esp32p4-nano/src/board/firmware.elf` |

Replace the preset name in both commands to select another board. The supported
Arm baseline is Arm GNU 15.2.Rel1 (GCC 15.2.1). P4 uses ESP-IDF v6.1 and its
GCC 15.2.0 toolchain. These are recorded local build environments; hosted
Linux/Windows qualification requires successful CI runs.

UNO preserves its DFU bootloader: application flash starts at `0x4000`.
NUCLEO uses HSI at 64 MHz and vectors at `0x08000000`. P4 supplied startup
requires development-v1.0 silicon at 360 MHz and rejects another revision.
P4 Blink toggles free expansion GPIO2; observing it requires an external LED
and series resistor. The NANO has no claimed built-in GPIO LED.

For P4, activate the installed ESP-IDF environment first. Set `IDF_PATH`,
`IDF_TOOLS_PATH` and, when necessary, `-DPYTHON=/path/to/idf/venv/bin/python`.
The SDK, Python packages and constraints file must belong to that installation.
The build uses the installed SDK without component downloads. The complete
flash bundle remains in `src/board/firmware-idf-build/`, including the bootloader,
partition table, application and `flasher_args.json`.

Select an application or exact revision explicitly:

```sh
cmake --preset arduino-uno-r4-minima -DMCUBSP_APPLICATION=/absolute/path/main.cpp -DMCUBSP_BOARD_REVISION=r4
cmake --build --preset arduino-uno-r4-minima --target firmware
```

Use a separate build directory when changing board, toolchain or startup owner.
Every linked image has an adjacent `.elf.build.json` recording its SHA-256,
size, board, revision, startup choice and compiler path. Ninja builds also bind
an adjacent `.elf.inputs.json` with selected source/header/configuration,
linker-input and compiler/tool fingerprints. See
[input identity and scope](../development/firmware-inputs.md).

## Create an external application

Create `main.cpp` and the following `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_firmware LANGUAGES CXX ASM)
find_package(mcubsp 0.1 CONFIG REQUIRED)
mcubsp_add_firmware(firmware
    BOARD arduino/uno_r4_minima REVISION r4
    STARTUP supplied SOURCES main.cpp)
```

After [installing the library](installation.md), configure with its toolchain
and package prefix, then build:

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/install -DCMAKE_TOOLCHAIN_FILE=/path/to/install/share/mcubsp/cmake/board-toolchain.cmake -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build build --target firmware
```

A checkout consumer can replace `find_package` with
`add_subdirectory(/path/to/mcubsp mcubsp EXCLUDE_FROM_ALL)`. The public helper is
available automatically in both modes.

`STARTUP external` defines `MCUBSP_EXTERNAL_STARTUP` and adds no startup
sources or linker script. Supply the existing BSP's startup, linker options,
clock configuration and interrupt routing yourself. Its clocks must match the
selected C++ profile. Generic drivers never take ownership of these services.

## Hardware procedures

Building never flashes. The [hardware guide](hardware.md) documents the shared
runner's dry-run/flash commands, exact probe/port selection, image-region and
checksum checks, and board-owned capture scripts. Read the board's wiring and
recovery notes before running its physical procedure. Existing observations
retain their original firmware/fixture identity; current software checks do not
refresh them. The [support matrix](../reference/support-matrix.md) lists pending
physical qualification.
