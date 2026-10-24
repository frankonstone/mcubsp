# Compile checks

This directory contains compile-only checks. The host compiler runs positive
and expected-failure cases; the Arm and ESP32-P4 toolchains build architecture
probes. Nothing produced here is executed on an MCU.

## CMake files

| File | Role |
| --- | --- |
| `CMakeLists.txt` | Registers host compiler cases with CTest, builds positive object probes, defines the host `check-compile` target, and starts one separate CMake build per target toolchain. |
| `CompileCase.cmake` | Invoked by each host compiler CTest case. Runs the compiler with `-fsyntax-only` and checks whether compilation succeeds or fails as expected. |
| `target/CMakeLists.txt` | Entry point for each separate cross-compiler build. Loads `Probes.cmake` with the selected toolchain. |
| `Probes.cmake` | Defines cross-compiled object probes and the cross-build `check-compile` target, including barrier verification. |
| `VerifyBarrierInstructions.cmake` | Disassembles the barrier probe and checks for the expected Arm or RISC-V instructions. |

## How they are called

The top-level `host-tests` target depends on `target-compile-probes`. That
target is defined in this directory's `CMakeLists.txt`; it configures
`target/CMakeLists.txt` three times, using the Cortex-M4, Cortex-M7, and
ESP32-P4 toolchain files. A separate CMake build directory is needed for each
compiler. Each cross build runs its own `check-compile` target. The host build
then runs all CTest entries, including the cases handled by `CompileCase.cmake`.

```sh
cmake --preset macos
cmake --build --preset macos
```

Use `linux-x86_64` on Linux. To run only one part after configuring:

```sh
cmake --build --preset macos --target target-compile-probes
cmake --build --preset macos --target check-compile
```

The cross-build outputs are under
`build/<preset>/test/compile/target-probes/<architecture>/`. Each cross build
writes a barrier verification report there.
