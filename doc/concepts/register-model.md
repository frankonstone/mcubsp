# Register and metadata model

PLN-0003 defines reusable metadata shapes and safe access primitives. The
examples in `tests/support/source_backed_examples.hpp` are deliberately narrow pressure
tests, not complete MCU descriptions and not support claims for any peripheral.
Complete source-backed descriptions remain in PLN-0004, PLN-0014 and PLN-0015.

## Register and access differences

- RA4M1 `PmnPFS`, `PmnPFS_HA`, and `PmnPFS_BY` demonstrate 32-, 16-, and
  8-bit aliases of one pin register, a read-only `PIDR` field inside a mixed
  register, pin-dependent reset knowledge, reserved bits, and a prohibited
  `P408` drive encoding. Source: R01UH0887EJ0110 Rev.1.10, section 19.2.5,
  [PDF pages 361-362](../_generated/manuals/ra4m1/pages-0351-0400.md#pdf-page-361).
- STM32H753 `GPIOx_BSRR` demonstrates a 32-bit atomic write command rather
  than a generic RMW operation. `EXTI_CPUPR1` demonstrates an undefined reset,
  reserved upper bits, and write-one-to-clear pending flags. Sources: RM0433
  Rev.8, sections 11.3.5/11.4.7 and 20.6.21,
  [PDF page 783](../_generated/manuals/rm0433/pages-0751-0800.md#pdf-page-783).
- ESP32-P4 `GPIO_OUT_W1TS_REG` and `GPIO_OUT_W1TC_REG` demonstrate separate
  write-trigger set/clear aliases with opaque native access semantics. Source:
  ESP32-P4 TRM v1.0 preliminary, section 10.19 and registers 10.1-10.3,
  [PDF pages 1018 and 1026-1027](../_generated/manuals/esp32-p4/pages-1001-1050.md#pdf-page-1018).

The host access tests use local fixed storage. Numeric target addresses are
trace keys only and are never dereferenced.

## Topology differences retained by the model

- Pin routes carry fixed, alternate-function, or programmable-matrix kind plus
  an opaque vendor encoding. This accommodates RA4M1 PFS selection, STM32
  alternate functions, and the ESP32-P4 GPIO matrix without equating them.
- Clock edges permit multiple selectable parents and distinguish integer from
  fractional division. ESP32-P4 peripherals with fractional numerator and
  denominator fields therefore do not have to mimic Arm clock trees.
- Interrupt routes distinguish fixed CPU lines, selectable event links, and an
  interrupt matrix. RA4M1 IELSR event selection and ESP32-P4 interrupt-matrix
  routing remain different representations.
- DMA engines and memory reachability are explicit. RA4M1 DMAC and DTC, STM32
  DMA/BDMA domains, and ESP32-P4 GDMA-AHB/GDMA-AXI remain distinct engine kinds.

The metadata IDs used by tests are local labels, not undocumented vendor
encodings. Actual selector values, package positions, limits, and address maps
will only enter the MCU descriptions after their source tables are reviewed.
