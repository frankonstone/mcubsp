# Repository ownership

`src/` mirrors the directories beneath `include/mcubsp/`. Public headers remain
under `include/`; implementation resources and module-owned tests live under
matching source directories:

```text
include/mcubsp/                    src/
  arch/                             arch/test/
  core/                             core/test/
  execution/                        execution/test/
  testing/                          testing/test/
  driver/                           driver/test/
  backend/<mcu>/                    backend/<mcu>/test/
  mcu/<mcu>/                        mcu/<mcu>/test/
  device/{audio,ethernet,sensor}/    device/{audio,ethernet,sensor}/test/
  io/                               io/test/
  io/detail/                        io/detail/test/
  board/<vendor>/<board>/           board/<vendor>/<board>/
                                      board.cmake
                                      startup/
                                      linker.ld
                                      test/
                                      doc/

example/<topic>/                  Standalone application projects
  CMakeLists.txt
  README.md
  <vendor>/<board>.cpp

test/                             Project-wide verification infrastructure
  compile/                        Compiler policy and cross-build orchestration
  workflow/                       Build, install, provenance and runner checks
  support/                        Shared runners, fixtures and reporting
```

Each public-header leaf has a matching source directory with a `test/` folder.
Directories that own headers and subdirectories can also own tests. Shared MCU
contracts live in `src/mcu/test/`; shared backend contracts live in
`src/backend/test/`. A module's positive and negative compiler cases live beside
its behavioral tests. The root `test/compile/` registers compiler cases and
cross-build probes without owning module-specific C++ sources.

Board startup, linker resources, runner settings and physical tests live in
`src/board/<vendor>/<board>/`. Native backend tests use recorded MMIO and live
under `src/backend/<mcu>/test/`; physical fixtures remain with their concrete
board. Examples live exclusively under `example/` and use the same sources for
standalone projects and repository targets.

## Header responsibilities

- `<vendor>/<board>.hpp` defines the concrete board type in
  `mcubsp::board::<vendor>`. It composes the default backends, execution policy
  and I/O bindings and can be passed to `mcubsp::io::board`.
- `<vendor>/<board>/pins.hpp` defines `<board>_pins`, containing physical pin
  names and fixed wiring addresses. It depends only on core metadata.
- `<vendor>/<board>/bindings.hpp` defines `<board>_bindings`, inheriting the pin
  constants and adding board identity/clock/memory facts and typed connections.
  It can use MCU descriptions, but depends on neither I/O nor backend
  implementations. The complete board type inherits these definitions.
- `mcu/` owns register addresses, chip capabilities, native pins, interrupts and
  DMA metadata. `backend/` owns register sequencing. Neither depends on a concrete
  board. `io/` owns generic containers and operations; concrete boards live in
  the public board tree.

`cmake/Firmware.cmake` owns the shared firmware recipe. `cmake/arch/` owns
compiler/ABI settings, and `cmake/board-toolchain.cmake` selects the chosen local
board toolchain before compiler configuration. The root board entry owns startup,
memory layout and runner settings. Device protocols take explicit transports.

## Board API migration

| Earlier API | Current API |
| --- | --- |
| `<mcubsp/io/board/uno_r4_minima.hpp>` | `<mcubsp/board/arduino/uno_r4_minima.hpp>` |
| `mcubsp::io::boards::uno_r4_minima` | `mcubsp::board::arduino::uno_r4_minima` |
| `<mcubsp/io/board/nucleo_h753zi.hpp>` | `<mcubsp/board/st/nucleo_h753zi.hpp>` |
| `mcubsp::io::boards::nucleo_h753zi` | `mcubsp::board::st::nucleo_h753zi` |
| `<mcubsp/io/board/esp32p4_nano.hpp>` | `<mcubsp/board/waveshare/esp32p4_nano.hpp>` |
| `mcubsp::io::boards::esp32p4_nano` | `mcubsp::board::waveshare::esp32p4_nano` |

Low-level users of the former flat `mcubsp/board/*.hpp` wiring namespaces use
the corresponding `pins.hpp` or `bindings.hpp` header:

| Earlier wiring header | Current connection header |
| --- | --- |
| `<mcubsp/board/arduino_uno_r4_minima.hpp>` | `<mcubsp/board/arduino/uno_r4_minima/bindings.hpp>` |
| `<mcubsp/board/nucleo_h753zi.hpp>` | `<mcubsp/board/st/nucleo_h753zi/bindings.hpp>` |
| `<mcubsp/board/esp32p4_nano.hpp>` | `<mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>` |

For example:

```cpp
#include <mcubsp/board/st/nucleo_h753zi/bindings.hpp>
using wiring = mcubsp::board::st::nucleo_h753zi_bindings;
```

