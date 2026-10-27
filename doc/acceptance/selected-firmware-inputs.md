# Selected firmware input records

Software verification recorded on 25 October 2026 for PLN-0035, following
[capture and coverage restoration](capture-and-coverage.md). This milestone
adds selected-input records; hosted and physical qualification remain open.

## Delivered behavior

Ninja/GNU firmware builds produce `.elf.inputs.json` after successful linking.
The completed `.elf.build.json` binds that sidecar's checksum and selected-input
identity to the ELF. Runner planning rejects missing/changed sidecars and shows
the recorded identity. Other generators explicitly report tracking unavailable.

Inputs include the selected Ninja graph, actual compiler header dependencies,
generated configuration, transitive libraries, GNU linker inputs, explicit
compiler specification files, expanded response-file commands and compiler/tool
fingerprints. Repository revision and tracked dirty state remain separate from
the selected identity. Selected-input changes alter identity; unrelated files
and repository-only commits leave it unchanged when selected inputs are unchanged.

P4 records include the separate bootloader graph and partition-table inputs.
This exposed an ignored test-project bootloader setting, now replaced by the
SDK's `BOOTLOADER_EXTRA_COMPONENT_DIRS` property. Both Blink and test records
contain mcubsp's factory-only `bootloader_start.cpp` and exclude the SDK default
`bootloader_start.c`. SDK application version is explicitly `0.1.0` by default,
overridable with `MCUBSP_APPLICATION_VERSION`, keeping repository Git descriptions
out of selected application version flags.

GNU ld writes unescaped paths with spaces on separate dependency-file lines.
The producer preserves those boundaries. Failed producers remove prior
completed records; missing/stale dependency logs or linker inputs are errors.

## Executed checks

| Check | Result |
| --- | --- |
| GCC 15.1 ordinary host | 43/43 CTest entries; 287 merged cases, zero failures |
| AppleClang 21 instrumented host | 43/43 CTest entries; 287 merged cases, zero failures; affected workflow checks rerun after producer refinements |
| Selected-input regressions | Real CMake/Ninja transitive-library build; selected header, generated config, flag and linker-input changes; unrelated Git commit; stale/missing records; deleted response-file expansion |
| Runner regressions | 17 cases, including missing/modified input sidecars and failed-producer invalidation; fake debugger tools only |
| Board test builds | Ten UNO, three NUCLEO and one P4 SDK image with recorded input sidecars; no target execution |
| P4 policy/version | Blink and test graphs select the factory-only boot source; application compile flags use explicit `0.1.0` |
| Installed consumers | Relocated package/build paths with spaces; UNO and NUCLEO supplied/external startup link and produce input records; exactly one Reset_Handler each |
| Real-image planning | All 14 board-test ELF/input bundles validate; no device command sent |
| Trace validation | Canonical graph: zero errors, 39 missing board-coverage warnings and three missing-test warnings; optional native-coverage graph validates |

Native reports remain at `build/host/artifact/native.json` and
`build/host-coverage-clang/artifact/native.json`. The instrumented native coverage
scope and denominators remain those in the preceding record: 161 modules/variants,
zero invalid MC/DC decisions and 781 explicitly skipped constructs. Generated
outputs are local evidence, not committed snapshots.

Current board test outputs use `build/uno-coverage`, `build/nucleo-tests` and
`build/p4-tests`; P4 Blink uses `build/review-p4`. Its test input record contains
2,591 selected files, while Blink contains 2,543. Counts describe their selected
graphs and toolchain, not coverage percentages. Installed-consumer tests used
temporary directories and the current relocated package.

## Remaining qualification

Automatic tracking is qualified locally for single-configuration Ninja with
the supplied GNU Arm/P4 linkers on macOS. Other host/toolchain combinations need
their own results. This record does not complete the setup/workspace milestone,
hosted CI or physical timing/traffic/production-silicon acceptance. Older source
identity strings embedded in test JSON remain explicitly unavailable; use the
bound input sidecar for the current firmware identity.

See [input identity details](../development/firmware-inputs.md), the
[hardware guide](../getting-started/hardware.md) and the
[support matrix](../reference/support-matrix.md) for commands and scope.
