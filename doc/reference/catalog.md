# Generated hardware catalog

Regenerate from component manifests; do not edit this output.

Capabilities describe implemented contracts. They do not imply a physically tested composition. Each evidence record retains its revision, scope and effective state; historical observations apply only to the named firmware and fixture.

| Component | Revisions | Category | Implemented / required capabilities | Build / runner |
| --- | --- | --- | --- | --- |
| board:arduino/uno_r4_minima | r4 | development-board | gpio, i2c, spi, uart, pwm, adc / none | cmake/toolchain-arm-cortex-m4.cmake / jlink |
| board:st/nucleo_h753zi | mb1364-rev-e | development-board | gpio, i2c, spi, uart, pwm, adc, ethernet / none | cmake/toolchain-arm-cortex-m7.cmake / openocd |
| board:waveshare/esp32p4_nano | development-v1.0 | development-board | gpio, i2c, spi, uart, i2s, sdmmc, ethernet / none | cmake/toolchain-esp32p4.cmake / esp-idf |
| device:everest/es8311 | 48khz-16bit | audio | audio / i2c, i2s | header-only / none |
| device:ti/tmp102 | datasheet-sbos397i | temperature | temperature / i2c | header-only / none |
| device:wiznet/w5100 | w5100 | ethernet | ethernet / spi | header-only / none |
| ip:espressif/esp32p4_native | implementation-v1 | native-backends | gpio, i2c, spi, uart, i2s, sdmmc, ethernet / none | header-only / none |
| ip:renesas/ra4m1_native | implementation-v1 | native-backends | gpio, i2c, spi, uart, pwm, adc, dma / none | header-only / none |
| ip:st/stm32h753_native | implementation-v1 | native-backends | gpio, i2c, spi, uart, pwm, adc, ethernet / none | header-only / none |
| mcu:espressif/esp32p4 | production-v3.x | microcontroller |  / none | header-only / none |
| mcu:renesas/ra4m1 | manual-rev1.10 | microcontroller |  / none | header-only / none |
| mcu:st/stm32h753 | description-v1 | microcontroller |  / none | header-only / none |

## board:arduino/uno_r4_minima

Arduino UNO R4 Minima board wiring and explicit startup integration.

Docs: [README.md](../../board/arduino/uno_r4_minima/README.md), [README.md](../../board/arduino/uno_r4_minima/docs/README.md).
Examples: [CMakeLists.txt](../../examples/apps/blink/CMakeLists.txt), [CMakeLists.txt](../../examples/apps/tmp102/CMakeLists.txt).

Described-only capabilities: none.

- physical **passed**, revision `r4`: Historical UNO R4 Minima fixture runs recorded in the linked report: startup, LED/GPIO, loopback, PWM and UART. Board PCB revision was not recorded; r4 identifies the supported UNO R4 Minima binding, not an inferred PCB revision. ([source](../../board/arduino/uno_r4_minima/docs/README.md))
- cross-build **passed**, revision `r4`: Complete external Blink and TMP102 applications linked on the macOS arm64 qualification host for this exact binding revision. No flashing or physical device acceptance. ([source](../../doc/acceptance/repository-adoption.md))

Limitations: Retains the Arduino DFU bootloader below 0x4000; supplied startup owns 48 MHz HOCO and vectors. The binding revision r4 names the documented board model; the historical records do not establish an exact PCB revision. External TMP102 wiring is an application requirement; no physical TMP102 acceptance is claimed.

## board:st/nucleo_h753zi

ST NUCLEO-H753ZI board wiring and explicit startup integration.

Docs: [README.md](../../board/st/nucleo_h753zi/README.md), [README.md](../../board/st/nucleo_h753zi/docs/README.md).
Examples: [CMakeLists.txt](../../examples/apps/blink/CMakeLists.txt), [CMakeLists.txt](../../examples/apps/tmp102/CMakeLists.txt).

Described-only capabilities: none.

- physical **passed**, revision `mb1364-rev-e`: Historical MB1364 revision E startup/LED/VCP and native-controller observations only. Ethernet PHY/MDIO identity observed with link down; frame exchange remains unexecuted. ([source](../../board/st/nucleo_h753zi/docs/README.md))
- cross-build **passed**, revision `mb1364-rev-e`: Complete external Blink and TMP102 applications linked on the macOS arm64 qualification host for this exact binding revision. No flashing or physical device acceptance. ([source](../../doc/acceptance/repository-adoption.md))

