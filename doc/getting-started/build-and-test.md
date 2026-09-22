# Build and verification

PLN-0002 uses one host build and three architecture compile-probe presets. The
probes compile only; they do not link startup code, an RTOS, or a board SDK.

```sh
cmake --preset host
cmake --build --preset host
ctest --preset host

cmake --preset arm-cortex-m4
cmake --build --preset arm-cortex-m4

cmake --preset arm-cortex-m7
cmake --build --preset arm-cortex-m7

cmake --preset esp32p4-riscv
cmake --build --preset esp32p4-riscv
```

Each configured build writes `toolchain-report.txt` with the compiler,
standard-library identity, CMake generator, selected architecture and exact
restricted-runtime flags. The host test also writes
`build/host/artifacts/mcutest.json`, the evidence file consumed by mcutrace.
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
