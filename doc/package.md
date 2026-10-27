# mcubsp application package

mcubsp provides C++23 hardware descriptions, drivers, attached-device protocols,
and optional startup integration for supported boards. The library interfaces
are header-only; the package supplies board startup sources and linker resources
when an application explicitly requests them.

## Use an installed package

The Apple Silicon macOS checkout workflow builds, tests and produces `output/install`:

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64
cmake --install output/build/macos-arm64
```

On Linux x86_64, replace `macos-arm64` with `linux-x86_64` in the commands and
build path. To install without test dependencies, configure with
`-DMCUBSP_BUILD_HOST_TESTS=OFF -DMCUBSP_CHECK_HEADERS=OFF`.

In a dependent project's `CMakeLists.txt`:

```cmake
find_package(mcubsp 0.1 CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE mcubsp::io mcubsp::device)
```

Configure that project with
`-DCMAKE_PREFIX_PATH=/absolute/path/to/mcubsp/output/install`.
Alternatively set `mcubsp_DIR` to that prefix's `lib/cmake/mcubsp` directory.
`mcubsp::mcubsp` exposes the core interface; `mcubsp::io` and `mcubsp::device`
provide the application and attached-device interfaces. An existing BSP retains
its toolchain, executable, startup, interrupts and linker layout.

The prefix is relocatable. Its `lib/cmake/mcubsp/` directory contains the package
configuration and exported targets; `share/mcubsp/` contains public headers,
firmware helpers, board resources, runner tools and example applications.
An explicit `cmake --install ... --prefix /another/prefix` overrides the default.
Toolchains and SDKs are separate prerequisites.

## Start a supported-board application

Copy `share/mcubsp/example/blink/` from the installed package into your project.
From that copied directory:

```sh
cmake -S . -B output/build/uno -G Ninja \
  -DCMAKE_PREFIX_PATH=/path/to/prefix \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/prefix/share/mcubsp/cmake/board-toolchain.cmake \
  -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/uno --target blink
```

| Board | Default revision | Toolchain and Blink output |
| --- | --- | --- |
| `arduino/uno_r4_minima` | `r4` | Arm GNU; built-in D13 LED |
| `st/nucleo_h753zi` | `mb1364-rev-e` | Arm GNU; built-in green LED |
| `waveshare/esp32p4_nano` | `development-v1.0` | ESP-IDF v6.1 with its Python and RISC-V compiler; external LED and series resistor on GPIO2 |

Use a fresh build directory for each board. Board toolchain selection occurs
before CMake's `project()`. Sources include and name their concrete board type;
CMake does not generate a C++ board alias. The ESP32-P4 integration targets the
documented development-v1.0 board; production-v3 startup is not claimed.

`mcubsp_add_firmware(app STARTUP supplied SOURCES main.cpp)` supplies the selected
board's startup and link integration. Choose `STARTUP external` when your BSP
owns those resources. Building an image does not flash or run it.

## Runner and board recovery information

Use the installed `share/mcubsp/tool/hardware.py` to validate an image and plan
flashing with an explicit probe or serial port. For an UNO application:

```sh
python3 /path/to/prefix/share/mcubsp/tool/hardware.py flash \
  --board arduino/uno_r4_minima --elf /path/to/blink.elf \
  --probe 123456789 --dry-run
```

NUCLEO uses the same interface with `--board st/nucleo_h753zi` and an explicit
ST-LINK probe serial. ESP32-P4 uses `--port` and `--sdk-build` pointing to the
complete SDK build bundle, rather than an ELF alone. Inspect the dry-run plan
before executing a flash command.

The UNO supplied application layout starts at `0x4000`, preserving the factory
boot area below it. NUCLEO uses its ST-LINK/SWD interface and flash beginning at
`0x08000000`. ESP32-P4 uses the selected development board's complete bootloader,
partition-table and application bundle; retain a compatible recovery image and
use the board's ROM-download procedure when application startup is unavailable.

## Package contents

The package contains public headers, public CMake integration, board metadata,
startup/linker/boot resources, firmware-record and runner tools, examples, and
this guide. Module tests, test dependencies, development tools, captured
evidence, private references and internal plans remain in the source checkout.
Rebuilding verification suites requires that checkout.