Limitations: MB1364 revision E, received jumper/bridge configuration; supplied startup uses HSI 64 MHz. Ethernet availability is implemented MAC/PHY wiring; a passed Ethernet protocol exchange is not claimed. PWM/ADC/I2C/SPI electrical and external-device acceptance remains limited as documented.

## board:waveshare/esp32p4_nano

Waveshare ESP32-P4-NANO board wiring and explicit startup integration.

Docs: [README.md](../../board/waveshare/esp32p4_nano/README.md), [README.md](../../board/waveshare/esp32p4_nano/docs/README.md).
Examples: [CMakeLists.txt](../../examples/apps/blink/CMakeLists.txt), [CMakeLists.txt](../../examples/apps/tmp102/CMakeLists.txt).

Described-only capabilities: none.

- physical **stale**, revision `development-v1.0`: Historical connected ESP32-P4 silicon v1.0 development board: UART TX, on-board ES8311 I2C/audio digital path, SDMMC/SDIO and Ethernet PHY identity; no production-v3.x acceptance, acoustic analysis or Ethernet frame exchange. ([source](../../board/waveshare/esp32p4_nano/docs/README.md))
- cross-build **passed**, revision `development-v1.0`: Complete external Blink and TMP102 applications linked on the macOS arm64 qualification host for this exact binding revision. No flashing or physical device acceptance. ([source](../../doc/acceptance/repository-adoption.md))

Limitations: development-v1.0 names observed silicon, not an independently established PCB revision. Production v3.x remains outside this board observation scope. Explicit ESP-IDF v6.1 boot path; HP CPU0 only. Existing controller-specific development I2C masks remain board-owned. Ethernet link was down in recorded observation; no network exchange claim.

## device:everest/es8311

Everest ES8311 reusable device protocol/configuration support through declared transport contracts.

Docs: [README.md](../../device/everest/es8311/README.md), [README.md](../../board/waveshare/esp32p4_nano/docs/README.md).
Examples: [audio.cpp](../../board/waveshare/esp32p4_nano/tests/audio.cpp).

Described-only capabilities: none.

- host-test **passed**, revision `48khz-16bit`: Registered protocol/configuration and simulated bus/backend host cases passed in the 241-case native qualification run on macOS arm64. This does not refresh physical evidence. ([source](../../doc/acceptance/repository-adoption.md))
- physical **stale**, revision `48khz-16bit`: Historical ESP32-P4-NANO silicon-v1.0 codec identity, microphone data, digital DMA playback observations; no independent acoustic measurement. ([source](../../board/waveshare/esp32p4_nano/docs/README.md))

Limitations: Bounded 48 kHz, stereo 16-bit Philips I2S startup register sequence with 12.288 MHz MCLK; not a generic codec mode solver. Board integration owns reset, analog power/amplifier sequencing, clocks and I2S transport. No acoustic accuracy claim.

## device:ti/tmp102

TI TMP102 reusable device protocol/configuration support through declared transport contracts.

Docs: [README.md](../../device/ti/tmp102/README.md).
Examples: [tmp102.cpp](../../examples/io/tmp102.cpp), [CMakeLists.txt](../../examples/apps/tmp102/CMakeLists.txt).

Described-only capabilities: none.

- host-test **passed**, revision `datasheet-sbos397i`: Registered protocol/configuration and simulated bus/backend host cases passed in the 241-case native qualification run on macOS arm64. This does not refresh physical evidence. ([source](../../doc/acceptance/repository-adoption.md))
- physical **missing**, revision `datasheet-sbos397i`: No physical TMP102 fixture has been recorded.

Limitations: Reads existing conversions through a claimed I2C endpoint; does not wake a shutdown sensor or trigger a fresh conversion. Physical TMP102 board/device acceptance is missing; bus compatibility does not imply a tested composition.

## device:wiznet/w5100

WIZnet W5100 reusable device protocol/configuration support through declared transport contracts.

