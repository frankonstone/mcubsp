# mcuhw

`mcuhw` is a C++23 hardware-description and bare-metal driver library for
microcontrollers. It combines source-traceable MCU metadata, compile-time
validation, type-safe MMIO access, reusable functional drivers, and explicit
board compositions without requiring a vendor HAL or an RTOS.

The project is designed for firmware that needs predictable ownership and
resource use:

- no heap allocation in core drivers;
- no exceptions or RTTI;
- fixed-capacity asynchronous operations with explicit completion,
  cancellation, timeout, and recovery;
- compile-time validation of clocks, pins, routes, interrupts, DMA requests,
  and peripheral capabilities;
- host-native tests of register sequences and failure paths;
- cross-compilation and physical-board evidence kept separate from host tests.

The first accepted bare-metal target is the Arduino UNO R4 Minima and its
Renesas RA4M1. The repository also contains STM32H753 and ESP32-P4 descriptions,
backends, and board integrations at the acceptance levels documented below.

## Architecture

The library separates hardware facts from behavior and board wiring:

```text
Application
    |
Device driver (optional) TMP102 temperature, W5100 Ethernet, ...
    |
mcuhw::io (optional)    board objects, named pins, selected storage and leases
    |
Functional driver       GPIO, PWM, UART, I2C, SPI, ADC, DMA, ...
    |
MCU backend             register sequencing for one peripheral family
    |          \
MCU description          Board binding
registers, clocks,       package pins, connector aliases,
IRQs, DMA, routes        startup and linker configuration
    |          /
MMIO + execution policy
```

The common abstraction is functional rather than a fabricated universal
register layout. For example, the UART driver exposes UART behavior while the
RA4M1 SCI and STM32 USART backends retain their native register sequences and
capabilities.

The main source layers are:

| Path | Purpose |
| --- | --- |
| `include/mcuhw/core/` | Register metadata, MMIO contracts, ownership, units, and fixed transfer state |
| `include/mcuhw/mcu/` | Declarative, source-backed MCU descriptions |
| `include/mcuhw/backend/` | MCU-specific peripheral sequencing |
| `include/mcuhw/driver/` | Target-independent functional drivers and external-device drivers |
| `include/mcuhw/io/` | Modern C++ application interface and explicit board profiles |
| `include/mcuhw/device/` | Attached-chip protocols using I2C/SPI device handles or supplied transports |
| `include/mcuhw/board/` | Board identities, package selection, and connector/pin aliases |
| `include/mcuhw/execution/` | Bare-metal and test execution policies |
| `include/mcuhw/integration/` | Optional integration boundaries such as MCUIP network devices |
| `board/<vendor>/<name>/` | Local board build rules, runner configuration and procedures |
| `mcu/`, `ip/`, `device/` | Component-owned documentation and fixtures |
| `test/` | Native behavioral tests, compile checks, and compile-fail cases |
| `doc/` | Design notes, source manifests, board procedures, and acceptance records |

## Current support

| MCU / board | Current scope |
| --- | --- |
| Renesas RA4M1 / Arduino UNO R4 Minima | Accepted bare-metal startup, GPIO, basic PWM, UART, I2C, SPI, ADC, DMAC, and W5100 Ethernet through MCUIP |
| STM32H753 / NUCLEO-H753ZI | Source-backed description and bare-metal clock, memory/cache, GPIO, timer, USART, I2C, SPI, ADC, DMA, and Ethernet implementation; native USB/Ethernet completion is tracked separately |
| ESP32-P4 / Waveshare ESP32-P4-NANO | Production-v3.x description with GPIO, UART, I2C, I2S, SDMMC/SDIO, SPI, and Ethernet work; the connected v1.0 board provides development evidence rather than production-silicon acceptance |

Support is intentionally evidence-specific. A described peripheral is not
automatically claimed as physically verified, and a successful cross-build is
not treated as an electrical test. See [requirements.md](requirements.md),
[implementation-plan.md](implementation-plan.md), and the
[documentation index](doc/README.md) for the exact status and acceptance
boundaries.

## Prerequisites

