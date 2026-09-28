# Repository adoption and scale qualification

REQ-0047 through REQ-0052 / PLN-0027 through PLN-0032, recorded 2026-09-26.
This record covers the implementation working tree based on repository revision
`8136cb2a190fb697d26ce00c9d7fb9ee3e9a126e`. Each firmware identity separately
records its actual selected file hashes, commands and toolchain identity; the
base Git revision alone does not identify the uncommitted implementation.

This is a historical qualification record. Its catalog and scale measurements
describe the 2026-09-26 implementation; those facilities have since been removed.

## Executed environment and results

The measured host is the local macOS development machine, Darwin 25.6.0,
arm64. Native tests use AppleClang 21.0.0.21000101 and libc++ 220106.
Tools: CMake 4.4.0, Ninja 1.12.1, tooling Python 3.14.7, Arm GNU
15.2.Rel1 / GCC 15.2.1, and Espressif GCC 15.2.0
(`esp-15.2.0_20251204`). ESP-IDF v6.1 is pinned to
`fff9895c82d744c7237be8847347bdd1b07c6643` and uses its Python 3.12.2
environment. The [setup lock](../../tool/toolchains.lock.json) and generated
environment reports record exact installations, checksums and overrides.

| Executed check | Result and scope |
| --- | --- |
| Full maintainer native run | 41 CTest entries pass, including 241 native behavioral cases, compile/compile-fail gates, expected fixture failure and the tooling suite |
| Tooling suite | 86 cases pass; zero failures or skips: 30 catalog, 20 frontend, 11 board, 10 provenance, 9 impact/scale, 3 packaging, 3 evidence-merging cases |
| Consumer packaging | External source and installed consumers, parent-owned tests, direct headers, and relocated package after removing its original source/build all pass |
| Complete board applications | Blink and TMP102 cross-build/link for UNO R4 Minima `r4`, NUCLEO-H753ZI `mb1364-rev-e`, and ESP32-P4-NANO `development-v1.0`; this is software build evidence on macOS |
| Startup/build correctness | External BSP has unique startup symbols; incompatible compiler/revision/build-directory selection fails; manifest edits reconfigure; conflicting supplied linker ownership fails |
| Optional dependencies | Native build and all 40 non-tooling CTest entries pass with MCUIP unavailable and its contracts/probes explicitly disabled |
| Existing maintainer gates | Cortex-M4, Cortex-M7 and ESP32-P4 RISC-V architecture/header/MCUIP probes pass; existing UNO and NUCLEO instrumented test images link |
| Setup/offline use | Real NUCLEO TMP102 and ESP Blink new/setup/doctor/build/rebuild pass with paths containing spaces; missing/incompatible tools, archive integrity, dirty SDK submodules and operation-specific runner arguments are tested |
| Traceability | REQ-0047 through REQ-0052 validate without diagnostics; imported results are actual producer outputs |

The current local aggregate evidence is
[`mcutest-native.json`](../../build/host/artifact/mcutest-native.json) and
[`mcutest-tooling.json`](../../build/host/artifact/mcutest-tooling.json).
Detailed board/provenance reports and frontend command observations are under
`build/adoption/`; these generated files are ignored by Git and are recreated by
the documented checks. The committed record summarizes the observed run rather
than pretending generated artifacts ship in every checkout.

TMP102, W5100 and ES8311 host results cover protocol/configuration and simulated
bus/backend behavior in their registered test sources. They do not establish new
physical chip acceptance. All original native test translation units remain
registered after suite decomposition; shared cases compile once per selection.

## Measured catalog growth

The actual external UNO Blink experiment adds **300 synthetic boards and 3,000
synthetic devices** to a temporary copy, growing 12 real components to 3,312 total
entries. Both clean builds retain **one target, four compiled sources, five build
commands and 149 selected identity inputs**. Selected dependency closure,
compiled sources, input hashes and firmware identity are equal before/after.
No synthetic entry is included in the published catalog.

The initial ARM measurement recorded:

| Operation | 12 components | 3,312 components |
| --- | ---: | ---: |
| Discovery | 0.0057 s | 1.8222 s |
| Full validation | 0.0331 s | 7.9272 s |
| Direct selected closure | 0.0011 s | 0.0015 s |
| Clean application configure | 0.4100 s | 0.5902 s |
| Clean application build | 0.7790 s | 2.1740 s |

These are single local wall-clock observations, not performance budgets or a
claim of timing invariance. Full catalog operations intentionally enumerate the
catalog; ordinary application configuration uses direct selected lookup. The
report includes tool identity, all selected inputs and per-phase timings:
[`build/adoption/scaling.json`](../../build/adoption/scaling.json).
The tooling suite also repeats the growth comparison with an actual native
TMP102-header consumer. Use [the verification guide](../development/verification.md)
to reproduce either experiment; later reports carry their own measurements.

Application ELF provenance covers Ninja's selected build edges and compiler
dependency records, including generated configuration, link inputs and flags.
ESP additionally emits separate bootloader provenance and a flash-bundle identity
for the exact images, offsets and settings in `flasher_args.json`. These reports
do not imply the images were flashed. External application paths remain explicit
inputs; moving an arbitrary application directory is not claimed to preserve its
identity.

## Remaining qualification

The hosted Linux/macOS/Windows native CI recipe is implemented but has not been
executed from this checkout. Linux and Windows clean pinned setup and the full
board-application first-build/offline workflows still require their own runs;
portable command construction and tests on macOS cannot substitute for them.
PLN-0030 and the integrated qualification in PLN-0032 therefore remain in progress.
PLN-0027, PLN-0028, PLN-0029 and PLN-0031 have delivered and locally verified
their scoped implementation outcomes.

No hardware was flashed during this work. Runner tests and dry runs establish
argument selection and image-boundary checks only. Existing physical records
retain their original firmware/fixture scope; the W5100 physical record remains
stale for the refactored driver. No new production ESP silicon, TMP102 wiring,
board/device Cartesian-product, or electrical acceptance is claimed.
