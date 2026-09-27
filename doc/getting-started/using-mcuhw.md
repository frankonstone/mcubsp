# Using mcuhw in firmware

Add the repository as a subdirectory and link its interface target:

```cmake
add_subdirectory(external/mcuhw EXCLUDE_FROM_ALL)
target_link_libraries(my_firmware PRIVATE mcuhw::mcuhw)
```

Ordinary source inclusion creates only the library interfaces. It does not
change the parent application's `BUILD_TESTING`, global compiler policy or
test selection, and it does not require adjacent development projects.

An installed package exposes the same targets:

```cmake
find_package(mcuhw 0.1 CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE mcuhw::io mcuhw::device)
```

Set `CMAKE_PREFIX_PATH` to the installation prefix. The package configuration
lives under `lib/cmake/mcuhw`; it supplies headers and reusable board resources
from `share/mcuhw` through imported targets and `MCUHW_ROOT`. Direct header use
remains available with the checkout's `include` directory or the installed
`share/mcuhw/include` directory on the include path. See
[installation](installation.md) for the complete layouts and commands.

An existing firmware owns its toolchain, startup, linker script, clocks,
interrupt dispatch and execution hooks. A new application can instead use the
[selected-board workflow](building.md) with an explicit `supplied` startup
choice. Applications integrating an existing BSP/RTOS choose `external`
startup so the board helper adds no duplicate vectors or reset owner.

At the source level, select:

1. a board or package pin identity from `mcuhw/board` or `mcuhw/core`;
2. a backend from `mcuhw/backend/<mcu>`;
3. a functional driver from `mcuhw/driver`;
4. an MMIO and execution policy appropriate for the target.

Configuration solvers and route bindings should be evaluated at compile time.
Runtime operations return `std::expected` or a fixed-storage transfer handle;
the caller must consume, cancel, or wait for every started transfer.

Before using an interrupt-driven backend, read
[Integrating mcuhw into a bare-metal application](../concepts/application-integration.md).
It shows the relationship between startup, `main()`, the execution policy,
global backend lifetime, IRQ dispatch, deadlines, and transfer consumption.

Then continue with the [examples index](../examples/README.md). The consumer
configuration and GPIO example are also shown in the repository
[README](../../README.md#installation-and-use-from-another-project).
