# mcubsp

`mcubsp` is a C++23 hardware-description and bare-metal driver library for
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

## Start from your application

If you already have a BSP/RTOS project, link `mcubsp::mcubsp`, `mcubsp::io` or
`mcubsp::device` into your executable. You keep your toolchain and startup.
If you are starting a supported-board application, use the public board
toolchain and `mcubsp_add_firmware` to prepare its integration explicitly.

Start with [the application guide](doc/getting-started/application.md) and the
[copyable standalone Blink project](example/blink/README.md). They distinguish
your application's build, tests and release from maintaining mcubsp itself.

## Architecture

The library separates hardware facts from behavior and board wiring:

```text
Application
    |
Device driver (optional) TMP102 temperature, W5100 Ethernet, ...
    |
mcubsp::io (optional)    board objects, named pins, selected storage and leases
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
| `include/mcubsp/core/` | Register metadata, MMIO contracts, ownership, units, and fixed transfer state |
| `include/mcubsp/mcu/` | Declarative, source-backed MCU descriptions |
| `include/mcubsp/backend/` | MCU-specific peripheral sequencing |
| `include/mcubsp/driver/` | Target-independent functional drivers and external-device drivers |
| `include/mcubsp/io/` | Generic application interfaces, peripheral handles and board container |
| `include/mcubsp/device/` | Attached-chip protocols using I2C/SPI device handles or supplied transports |
| `include/mcubsp/board/<vendor>/` | Named board types, lightweight pins and hardware bindings |
| `include/mcubsp/execution/` | Bare-metal and test execution policies |
| `src/` | Mirrors public-header modules; owns implementation resources and module-local `test/` folders |
| `src/board/<vendor>/<name>/` | Board startup, build rules, physical tests and procedures |
| `test/` | Project-wide workflows, compiler orchestration, shared runners and reporting |
| `doc/` | Design notes, source manifests, board procedures, and acceptance records |

## Current support

| MCU / board | Current scope |
| --- | --- |
| Renesas RA4M1 / Arduino UNO R4 Minima | Accepted bare-metal startup, GPIO, basic PWM, UART, I2C, SPI, ADC, DMAC, and W5100 Ethernet through MCUIP |
| STM32H753 / NUCLEO-H753ZI | Source-backed description and bare-metal clock, memory/cache, GPIO, timer, USART, I2C, SPI, ADC, DMA, and Ethernet implementation; native USB/Ethernet completion is tracked separately |
| ESP32-P4 / Waveshare ESP32-P4-NANO | Production-v3.x description with GPIO, UART, I2C, I2S, SDMMC/SDIO, SPI, and Ethernet work; the connected v1.0 board provides development evidence rather than production-silicon acceptance |

The [support matrix](doc/reference/support-matrix.md) separates source review,
typed metadata, driver implementation and physical verification, including
current gaps and required hardware refresh.

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
board applications use CMake, Ninja, Python 3.10+ and the selected cross compiler; flashing
tools are required only when flashing. See the
[first-build guide](doc/getting-started/building.md) for pinned setup, SDK and
host qualification details.

Board presets require Ninja and the matching cross compiler:

- `arm-none-eabi-g++` for the Arduino UNO R4 Minima and NUCLEO-H753ZI;
- an activated ESP-IDF toolchain containing `riscv32-esp-elf-g++` for ESP32-P4.

Board applications need no test dependencies. Optional UNO and NUCLEO test
images require `mcutest`, `mcucov` and `mcujson`;
see the [hardware build and capture guide](doc/getting-started/hardware.md).

## Try the repository's Blink

Configure the repository for a supported board:

```sh
cmake --preset arduino-uno-r4-minima
cmake --build --preset arduino-uno-r4-minima --target firmware
```

Each board has a `firmware` target that links Blink with supplied startup.
All example applications live under [example/](example/README.md), grouped by
topic. Build the `examples` target for the selected board's demos or choose a
named target. Physical-test images are opt-in. For one image, build its named target, for example
`arduino.uno_r4_minima.example.blink` or
`arduino.uno_r4_minima.test.startup_gpio`. Flashing and running physical tests
remain separate hardware procedures.

For application code, start with the [modern C++ I/O guide](doc/getting-started/io.md).
The optional `mcubsp::io`
CMake target provides `hw.output()`, `hw.serial`, `hw.i2c`, `hw.spi`,
`hw.analog`, `hw.pwm`, and explicit DMA leases. Storage is selected at compile
time; execution and MMIO can be supplied by the application. This new layer
has host and cross-build verification; its on-device acceptance is separate
from the existing driver evidence below.

The [components guide](doc/getting-started/components.md) shows how attached
chips share buses through `hw.i2c.device(address)` and
`hw.spi.device(configuration)`. TMP102 and W5100 drivers live under
`mcubsp::device`. Link `mcubsp::device` for device headers and `mcubsp::io` for
the board facade. MCUIP adapters, contract checks and combined Ethernet test
applications live in the sibling [integration project](../integration/README.md).
That project depends on mcubsp and MCUIP; mcubsp builds independently of it.

## Installation and use from another project

`mcubsp` is a header-only CMake interface library. Consume a source checkout,
install a relocatable CMake package, or vendor the public headers.

### Use the CMake target

Add the checkout to the firmware project and link `mcubsp::mcubsp`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_firmware LANGUAGES CXX ASM)

add_subdirectory(external/mcubsp EXCLUDE_FROM_ALL)

add_executable(my_firmware
    startup.S
    main.cpp
)
target_link_libraries(my_firmware PRIVATE mcubsp::mcubsp)
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

To build, test and install on Apple Silicon macOS, run
`cmake --preset macos-arm64`, then `cmake --build --preset macos-arm64` and
`cmake --install output/build/macos-arm64`. The package goes to `output/install`.
On Linux x86_64, use `linux-x86_64` in the commands and build path instead.
For installation without test dependencies, configure with
`-DMCUBSP_BUILD_HOST_TESTS=OFF -DMCUBSP_CHECK_HEADERS=OFF`.
In the consumer,
replace `add_subdirectory` with `find_package(mcubsp 0.1 CONFIG REQUIRED)` and
configure with `-DCMAKE_PREFIX_PATH=/absolute/path/to/mcubsp/output/install`.
An explicit install prefix remains supported. The same three targets
are available: `mcubsp::mcubsp`, `mcubsp::io` and `mcubsp::device`. See
[installation](doc/getting-started/installation.md) for package layout and
[firmware integration](doc/getting-started/using-mcubsp.md) for startup ownership.

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
#include <mcubsp/backend/ra4m1/gpio.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/gpio.hpp>

using board = mcubsp::board::arduino::uno_r4_minima_bindings;

mcubsp::core::direct_mmio mmio;
mcubsp::backend::ra4m1::gpio_backend gpio_hardware{mmio};
mcubsp::driver::gpio_pin led{gpio_hardware, board::led_builtin_d13};

bool initialize_led() {
    using namespace mcubsp::driver;
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

The [examples guide](src/board/arduino/uno_r4_minima/doc/example/README.md) provides reader-facing UART, I2C,
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

## Framework maintainer presets

The `macos-arm64` and `linux-x86_64` presets build and run native suites,
compiler contracts and isolated-header checks. Native verification needs Python
3.10+ and mcutest; cross compilers are optional. MCUIP compositions belong to
the sibling integration project. Board presets select
the matching cross compiler. `MCUBSP_BUILD_BOARD_TESTS=ON` opts into UNO physical
test images and `MCUBSP_COVERAGE=ON` instruments them. Exact board revisions
are selected with `MCUBSP_BOARD_REVISION`.

| Preset | Purpose / board | Toolchain |
| --- | --- | --- |
| `macos-arm64` | Apple Silicon host tests and installation | macOS arm64 Clang |
| `linux-x86_64` | Linux host tests and installation | Linux x86_64 GCC or Clang |
| `arduino-uno-r4-minima` | Arduino UNO R4 Minima | ARM Cortex-M4 |
| `st-nucleo-h753zi` | NUCLEO-H753ZI | ARM Cortex-M7 |
| `waveshare-esp32p4-nano` | Waveshare ESP32-P4-NANO | ESP32-P4 RISC-V |

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64

cmake --preset st-nucleo-h753zi
cmake --build --preset st-nucleo-h753zi
```

