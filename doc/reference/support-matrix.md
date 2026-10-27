# Support and evidence matrix

Software tooling updated 25 October 2026; physical records keep their original dates. Each column describes a distinct claim. A reviewed
manual chapter does not imply a complete typed register/field API, a working
driver, or a physical observation. Source-unit counts remain an inventory of
the reviewed baseline: RA4M1 48/48, STM32H753 61/61, P4 67/68 with I3C unavailable.
A register-by-register typed-coverage percentage is not currently measured.

| Area | RA4M1 / UNO | STM32H753 / NUCLEO | ESP32-P4 / NANO |
| --- | --- | --- | --- |
| Typed metadata | Initial resources and driver-facing peripheral subset | Initial resources and driver-facing peripheral subset | Initial resources and driver-facing peripheral subset; production-v3 description |
| GPIO | Implemented; scoped physical record | Implemented; onboard LED record | Implemented; development-v1 record |
| PWM | Basic GPT implemented; scoped physical record | TIM implemented; register readback, independent waveform pending | No facade/backend implementation |
| UART | Implemented; scoped physical record | Implemented; physical VCP TX | Implemented; physical TX, RX/error fixture pending |
| I2C | Implemented; scoped physical record | Implemented; initialization, external protocol tests pending | Implemented; onboard codec transactions on development-v1 |
| SPI | CPU and DMA implemented; original physical scope | CPU implemented; external device/timing pending | CPU implemented; controller execution, external response/timing pending |
| ADC | Finite scan and DMA implemented; original physical scope | Finite scan implemented; floating-input observation, calibrated tests pending | No facade/backend implementation |
| Generic DMA | Four DMAC channels | No generic facade DMA backend | No generic facade DMA backend |
| Ethernet | W5100/MCUIP implemented; physical record stale for refactored driver | Backend/MCUIP implemented; PHY ID observed, frames/link recovery pending | Backend/MCUIP implemented; PHY ID observed, frames/link recovery pending |
| I2S / SDMMC / SDIO | Outside current driver scope | Outside current driver scope | Implemented scoped audio, single-sector SD and SDIO enumeration; Wi-Fi connectivity pending |
| Linked application | Blink with supplied startup | Blink with supplied startup | Blink with SDK boot path; development-v1 only |

The application facade has native tests and cross-build evidence. This work does
not refresh physical facade acceptance. The fixed-clock single-core assumptions
and explicit ownership/lifetime contract remain in force. Generic MCUOS, camera,
USB device stack and production-v3 P4 hardware acceptance are separate work.

## Required physical refresh

- UNO W5100: rebuild the refactored path, record firmware/source identities,
  exchange short and MTU frames plus ARP/ICMP with a peer, exercise link loss,
  and record SPI/DMAC cleanup and resource/latency measurements.
- NUCLEO/P4 Ethernet: connect a peer, observe real frame traffic with caches
  enabled, test descriptor exhaustion and link down/up. PHY ID readback alone
  is insufficient.
- Peripheral DMA/timing: independently capture wire completion and cancellation
  order, PWM frequency/duty and bus timing. Record wiring and firmware identity.
- Production-v3 P4: use the correct revision-specific startup/register binding
  and rerun that hardware scope independently of the development-v1 board.

Historical scope and observations: [UNO release](../acceptance/ra4m1-release.md),
[NUCLEO](../../src/board/st/nucleo_h753zi/doc/README.md),
[P4](../../src/board/waveshare/esp32p4_nano/doc/README.md), and
[repository adoption](../acceptance/repository-adoption.md). Shared runner helpers and the UNO/NUCLEO/P4 capture entry points have been
restored and tested without hardware; see [current procedures](../getting-started/hardware.md).
Historical command paths and observations below remain scoped to their original builds.
