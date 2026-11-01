# Pro Micro watchdog reset check

This explicit variant runs the [system check](../system/README.md), completes
three RTC pulse cycles, holds the LED on and stops feeding its three-second
watchdog. A working reset returns to the four-second TIMER/PPI/GPIOTE startup
sequence. The test repeats; it does not write flash, UICR or bootloader flags.

```sh
cmake --build --preset promicro-nrf52840 --target others.promicro_nrf52840.example.system-watchdog
python3 tool/hardware.py flash --board others/promicro_nrf52840 \
  --elf output/build/promicro-nrf52840/example/others.promicro_nrf52840.example.system-watchdog.elf \
  --mount /Volumes/NICENANO --dry-run
```

Remove `--dry-run` only for the intended upload to the actual UF2 mount. The
regular system image keeps feeding; use it to leave the deliberate reset cycle.
Enter the existing bootloader with double reset to change images. Recovery on
this exact clone after application startup remains a physical check.

If the bootloader preserves RESETREAS.DOG, pulses change from two to **three**
after the reset. The application records and clears that bit on every boot,
so a subsequent triple-pulse cycle can identify a new watchdog reset. Begin
from power-on with no stale DOG reason when comparing the patterns. If the
bootloader consumes the reset reason, repeated startup establishes restart
behavior but the cause remains inconclusive; do not record a confirmed watchdog
reset from repeats alone. No RAM cookie or bootloader-reserved retention byte
is treated as proof. A failed reset is detected after ten TIMER guard interrupts.

For standalone source/package consumption, configure this directory with the
same board toolchain as the system example; the target is `system_watchdog_demo`.
The image has been built in source and relocated-package configurations and
uploaded on 28 October. The user confirmed restarts and later triple-pulse groups,
observing the no-feed reset path and firmware DOG-reason indication. Independent
timeout accuracy remains pending. UF2 recovery after this image was observed
and the regular image restored successfully; see the
[upload record](../../doc/acceptance/evidence/nrf52840-system/2026-10-28-watchdog-upload.json).