For an ordinary application, a C++23 compiler and the public headers suffice.
CMake consumers additionally need CMake 3.25 or newer. Library consumption
requires no sibling projects or board selection. Complete
board applications use CMake, Ninja and the selected cross compiler; flashing
tools are required only when flashing. See the
[first-build guide](doc/getting-started/building.md) for pinned setup, SDK and
host qualification details.

For maintainer verification:

- CMake 3.25 or newer;
- Ninja;
- a C++23 compiler;
- a checkout of `mcutest` for the host test suite.

Cross-builds additionally require:

- `arm-none-eabi-g++`, binutils, and size tools for Cortex-M4/M7;
- an activated ESP-IDF toolchain containing `riscv32-esp-elf-g++` for ESP32-P4;
- the board-specific debugger and command-line tools described in the board
  documentation.

Explicit maintainer workflows use these configurable sibling locations by
default; ordinary applications do not resolve them:

```text
workspace/
├── mcuhw/
├── mcutest/    # host and target test framework
├── mcujson/    # target evidence serialization
├── mcucov/     # target coverage instrumentation and reports
└── mcuip/      # allocation-free IP stack used by Ethernet integrations
```

Every path is configurable:

| CMake variable | Default |
| --- | --- |
| `MCUHW_MCUTEST_SOURCE_DIR` | `../mcutest` |
| `MCUHW_MCUJSON_SOURCE_DIR` | `../mcujson` |
| `MCUHW_MCUCOV_SOURCE_DIR` | `../mcucov` |
| `MCUHW_MCUIP_SOURCE_DIR` | `../mcuip` |

`mcutest` is required for native tests. `mcujson` and an installed `mcucov`
package are required by the instrumented board-test images. `mcuip` is optional
for description/driver-only builds but required for the Ethernet contract and
hardware tests.

## Quick start

Create a complete external application:

```sh
python3 tool/mcuhw.py new ../my-blink --board arduino/uno_r4_minima --example blink
python3 tool/mcuhw.py setup --project ../my-blink
python3 tool/mcuhw.py build --project ../my-blink
```

Setup imports the pinned installed tools (or an explicitly configured archive
cache). It reports missing prerequisites before building. The
[first-build guide](doc/getting-started/building.md) includes installation,
offline use, direct CMake commands and separately requested flashing. Complete
[Blink](example/app/blink/README.md) and
[TMP102](example/app/tmp102/README.md) projects currently target the Arduino
UNO R4 Minima.
See [attached devices](doc/getting-started/components.md) for chip support.

For application code, start with the [modern C++ I/O guide](doc/getting-started/io.md).
The optional `mcuhw::io`
CMake target provides `hw.output()`, `hw.serial`, `hw.i2c`, `hw.spi`,
`hw.analog`, `hw.pwm`, and explicit DMA leases. Storage is selected at compile
time; execution and MMIO can be supplied by the application. This new layer
has host and cross-build verification; its on-device acceptance is separate
from the existing driver evidence below.

The [components guide](doc/getting-started/components.md) shows how attached
chips share buses through `hw.i2c.device(address)` and
`hw.spi.device(configuration)`. TMP102 and W5100 drivers live under
`mcuhw::device`; the W5100 exposes raw Ethernet through the generic MCUIP
integration. Link `mcuhw::device` for device headers and `mcuhw::io` for the
board facade.

For focused native verification, with `mcutest` in its default sibling location:

```sh
python3 tool/verify.py --build build/verification --suites all
python3 tool/verify.py --build build/tmp102-tests --suites device:ti/tmp102
```

Enable architecture probes, independent header checks and optional MCUIP
contracts explicitly, using the configured development checkouts:

```sh
cmake --preset native -B build/host -DMCUHW_BUILD_TESTS=ON -DMCUHW_BUILD_TOOLING_TESTS=ON -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON -DMCUHW_BUILD_MCUIP_CONTRACTS=ON
python3 tool/verify.py --build build/host --existing
```

The native suite exercises metadata, solvers, ownership, register ordering,
drivers, error paths, cancellation, and fake-MMIO behavior without accessing
real MCU addresses. It also builds negative cases that must fail to compile.

## Installation and use from another project

`mcuhw` is a header-only CMake interface library. Consume a source checkout,
install a relocatable CMake package, or vendor the public headers.

### Use the CMake target

