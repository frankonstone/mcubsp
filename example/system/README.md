# Pro Micro native system example

This application reads the factory identity into `mcubsp_system_identity`,
starts RTC1 as a 1,024 Hz alarm/overflow clock, and connects TIMER2 through PPI0
to GPIOTE0 on the existing P0.15 LED for four seconds. It then releases the PPI
route and displays **two short flashes followed by a pause**, repeatedly. Each
flash requires RTC alarm progress and an RTC ISR; TIMER2 remains an independent
500 ms interrupt/wake guard and rejects a stalled RTC after about 2.5 seconds.
The sleep-predicate race is handled as normal completion. A three-second watchdog
starts after the first successful cycle and is fed only after later complete
cycles. Sustained pulses beyond its timeout exercise feeding while idle is used.
LFRC timing is not independently calibrated.

```sh
cmake --preset promicro-nrf52840
cmake --build --preset promicro-nrf52840 --target others.promicro_nrf52840.example.system
```

The ELF and UF2 are under `output/build/promicro-nrf52840/example/`.
After entering the existing UF2 bootloader, use the actual mounted drive:

```sh
python3 tool/hardware.py flash --board others/promicro_nrf52840 \
  --elf output/build/promicro-nrf52840/example/others.promicro_nrf52840.example.system.elf \
  --mount /Volumes/NICENANO --dry-run
```

Remove `--dry-run` for an explicit upload. The example uses the same application
partition as Blink; it starts no USB application device, radio stack or System
OFF reset. A failure after LED setup holds the LED on (a running watchdog can
subsequently reset it). Startup/early failures can leave it off. This is a
functional check of alarm/ISR progress around WFI, not a measurement of sleep
residency, oscillator accuracy, overflow endurance or current consumption.

`mcubsp_system_status` records timer completion (bit 0), RTC completion (bit 1),
watchdog started (bit 2), a feed issued (bit 3) and a captured watchdog reset
reason (bit 4); bit 31 marks failure. Identity, raw reset reason and ISR counters
are also stored in named RAM variables. There is no USB readback yet.
Historical regular-blink observations apply only to the old identified image;
see [the checkpoint](../../doc/acceptance/nrf52840-system.md).

The separate [watchdog timeout variant](../system-watchdog/README.md) deliberately
stops feeding. The regular image keeps feeding while RTC progress continues.

For a standalone source consumer, configure this directory with the common
board toolchain before `project()` and `MCUBSP_SOURCE_DIR` set to the checkout.
Installed consumption uses the installed toolchain and `CMAKE_PREFIX_PATH`, with
`MCUBSP_SOURCE_DIR` empty. The target is `system_demo` in both cases. See the
[application guide](../../doc/getting-started/application.md) and
[native runtime contracts](../../doc/reference/nrf52840/README.md).
