# Verification workflows and evidence

Applications build selected interfaces and firmware. Contributor and maintainer
verification are explicit operations. The workflow in
[`.github/workflows/verification.yml`](../../.github/workflows/verification.yml)
provides a hosted Linux/macOS/Windows native verification recipe. Its presence is
not evidence that those hosted jobs have run: the initial implementation was
exercised locally on macOS, while hosted Linux and Windows qualification remains
pending until their own recorded jobs succeed.

## Targets and presets

The shared `base` preset explicitly disables optional checks and clears the
application/board-test selectors. Reconfiguring a reused build directory through
a contributor preset therefore does not retain old maintainer options. Choose
a separate workspace build directory for every architecture/toolchain. Board and
revision artifacts are isolated underneath that workspace.

| Workflow | Configure preset | Build preset selects |
| --- | --- | --- |
| Selected device/MCU/core development | `component` | `native-tests`, containing only selected suites |
| All native behavior | `native` | `native-tests` |
| Full local maintainer verification | `host` | `native-tests`, `check-headers`, `check-compile` |
| Active-board workspace | `arm-cortex-m4`, `arm-cortex-m7`, `esp32p4-riscv` | `examples`, containing only active boards |
| Cross-architecture verification | Architecture preset with `-checks` suffix | `check-headers`, `check-compile` |
| Board test firmware | Existing UNO/NUCLEO test preset | `board-tests` |
| One complete example | Board's `*-app` preset | `firmware` |

`native-tests` and `board-tests` build their executables. Execute host tests with
the test preset or Test Explorer; board firmware still requires an explicitly
requested hardware procedure. Build, test and workflow presets never flash.

### Active-board workspaces

Configure the architecture, then choose task groups for the boards you are working
on. `MCUHW_ACTIVE_BOARDS` is a STRING cache setting accepting a semicolon-separated
list of exact `vendor/board` or `vendor/board@revision` IDs. It starts empty;
omitting a revision selects the manifest's default. For the current boards:

| Configure preset | Compatible board ID |
| --- | --- |
| `arm-cortex-m4` | `arduino/uno_r4_minima` |
| `arm-cortex-m7` | `st/nucleo_h753zi` |
| `esp32p4-riscv` | `waveshare/esp32p4_nano` |

```sh
cmake --preset arm-cortex-m4 -DMCUHW_ACTIVE_BOARDS=arduino/uno_r4_minima
cmake --build --preset arm-cortex-m4 --target arduino.uno_r4_minima.examples
```

The resulting project targets are:

```text
examples                                  all active boards' examples
arduino.uno_r4_minima.examples             this board's examples
arduino.uno_r4_minima.examples.blink       individual firmware
arduino.uno_r4_minima.examples.tmp102      individual firmware
arduino.uno_r4_minima.tests                instrumented board-test build
```

An aggregate builds separate images, not a combined executable. Vendor-qualified
names remain stable when other boards are selected; `/` becomes `.` in target
names. Individual examples remain independently buildable. ARM executable targets
are available to CMake debugging; ESP firmware uses its SDK/debugger's ELF
configuration. CMake folders group entries in supporting IDEs, but do not hide
those targets.

Edit the cache setting through CMake Tools or replace the list on the command
line, quoting multiple entries: `-DMCUHW_ACTIVE_BOARDS="vendor/board_a;vendor/board_b"`.
Both boards must match the architecture and each board may appear only once.
Unknown IDs, revisions and incompatible architectures fail during configuration.
Reconfigure after a selection change. Ordinary `cmake --preset arm-cortex-m4`
preserves the saved list; `-DMCUHW_ACTIVE_BOARDS=` removes all board targets.
Unselected manifests are never enumerated by this focused workflow.

For a reproducible personal workspace, save the setting in the ignored
`CMakeUserPresets.json`:

```json
{
  "version": 6,
  "configurePresets": [{
    "name": "my-m4",
    "inherits": "arm-cortex-m4",
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": {
      "MCUHW_ACTIVE_BOARDS": "arduino/uno_r4_minima@r4"
    }
  }],
  "buildPresets": [{
    "name": "my-m4", "inherits": "arm-cortex-m4", "configurePreset": "my-m4"
  }]
}
```

