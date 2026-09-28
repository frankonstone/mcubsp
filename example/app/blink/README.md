# Blink

This UNO R4 Minima application toggles the built-in D13 LED every 500 ms.
Copy this directory outside the repository, then build with ordinary CMake:

```sh
cmake -S . -B build-uno -G Ninja \
  -DMCUHW_SOURCE_DIR=/path/to/mcuhw \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/mcuhw/cmake/McuhwToolchain.cmake \
  -DMCUHW_BOARD=arduino/uno_r4_minima
cmake --build build-uno
```

The supplied profile uses a 48 MHz HOCO clock and starts the application at
`0x4000`, preserving the Arduino bootloader. Flashing is a separate action.

The board choice is explicit in `main.cpp`: its board-header include and
`io::board` template argument name the UNO R4 profile. To port the example,
change those two lines and the indicator pin, then declare the example in the
new board's manifest and update the CMake board check. The new board must also
provide a compatible startup and clock-status contract for `hw.startup_ok()`.
For an existing BSP, use `mcuhw_add_firmware(... STARTUP external)` with your
own startup sources, linker settings and clock setup.
