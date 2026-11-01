# Verification

Use the [build-and-test guide](../getting-started/build-and-test.md) for executable
commands. CMake/CTest are the supported maintainer entry points. The former
Python verification frontend, catalog and workspace helpers were removed.

The host build runs native suites and positive/negative compiler cases. It has
no cross-compiler or MCUIP requirement by default. `MCUBSP_CHECK_HEADERS=ON`
compiles each public header independently, with unity and precompiled headers
disabled. Negative cases must fail with their expected diagnostic category;
an unrelated missing header cannot satisfy a static-constraint test.

`mcubsp.test.build` checks source consumers, parent-owned tests, library-only
installation, package relocation, board diagnostics and false compiler
rejections. Temporary paths contain spaces. These tests use Python 3.10+,
CMake, Ninja and the selected native compiler; they do not fetch dependencies.

The [CI workflow](../../.github/workflows/verification.yml) runs native/header/
consumer checks on Linux, macOS and Windows, plus linked Blink applications and
architecture/header checks for Cortex-M4, Cortex-M7 and P4. Native checks use a
pinned mcutest checkout. Arm jobs use the checksum-pinned Arm toolchain; P4 uses
the pinned ESP-IDF container. A workflow definition is not evidence that the
hosted job has executed. MCUIP checks belong to the sibling
[integration project](../../../integration/README.md), with independently selected
mcubsp and MCUIP dependencies.

## Trace evidence

`mcutrace.toml` imports eight reports under `output/artifact/native/`. The `host-tests` target
merges only the producer files declared by the configured test registry;
unregistered stale results cannot enter it. Individual reports remain under
`output/build/<preset>/artifact/native/`. The merged report records their SHA-256
identities. Build `macos-arm64` or `linux-x86_64` before loading the analyzed graph
on a clean checkout; the server requires its declared artifact files to exist.
Each native build retains its own merged report under `artifact/native.json` and
publishes the combined `output/artifact/native.json` plus eight trace reports after
its tests pass. Stable hashing of the original prefixed test names distributes
every case exactly once; identifiers, requirement tags and producer hashes are
preserved. This avoids the trace importer's fixed JSON string arena limit without
dropping tests. Local trace reports live under `artifact/native-trace/`; coverage
graph export uses those reports from its own instrumented build.
The shared reports represent the latest successful native build, without combining platforms.
They are invalidated before native tests run so a failed test run cannot leave old canonical
results. Compiler checks remain CTest results; native
JSON files contain the actual behavior-test results.

Historical physical and coverage paths removed during the build migration are
not imported as current evidence. Their reports and observation limits remain
indexed in the acceptance documents. Missing coverage/target evidence remains
visible in mcutrace diagnostics. Validation reads existing artifacts; it never
runs a producer or makes an old hardware result current.

## Native coverage

The optional `MCUBSP_HOST_COVERAGE=ON` instruments all native suites with MCUCOV
FULL and captures binary data separately from mcutest JSON. It requires an
installed host mcucov package and enables the `host-coverage` target. The locally
verified macOS configuration uses AppleClang in a fresh build directory:

```sh
cmake --preset macos-arm64 -B output/build/native-coverage -DMCUBSP_HOST_COVERAGE=ON -DMCUBSP_CHECK_HEADERS=OFF -Dmcucov_DIR=/path/to/host/mcucov/lib/cmake/mcucov
cmake --build output/build/native-coverage --target host-coverage --parallel 2
```

Native GCC/macOS header analysis is rejected during configuration with guidance
to use AppleClang. Ordinary GCC verification remains supported. Other coverage
host/compiler combinations need their own qualification.

`artifact/coverage/` contains the merged capture, JSON, standalone HTML, LCOV,
producer hashes and a generated `mcutrace.toml`. The latter imports that
instrumented build's native results and coverage without making coverage a
prerequisite for the ordinary graph. To inspect it with the mcutrace CLI:

```sh
mcutrace --config output/build/native-coverage/artifact/coverage/mcutrace.toml validate
```

The denominator is the source modules/variants selected by the native suites,
including their test code. It is not whole-device or electrical coverage.
Skipped/unsupported constructs and invalid MC/DC decisions remain in the report;
no exclusions are silently counted as covered. Conditional constructor checks
use named concepts so runtime decision instrumentation cannot enter their
constant-expression constraints.

Shared physical-runner planning, bundle integrity and serial-capture tests run
in `mcubsp.test.runner`. See [hardware procedures](../getting-started/hardware.md).

Selected-source, transitive-library and build-configuration identity regressions
run in `mcubsp.test.provenance`. See [firmware input identity](firmware-inputs.md)
for the recorded scope and generator/toolchain limits.
