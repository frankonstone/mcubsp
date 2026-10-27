# Adding a board on an existing MCU

Application consumers use the public `cmake/board-toolchain.cmake` entry point
and `mcubsp_add_firmware`; see the [application guide](../getting-started/application.md).
Set `MCUBSP_BOARD_PROFILE_HEADER` to the public board header in the local entry.
Applications include that header and name the concrete C++ board type explicitly.
Board toolchain paths remain local metadata, resolved before the consuming
project's `project()`.

Create `src/board/<vendor>/<name>/board.cmake`, a source at
`example/blink/<vendor>/<name>.cpp`, and local wiring/recovery documentation.
Use an existing board entry for the architecture
as a starting point. Declare the exact supported revisions, architecture,
toolchain and public profile header. Supplied Arm startup declares
`MCUBSP_BOARD_STARTUP` and `MCUBSP_BOARD_LINKER`. SDK boards declare their boot
integration explicitly. Keep startup sources under `startup/`, Arm linker scripts
beside `board.cmake`, and physical fixtures
under `test/`.

Add pin aliases under `include/mcubsp/board/` and the application profile under
`include/mcubsp/board/<vendor>/<board>.hpp`, with lightweight `pins.hpp` and
`bindings.hpp` beside it under `<vendor>/<board>/`. MCU register facts stay in `mcu/`, sequencing in
`backend/`. Board profiles supply backend factories for the supported
`io::select` selectors; unsupported capabilities remain compile-time failures.

Select a matching toolchain and `MCUBSP_BOARD=vendor/name`. The common
`mcubsp_add_firmware` helper builds the selected source using board-local
startup and linker resources. Add a preset for convenience; no global board
registry or workspace catalog is required. Add example sources under
`example/<topic>/<vendor>/<name>.cpp`; the repository discovers matching sources
for the selected board. Board-local CMake files may add physical tests. Only
requested physical-test images may resolve test/coverage dependencies.

Follow the [application guide](../getting-started/building.md) and
[verification guide](verification.md). Check supported revisions, incompatible
toolchains, external startup ownership, isolated headers and linked images.
Record actual wiring/firmware results separately from compilation before
claiming hardware acceptance. Update the [support matrix](../reference/support-matrix.md).
