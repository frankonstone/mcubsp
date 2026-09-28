# TMP102

This UNO R4 Minima application reads a TMP102 at I2C address `0x48` and lights
the built-in D13 LED when the sample exceeds 25 °C.

```sh
cmake -S . -B build-uno -G Ninja \
  -DMCUHW_SOURCE_DIR=/path/to/mcuhw \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/mcuhw/cmake/McuhwToolchain.cmake \
  -DMCUHW_BOARD=arduino/uno_r4_minima
cmake --build build-uno
```

Connect SDA to A4 and SCL to A5, with common ground. Supply the TMP102 from
3.3 V and use suitable pull-ups and level shifting where required; do not pull
its pins above its supply. Tie ADD0 to GND for address `0x48`.

The board choice is explicit in `main.cpp`. To port the example, change its
board-header include, `io::board` template argument and indicator pin, then
declare the example in the new board's manifest and update the CMake board
check. The new board must provide an I2C binding and a compatible startup
contract for `hw.startup_ok()`.
