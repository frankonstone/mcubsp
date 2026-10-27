# Public board tree consolidation

Software verification recorded on 26 October 2026 for PLN-0037. Concrete board
types and their wiring now share the vendor-qualified public board tree. The
root `board/` build/startup/fixture tree remains in place; `mcu/`, `backend/`,
`driver/` and generic `io/` retain their responsibilities.

The subsequent [BSP completion](bsp-layout.md) moves that support tree to `bsp/`
and startup files to board-local `startup/`. The record below describes the
earlier public-header step.

## Delivered layout and API

Each supported board has a named entry header, for example
`mcubsp/board/arduino/uno_r4_minima.hpp`, defining
`mcubsp::board::arduino::uno_r4_minima`. The adjacent board directory contains
`pins.hpp` and `bindings.hpp`, defining named `_pins` and `_bindings` structs.
The complete board type inherits its low-level definitions and composes the
existing default backends and execution policy.

The former flat wiring headers and `io/board/` profiles are removed. Includes,
types, CMake board metadata, starter projects, examples, tests and authored
guides use the new paths. Low-level namespace aliases migrate to type aliases.
The [layout/migration guide](../development/structure.md) records the mapping.
Historical capture files and their source/firmware identities are preserved.

## Executed checks

| Check | Result |
| --- | --- |
| GCC 15.1 host | 43/43 CTest entries pass; affected workflow rerun with the new dependency-layer case; 290 merged cases, zero failures |
| AppleClang 21 native coverage host | 43/43 CTest entries; 290 merged cases, zero failures |
| Public headers | 113 active isolated-header compilations on host, Cortex-M4, Cortex-M7 and P4 |
| Architecture probes | All three architectures pass compiler contracts and barrier checks |
| Lightweight board definitions | All three bindings compile with concrete pin/connection assertions; compiler dependency lists contain neither `mcubsp/io/` nor `mcubsp/backend/` |
| Source starter | UNO, NUCLEO and P4 firmware link with the vendor-qualified board types |
| Relocated installed starter | Same three boards link; library header inputs resolve exclusively within the relocated prefix; old public board trees are absent |
| Physical-test images | All ten UNO, three NUCLEO and one P4 SDK test images build; no target execution |
| Coverage scope | Report regenerated from the current native suites; no removed board paths remain; zero invalid MC/DC decisions |
| Trace validation | Zero errors; 39 missing board-coverage and three missing-test warnings remain; optional coverage graph validates |

Native coverage retains 161 selected modules/variants and the preceding native
denominators: 1,441/1,975 functions, 9,964/12,702 statements,
10,904/13,439 lines, 2,862/5,008 branches and 849/3,621 MC/DC conditions covered.
Its 781 skipped constructs remain explicitly visible. These are native-suite
figures, not whole-device or electrical coverage.

Canonical native evidence is `build/host/artifact/native.json`; instrumented
native evidence is in `build/host-coverage-clang/artifact/`. Cross checks use
the existing `build/host/test/compile/target-probes/` directories. Source starters
use `build/consumer-uno`, `build/consumer-nucleo` and `build/consumer-p4`.
Installed starter checks use a relocated `build/board-layout-relocated` prefix,
temporary Arm applications and `build/board-layout-p4-installed`. These local
outputs are not committed evidence snapshots.

No hardware was flashed and no physical claim was refreshed. Hosted CI and the
broader setup/workspace milestones retain their independent open acceptance.
