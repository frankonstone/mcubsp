# Installation and dependencies

`mcuhw` is currently a header-only CMake interface library. There is no
system-wide `cmake --install` package yet; use a checkout directly or vendor
the `include/mcuhw` tree.

## Native development

Install CMake 3.25 or newer, Ninja, and a C++23 compiler. The host test suite
also needs the sibling `mcutest` project. By default, the development projects
are arranged as:

```text
workspace/
├── mcuhw/
├── mcutest/
├── mcujson/
├── mcucov/
└── mcuip/
```

The dependency locations can be overridden with
`MCUHW_MCUTEST_SOURCE_DIR`, `MCUHW_MCUJSON_SOURCE_DIR`,
`MCUHW_MCUCOV_SOURCE_DIR`, and `MCUHW_MCUIP_SOURCE_DIR`.

For cross-builds, install `arm-none-eabi-g++` for Cortex-M or activate an
ESP-IDF environment containing `riscv32-esp-elf-g++` for ESP32-P4. Hardware
runners have additional debugger and command-line requirements described by
the corresponding [board guide](../README.md#boards).

Continue with [Build and verification](build-and-test.md).