Select `my-m4` in the IDE, or run `cmake --preset my-m4` followed by
`cmake --build --preset my-m4`. Values explicitly written in a user preset are
reapplied on each configuration. Other cache edits persist until changed or the
cache is cleared. An explicit `MCUHW_ACTIVE_BOARDS=all` is available for exhaustive
maintainer builds; only this selection scans the catalog for compatible boards.

Example outputs live under
`build/<preset>/boards/<vendor>/<board>/<revision>/examples/<example>/`.
The `.tests` target configures a separate build under the same board/revision's
`tests/` directory only when requested. Coverage, mcutest, MCUIP and SDK test
support targets stay in that child build; example builds need none of those test
dependencies. Existing board-test presets and physical runners continue to use
their documented original directories. Use those presets for their existing
hardware procedures and individual test-image debugging.

ESP workspaces require the same installed SDK and Python environment as external
applications; pass `MCUHW_IDF_PATH`, `PYTHON` and the SDK tools path as described in
[the build guide](../getting-started/building.md). Each example receives its own
SDK project and build directory. Building `.tests` compiles the existing
instrumented SDK test project without flashing or running it.

The architecture presets now select board workspaces. Existing scripts that
used them for probes should use `arm-cortex-m4-checks`, `arm-cortex-m7-checks`
or `esp32p4-riscv-checks`. Single-example `*-app` shortcuts remain available.

### Native selection and workflows

The default `component` selection is `core`; select a device, MCU or IP suite:

```sh
cmake --preset component -DMCUHW_TEST_SUITES=device:ti/tmp102
cmake --build --preset component
ctest --preset component
```

This creates the TMP102 suite executable and its `native-tests` aggregate, with
no unrelated MCU/device suites, header checks or probes. Each test selection
retains its own artifact. Tests can be selected individually in Test Explorer;
the build menu contains executable suites, not every test case.

For repeatable configure/build/test sequences use:

```sh
cmake --workflow --preset verify-native
cmake --workflow --preset verify-host
```

`verify-component` runs the preset's default core selection. Workflows reapply
their configure preset; a prior command-line override is not a persistent
workflow setting. Use an ignored `CMakeUserPresets.json` for a named selection:

```json
{
  "version": 6,
  "configurePresets": [{
    "name": "tmp102",
    "inherits": "component",
    "binaryDir": "${sourceDir}/build/${presetName}",
    "cacheVariables": {"MCUHW_TEST_SUITES": "device:ti/tmp102"}
  }],
  "buildPresets": [{
    "name": "tmp102", "inherits": "component", "configurePreset": "tmp102"
  }],
  "testPresets": [{
    "name": "tmp102", "inherits": "component", "configurePreset": "tmp102"
  }]
}
```

No central list changes are needed for another catalog component. Its own
manifest continues to own its focused sources and tests.

`check-headers` compiles one generated translation unit per header within one
object library. Adding headers increases compiler jobs without adding menu
entries. Unity compilation and precompiled headers are explicitly disabled,
so another header cannot supply a missing include accidentally. The old
`mcuhw_headers_<header>` target names are replaced by `check-headers`; normal
build presets select it where required.

Target folders group verification suites and internal board-support objects in
VS Code's target dropdown. They do not hide targets. Application configurations
omit verification targets entirely. The repository enables
`cmake.useFolderPropertyInBuildTargetDropdown` and preserves individual native
executables and board images for focused debugging.

Build presets now select explicit targets. For an additional opt-in target,
name it using `--target`; for example, enabling `MCUHW_BUILD_IO_EXAMPLES` does
not add legacy examples to the cross-verification preset's target selection.
Use `tools/verify.py --build build/host --existing` when refreshing the merged
mcutrace native artifact; ordinary CTest/workflow runs write per-suite results.

### Measured target counts

The local macOS arm64 run with CMake 4.4.0 reports these project targets through
the CMake File API. Generator built-ins such as `all` and `clean` are additional.
The measurement is recorded in `build/targets-selected-counts.json`.