These are named structs with static constants/types. Change namespace aliases
such as `namespace wiring = ...` to type aliases `using wiring = ...`.
There are no forwarding headers in the old `io/board/` or flat board locations.
Historical evidence retains its original source/firmware identity.

## Board and test path migration

The former root `board/` and `bsp/` support trees are now `src/board/`. Startup
sources remain under each board's `startup/`, and Arm `linker.ld` files sit
beside `board.cmake`. Public headers, board identities and `MCUBSP_BOARD` remain
unchanged.

Repository firmware outputs now include the source subdirectory, for example
`output/build/arduino-uno-r4-minima/src/board/firmware.elf`. External applications retain
their own output paths. Cached application selections pointing to former
board-local examples migrate to the matching `example/<topic>/` source. Use a
fresh build directory for the relocated standalone P4 physical-test SDK project;
CMake records its source directory in the cache.

The former root `test/core/`, `test/backend/`, `test/device/`, `test/driver/`,
`test/io/` and `test/mcu/` trees have been distributed to their module owners.
Execution-policy tests moved to `src/execution/test/`. Recording-MMIO and
description-suite tests moved to `src/testing/test/`. Host binaries remain under
`output/build/<preset>/test/`, and merged reports remain under `output/build/<preset>/artifact/`.
Historical evidence retains its original source and firmware identity.

## Build and installation output

Repository presets build under `output/build/<preset>/`. The `macos-arm64` and
`linux-x86_64` presets build and test the library and prepare the consumer package;
`cmake --install output/build/<preset>` installs it
under `output/install` by default. Dependent projects use that prefix through
`CMAKE_PREFIX_PATH`, or its `lib/cmake/mcubsp` directory through `mcubsp_DIR`.
Explicit install-prefix overrides remain supported.

The consumer package contains public headers, public CMake/toolchain helpers,
board metadata and startup/linker/SDK boot resources, required build/runner tools,
examples and the application package guide. Module tests, test infrastructure,
captured evidence and internal plans stay in the checkout. Existing build
artifacts were moved intact; reconfigure moved CMake caches with `--fresh` before
reusing them, because generated build systems contain absolute directory paths.

## C++ source migration

Generic timing requests and reports remain under `mcubsp::driver`. Register-bearing
timing configurations now belong to their backend and cannot initialize a different
vendor's controller. RA callers replace `driver::solve_uart_baud`,
`driver::solve_i2c_timing`, `driver::solve_spi_timing` and
`driver::solve_pwm_timing` with the corresponding `backend::ra4m1::` function.
Include its peripheral header, or `backend/ra4m1/timing/{uart,i2c,spi,pwm}.hpp`
when only solving timings. Prefer `auto` for native timing results. Conversion to
the generic report preserves requested/achieved rates and error information;
register fields are available only on the native type.

Ethernet now takes a board binding explicitly:

```cpp
using mac = mcubsp::backend::stm32h753::ethernet_rmii_mac<
    mmio_type, mcubsp::board::st::nucleo_h753zi_bindings::ethernet_binding>;
```

ESP Ethernet similarly uses `mcubsp::board::waveshare::esp32p4_nano_bindings::ethernet_binding`.
ESP SDMMC requires a third template argument, such as
`mcubsp::board::waveshare::esp32p4_nano_bindings::sdmmc_slot0_binding` or
`sdmmc_slot1_wifi_binding`. Another board supplies its own binding; controller
code no longer selects the development board's pins implicitly.

The DMA fixes also change internal storage: STM Ethernet descriptors occupy a
32-byte cache line each, with matching hardware stride. ESP SDMMC uses one
static 512-byte staging sector plus its 64-byte descriptor; DMA never addresses
caller buffers. A failed DMA reset keeps the controller owned until shutdown
confirms quiescence, preventing reuse of active DMA storage.

## Build and evidence ownership

An existing application links interface targets and retains its BSP. New
applications explicitly call `mcubsp_add_firmware`; see the
[application guide](../getting-started/application.md). Successful builds write
ELF and selected-input records. Flashing is an explicit separate operation.
Current CMake presets and the CI workflow compile/link applications and probes;
they do not execute physical fixtures. See [verification](verification.md).

Cross-library compositions live one level above this repository:

```text
pocs/
  mcubsp/       # mcubsp: hardware descriptions, backends, devices and BSPs
  mcuip/       # network stack
  integration/ # adapters, cross-library contract checks and composed applications
```

The integration project depends on the libraries. mcubsp has no dependency on
that project and installs no integration headers. MCUIP adapters use the
`mcuintegration::mcubsp_mcuip` target and namespace; see the
[integration guide](../../../integration/README.md).
