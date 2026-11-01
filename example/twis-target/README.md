# I²C target with explicit clock stretching

Connect an external single master to P0.11 SCL, P1.00 SDA and common GND, with suitable pull-ups to 3.3 V. The target listens on unshifted addresses 0x42 and 0x43. Use 100 kHz initially and a master that permits clock stretching. The fixed eight-byte reply is `00 55 aa ff 54 57 49 53`.

The CPU observes every READ/WRITE address request before explicitly resuming. One write and one read, in either order, may share a repeated START; finish with STOP. Repeating a consumed direction requires a new listen operation. Error flags and actual counts are exposed in `mcubsp_twis_*`. A ten-second deadline cancels. Unknown/Engineering A silicon is explicitly rejected pending anomaly-83 qualification.

Build without uploading:

```sh
cmake -S example/twis-target -B /tmp/twis-target -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=others/promicro_nrf52840 -DMCUBSP_SOURCE_DIR="$PWD"
cmake --build /tmp/twis-target
```

For an installed package, use its `share/mcubsp/cmake/board-toolchain.cmake`, set `CMAKE_PREFIX_PATH` to the package prefix and omit `MCUBSP_SOURCE_DIR`. The application uses supplied startup and the UF2/S140-v6 flash layout.

This example has software build evidence only. No image upload, peer result or independent wire-timing measurement has been recorded. See the [serial acceptance record](../../doc/acceptance/nrf52840-serial.md).
