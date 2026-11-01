# Bounded UARTE receive stream

Join P0.06 (TX) to P0.08 (RX). The example explicitly selects UARTE0; it sends 48 binary bytes into two caller-owned 16-byte receive buffers, checks each completed prefix and requeues storage. Poll service must keep up with the FIFO. This route has no hardware flow control.

A two-second deadline cancels both directions. `mcubsp_stream_status == 1`, 48 bytes and three blocks indicate firmware success; the high bit marks failure.

Build without uploading:

```sh
cmake -S example/serial-stream -B /tmp/serial-stream -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/board-toolchain.cmake" \
  -DMCUBSP_BOARD=others/promicro_nrf52840 -DMCUBSP_SOURCE_DIR="$PWD"
cmake --build /tmp/serial-stream
```

For an installed package, use its `share/mcubsp/cmake/board-toolchain.cmake`, set `CMAKE_PREFIX_PATH` to the package prefix and omit `MCUBSP_SOURCE_DIR`. The application uses supplied startup and the UF2/S140-v6 flash layout.

This example has software build evidence only. No image upload, peer result or independent wire-timing measurement has been recorded. See the [serial acceptance record](../../doc/acceptance/nrf52840-serial.md).
