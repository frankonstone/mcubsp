# mcuhw documentation

This directory contains the authored documentation for `mcuhw`. Start with the
repository [README](../README.md) for the project overview and five-minute
native build.

## Getting started

- [Installation and dependencies](getting-started/installation.md)
- [First board application: setup, build and flash](getting-started/building.md)
- [Maintainer build and verification](getting-started/build-and-test.md)
- [Using mcuhw in firmware](getting-started/using-mcuhw.md)
- [The `mcuhw::io` application interface](getting-started/io.md)
- [Attached I2C/SPI devices and reusable components](getting-started/components.md)

## Concepts

- [Architecture and layer boundaries](concepts/architecture.md)
- [Bare-metal application integration](concepts/application-integration.md)
- [Register and metadata model](concepts/register-model.md)
- [Execution and transfers](concepts/execution-and-transfers.md)

## Examples

The [examples index](examples/README.md) orders the examples from GPIO through
asynchronous peripheral and network use. Each page contains its code, wiring,
lifecycle, and bring-up guidance directly in the documentation.

## Boards

- [Arduino UNO R4 Minima](../board/arduino/uno_r4_minima/docs/README.md)
  - [W5100 Ethernet](../board/arduino/uno_r4_minima/docs/ethernet-w5100.md)
- [NUCLEO-H753ZI](../board/st/nucleo_h753zi/docs/README.md)
- [ESP32-P4-NANO](../board/waveshare/esp32p4_nano/docs/README.md)

## Development

- [Repository ownership and source migration](development/structure.md)
- [Adding a board](development/adding-a-board.md)
- [Adding an external device](development/adding-a-device.md)
- [Adding a microcontroller](development/adding-an-mcu.md)
- [Catalog schema, contribution and compatibility](development/catalog.md)
- [Focused tests, CI and scaling checks](development/verification.md)
- [Description coverage](development/coverage.md)
- [Peripheral implementation notes](development/peripheral-notes/)

## Reference and acceptance

- [Generated searchable hardware catalog](reference/catalog.md)
- [MCU source baseline](reference/source-baseline.md)
- [Pinned source manifest](reference/source-manifest.json)
- [MCU description reviews](reference/mcu-reviews/)
- [Three-MCU description gate](acceptance/description-gate.md)
- [RA4M1 release acceptance](acceptance/ra4m1-release.md)
- [Repository adoption and scale qualification](acceptance/repository-adoption.md)

## Local source material

`_vendor/` contains locally pinned vendor PDFs. `_generated/` contains
searchable Markdown extracted from those PDFs. Both directories are ignored by
Git. Their identities and checksums remain tracked in
[`reference/source-manifest.json`](reference/source-manifest.json), and the
generated copies can be recreated with:

```sh
python3 tools/convert_manuals.py
```

Do not derive register values or pin mappings from extracted text without
checking the corresponding vendor PDF.
