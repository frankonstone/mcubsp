# UNO R4 examples

Configure this directory to build one example:

```sh
cmake -S . -B build -G Ninja \
  -DMCUHW_SOURCE_DIR=/path/to/mcuhw \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/mcuhw/cmake/McuhwToolchain.cmake \
  -DMCUHW_BOARD=arduino/uno_r4_minima \
  -DMCUHW_IO_EXAMPLE=blink
cmake --build build
```

Choose `blink`, `serial_echo`, `spi_dma`, `tmp102`, or `w5100`. The repository
board build also exposes these through its `examples` target.
The project imports mcuhw with `find_package(mcuhw CONFIG REQUIRED)`. Set
`MCUHW_SOURCE_DIR` to the checkout or installed `share/mcuhw` directory, or
provide an installed package through `CMAKE_PREFIX_PATH`.
