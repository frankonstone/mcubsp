# Using mcuhw in firmware

Add the repository as a subdirectory and link its interface target:

```cmake
add_subdirectory(external/mcuhw EXCLUDE_FROM_ALL)
target_link_libraries(my_firmware PRIVATE mcuhw::mcuhw)
```

Configure a consumer without the mcuhw test dependencies by passing
`-DBUILD_TESTING=OFF`. The firmware remains responsible for its toolchain,
startup, linker script, clocks, interrupt dispatch, and execution hooks.

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
