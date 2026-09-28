# Build and verification

For application builds, use the [first-build guide](building.md). The Cortex-M4
preset also selects the UNO R4 Minima board. Enable verification
with explicit CMake cache options, then choose the targets to build. Probes
compile only; they do not link startup code, an RTOS, or a board SDK.
Install the [development dependencies](installation.md) before enabling optional
MCUIP contracts. A focused native run needs only mcutest:

```sh
python3 tools/verify.py --build build/verification --suites all
python3 tools/verify.py --build build/tmp102-tests --suites device:ti/tmp102
```

Select a board with `MCUHW_BOARD` to expose `firmware`, `tests` and `examples`.
Without a board, enable `MCUHW_BUILD_TESTS`, `MCUHW_BUILD_PROBES` or
`MCUHW_CHECK_HEADERS` to expose their corresponding verification targets.
See [targets and presets](../development/verification.md#targets-and-presets)
for cache settings and target organization.

For the exhaustive local maintainer workflow:

```sh
cmake --preset native -B build/host -DMCUHW_BUILD_TESTS=ON -DMCUHW_BUILD_TOOLING_TESTS=ON -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON -DMCUHW_BUILD_MCUIP_CONTRACTS=ON
python3 tools/verify.py --build build/host --existing

cmake --preset arm-cortex-m4 -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON
cmake --build --preset arm-cortex-m4

cmake --preset arm-cortex-m7 -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON
cmake --build --preset arm-cortex-m7

cmake --preset esp32p4-riscv -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON
cmake --build --preset esp32p4-riscv
```

With `MCUHW_WRITE_TOOLCHAIN_REPORT=ON`, the build writes `toolchain-report.txt` with the compiler,
standard-library identity, CMake generator, selected architecture and exact
restricted-runtime flags. Native suites write separate results under
`build/host/artifacts/native/`. `tools/verify.py` merges only the current CTest
selection into `build/host/artifacts/mcutest-native.json` for mcutrace; it deletes
previous selected results before running, so stale artifacts are not reused.
Tooling tests write `build/host/artifacts/mcutest-tooling.json`. Direct CTest
invocation remains supported but does not refresh the merged native artifact.
See [verification and CI](../development/verification.md) for hosted checks and evidence scope.
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

For the legacy physical runners and their original output directory, configure
the instrumented board images explicitly:

```sh
cmake --preset arm-cortex-m4 -B build/uno-r4-minima -DMCUHW_BOARD_TESTS=arduino/uno_r4_minima
cmake --build build/uno-r4-minima --target board-tests
./tools/run-uno-r4-minima-tests.sh startup-gpio
./tools/run-uno-r4-minima-tests.sh ethernet
```

This configuration builds separate startup/GPIO, D2-to-D3 GPIO loopback, PWM, and UART
executables. The board procedure, bootloader boundary, fixture wiring, RTT
protocol, suite-selection commands, and current measurements are documented in
[Arduino UNO R4 Minima guide](../boards/arduino-uno-r4-minima/README.md).
The W5100 raw-frame and MCUIP ARP/ICMP procedure is documented separately in
[the W5100 Ethernet guide](../boards/arduino-uno-r4-minima/ethernet-w5100.md).
