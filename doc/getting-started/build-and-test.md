# Build and verification

This page is for contributors testing mcubsp itself. Library users should start
with [their application's build](application.md); these test dependencies and
repository presets are not required just to consume mcubsp.

The native presets need CMake 3.25+, Ninja, Git, Python 3.10+, a C++23 compiler
with matching standard library, and mcutest. It defaults to `../mcutest`; pass
`-DMCUBSP_MCUTEST_SOURCE_DIR=/path/to/mcutest` or supply its installed package.

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64 --parallel 4
```

The build compiles isolated public headers, native suites and positive probes,
then runs CTest, including compiler rejection and external-consumer checks.
Use `macos-arm64` on Apple Silicon macOS, as above, or replace it with
`linux-x86_64` on Linux x86_64, including in build paths and CTest commands. Both
presets also support installation with `cmake --install output/build/<preset>`.
Select a compiler
with `-DCMAKE_CXX_COMPILER=g++-15` on the first configuration. Cross compilers,
MCUIP and coverage tools are unnecessary for native verification.

After building, run focused tests using labels:

```sh
ctest --test-dir output/build/macos-arm64 -L backend --output-on-failure
ctest --test-dir output/build/macos-arm64 -L io --output-on-failure
ctest --preset macos-arm64
```

Native reports are under `output/build/macos-arm64/artifact/native/`. CTest has the complete
compile/build-test log under `output/build/macos-arm64/Testing/Temporary/LastTest.log`.
Successful builds also publish the merged trace report to `output/artifact/native.json`.

## Optional checks

| Option/target | Purpose |
| --- | --- |
| `MCUBSP_BUILD_HOST_TESTS=ON` | Native behavior and compiler checks |
| `MCUBSP_CHECK_HEADERS=ON` | One independent translation unit per public header |
| `MCUBSP_BUILD_TARGET_PROBES=ON` | Include all three cross-compiler probe groups in the default host build |
| `target-probes-cortex-m4`, `target-probes-cortex-m7`, `target-probes-esp32p4-riscv` | Explicit individual architecture checks |
| `MCUBSP_BUILD_BOARD_TESTS=ON` | Opt-in UNO/NUCLEO physical-test image builds with mcutest/mcujson sources or packages, plus mcucov |
| `MCUBSP_COVERAGE=ON` | Instrument selected UNO/NUCLEO test images |

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64 --target target-probes-cortex-m4
```

For cross checks without any native test dependency:

```sh
cmake -S test/compile/target -B output/build/arm-checks -G Ninja -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/toolchain-arm-cortex-m4.cmake" -DMCUBSP_CHECK_HEADERS=ON
cmake --build output/build/arm-checks --target check-compile
```

[Board application builds](building.md) create linked firmware. No test in
`test/` runs on a target or flashes a device. Physical test procedures and their
current limitations are listed in the [support matrix](../reference/support-matrix.md).

Optional board tests default to sibling mcutest and mcujson checkouts;
`MCUBSP_MCUTEST_SOURCE_DIR` and `MCUBSP_MCUJSON_SOURCE_DIR` select explicit paths. When using installed mcutest/
mcujson packages instead, configure those header-only packages with the target
compiler so their CMake architecture checks match the firmware. Coverage uses a
host-installed mcucov tool package and its target runtime.

UNO report images explicitly select hosted standard-library headers for the
mcujson reporter (`HOSTED` in the firmware helper); restricted runtime flags and
fixed storage remain enabled. Ordinary firmware keeps `-ffreestanding`.

Native coverage uses `MCUBSP_HOST_COVERAGE=ON` and the `host-coverage` target;
see [coverage verification](../development/verification.md#native-coverage).
The [hardware guide](hardware.md) covers current runner planning and capture.

MCUIP contract probes and combined Ethernet images are built by the sibling
[integration project](../../../integration/README.md). mcubsp has no MCUIP build
option or dependency, including for its native and board tests.