The ESP32-P4 physical test image remains an independent ESP-IDF project under
`src/board/waveshare/esp32p4_nano/test/firmware/`. Building or flashing any board
image requires the matching board procedure.

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

## Extending mcubsp

### Add a board for an existing MCU

Use this route when the MCU description and required peripheral backends
already exist.

1. **Pin the board sources.** Record the board revision, schematic, connector
   pinout, oscillator, memory, debugger, bootloader, jumper, and solder-bridge
   assumptions. Do not copy pin mappings from a similar board without checking
   its exact assembly.
2. **Add the board binding.** Create
   `include/mcubsp/board/<vendor>/<board_name>.hpp`. Select the exact MCU/package and add
   only board-level identities and connector aliases. Keep MCU register facts
   in `mcu/` and peripheral sequences in `backend/`.
3. **Add the physical composition.** Create `src/board/<vendor>/<name>/` with the
   startup code, linker script, restricted runtime, execution hooks, interrupt
   dispatch, and focused test images required by the board.
4. **Bind existing backends.** Form compile-time bindings from the selected
   peripheral instances and board pins. Invalid routes must fail at compile
   time rather than silently selecting another pin or mode.
5. **Add a local build entry.** Configure the board's `board.cmake` with its
   revision, architecture, profile, toolchain and runner settings.
   The common selection interface loads it from the board directory. Add demos
   under `example/<topic>/<vendor>/<name>.cpp`.
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
2. **Create the description.** Add `include/mcubsp/mcu/<mcu>/` following the
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
   `include/mcubsp/backend/<mcu>/` one peripheral at a time. Reuse functional
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