| Configuration | Project targets |
| --- | ---: |
| Architecture workspace with one current board active | 5 (two examples, board examples/test groups, workspace examples group) |
| Each of the three board application presets | 1 (`firmware`) |
| `component`, selecting `device:ti/tmp102` | 2 (suite executable and aggregate) |
| `native` | 16 |
| `host` | 21 |

The three workspace measurements are recorded in `build/workspace-target-counts.json`.
The regression fixture builds two active board identities independently, then
adds 100 unselected boards and verifies unchanged targets and compile commands.
It also verifies persistent cache/user-preset selection, target removal, invalid
board/revision/architecture diagnostics and deferred test dependencies.

The host compile database contains 104 separate header translation units under
one `check-headers` target. The target regression fixture deliberately enables
unity compilation and a precompiled header, verifies that a missing include
still fails, then verifies that adding headers adds compilation jobs without
adding selectable targets.

The workspace follow-up maintainer run passed all 41 CTest entries, including
241 native cases and 96 tooling cases with no skips. Example and instrumented
test groups built for all three boards, and the Cortex-M4, Cortex-M7 and RISC-V
`-checks` presets passed. The earlier `verify-native` workflow also passed.
These are local software checks; hosted Linux/Windows qualification remains open.

## Local focused verification

Choose a C++23 compiler with the required standard library, CMake 3.25 or newer,
Ninja and Python 3.10 or newer. Native behavioral tests additionally need mcutest;
ordinary library users do not. The hosted workflow checks out
[`frankonstone/mcutest`](https://github.com/frankonstone/mcutest)
at exact commit `532327858ead93140535b539cf65b35eb96fdf01` and passes its path
explicitly. No other sibling checkout is required for these checks.

```sh
python tools/catalog.py validate
python tools/check-impact.py include/mcuhw/device/sensor/tmp102.hpp
python tools/verify.py --build build/verification-tmp102 \
  --suites 'device:ti/tmp102;io;compile' \
  --mcutest /path/to/pinned/mcutest --compiler g++-15 --tooling
```

Use an isolated build directory when changing compiler/toolchain configurations.
`verify.py` configures the named native suites, builds, runs CTest, and merges only
result files declared by the selected CTest registry. Old results from another
selection cannot enter its `artifacts/mcutest-native.json`. The tooling suite
writes its own `artifacts/mcutest-tooling.json`, preserving skipped cases as skipped.
These reports establish software execution; they never create physical evidence.

Suite names are `core`, `drivers`, `io`, `descriptions`, `compile`, and local
`mcu:vendor/name`, `ip:vendor/name` or `device:vendor/name` identities. `all` includes
every native component suite. Board test sources remain cross-target fixtures;
a board's native impact selects its MCU/IP/device contracts plus its board ID for
separate firmware verification.

## Changed paths and exhaustive runs

`tools/check-impact.py` and the CI diff adapter share the same catalog dependency
implementation. A local component change selects that component and all transitive
dependents. Shared infrastructure, multiply owned inputs, deleted paths, unknown
paths or incomplete dependency information conservatively select `all`.

The JSON report includes:

| Field | Meaning |
| --- | --- |
| `mode` / `reasons` | Focused selection or the reason for a full fallback |
| `components` | Changed components and transitive dependents, as `kind:vendor/name` |
| `boards` | Board IDs selected for cross-building their declared applications |
| `suites` | Native CMake suites selected for this change |
| `cmake_args` | The corresponding `MCUHW_TEST_SUITES` cache argument |

Pull requests compare the checked-out merge result against the recorded base SHA.
NUL-delimited Git output preserves unusual file names. An unavailable base/diff or
invalid catalog falls back to all native suites; the independent catalog gate still
rejects invalid metadata. Empty diffs skip native cases while retaining tooling.
Every PR runs packaging/catalog/frontend/provenance/scaling tooling on each host.

The CI selector expands `boards` into `cross_builds`, grouped by provisioned tool
environment. Every declared example is built for each supported revision. ARM jobs
use the checksum-pinned Arm GNU 15.2.Rel1 archive; ESP jobs run in the official
digest-pinned `espressif/idf:v6.1` environment. The ESP action also verifies
the SDK commit and RISC-V compiler against the tool lock. The board IDs and example list come from component
manifests. Missing compilers/SDKs fail these jobs; the optional tooling fixture's
SDK skip cannot substitute for a selected firmware build.

Run the same action locally with the toolchain on `PATH`:

```sh
python3 tools/verify-boards.py --board arduino/uno_r4_minima --revision r4 \
  --example examples/apps/blink --build build/verify-uno-blink
```

ESP actions need the activated ESP-IDF environment. Each action writes
`artifacts/cross-build.json`; success requires the linked `firmware.elf`. CI
uploads these results under distinct board/revision/example keys. These hosted
jobs require their own successful execution before being counted as evidence.

Default-branch pushes, weekly schedules, manual dispatch and published releases
run all native suites and every public-header self-containment compilation. MCUIP
integration contracts and architecture probes are explicitly disabled in these
host jobs; they require their own selected dependencies/cross-toolchains. Full
native test execution is not a replacement for those separately scoped checks.

## Hosted matrix and reproducibility limits

| Runner recipe | Compiler selection | Other inputs |
| --- | --- | --- |
| `ubuntu-24.04` | Distribution `g++-14` | Distribution CMake/Ninja, Python 3.12.10 |
| `macos-14` | Homebrew `gcc@15`, `g++-15` | Homebrew CMake/Ninja, Python 3.12.10 |
| `windows-2022` | MSYS2 UCRT64 native GCC | UCRT64 CMake/Ninja/Python |

Ubuntu publishes the [GCC 14 package](https://packages.ubuntu.com/noble/g%2B%2B-14),
and Homebrew provides the versioned [`gcc@15` formula](https://formulae.brew.sh/formula/gcc%4015).
Windows uses the UCRT64 environment and shell provided by the
[MSYS2 setup action](https://github.com/msys2/setup-msys2), so its compiler,
standard library, Python and build tools come from the same environment.

Action implementations and mcutest are pinned to full commit identities. The host
package managers can publish patch updates; these recipes are not frozen host
images or a claim of offline setup. Each job records the exact host, compiler
version/target, Python, CMake, Ninja and mcutest commit in `host-tools.json`.
Keep that record with any accepted evidence. Use the separate application setup
lock and reusable caches for its pinned toolchain/offline workflow; the CI
compatibility matrix does not silently replace that lock.

No hosted deployment, messages, secrets or physical flashing are part of this
workflow. Artifacts are separate per host, including native/tooling results,
catalog output, scaling report and CTest failure log. Failed jobs still upload
available diagnostics. A successful scheduled job does not automatically change
component evidence manifests or mcutrace acceptance.

## Measuring catalog growth

```sh
python tools/check-scale.py --mode host-device --compiler g++-15 \
  --output build/adoption/scaling.json
```

This command copies shipping inputs into a temporary resource tree, builds a clean
external native TMP102-header consumer, adds at least 300 synthetic board manifests
and 3,000 synthetic devices, validates/discovers the expanded catalog, and repeats
the clean configure/build. It compares actual CMake file-API targets, compilation
commands, selected components and compiler-recorded provenance inputs/identity.
Timings and exact environment accompany the result. Synthetic entries never enter
the repository catalog or claim hardware support.

With Arm GNU installed, `--mode arm-blink` performs the same experiment using the
complete external UNO R4 Minima Blink firmware instead. Cross-build evidence is
still distinct from flashing and physical observation. The local measured report
is `build/adoption/scaling.json`; a hosted run writes its own artifact under
`build/verification/artifacts/scaling.json`.

Board Blink/TMP102 cross-builds, externally supplied startup checks, SDK boot paths,
board/device wiring, and physical runner procedures are documented in
[the application build guide](../getting-started/building.md) and each component's
board documentation. Record those exact environments and fixtures separately;
unexecuted host/board combinations remain pending.
