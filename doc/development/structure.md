# Repository ownership

Board-specific startup, linker settings, toolchain selection, C++ profile,
examples and runner settings live in `board/<vendor>/<name>/board.cmake`.
Public headers keep their existing paths. A board directory also contains BSP
sources, optional `tests.cmake` or SDK test project, and wiring and recovery
procedures. Native test sources live under `test/` by MCU, backend or device.

`board/Support.cmake` owns the shared firmware recipe. `board/architecture/` owns
compiler and ABI settings. The board entry owns startup and memory layout.
Device protocols stay reusable across boards and take explicit transport handles.

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

Full CI verification creates board/revision/example build actions
from local board entries. Provisioned ARM and ESP-IDF jobs compile and
link those applications; missing tools fail the job. These are software checks
and do not execute physical fixtures. See [verification](verification.md).