Add the checkout to the firmware project and link `mcuhw::mcuhw`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_firmware LANGUAGES CXX ASM)

# Defaults expose library targets without enabling mcuhw's maintainer checks.
add_subdirectory(external/mcuhw EXCLUDE_FROM_ALL)

add_executable(my_firmware
    startup.S
    main.cpp
)
target_link_libraries(my_firmware PRIVATE mcuhw::mcuhw)
target_compile_features(my_firmware PRIVATE cxx_std_23)
target_compile_options(my_firmware PRIVATE
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
    -fno-use-cxa-atexit
)
```

Configure that application with its MCU toolchain:

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/my-mcu-toolchain.cmake
cmake --build build
```

To install a package, run `cmake -S . -B build/package`, then
`cmake --install build/package --prefix /path/to/prefix`. In the consumer,
replace `add_subdirectory` with `find_package(mcuhw 0.1 CONFIG REQUIRED)` and
configure with `-DCMAKE_PREFIX_PATH=/path/to/prefix`. The same three targets
are available: `mcuhw::mcuhw`, `mcuhw::io` and `mcuhw::device`. See
[installation](doc/getting-started/installation.md) for package layout and
[firmware integration](doc/getting-started/using-mcuhw.md) for startup ownership.

### Use the headers directly

If the application does not use CMake, add this repository's `include/`
directory to the compiler include path and compile as C++23. The application
still owns startup, the linker script, interrupt dispatch, clocks, and the
execution hooks required by the selected backend.

## Basic usage

A board header supplies connector aliases, an MCU backend performs native
register access, and a functional driver presents the application API. This
minimal RA4M1 example configures the UNO R4 Minima built-in LED after startup
has established the board clock and memory environment:

```cpp
#include <mcuhw/backend/ra4m1/gpio.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/gpio.hpp>

namespace board = mcuhw::board::arduino_uno_r4_minima;

mcuhw::core::direct_mmio mmio;
mcuhw::backend::ra4m1::gpio_backend gpio_hardware{mmio};
mcuhw::driver::gpio_pin led{gpio_hardware, board::led_builtin_d13};

bool initialize_led() {
    using namespace mcuhw::driver;
    return led.initialize({
        .direction = gpio_direction::output,
        .initial_level = gpio_level::low,
        .output_type = gpio_output_type::push_pull,
        .pull = gpio_pull::none,
        .drive = gpio_drive::middle,
    }).has_value();
}

bool set_led(bool enabled) {
    const auto result = enabled ? led.set() : led.clear();
    return result.has_value();
}
```

The [examples guide](doc/example/README.md) provides reader-facing UART, I2C,
SPI, ADC, DMA, PWM, and W5100 Ethernet examples with the code, wiring,
lifecycle rules, and troubleshooting hints together on each page.

For interrupt-driven peripherals, first read the
[bare-metal application integration guide](doc/concept/application-integration.md).
It provides the application-level context: a concrete execution policy, static
backend lifetime, `main()`, deadline creation, interrupt handlers, buffer
ownership, and safe shutdown order.

Operations that may outlive a call return a fixed-storage transfer handle. The
caller retains the handle until it consumes the result, cancels it, or waits to
a deadline. Destroying an active handle is a contract violation rather than an
implicit cancellation.

## Build presets

The `arm-cortex-m4` preset selects the toolchain and the Arduino UNO R4 Minima
board by default. Other presets select only their toolchain/architecture and
global compiler settings. Presets do not select applications, tests or build
targets.

| Preset | Architecture |
| --- | --- |
| `native` | Host compiler |
| `arm-cortex-m4` | ARM Cortex-M4 |
| `arm-cortex-m7` | ARM Cortex-M7 |
| `esp32p4-riscv` | ESP32-P4 RISC-V |

The UNO board targets are available after configuring the preset:

```sh
cmake --preset arm-cortex-m4
cmake --build --preset arm-cortex-m4 --target firmware
cmake --build --preset arm-cortex-m4 --target examples
cmake --build --preset arm-cortex-m4 --target test.board
cmake --build --preset arm-cortex-m4 --target test.backend test.device
```

