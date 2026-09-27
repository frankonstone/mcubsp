# Verification workflows and evidence

Applications build selected interfaces and firmware. Contributor and maintainer
verification are explicit operations. The workflow in
[`.github/workflows/verification.yml`](../../.github/workflows/verification.yml)
provides a hosted Linux/macOS/Windows native verification recipe. Its presence is
not evidence that those hosted jobs have run: the initial implementation was
exercised locally on macOS, while hosted Linux and Windows qualification remains
pending until their own recorded jobs succeed.

The [test-group guide](../../tests/README.md) describes source directories,
CMake folders and CTest category labels.

## Targets and presets

Presets select the toolchain/architecture and global compiler settings only.
The four choices are `native`, `arm-cortex-m4`, `arm-cortex-m7` and
`esp32p4-riscv`. Build presets leave target selection to the caller or IDE.
Use a separate build directory for each architecture/toolchain.

### Board selection

Set `MCUHW_BOARD` using `-D` or edit its visible STRING entry in the CMake cache.
`MCUHW_BOARD_REVISION` optionally overrides the manifest's default revision.
Reapplying an architecture preset preserves both settings; reconfigure after
editing them. The configure output prints the selected board and architecture.

| Configure preset | Compatible board ID |
| --- | --- |
| `arm-cortex-m4` | `arduino/uno_r4_minima` |
| `arm-cortex-m7` | `st/nucleo_h753zi` |
| `esp32p4-riscv` | `waveshare/esp32p4_nano` |

```sh
cmake --preset arm-cortex-m4 -DMCUHW_BOARD=arduino/uno_r4_minima
cmake --build --preset arm-cortex-m4 --target firmware
cmake --build --preset arm-cortex-m4 --target examples
cmake --build --preset arm-cortex-m4 --target test.board
cmake --build --preset arm-cortex-m4 --target test.backend test.device
```

| Target | Behavior |
| --- | --- |
| `firmware` | Selected application, Blink by default; override `MCUHW_EXAMPLE` |
| `examples` | All supported application and I/O examples, as separate images |
| `example.app.blink`, `example.app.tmp102` | Individual example builds |
| `example.io.blink`, `example.io.serial_echo`, `example.io.spi_dma`, `example.io.tmp102`, `example.io.w5100` | Standalone I/O firmware for the UNO board binding |
| `test.board` | Configure and build the board's instrumented physical-test images |
| `test.backend`, `test.device` | Build and run backend or device host suites |
| `test.core`, `test.driver`, `test.io`, `test.mcu`, `test.compile` | Build and run focused host suites and compile contracts |
| `tests` | All host groups plus physical-test image builds |

An empty `MCUHW_BOARD` provides no board targets. Invalid boards, revisions,
architectures and undeclared firmware applications fail during configuration.
Unselected manifests are not scanned. Test dependencies are resolved only when
`test.board` is built, so application builds need no mcutest, MCUIP or coverage setup.
Firmware and additional examples live in `application/` and `examples/` beneath
the architecture build directory. The child test build lives in
`boards/<vendor>/<board>/<revision>/tests/` and inherits global compiler settings.
Physical-test images are built without running or flashing them; use the
documented hardware runners. Native groups run CTest after building.
For UNO runners, point `MCUHW_BUILD_DIR` at the child test build. Nucleo runners
accept an explicit ELF path from that directory.

ESP boards require the installed SDK and Python environment described in
[the build guide](../getting-started/building.md). Each application has its own
SDK project and build directory. `test.board` builds the existing instrumented SDK
project.

Host groups use `MCUHW_NATIVE_CXX_COMPILER` and `MCUHW_NATIVE_CXX_FLAGS` in
separate builds under `native-tests/<group>/`; cross-architecture flags stay in
the board firmware builds. Set `MCUHW_MCUTEST_SOURCE_DIR` for a non-default
mcutest checkout. Each group resolves dependencies only when selected.

Explicit multi-board maintainer configurations remain available through
`MCUHW_WORKSPACE=ON` and `MCUHW_ACTIVE_BOARDS`; they are not preset defaults.
They expose vendor-qualified groups and retain their isolated board outputs.

### Native and compile verification

Enable native suites independently of the preset:

```sh
cmake --preset native -DMCUHW_BUILD_TESTS=ON -DMCUHW_TEST_SUITES=device:ti/tmp102
cmake --build --preset native --target native-tests
ctest --test-dir build/native --output-on-failure
```

For all suites select `MCUHW_TEST_SUITES=all`. Enable tooling tests with
`MCUHW_BUILD_TOOLING_TESTS=ON`, probes with `MCUHW_BUILD_PROBES=ON`, header
checks with `MCUHW_CHECK_HEADERS=ON`, and optional MCUIP contracts with
`MCUHW_BUILD_MCUIP_CONTRACTS=ON` together with probes. Presets do not reset these
options. Use separate build directories for different verification scopes.

```sh
cmake --preset arm-cortex-m4 -B build/arm-checks -DMCUHW_BUILD_PROBES=ON -DMCUHW_CHECK_HEADERS=ON
cmake --build build/arm-checks --target check-headers check-compile
```

`check-headers` compiles one generated translation unit per header inside one
object library, with unity builds and precompiled headers disabled. Each native
suite remains selectable in Test Explorer and has its own result artifact.
Target folders group board tasks and verification suites in supporting IDEs.
Use `tools/verify.py --build build/host --existing` to refresh the merged native
artifact after explicitly configuring that directory for native verification.

### Historical target counts

Before presets were limited to architecture selection, the local macOS arm64
run with CMake 4.4.0 recorded these project targets through
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
