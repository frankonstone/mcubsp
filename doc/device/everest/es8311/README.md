# Everest ES8311


The initial physical composition is the on-board ESP32-P4-NANO codec. The linked board procedure specifies I2C address 0x18, GPIO wiring, clocks, microphone and amplifier sequencing. Other boards must supply and verify their own wiring.

## Implementation and focused checks

Public implementation: `include/mcubsp/device/audio/es8311.hpp`. Required transports: i2c, i2s.

Focused test sources:

- `test/backend/esp32p4/test_i2s.cpp`

See [the device contribution guide](../../../doc/development/adding-a-device.md) for checks.

## Limits

Bounded 48 kHz, stereo 16-bit Philips I2S startup register sequence with 12.288 MHz MCLK; not a generic codec mode solver.

Board integration owns reset, analog power/amplifier sequencing, clocks and I2S transport. No acoustic accuracy claim.
