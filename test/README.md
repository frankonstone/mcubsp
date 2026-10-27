# Project-wide verification

Module tests live under `src/<module>/test/`, matching the directories beneath
`include/mcubsp/`. This root directory holds shared infrastructure and checks
that exercise the project as a whole.

| Directory | Responsibility |
| --- | --- |
| [compile/](compile/README.md) | Compiler policy, registration of module compile cases and cross-compiler probe orchestration |
| workflow/ | Build consumers, package relocation, diagnostics, selected-input provenance and runner/bundle planning |
| support/ | Shared native entry point, fixtures, reporting and coverage helpers |

Native suites and module-specific compiler checks are owned by:

| Source directory | Verification |
| --- | --- |
| `src/core/test/` | Metadata, ownership, registers and transfers |
| `src/execution/test/` | Execution policies and deadlines |
| `src/testing/test/` | Recording MMIO and the description-suite helper |
| `src/driver/test/` | Shared driver and timing contracts |
| `src/backend/<mcu>/test/`, `src/backend/test/` | Peripheral behavior and shared backend contracts |
| `src/device/<category>/test/` | Attached-chip protocols and transport integration |
| `src/io/test/` | Board facade, peripherals, bus endpoints and contract diagnostics |
| `src/mcu/<mcu>/test/`, `src/mcu/test/` | MCU descriptions and shared contracts |
| `src/arch/test/` | Barrier compile probes and instruction verification |

Host presets build the native suites, positive probes and expected compiler
rejections and run them through CTest:

```sh
cmake --preset macos
cmake --build --preset macos
```

Use `linux-x86_64` on Linux x86_64. Native suites require `mcutest` at
`../mcutest` or a discoverable package; `MCUBSP_MCUTEST_SOURCE_DIR` selects an
explicit checkout. After building, CTest labels allow focused runs:

```sh
ctest --test-dir output/build/macos -L backend --output-on-failure
```

Physical tests live under `src/board/<vendor>/<board>/test/`. They are built only
when explicitly selected and run through the separate
[hardware workflow](../doc/getting-started/hardware.md). Ordinary host checks do
not execute on a board. MCUIP contracts and composed firmware belong to the
sibling [integration project](../../integration/README.md).

Cross-build outputs remain under `output/build/<preset>/test/compile/target-probes/`.
Barrier reports are inspected on the host. See
[build and verification](../doc/getting-started/build-and-test.md).
