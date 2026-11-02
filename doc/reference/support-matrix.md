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

## Additional initial target: Pro Micro nRF52840

Full-peripheral implementation is now the intended target in the
[board requirements](../../src/board/others/promicro_nrf52840/requirements.md)
(REQ-0055–REQ-0074), with delivery stages in the
[board plan](../../src/board/others/promicro_nrf52840/implementation-plan.md)
(PLN-0042–PLN-0053). Those additions are planned scope; the implemented/evidence
state remains as described below.

The nice!nano-compatible Pro Micro family has a reproducible MDK register-facts
catalogue, PS v1.11 peripheral/field/instance and package inventories, selected
typed Arm core descriptions and reproducible source audits, plus
GPIO and finite I2C backends/facade, bootloader-handoff startup and Blink/TMP102
applications with UF2/S140-v6 output. I2C0/I2C1 provide 100/250/400 kHz nominal
rates, shared device leases and CPU-driven finite/repeated-START transfers;
native simulation and linked applications do not establish physical I2C timing
or sensor behavior. See the [I2C software record](../acceptance/nrf52840-i2c.md).
This does not claim the complete description gate used for the original three
MCUs. Explicit native system services now provide clock/power, GPIO events,
all 32 PPI channels/groups/forks, POWER/CLOCK event interfaces, EGU/SWI,
TIMER/RTC/watchdog and selected core/IRQ APIs,
with native tests and linked example evidence. The user confirms the identified
system example's hardware timer/PPI/GPIOTE LED behavior. RTC sleep residency, low-power
current, exact silicon identity and full system acceptance remain unverified.
Optional FPB/DWT inspection and SWO/ETM-M4 debugger scripts pass software checks;
physical debug/trace capture remains pending. Native legacy UART0 finite TX/RX
and common DMA/resource coordination now have software support; their new
physical tests remain pending. The [serial delivery](../acceptance/nrf52840-serial.md)
adds bounded UART/UARTE receive streams, SPI/SPIS/TWIS and common selections.
The [native delivery](../acceptance/nrf52840-native-delivery.md) adds PWM/SAADC/
comparators and common PWM/ADC, QSPI/protection, I2S/PDM, USB CDC/HID, ECB/CCM/AAR/
CryptoCell, radio and NFC. Its [per-instance/mode matrix](nrf52840/implementation-matrix.json)
separates the implemented APIs, representative native checks, cross/link results
and pending physical qualification. Full board qualification remains open;
prior observations do not qualify new images or replace absent peers/measurements.
The [coordination/UART checkpoint](../acceptance/nrf52840-dma-uart.md) records
the new shared ownership contracts and initial native UART0 support.
The current regular system test requires RTC-driven pulses and watchdog feeding;
a separate timeout variant deliberately resets. Both have software build evidence;
the earlier regular-blink observation does not qualify these new images.
The regular test was uploaded on 28 October; the user confirmed repeating double
pulses, supporting RTC1 alarm/ISR progress and the watchdog-feeding application
path around System ON idle. The [upload record](../acceptance/evidence/nrf52840-system/2026-10-28-audit-upload.json)
retains the image identity and limits. Independent timing/current measurements
remain pending.
UF2 bootloader re-entry after that regular image was observed. The separate
watchdog timeout image was uploaded; the user confirmed restarts and subsequent
triple pulses, observing the firmware's DOG reset-reason indication. The
[watchdog record](../acceptance/evidence/nrf52840-system/2026-10-28-watchdog-upload.json)
retains its exact image and remaining qualification scope.
On 27 October 2026 the connected V1940-marked board reported nice!nano UF2 0.6.0
and S140 6.1.1; UF2 upload completed and the user confirmed LED blinking.
Independent timing and silicon/PCB identity remain unverified. Double-reset
re-entry is observed for both 28 October audit images, and the regular image was
restored successfully after the watchdog test. The [image-bound observation](../acceptance/promicro-nrf52840.md)
records the exact ELF, UF2 and selected-input identities. See the
[board guide](../../src/board/others/promicro_nrf52840/README.md).

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