Docs: [README.md](../../device/wiznet/w5100/README.md), [ethernet-w5100.md](../../doc/examples/ethernet-w5100.md), [ethernet-w5100.md](../../board/arduino/uno_r4_minima/docs/ethernet-w5100.md).
Examples: [w5100.cpp](../../examples/io/w5100.cpp).

Described-only capabilities: none.

- host-test **passed**, revision `w5100`: Registered protocol/configuration and simulated bus/backend host cases passed in the 241-case native qualification run on macOS arm64. This does not refresh physical evidence. ([source](../../doc/acceptance/repository-adoption.md))
- physical **stale**, revision `w5100`: Historical UNO R4 Minima Ethernet Shield R3 W5100 fixture, before the attached-device refactor. Current throughput and physical acceptance have not been remeasured; see exact observations and exclusions in the report. ([source](../../board/arduino/uno_r4_minima/docs/ethernet-w5100.md))

Limitations: W5100 SPI and MACRAW mode only; W5100S/W5500 support is not implied. Requires board-owned chip-select/reset and optional shield SD deselection. See exact historical fixture report.

## ip:espressif/esp32p4_native

Shared ESP32-P4 backend implementation collection. Board-specific pins, clocks and startup remain outside this component.

Docs: [esp32p4.md](../../doc/reference/mcu-reviews/esp32p4.md).
Examples: none.

Described-only capabilities: none.

- host-test **missing**, revision `implementation-v1`: Backend host-test sources are available; catalog existence does not report a test execution.

Limitations: Capabilities identify implemented interfaces, not arbitrary routes, electrical timing or completed physical acceptance.

## ip:renesas/ra4m1_native

Shared RA4M1 backend implementation collection. Board-specific pins, clocks and startup remain outside this component.

Docs: [ra4m1.md](../../doc/reference/mcu-reviews/ra4m1.md).
Examples: none.

Described-only capabilities: none.

- host-test **missing**, revision `implementation-v1`: Backend host-test sources are available; catalog existence does not report a test execution.

Limitations: Capabilities identify implemented interfaces, not arbitrary routes, electrical timing or completed physical acceptance.

## ip:st/stm32h753_native

Shared STM32H753 backend implementation collection. Board-specific pins, clocks and startup remain outside this component.

Docs: [stm32h753.md](../../doc/reference/mcu-reviews/stm32h753.md).
Examples: none.

Described-only capabilities: none.

- host-test **missing**, revision `implementation-v1`: Backend host-test sources are available; catalog existence does not report a test execution.

Limitations: Capabilities identify implemented interfaces, not arbitrary routes, electrical timing or completed physical acceptance.

## mcu:espressif/esp32p4

ESP32-P4 authoritative part/package/silicon description; shared native implementations are separate IP components.

Docs: [esp32p4.md](../../doc/reference/mcu-reviews/esp32p4.md).
Examples: none.

Described-only capabilities: none.

- host-test **missing**, revision `production-v3.x`: Description test sources are listed; consult current test artifacts for execution results.

Limitations: Described hardware and implemented backend capabilities are separate; package, routes and silicon constraints remain authoritative in C++. Production v3.x description is not physically qualified by the ESP32-P4-NANO development-v1.0 board observations.

## mcu:renesas/ra4m1

RA4M1 authoritative part/package/silicon description; shared native implementations are separate IP components.

Docs: [ra4m1.md](../../doc/reference/mcu-reviews/ra4m1.md).
Examples: none.

Described-only capabilities: none.

- host-test **missing**, revision `manual-rev1.10`: Description test sources are listed; consult current test artifacts for execution results.

Limitations: Described hardware and implemented backend capabilities are separate; package, routes and silicon constraints remain authoritative in C++.

## mcu:st/stm32h753

STM32H753 authoritative part/package/silicon description; shared native implementations are separate IP components.

Docs: [stm32h753.md](../../doc/reference/mcu-reviews/stm32h753.md).
Examples: none.

Described-only capabilities: none.

- host-test **missing**, revision `description-v1`: Description test sources are listed; consult current test artifacts for execution results.

Limitations: Described hardware and implemented backend capabilities are separate; package, routes and silicon constraints remain authoritative in C++.
