# Repository ownership and migration

Board-private implementation lives under `board/<vendor>/<name>/`. Public include
paths and board IDs retain their existing names. A board directory contains:

```text
component.json       identity, dependencies, profile, tools and runner
board.cmake          board startup, linker and clock bindings
bsp/                 startup sources, linker layout and SDK boot components
tests.cmake          optional instrumented firmware targets
tests/               physical fixture sources and execution scripts
docs/                wiring, procedures and historical observations
evidence/            captured board observations, where available
```

The ESP-IDF bootloader and application startup belong in the board BSP. The
instrumented ESP test project is `tests/firmware/`; applications and fixtures use
the shared bootloader. Old `doc/boards/` pages forward to the component-owned
documents. The existing `tools/run-*.sh` commands remain compatibility entry points
to board-owned fixture scripts. Generated build directories are not migrated:
configure a fresh directory because CMake caches contain absolute source paths.

`cmake/McuhwBoard.cmake` owns the common firmware compile/link/image recipe.
Applications, legacy IO examples and instrumented ARM fixtures use that recipe.
Board metadata chooses the public profile; `cmake/architectures/` owns compiler
and ABI facts shared by supplied toolchains and existing external BSPs; board
entries own startup and memory layout. A fixture layers coverage and
reporting onto the common recipe without copying compiler and linker policy.

Backend headers own register encodings and controller state machines. Board
profiles choose routes and supply wiring bindings. Shared chip protocols stay in
`device/` ownership even when their first fixture is a permanently mounted chip.
RA SCI is the first register authority conversion: its audited catalogue and
backend accesses derive from the same typed register definitions. Other peripheral
catalogues retain their existing audited definitions and can migrate individually.

Native tests have one owner. RA-specific cases are under `tests/backend/ra4m1/`
and registered by the RA IP manifest. Codec-only cases are in
`tests/device/test_es8311.cpp`, separate from ESP I2S controller tests. Shared
transport contracts belong to the `drivers` or `io` suites. Catalog validation
and CMake reject duplicate test ownership; changing suite selection cannot silently
move a shared source into a different executable.

## Component schema 1 to 2

All component manifests now use `schema_version: 2`. This version applies to
`component.json`; project configuration, tool locks and result reports keep their
own independent version numbers.

To migrate a local component:

1. Set its component schema version to 2. Remove the ambiguous `build.profile`.
2. For a board, set `build.profile_header` to the repository-relative public
   profile header and `build.profile_type` to its qualified C++ type. Declare
   `build.architecture`, `build.entry` and `build.toolchain` explicitly. Remove
   duplicate profile and compiler/ABI declarations from `board.cmake`.
3. Keep runner metadata only if supported. Adapters are `jlink`, `openocd` and
   `esp-idf`. Each declares its tool, nonoverlapping flash regions, recovery
   document and adapter fields. Unknown keys and incomplete configurations fail
   validation. A board without a flashing adapter may omit `runner`.
4. Update board-private source/document paths and ensure each test file has one
   owning component. Run `python3 tools/catalog.py validate` and the board's
   declared cross-builds before accepting the migration.

Historical observation hashes are preserved. Moving or editing an observation
does not rerun hardware or upgrade its evidence; a changed report remains `stale`
until reviewed against the original record.

## C++ source migration

Generic timing requests and reports remain under `mcuhw::driver`. Register-bearing
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
using mac = mcuhw::backend::stm32h753::ethernet_rmii_mac<
    mmio_type, mcuhw::board::nucleo_h753zi::ethernet_binding>;
```

ESP Ethernet similarly uses `mcuhw::board::esp32p4_nano::ethernet_binding`.
ESP SDMMC requires a third template argument, such as
`mcuhw::board::esp32p4_nano::sdmmc_slot0_binding` or
`sdmmc_slot1_wifi_binding`. Another board supplies its own binding; controller
code no longer selects the development board's pins implicitly.

The DMA fixes also change internal storage: STM Ethernet descriptors occupy a
32-byte cache line each, with matching hardware stride. ESP SDMMC uses one
static 512-byte staging sector plus its 64-byte descriptor; DMA never addresses
caller buffers. A failed DMA reset keeps the controller owned until shutdown
confirms quiescence, preventing reuse of active DMA storage.

## One selected build for CLI and IDE

Application setup writes the standard `mcuhw` configure/build preset and includes
it from `CMakeUserPresets.json`, preserving user presets. The frontend invokes
those same presets. Successful firmware targets write per-target `*.build.json`
records with the board, revision, startup selection and ELF digest. Flashing reads
the selected completed build record instead of reconstructing a directory from
tool versions. See [the application guide](../getting-started/building.md).

Changed-component CI selection also creates board/revision/example build actions
from each affected board's manifest. Provisioned ARM and ESP-IDF jobs compile and
link those applications; missing tools fail the job. These are software checks
and do not execute physical fixtures. See [verification](verification.md).
