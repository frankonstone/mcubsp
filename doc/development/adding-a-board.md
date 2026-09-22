# Adding a board

Use this workflow when the MCU description and required peripheral backends
already exist.

1. Pin the exact board revision, schematic, connector pinout, oscillator,
   memory, debugger, bootloader, jumper, and solder-bridge assumptions.
2. Add `include/mcuhw/board/<board_name>.hpp` with the exact MCU/package and
   board-level connector aliases. Do not duplicate MCU register facts there.
3. Add `board/<board_name>/` with startup, linker script, restricted runtime,
   execution hooks, interrupt dispatch, and focused physical test images.
4. Bind existing backends to valid peripheral instances and board pins.
   Invalid routes must fail at compile time.
5. Add a CMake preset, board build option, and reproducible runner. Record the
   board, firmware, toolchain, fixture, and observation method in evidence.
6. Verify the binding, cross-build every public header and linked image, then
   run the board-native and directly connected tests.
7. Add or update requirements and a plan item, tag implementations/tests, add
   evidence artifacts to `mcutrace.toml`, and document explicit non-claims.

Use the existing UNO, NUCLEO, and ESP32-P4 directories as concrete patterns,
but re-check every pin and assembly choice against the new board's own sources.
