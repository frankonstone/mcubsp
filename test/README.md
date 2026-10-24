# Tests

The host presets build and run every host-executable suite in this directory.
Use `macos` on macOS or `linux-x86_64` on Linux x86_64:

```sh
cmake --preset macos
cmake --build --preset macos
```

The build compiles positive probes, checks expected compiler rejections, and
runs the native suites through CTest. It also cross-compiles Cortex-M4,
Cortex-M7, and ESP32-P4 probes and checks generated barrier instructions.
No check in `test/` executes on a target
or needs a connected board. Install the GNU Arm and ESP32-P4 RISC-V toolchains
before building a host preset.

The build compiles the MCUIP contracts;
set `MCUBSP_MCUIP_SOURCE_DIR` if the MCUIP checkout is not at `../mcuip`.
The native suites require mcutest at `../mcutest`, or a discoverable mcutest
CMake package. An explicit `MCUBSP_MCUTEST_SOURCE_DIR` selects another checkout.

| Directory | Host verification |
| --- | --- |
| `core/` | Metadata, ownership, execution, register, and transfer contracts |
| `driver/` | Shared driver and timing contracts |
| `backend/<mcu>/` | Peripheral behavior against fake or recorded MMIO |
| `device/` | Device protocols and transport integration |
| `io/` | Board facade, peripherals, and bus endpoints |
| `mcu/<mcu>/`, `mcu/` | MCU descriptions and shared contracts |
| [`compile/`](compile/README.md) | Positive and negative compiler cases and object probes |
| `support/` | Shared runners and fixtures used by the suites |

CTest labels support focused runs after building, for example:

```sh
ctest --test-dir build/macos -L backend --output-on-failure
```

Cross-compiler outputs are under `build/<preset>/test/compile/target-probes/`.
The barrier reports are inspected on the host; no image is run on a board.
