# Build and verification

For application builds, use the [first-build guide](building.md). Maintainer
verification has selectable native suites and three architecture probe presets.
The probes compile only; they do not link startup code, an RTOS, or a board SDK.
The full presets explicitly enable independent header checks and MCUIP contracts,
so install the [development dependencies](installation.md) first. For a focused
native run requiring only mcutest:

```sh
python3 tools/verify.py --build build/verification --suites all
python3 tools/verify.py --build build/tmp102-tests --suites device:ti/tmp102
```

The `component` and `native` configure/build/test presets also provide focused
IDE workflows; `host` remains the explicit full maintainer configuration.
Build presets select `native-tests`, `check-headers`, `check-compile` or
`board-tests` as appropriate. Header checks remain independent compiler jobs
inside one target. See [targets and presets](../development/verification.md#targets-and-presets)
for menu organization and persistent user presets.

For the exhaustive local maintainer workflow:

```sh
cmake --preset host
python3 tools/verify.py --build build/host --existing

cmake --preset arm-cortex-m4-checks
cmake --build --preset arm-cortex-m4-checks

cmake --preset arm-cortex-m7-checks
cmake --build --preset arm-cortex-m7-checks

cmake --preset esp32p4-riscv-checks
cmake --build --preset esp32p4-riscv-checks
```

Each configured build writes `toolchain-report.txt` with the compiler,
standard-library identity, CMake generator, selected architecture and exact
restricted-runtime flags. Native suites write separate results under
`build/host/artifacts/native/`. `tools/verify.py` merges only the current CTest
selection into `build/host/artifacts/mcutest-native.json` for mcutrace; it deletes
previous selected results before running, so stale artifacts are not reused.
Tooling tests write `build/host/artifacts/mcutest-tooling.json`. Direct CTest
invocation remains supported but does not refresh the merged native artifact.
See [verification and CI](../development/verification.md) for affected checks,
scale measurements and evidence scope.
Each cross build writes `barrier-report.txt` after disassembling the barrier
probe and checking the architecture's required instructions.
The `mcuhw.fixture_failure` CTest entry uses `WILL_FAIL` so a deliberately
failed assertion is a passing fixture check; running
`build/host/tests/mcuhw_fixture_failure` directly displays the assertion and
returns a nonzero status.

The ESP32-P4 preset discovers `riscv32-esp-elf-g++` on `PATH`, under the
standard `IDF_TOOLS_PATH`/`~/.espressif` layout, or through the explicit
`MCUHW_ESP32P4_TOOLCHAIN_ROOT` cache setting. Configuration stops with an
actionable error when it is not installed; an unavailable compiler is never
recorded as passing evidence.

The all-three description translation unit is registered automatically once
the real RA4M1, STM32H753 and ESP32-P4 description headers exist. Until then,
the configure log reports that gate as deferred. This keeps the infrastructure
ready without introducing placeholder target types.

PLN-0005 adds an independent, linked board preset without changing the three
description-only probes:

```sh
cmake --preset uno-r4-minima
cmake --build --preset uno-r4-minima
./tools/run-uno-r4-minima-tests.sh startup-gpio
./tools/run-uno-r4-minima-tests.sh ethernet
```

The preset builds separate startup/GPIO, D2-to-D3 GPIO loopback, PWM, and UART
executables. The board procedure, bootloader boundary, fixture wiring, RTT
protocol, suite-selection commands, and current measurements are documented in
[Arduino UNO R4 Minima guide](../boards/arduino-uno-r4-minima/README.md).
The W5100 raw-frame and MCUIP ARP/ICMP procedure is documented separately in
[the W5100 Ethernet guide](../boards/arduino-uno-r4-minima/ethernet-w5100.md).
