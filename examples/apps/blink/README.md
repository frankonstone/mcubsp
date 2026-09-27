# Blink

This complete application toggles the selected profile's LED every 500 ms.
Copy this directory outside the repository, then build with ordinary CMake:

```sh
cmake -S . -B build-uno -G Ninja \
  -DMCUHW_SOURCE_DIR=/path/to/mcuhw \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/mcuhw/cmake/McuhwToolchain.cmake \
  -DMCUHW_BOARD=arduino/uno_r4_minima
cmake --build build-uno
```

Use a different build directory for every board/revision/toolchain. For an
installed package, omit `MCUHW_SOURCE_DIR`, set `CMAKE_PREFIX_PATH` to the install
prefix, and use its `share/mcuhw/cmake/McuhwToolchain.cmake`. CMake resolves the
manifest's default revision; pin it explicitly with `MCUHW_BOARD_REVISION`.

| Board ID | Output | Supplied profile |
| --- | --- | --- |
| `arduino/uno_r4_minima` | Built-in LED, D13 | HOCO 48 MHz; application at `0x4000` preserves Arduino bootloader |
| `st/nucleo_h753zi` | Yellow user LED | HSI 64 MHz; application at `0x08000000` |
| `waveshare/esp32p4_nano` | **External LED** on GPIO2 through a series resistor to GND | Development silicon v1.0 only, HP CPU0 at 360 MHz |

ESP32-P4 additionally requires the pinned ESP-IDF SDK and Python environment:
pass `-DMCUHW_IDF_PATH=/path/to/esp-idf -DPYTHON=/path/to/idf/python`.
If the SDK's tools cache is outside its default location, also pass
`-DMCUHW_IDF_TOOLS_PATH=/path/to/idf-tools-cache` so dependency checks use the
pinned constraints file.
The SDK subbuild is `build/esp-idf-build`; the outer build copies its
`firmware.elf` and `firmware.bin`. The supplied factory-only second-stage
bootloader retains SDK image/cache/libc initialization and enters the application
before the FreeRTOS scheduler. It rejects other silicon revisions or CPU clocks.
Build success does not establish physical verification.

Ninja builds record selected source dependencies in `firmware.identity.json`.
ESP32-P4 additionally records the separate bootloader ELF's source dependencies
in `firmware.bootloader.identity.json`, and the exact flashed image bytes,
offsets and SDK flash settings in `firmware.flash.identity.json`.

Flashing is a separate action. Use the selected board's documented runner and
flash regions. The board component documentation describes wiring and recovery.
For an existing BSP, call `mcuhw_add_firmware(... STARTUP external)` from your
own project and supply your startup sources, linker script/link options and
clock setup. That mode adds no mcuhw reset/vector/clock objects or SDK wrapper.
The stock example deliberately chooses supplied startup.
