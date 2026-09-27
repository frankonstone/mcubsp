# RA4M1 bare-metal release acceptance

This is the evidence index and operating contract for PLN-0011. The release is
**accepted** under D14: the user retired the cross-target fixture requirement
and selected representative directly connected UNO R4 Minima hardware as the
physical acceptance scope. Software, board-native, resource and latency
evidence pass. The numeric limits below are the initial release budgets adopted
from the optimized hardware measurements.

## Release boundary

The first RA4M1 release is scheduler-independent C++23 for the Arduino UNO R4
Minima (`R7FA4M1AB3CFM#AA0`). It includes startup/clock, GPIO, GPT basic PWM,
SCI UART, IIC, SPI, ADC14 and four fixed DMAC channels. MCUOS, DTC, advanced
motor PWM, networking protocols and unimplemented peripheral classes are not
part of this release.

Drivers use caller-owned buffers and fixed operation pools. A returned transfer
handle owns one pool slot until its result is consumed, cancelled or retired.
Polling and blocking use the same state machine. Interrupt handlers publish
completion only after the peripheral or DMA engine is quiescent; waiting from
an ISR is rejected. The restricted runtime disables exceptions, RTTI,
thread-safe statics and dynamic allocation. Functional wrappers contain only a
backend reference plus their documented lifecycle state; register addresses,
ownership and interrupt sequencing remain in the RA4M1 backends.

## Current evidence

The authoritative artifacts are generated from the current working tree:

- `build/host/artifacts/mcutest.json`: 174 host cases, zero failures.
- `build/uno-r4-minima/artifacts/mcutest-hardware.json`: 17 passing physical
  cases across all nine UNO runs, including the optimized metrics image.
- `build/uno-r4-minima/artifacts/mcuhw_uno_r4_minima_all_tests.mcucov.json`:
  MCUCOV build `0x4d485701`; 155/238 functions, 1,059/1,680 statements,
  1,205/1,865 lines, 359/814 branches and 45/598 MC/DC conditions. Its 134
  skipped constructs contain 36 warnings and zero errors.
- `build/uno-r4-minima/artifacts/mcuhw_uno_r4_minima_release_metrics.json`:
  three passing physical cases from the non-instrumented `-Os` image.
- `build/uno-r4-minima/artifacts/mcuhw_uno_r4_minima_resources.json`:
  section sizes, RAM headroom and SHA-256 identities for every UNO image.
- `build/uno-r4-minima/artifacts/mcuhw_uno_r4_minima_resources.source-identity.txt`:
  exact content hashes whose manifest hash is recorded by the resource report.

The optimized metrics image measured at 48 MHz:

| Measurement | Result | Release budget |
| --- | ---: | ---: |
| Checked GPIO write | 109 cycles / 2.27 us | <= 128 cycles |
| Direct-register GPIO write | 4 cycles / 0.083 us | baseline only |
| Critical enter/leave pair | 9 cycles / 0.188 us | <= 16 cycles |
| DMAC completion ISR body | 165 cycles / 3.44 us | <= 192 cycles |
| Largest backend instance (SCI UART) | 152 bytes | <= 192 bytes |
| Metrics image flash | 21,248 bytes | <= 24 KiB |
| Metrics image static RAM | 12,984 bytes | <= 16 KiB |
| Stack peak upper bound | 1,272 bytes | <= 2 KiB |
| Reserved stack | 4,096 bytes | 4 KiB fixed |
| Largest instrumented acceptance image | 115,472 bytes flash | <= 128 KiB |
| Largest instrumented static RAM | 14,928 bytes | <= 16 KiB |

The metrics image's static RAM includes the 8 KiB JSON evidence buffer. The
instrumented-image limits are test-infrastructure budgets, not claims about a
minimal production application. The DWT ISR value covers the handler body; it
does not include architectural exception entry and return.

## Reproduction

Configure and run the native suite:

```sh
cmake --preset native -B build/host -DMCUHW_BUILD_TESTS=ON -DMCUHW_BUILD_TOOLING_TESTS=ON -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON -DMCUHW_BUILD_MCUIP_CONTRACTS=ON
cmake --build build/host -j4
ctest --test-dir build/host --output-on-failure
```

Cross-build all public headers, examples and architecture probes, then link the
UNO images:

```sh
cmake --preset arm-cortex-m4 -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON
cmake --build --preset arm-cortex-m4 -j4
cmake --preset arm-cortex-m4 -B build/uno-r4-minima -DMCUHW_BOARD_TESTS=arduino/uno_r4_minima
cmake --build build/uno-r4-minima --target board-tests -j4
```

The UNO linker preserves the Arduino bootloader by placing vectors and code at
`0x00004000`; SRAM is `0x20000000..0x20007fff`, with the top 4 KiB reserved for
the stack. The runner uses J-Link SWD at 4 MHz, resolves the RTT control block
from the ELF, flashes with SEGGER Commander and captures channel 0:

```sh
./tools/run-uno-r4-minima-tests.sh release-metrics
./tools/run-uno-r4-minima-tests.sh aggregate
./tools/run-uno-r4-minima-tests.sh all
./tools/report-uno-r4-minima-resources.sh
```

`aggregate` rebuilds the combined mcutest and MCUCOV reports from existing
per-suite captures without reflashing hardware. `all` reruns the eight
instrumented functional suites plus the optimized metrics suite before doing
the same aggregation. Each physical suite records its board, firmware, clocks,
observation path, connected hardware and tolerances.

## Acceptance result and limits

PLN-0011 is complete. The accepted physical evidence comprises direct
GPIO/PWM/UART procedures, Grove PAJ7620U2 I2C transactions at approximately
100/400 kHz, W5100 SPI traffic at 1/6 MHz, W5100 peripheral-DMAC traffic,
potentiometer ADC conversion at 12/14 bits and ADC-to-memory DMAC capture.

This release does not claim safe translated-voltage operation, no-backfeed or
current-limit measurements, independent PWM/bus waveform capture, calibrated
absolute ADC accuracy, UART receive/error injection, supervised physical fault
injection or complete-board electrical coverage. Those are explicit non-claims,
not missing evidence for the D14 release scope.
