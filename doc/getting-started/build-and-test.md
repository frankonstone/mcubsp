# Build and verification

The top-level CMake presets select a board and its toolchain:

```sh
cmake --preset arduino-uno-r4-minima
cmake --build --preset arduino-uno-r4-minima

cmake --preset st-nucleo-h753zi
cmake --build --preset st-nucleo-h753zi

cmake --preset waveshare-esp32p4-nano
cmake --build --preset waveshare-esp32p4-nano
```

The UNO preset builds one image for every `.cpp` in its `example/` and `test/`
directories. It requires discoverable `mcutest`, `mcucov` and `mcujson`
CMake packages. Native host tests use the host presets:

```sh
cmake --preset macos
cmake --build --preset macos
```

Use `linux-x86_64` on Linux x86_64. These commands build and run the host
compiler, core, driver, backend, device, I/O, and MCU suites. They also build
target compile checks using the GNU Arm and ESP32-P4 RISC-V toolchains. They
require mcutest and MCUIP; no test in `test/` runs on a board. To instrument
UNO test images for coverage, configure a separate build directory:

```sh
cmake --preset arduino-uno-r4-minima -B build/arduino-uno-r4-minima-coverage -DMCUBSP_COVERAGE=ON
cmake --build build/arduino-uno-r4-minima-coverage
```

`MCUBSP_COVERAGE` defaults to `OFF`.
Board procedures remain in each board directory, and the ESP32-P4 physical
test image has its own ESP-IDF project at
`board/waveshare/esp32p4_nano/test/firmware/`.
