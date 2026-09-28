# Test groups

Tests are grouped by the layer they exercise. Each group registers its source
directory through its own `CMakeLists.txt`; shared native registration helpers
live in `support/NativeSuites.cmake`. Suite names remain stable when sources move.

| Directory | Scope | Selectable board-menu target |
| --- | --- | --- |
| `core/` | Metadata, ownership, execution, registers and transfer contracts | `test.core` |
| `driver/` | Shared driver and timing contracts | `test.driver` |
| `backend/<mcu>/` | Native peripheral implementations for each MCU | `test.backend` |
| `device/` | Attached-device protocols and transport integration | `test.device` |
| `io/` | Board facade, peripherals and bus endpoints | `test.io` |
| `mcu/<mcu>/`, `mcu/` | MCU descriptions and shared description contracts | `test.mcu` |
| `compile/` | Positive/negative compile cases and architecture probes | `test.compile` |
| `support/` | Shared runner, fixtures and source-backed examples | Used by the suites |
| `tooling/` | Packaging, workflow and verification tooling | `MCUHW_BUILD_TOOLING_TESTS=ON` |
| `io_cost/` | Paired driver/facade resource probes | `mcuhw_io_cost_report` |

With `MCUHW_BOARD` selected, the `test.*` native groups build and run in
separate host configurations. `test.board` builds physical-test firmware from
`board/<vendor>/<board>/tests/`. The `tests` aggregate selects all groups.
Presets select architecture; changing `MCUHW_BOARD` and reconfiguring updates
the board menu.

Native CMake targets use `Tests/<group>` folders. CTest labels include `core`,
`driver`, `backend`, `device`, `io`, `mcu` and `compile`, alongside the original
suite identities. After a native configuration, run a category with:

```sh
ctest --test-dir build/native -L backend --output-on-failure
```

For native suite selection, use `MCUHW_TEST_SUITES=core`, `drivers`, `io`,
`backend`, `device`, `mcu`, `descriptions`, `compile`, `all`, or an exact
component ID such as `device:ti/tmp102`. See
[verification workflows](../doc/development/verification.md).
