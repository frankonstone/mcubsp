# mcubsp documentation

This directory contains the authored documentation for `mcubsp`. Start with the
repository [README](../README.md) for the project overview and native build workflow.

For library users, start with [your application's integration boundary](getting-started/application.md)
and the [standalone starter project](../example/blink/README.md).

## Getting started

- [Installation and dependencies](getting-started/installation.md)
- [Your application: library or supplied board integration](getting-started/application.md)
- [Repository board demos and detailed build settings](getting-started/building.md)
- [Flash planning and physical capture](getting-started/hardware.md)
- [Maintainer build and verification](getting-started/build-and-test.md)
- [Using mcubsp in firmware](getting-started/using-mcubsp.md)
- [The `mcubsp::io` application interface](getting-started/io.md)
- [Attached I2C/SPI devices and reusable components](getting-started/components.md)

## Concepts

- [Architecture and layer boundaries](concept/architecture.md)
- [Bare-metal application integration](concept/application-integration.md)
- [Register and metadata model](concept/register-model.md)
- [Execution and transfers](concept/execution-and-transfers.md)

## Examples

The [examples index](../src/board/arduino/uno_r4_minima/doc/example/README.md) orders the examples from GPIO through
asynchronous peripheral and network use. Each page contains its code, wiring,
lifecycle, and bring-up guidance directly in the documentation.

## Boards

- [Pro Micro nRF52840, initial GPIO / UF2](../src/board/others/promicro_nrf52840/README.md)

- [Arduino UNO R4 Minima](../src/board/arduino/uno_r4_minima/doc/README.md)
  - [W5100 Ethernet](../src/board/arduino/uno_r4_minima/doc/ethernet-w5100.md)
- [NUCLEO-H753ZI](../src/board/st/nucleo_h753zi/doc/README.md)
- [ESP32-P4-NANO](../src/board/waveshare/esp32p4_nano/doc/README.md)

## Development

- [Repository ownership and source migration](development/structure.md)
- [Adding a board](development/adding-a-board.md)
- [Adding an external device](development/adding-a-device.md)
- [Adding a microcontroller](development/adding-an-mcu.md)
- [Focused tests, CI and scaling checks](development/verification.md)
- [Selected firmware inputs and provenance](development/firmware-inputs.md)
- [Description coverage](development/coverage.md)
- [Peripheral implementation notes](development/peripheral-note/)

## Reference and acceptance

- [Pro Micro nRF52840 software verification and physical Blink observation](acceptance/promicro-nrf52840.md)

- [Support and evidence matrix](reference/support-matrix.md)
- [MCU source baseline](reference/source-baseline.md)
- [Pinned source manifest](reference/source-manifest.json)
- [MCU description reviews](reference/mcu-review/)
- [Three-MCU description gate](acceptance/description-gate.md)
- [Framework usability repair](acceptance/usability-repair.md)
- [Capture tooling and native coverage restoration](acceptance/capture-and-coverage.md)
- [Selected firmware input records](acceptance/selected-firmware-inputs.md)
- [Library-user CMake boundary](acceptance/application-cmake.md)
- [Public board tree consolidation](acceptance/board-layout.md)
- [Completed BSP directory layout](acceptance/bsp-layout.md)
- [RA4M1 release acceptance](acceptance/ra4m1-release.md)
- [Repository adoption and scale qualification](acceptance/repository-adoption.md)

## Local source material

`_vendor/` contains locally pinned vendor PDFs. `_generated/` contains
searchable Markdown extracted from those PDFs. Both directories are ignored by
Git. Their identities and checksums remain tracked in
[`reference/source-manifest.json`](reference/source-manifest.json), and the generated copies are local historical aids. The former conversion
frontend was removed; use the original pinned PDFs for authoritative review.

Do not derive register values or pin mappings from extracted text without
checking the corresponding vendor PDF.