For the UNO R4 Minima, `firmware` builds Blink by default; set
`-DMCUHW_EXAMPLE=tmp102` to change the application. `examples` builds all
declared application and I/O examples,
with `example.app.blink` and `example.app.tmp102` available individually. UNO
also exposes `example.io.blink`, `example.io.serial_echo`, `example.io.spi_dma`,
`example.io.tmp102` and `example.io.w5100`. Other boards currently expose no
application examples. `test.board` builds the board's physical-test firmware in a separate child build.
`test.backend`, `test.device`, `test.core`, `test.driver`, `test.io`, `test.mcu`
and `test.compile` build and run focused host suites; `tests` selects all groups. Running or
flashing these images remains an explicit hardware procedure.

`MCUHW_BOARD`, `MCUHW_BOARD_REVISION` and `MCUHW_EXAMPLE` are visible in the CMake
cache and editable through CMake Tools. Reconfigure after editing them. The
configure log displays the resolved board, revision and architecture. Reapplying
the Cortex-M4 preset selects the UNO board again; the other presets preserve an
explicit board selection. An empty board selection exposes no board targets.
Unselected board entries are not loaded.

Firmware is written under `build/<preset>/application/`, additional examples
under `build/<preset>/example/`, and tests under
`build/<preset>/board/<vendor>/<board>/<revision>/test/`. Test dependencies
are resolved only when their test group is built. Global compiler flags are forwarded
to the child test configuration. See [focused targets and presets](doc/development/verification.md#targets-and-presets).

Enable verification independently of the architecture preset:

```sh
cmake --preset native -DMCUHW_BUILD_TESTS=ON -DMCUHW_TEST_SUITES=device:ti/tmp102
cmake --build --preset native --target native-tests
ctest --test-dir build/native --output-on-failure

cmake --preset arm-cortex-m4 -B build/arm-checks -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON
cmake --build build/arm-checks --target check-headers check-compile -j4
```

The same explicit verification options work with `arm-cortex-m7` and
`esp32p4-riscv`. `check-headers` compiles each public header independently.

## Running on supported boards

Board builds create firmware but do not automatically flash hardware. Use the
matching runner and follow its wiring, debugger, and recovery notes.

### Arduino UNO R4 Minima

```sh
cmake --preset arm-cortex-m4 -B build/uno-r4-minima -DMCUHW_BOARD_TESTS=arduino/uno_r4_minima
cmake --build build/uno-r4-minima --target board-tests -j4
./tool/run-uno-r4-minima-tests.sh startup-gpio
```

Additional runner selections cover GPIO loopback, PWM, UART, I2C, SPI, ADC,
DMAC, W5100 Ethernet, metrics, and link recovery. See the
[UNO R4 Minima guide](board/arduino/uno_r4_minima/doc/README.md),
[RA4M1 acceptance record](doc/acceptance/ra4m1-release.md), and
[W5100 Ethernet guide](board/arduino/uno_r4_minima/doc/ethernet-w5100.md).

### NUCLEO-H753ZI

```sh
cmake --preset arm-cortex-m7 -B build/nucleo-h753zi -DMCUHW_BOARD_TESTS=st/nucleo_h753zi
cmake --build build/nucleo-h753zi --target board-tests -j4
./tool/run-nucleo-h753zi-smoke.sh
./tool/run-nucleo-h753zi-native.sh
```

See the [NUCLEO-H753ZI guide](board/st/nucleo_h753zi/doc/README.md) for
ST-LINK, VCP, Ethernet jumper, and physical-evidence details.

### ESP32-P4-NANO

The generic compile probe uses `esp32p4-riscv` with `MCUHW_BUILD_PROBES=ON`. The physical board image is a
separate ESP-IDF build:

```sh
idf.py -C board/waveshare/esp32p4_nano/test/firmware set-target esp32p4
idf.py -C board/waveshare/esp32p4_nano/test/firmware build
./tool/run-esp32p4-nano-smoke.sh /dev/cu.usbmodemXXXXXXXXXXXX
```

This runner replaces selected flash regions. Read the recovery and silicon-
revision notes in
[ESP32-P4-NANO guide](board/waveshare/esp32p4_nano/doc/README.md) first.

## Verification philosophy

The project keeps different claims separate:

- native tests verify logic, bounds, register access order, and failure paths;
- compile-fail tests verify that invalid configurations are rejected;
- cross-builds verify freestanding compilation and architecture instructions;
- linked board images verify startup, memory, interrupt, and linker integration;
- physical procedures verify only the signals and devices actually observed.

Tests and implementation files carry requirement tags consumed by `mcutrace`.
Generated `mcutest` and `mcucov` artifacts are imported through
[`mcutrace.toml`](mcutrace.toml). Plan status is not used as a substitute for
test evidence.

## Extending mcuhw

### Add a board for an existing MCU

Use this route when the MCU description and required peripheral backends
already exist.

1. **Pin the board sources.** Record the board revision, schematic, connector
   pinout, oscillator, memory, debugger, bootloader, jumper, and solder-bridge
   assumptions. Do not copy pin mappings from a similar board without checking
   its exact assembly.
2. **Add the board binding.** Create
   `include/mcuhw/board/<board_name>.hpp`. Select the exact MCU/package and add
   only board-level identities and connector aliases. Keep MCU register facts
   in `mcu/` and peripheral sequences in `backend/`.
3. **Add the physical composition.** Create `board/<vendor>/<name>/` with the
   startup code, linker script, restricted runtime, execution hooks, interrupt
   dispatch, and focused test images required by the board.
4. **Bind existing backends.** Form compile-time bindings from the selected
   peripheral instances and board pins. Invalid routes must fail at compile
   time rather than silently selecting another pin or mode.
5. **Add a local build entry.** Configure the board's `board.cmake` with its
   revision, architecture, profile, toolchain, examples and runner settings.
   The common selection interface loads it from the board directory.
6. **Verify in layers.** Compile the board binding on the host where possible,
   cross-build every public header and the linked image, then run board-native
   and directly connected tests. Document what was not electrically observed.
7. **Update traceability.** Add or update requirements and a plan item before
   claiming new support. Tag implementation and tests with the applicable
   requirement IDs, register evidence artifacts in `mcutrace.toml`, and update
   the board's evidence references and documentation.

Follow the [board](doc/development/adding-a-board.md),
[external-device](doc/development/adding-a-device.md), and
[MCU/backend](doc/development/adding-an-mcu.md) guides for complete local workflows.

### Add a new microcontroller

A new MCU requires a description before it receives functional backends.

1. **Define the supported identity.** Select the exact family, part/package
   variants, silicon revisions, and authoritative manual/datasheet/errata
   revisions. Add them to the source manifest and record unresolved conflicts.
2. **Create the description.** Add `include/mcuhw/mcu/<mcu>/` following the
   existing split: `model.hpp`, `system.hpp`, `peripherals.hpp`, `pins.hpp`,
   `interrupts_dma.hpp`, `coverage.hpp`, and `description.hpp`. Describe native
   capabilities; do not force them into another vendor's register model.
3. **Pass the description gate.** Add the MCU to the shared metadata tests,
   header self-containment checks, source/coverage inventory, positive route
   tests, and compile-fail cases. Every published block is described, explicitly
   deferred, unavailable in the selected sources, or not applicable.
4. **Add the architecture/toolchain path.** Supply or reuse barrier support,
   create a CMake toolchain and preset, apply the restricted-runtime flags, and
   verify generated barrier instructions and public-header compilation.
5. **Implement backends incrementally.** Add
   `include/mcuhw/backend/<mcu>/` one peripheral at a time. Reuse functional
   drivers, typed errors, fixed storage, ownership, deadlines, and transfer
   contracts. Test each backend first with recording/fake MMIO, including
   cleanup and failure paths.
6. **Add a real board.** A description alone cannot establish runtime support.
   Add at least one exact board binding, startup/linker integration, execution
   hooks, interrupt routing, and physical procedure as described above.
7. **Qualify the claim.** Run native tests, negative compile tests, the new
   architecture preset, linked firmware, and scoped physical tests. Record
   toolchain and source identities, image sizes, fixed resource bounds, results,
   and explicit non-claims before marking the MCU or board milestone complete.

When a new board or MCU reveals a genuine capability difference, extend the
metadata or functional contract explicitly and re-run the existing targets.
Do not hide the difference in a board macro, duplicate a driver, or weaken an
existing compile-time check.
