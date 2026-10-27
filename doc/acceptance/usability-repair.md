# Framework usability repair

Software verification recorded on 24 October 2026 for PLN-0033. This record
covers the migration repair and application-interface changes; it creates no
new physical acceptance and does not close the older setup/workspace or hosted
qualification milestones.

The [25 October continuation](capture-and-coverage.md) restores current capture
helpers and regenerates native coverage; the results below describe the earlier
24 October milestone.

## Delivered behavior

- Plain CMake configuration provides the installable library. Host tests,
  cross checks, MCUIP contracts, header checks and board-test images are explicit.
- The shared firmware helper links applications with exact board/revision and
  supplied/external startup. All three boards provide Blink. Linked ELF files
  have adjacent board/revision/startup/compiler-path/size/SHA-256 records.
- CI calls current CMake/CTest entry points and retains native/build artifacts.
  Native jobs require no cross compiler, MCUIP or coverage installation.
- Compiler-rejection cases match diagnostic categories. Independent header
  translation units prevent transitive includes from hiding missing dependencies.
- Native evidence is merged only from the configured producer registry, with
  per-producer hashes. Unregistered old files cannot enter the report. Missing,
  failed or skipped producer cases prevent publishing an all-passing report.
- `io::select` rebinds native peripheral routes; `composed_board` and
  `make_board` can add instances in fixed storage, sharing pin ownership,
  polling and shutdown. Multi-input ADC scans retain binding order.
- CPU SPI accepts byte spans and byte segments without staging allocations on
  all three backends. Wide frames remain supported; invalid mixed formats fail.
- `execution::monotonic_policy` separates time from synchronization/wait units.
  An application-supplied 1 kHz monotonic clock permits intervals up to 24.8 days.
  Default board cycle clocks and their sleep/frequency assumptions are unchanged.
- Lifecycle diagnostics expose component/reason/location and pending operation
  slot/generation before trapping. The UNO recovery example demonstrates
  cancellation, reinitialization and shutdown.

## Executed checks

| Check | Result |
| --- | --- |
| macOS arm64 GCC 15.1.0 host build | 41/41 CTest entries; merged 266 native/workflow cases, zero failures |
| macOS arm64 AppleClang 21 host build | 41/41 CTest entries; same behavior/workflow cases |
| Public headers | 110 separate translation units on host and each cross architecture |
| Cortex-M4 / Cortex-M7 / P4 probes | Compile and barrier-instruction checks pass; no target execution |
| Optional native MCUIP integration | Native suite and integration contract compilation pass with explicit checkout |
| UNO examples | Blink, serial echo, SPI DMA, TMP102, W5100 and recovery link |
| UNO optional physical-test images | All ten non-instrumented images link, including Ethernet/MCUIP; not run |
| NUCLEO / P4 Blink | Linked firmware; P4 uses the existing development-v1 SDK boot policy |
| External consumers | Source inclusion and relocated installed library pass, including paths with spaces and parent-owned tests |
| Installed firmware helper | UNO and NUCLEO supplied/external startup compositions link; exactly one Reset_Handler each |
| Evidence/diagnostic regressions | Unrelated compiler failure rejected; stale unregistered report excluded; missing report removes merged output; pending-operation diagnostic observed |
| Documentation / workflow | Active README/getting-started/development links resolve; CI YAML parses |

Local tools: CMake 4.4.0, Ninja 1.12.1, Python 3.12.2, Arm GNU GCC 15.2.1,
and ESP GCC 15.2.0 with the installed ESP-IDF v6.1 environment. Local test and
integration dependencies are explicit source checkouts; this is not an assertion
that every host or dependency revision is qualified. CI pins its own inputs.

The canonical native result is `build/host/artifact/native.json`; individual
suite results remain below `artifact/native/`. Cross reports are under
`build/host/test/compile/target-probes/`. Local application outputs used
`build/review-uno`, `build/review-nucleo` and `build/review-p4`. External installed
startup checks used a temporary installation. These generated files are not
committed evidence snapshots.

## Outstanding qualification

Hosted CI has not been executed from this session. Coverage was not regenerated.
No hardware was flashed. Existing physical evidence retains its original
firmware/fixture identity. Historical runners depending on removed helpers
remain outside the current automated path. The [support matrix](../reference/support-matrix.md)
records Ethernet traffic/link recovery, peripheral DMA/timing and production-v3
P4 hardware work still requiring real observations.

Mcutrace now loads current native reports. Missing coverage, unimplemented
requirements and pre-existing annotation diagnostics remain visible; passing
native results do not satisfy those other evidence classes.
