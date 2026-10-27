# Capture tooling and native coverage restoration

Software verification recorded on 25 October 2026 for PLN-0034, following the
[framework usability repair](usability-repair.md). No board was flashed or
electrical observation refreshed during this work.

The [selected-input continuation](selected-firmware-inputs.md) adds bound
provenance sidecars and corrects P4 test bootloader selection. Counts below
describe the preceding capture/coverage milestone.

## Delivered behavior

- Shared J-Link, OpenOCD and ESP-IDF helpers read only the selected board entry.
  Plans require an explicit probe/port and a completed, matching ELF hash record.
  Arm load segments must fit application flash regions. P4 bundles verify all
  three declared regions and their recorded manifest/image hashes.
- `--dry-run` produces a reviewable command plan without opening hardware.
  Paths with spaces remain individual arguments, debugger text is escaped and
  explicit executable overrides retain the selected Python virtual environment.
- Current UNO/NUCLEO build paths, UNO RTT symbol lookup, NUCLEO explicit VCP
  selection and P4 SDK-build selection are restored. Coverage manifest splitting
  retains every translation unit and module without filename collisions or
  oversized multi-module inputs to the coverage reader.
- Optional native FULL coverage captures separately from test JSON and exports
  JSON, standalone HTML, LCOV and producer hashes. Its generated mcutrace config
  imports that same instrumented run. Ordinary builds keep coverage optional.
- NUCLEO test images use the shared firmware/startup helper and explicit test
  dependencies. P4 test builds record the completed bootloader/partition/app
  bundle. Five unsupported trace annotations now attach to supported class
  declarations; requirement meanings are unchanged.

## Executed checks

| Check | Result |
| --- | --- |
| GCC 15.1 ordinary host / AppleClang 21 FULL coverage host | 42/42 CTest entries each; 283 merged cases, zero failures |
| Runner regressions | 16 cases covering image/probe/port validation, bundle tampering/bounds, tool selection, manifest preservation and bounded serial scanning; fake tools only |
| Build/documentation regressions | 7 cases including relocated installation/consumer and installed runner metadata, active documentation links and evidence rejection |
| Public headers and architecture probes | 110 isolated headers on host and Cortex-M4/M7/P4; compiler contracts and barrier checks pass |
| UNO board tests | All ten instrumented images link; no target execution |
| NUCLEO board tests | All three images link; native and Ethernet images instrumented; no target execution |
| P4 SDK board tests | Instrumented application, bootloader and partition table build with ESP-IDF v6.1; three-image hash record produced |
| Real-image runner plans | UNO, NUCLEO and P4 dry runs validate their current images; no debugger/serial command sent |
| Trace validation | Canonical graph has zero errors; optional native-coverage graph validates with its explicit coverage diagnostics |

Coverage from the 13 executed instrumented native suites contains 161 source
modules/variants, including test code. It reports 1,441/1,975 functions,
9,964/12,702 statements, 10,904/13,439 lines, 2,862/5,008 branches and
849/3,621 MC/DC conditions covered, with zero invalid MC/DC decisions. There
are 781 skipped constructs: 507 informational, 274 warnings and zero errors.
They remain explicitly not instrumented; they are not counted as covered or
justified exclusions. This denominator does not describe whole-device coverage.

Generated native evidence is in `build/host/artifact/native.json`. Instrumented
evidence and reports are in `build/host-coverage-clang/artifact/coverage/`;
the optional graph uses its adjacent `mcutrace.toml`. Board build outputs used
`build/uno-coverage`, `build/nucleo-tests` and `build/p4-tests`. These local
generated outputs are not committed evidence snapshots. Tools and dependency
locations follow the preceding repair record; native coverage additionally uses
the locally installed mcucov package. GCC/macOS coverage is rejected explicitly
because its analyzer/system-header combination failed; AppleClang is the locally
verified coverage configuration.

## Remaining qualification

Hosted CI, new physical captures and production-v3 P4 acceptance remain open.
The canonical graph still has 39 board-source coverage warnings and three
missing-test warnings for synchronized motor PWM, RTOS integration and target
measurements. Native coverage does not satisfy those physical requirements.
Firmware hash records do not complete the broader selected-source provenance
qualification in REQ-0052; unavailable source identities remain marked as such.

Use the [current hardware guide](../getting-started/hardware.md) for commands,
fixture notes and explicit board selection. Historical captures retain their
original firmware/fixture scope. PLN-0030/PLN-0032 and the target milestones keep
their independent outstanding acceptance.
