# SPI slave with caller-owned buffers

Connect an external master to P0.06 SCK, P0.08 MOSI, P0.07 MISO, P0.12 active-low CSN and common GND. Use 3.3 V signaling, mode 0, MSB first and at most 8 MHz; begin slowly. Clock one eight-byte transaction, then deassert CSN. The reply is `00 55 aa ff 53 50 49 53`.

The semaphore grants one transaction. Additional clocks return ORC and set status flags; subsequent ungranted CSN transactions return DEF without reusing RAM. A ten-second deadline cancels. A CSN held low may prevent cancellation; claims remain held until a later semaphore handover.

Build without uploading:

```sh
cmake -S example/spis-target -B /tmp/spis-target -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=others/promicro_nrf52840 -DMCUBSP_SOURCE_DIR="$PWD"
cmake --build /tmp/spis-target
```

For an installed package, use its `share/mcubsp/cmake/board-toolchain.cmake`, set `CMAKE_PREFIX_PATH` to the package prefix and omit `MCUBSP_SOURCE_DIR`. The application uses supplied startup and the UF2/S140-v6 flash layout.

This example has software build evidence only. No image upload, peer result or independent wire-timing measurement has been recorded. See the [serial acceptance record](../../doc/acceptance/nrf52840-serial.md).
