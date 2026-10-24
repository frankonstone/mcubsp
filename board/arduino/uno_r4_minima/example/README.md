# UNO R4 examples

Configure this directory to build one example:

```sh
cmake -S . -B build -G Ninja \
  -DMCUBSP_SOURCE_DIR=/path/to/mcubsp \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/mcubsp/cmake/McubspToolchain.cmake \
  -DMCUBSP_BOARD=arduino/uno_r4_minima \
  -DMCUBSP_EXAMPLE=blink
cmake --build build
```

Choose `blink`, `serial_echo`, `spi_dma`, `tmp102`, or `w5100`. The repository
board build also exposes these through its `examples` target.
The project imports mcubsp with `find_package(mcubsp CONFIG REQUIRED)`. Set
`MCUBSP_SOURCE_DIR` to the checkout or installed `share/mcubsp` directory, or
provide an installed package through `CMAKE_PREFIX_PATH`.

The `tmp102` example reads I2C address `0x48` and lights the built-in LED
above 25 °C. Connect SDA to A4 and SCL to A5 with a common ground. Power the
sensor from 3.3 V and use pull-ups and level shifting appropriate to its
supply; tie ADD0 to ground for address `0x48`.
