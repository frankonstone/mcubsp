# WIZnet W5100


Use the linked UNO Ethernet Shield R3 wiring procedure. The device owns protocol state; the board owns SPI pins, chip select, reset and SD deselection. Other boards must supply their own explicit wiring.

## Implementation and focused checks

Public implementation: `include/mcuhw/device/ethernet/w5100.hpp`. Required transports: spi.

Focused test sources:

- `test/device/test_w5100.cpp`
- `test/device/test_w5100_io.cpp`

See [the device contribution guide](../../../doc/development/adding-a-device.md) for checks.

## Limits

W5100 SPI and MACRAW mode only; W5100S/W5500 support is not implied.

Requires board-owned chip-select/reset and optional shield SD deselection. See exact historical fixture report.
