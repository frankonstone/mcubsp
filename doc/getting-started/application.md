# Use mcubsp in your application

Choose the integration boundary first. Your application owns its sources,
dependencies, tests and release process. mcubsp supplies headers and optional
board integration. You do not need to build or release the mcubsp repository
each time you build your firmware.

| Your project | You provide | mcubsp provides |
| --- | --- | --- |
| Existing BSP, vendor project or RTOS | Toolchain, executable, startup, linker layout, clocks and interrupt routing | Header-only library targets to link into that executable |
| New application on a supported board | Application sources naming the C++ board type, selected board/revision, explicit startup choice and installed tools | Board-selected toolchain entry point, supplied startup/linker settings, board headers and completed firmware records |

## Existing firmware: link a library target

Keep your current toolchain and executable. Add a checkout:

```cmake
add_subdirectory(external/mcubsp EXCLUDE_FROM_ALL)
target_link_libraries(my_firmware PRIVATE mcubsp::io mcubsp::device)
```

Or use a package installed once by you or your organization:

```cmake
find_package(mcubsp 0.1 CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE mcubsp::io mcubsp::device)
```

`mcubsp::mcubsp` is the core interface, `mcubsp::io` adds the application facade,
and `mcubsp::device` exposes attached-device drivers. All are interface targets;
they supply includes and C++23 requirements. Configure the compiler options and
runtime policy appropriate for your BSP. Library inclusion adds no firmware,
test targets, startup, compiler flags or ASM language, even if the parent has
an `MCUBSP_BOARD` setting. Direct header use remains supported.

This route needs a C++23 compiler with matching library headers and, when using
CMake, CMake 3.25+. It does not require Python, Ninja, Git, a debugger, mcutest,
mcucov, mcutrace or a sibling project just to consume the library. Your existing
build may independently require some of those tools.

## New board application: request firmware explicitly

Start with the complete [standalone Blink project](../../example/blink/README.md).
Its project file is ordinary CMake:

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_application LANGUAGES CXX ASM)
add_subdirectory(external/mcubsp EXCLUDE_FROM_ALL)
mcubsp_add_firmware(firmware STARTUP supplied SOURCES main.cpp)
```

For an installed dependency, replace `add_subdirectory` with
`find_package(mcubsp 0.1 CONFIG REQUIRED)`. Both expose the same public function;
no internal helper include is required.

Configure your application using the dependency's board toolchain:

```sh
cmake -S . -B output/build/uno -G Ninja -DCMAKE_TOOLCHAIN_FILE=external/mcubsp/cmake/board-toolchain.cmake -DMCUBSP_BOARD=arduino/uno_r4_minima
cmake --build output/build/uno --target firmware
```

The toolchain reads only the chosen board and selects its compiler **before**
`project()` checks compilers. `MCUBSP_BOARD_REVISION` defaults to that board's
supported revision. Use separate build directories for different boards.
See the [starter's board table](../../example/blink/README.md#choose-a-board).

`mcubsp_add_firmware` defaults `BOARD` and `REVISION` from that selection.
They may also be explicit function arguments when integrating compatible
targets with an existing toolchain. `STARTUP` remains mandatory:

- `supplied`: mcubsp supplies the board's startup and link integration.
- `external`: your BSP supplies startup, clocks, vectors and linker options.

Include the concrete board header and name its type explicitly in C++:

```cpp
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>

static mcubsp::io::board<mcubsp::board::arduino::uno_r4_minima> hw;
```

| CMake board identity | C++ board type |
| --- | --- |
| `arduino/uno_r4_minima` | `mcubsp::board::arduino::uno_r4_minima` |
| `st/nucleo_h753zi` | `mcubsp::board::st::nucleo_h753zi` |
| `waveshare/esp32p4_nano` | `mcubsp::board::waveshare::esp32p4_nano` |

CMake selects the compiler and startup; application code names the matching
board type. There is no generated board alias. The starter keeps each board's
code and LED pin in a source file named after that board.

Low-level users can include `<mcubsp/board/<vendor>/<board>/pins.hpp>` or
`bindings.hpp` and use the named `<board>_pins` or `<board>_bindings` type.
These headers expose wiring/connection facts without loading the I/O facade
or backend implementations. See the [layout and migration table](../development/structure.md).

For Arm, the result is a normal executable target that accepts your usual
`target_sources`, `target_link_libraries`, include directories and definitions.
For supplied P4 startup, it is an explicit SDK subproject target; put application
translation units in `SOURCES`. It does not currently expose the full native
ESP-IDF component-configuration API through the outer target. Applications
requiring that API should retain their ESP-IDF project and consume mcubsp's
headers there. P4's SDK/boot ownership remains explicit.

## Required tools versus optional tooling

| Task in your application | Required for that task |
| --- | --- |
| Use headers in an existing BSP | Your target's C++23 compiler; CMake only if your build uses it |
| Use mcubsp's board firmware helper | CMake 3.25+, Ninja and Python 3.10+ for the documented input-record workflow, matching cross compiler; ESP-IDF and its matching environment for P4 |
| Run your application's tests | Your chosen test dependencies; mcubsp does not prescribe a test framework |
| Flash an image | The selected board's debugger/serial tools and explicit probe/port, only when flashing |
| Release your firmware | Your project's release process; archive the complete image/bundle and adjacent output/build/input records when using the helper |
| Test or release mcubsp itself | Framework-maintainer tooling, covered by the separate [maintainer guide](build-and-test.md) |

`mcutest`, `mcujson`, `mcucov`, MCUIP and `mcutrace` are not application build
prerequisites unless you explicitly use their corresponding features. Firmware
input recording is automatic in the documented helper workflow; Python is used
for that build step, while mcutrace analysis is optional maintainer tooling.

Building creates firmware and its records. Flashing is a separate action.
Publishing a firmware release is also a separate action; CMake's `Release`
configuration is a compiler configuration, not a publishing command. Follow
[hardware planning](hardware.md) and [input-record archiving](../development/firmware-inputs.md)
when those tasks apply to your project.

## Moving from the earlier helper examples

Explicit `include("${MCUBSP_ROOT}/cmake/Firmware.cmake")` calls continue to work
but are no longer needed. The architecture-specific toolchain files remain
available; new projects can select the board through `board-toolchain.cmake`.
Source inclusion no longer creates the repository's default `firmware` target
from a parent's `MCUBSP_BOARD`; add your own `mcubsp_add_firmware` call explicitly.
Declare `ASM` in your application's `project(... LANGUAGES CXX ASM)` when using
supplied Arm startup. Loading the helpers alone no longer enables a language.
The short-lived `mcubsp::selected_board` alias and generated header have been
removed. Include the concrete board header and use the corresponding named type
from the table above.
