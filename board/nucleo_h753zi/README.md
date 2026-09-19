# NUCLEO-H753ZI bare-metal binding

This binding targets the STM32H753ZI in the NUCLEO-H753ZI MB1364 revision E
assembly. MCU metadata remains under `include/mcuhw/mcu/stm32h753`; connector
aliases and test-image choices are board-specific.

## Sources and received configuration

- ST UM2407 Rev. 6, *STM32H7 Nucleo-144 boards*: board assembly, ST-LINK,
  virtual COM, power, LEDs, connectors, Ethernet and USB.
- ST MB1364-H753ZI-E01 schematic pack: board nets and assembly options.
- ST's `stm32h7xx-nucleo-bsp` at CubeH7-pinned revision
  `5186a1cd73efcf92f78b098961ff098fb87e5eef`: USART3 VCP on PD8/PD9 and the
  H753ZI LED assignments.
- ST's `Arduino_Core_STM32` `variant_NUCLEO_H753ZI.cpp`: D10=PD14,
  D11=PB5, D12=PA6, D13=PA5, D14=PB9, D15=PB8, and A0=PA3.

The board was tested in its received configuration, powered and debugged over
the ST-LINK USB connector. No jumper or solder bridge was moved, opened or
closed. Consequently there is no restoration action. The successful PD8/PD9
VCP test confirms the received VCP path is connected, but is not presented as
a complete audit of every factory assembly option.

## Reproducible tests

Build both images:

```sh
cmake --preset nucleo-h753zi
cmake --build build/nucleo-h753zi -j4
```

Run reset/clock/GPIO bring-up:

```sh
./tools/run-nucleo-h753zi-smoke.sh
```

Run the native peripheral image:

```sh
./tools/run-nucleo-h753zi-native.sh
```

The native runner flashes with ST-LINK, runs eight requirement-tagged mcutest
cases, extracts the mcucov frame from cache-cleaned AXI SRAM, checks the result
block through SWD, and reads the real USART3 banner through the ST-LINK VCP. It
verifies:

- HSI 64 MHz startup and the flash/vector/RAM path;
- built-in PE1 LED set/read/clear through the common GPIO driver;
- TIM1 channel 1 configuration at 20 kHz and 25% duty by register readback;
- USART3 at 115200 baud and the physical PD8/PD9-to-VCP wire path;
- I2C1 and SPI1 clock/pin/controller initialization on the Arduino headers;
- ADC1 calibration and a real 14-bit conversion on floating A0/PA3;
- Cortex-M7 D-cache enable, a source buffer clean by address, DMA1 AXI-SRAM
  reachability, and SWD readback of the cleaned backing memory.

The all-backend image uses fixed operation pools of capacity one and no heap.
Timing solvers are evaluated at compile time in the image. The latest measured
linked image is reported by `arm-none-eabi-size` during every build; numeric
release budgets remain the project-level REQ-0037 acceptance decision.

The latest instrumented run passed 8/8 tests and captured 4,255 coverage bytes.
The target-specific report includes early startup, clock, GPIO, memory/cache,
TIM/PWM, USART, I2C, SPI and ADC modules: 96/171 functions, 512/1062 statements,
564/1061 lines, 185/602 branches and 19/502 MC/DC conditions. Forty-six
constant-evaluated or unsupported constructs are explicitly reported as not
instrumented. The reproducible artifacts are written to:

- `build/nucleo-h753zi/artifacts/mcuhw_nucleo_h753zi_native_peripherals.json`;
- `build/nucleo-h753zi/artifacts/mcuhw_nucleo_h753zi_native_peripherals.mcucov.json`;
- `build/nucleo-h753zi/artifacts/mcuhw_nucleo_h753zi_native_peripherals.mcucov.html`;
- `build/nucleo-h753zi/artifacts/coverage.lcov`.

The `mcucov Coverage` VS Code extension loads the LCOV report into the native
Test Coverage view. Run `mcucov: Refresh Coverage` if the view was already open
when the report was generated.

## Observation limits and deferred rows

Register readback proves configuration, not external timing. Floating A0 proves
the converter operates, not ADC accuracy. These cases remain for PLN-0022 once
the adapter supplies independent stimulus or observation:

- independently captured PWM frequency/duty and endpoint behavior;
- calibrated ADC levels, reference/linearity checks and analog fault cases;
- translated/shared-bus I2C and SPI targets, protocol capture and injected
  NACK/arbitration/overrun conditions;
- peripheral DMA wire paths and completion-order observation;
- supervisor-driven GPIO, timeout, contention and electrical-limit cases.

No temporary point-to-point jumper was installed for this run. A D11-PB5 to
D12-PA6 SPI loopback and controlled A0 ties are safe pre-adapter procedures but
remain unexecuted until their wiring is installed and recorded. On-board
Ethernet PHY/MAC/raw-frame and native USB work do not require the adapter, but
are outside the agreed GPIO/PWM/UART/I2C/SPI/ADC backend set closed by
PLN-0018; they are tracked as the separate pre-adapter PLN-0023 milestone.
