# Pro Micro native system example

This application reads the factory identity into `mcubsp_system_identity`,
starts RTC1 as a 1,024 Hz alarm/overflow clock, and connects TIMER2 through PPI0
to GPIOTE0 on the existing P0.15 LED. The timer toggles the LED every nominal
500 ms without foreground GPIO writes. The foreground waits on a separate RTC
alarm using native System ON idle. LFRC timing is not independently calibrated.

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
partition as Blink; it starts no USB application device, radio stack, watchdog
or System OFF reset. Physical upload and observed LED behavior are recorded in
[the checkpoint](../../doc/acceptance/nrf52840-system.md). A blinking LED proves
the hardware timer/PPI/GPIOTE path for that image; it does not independently
prove RTC interrupts, alarm wakeup or low-power current because LED toggling
continues without the foreground.

For a standalone source consumer, configure this directory with the common
board toolchain before `project()` and `MCUBSP_SOURCE_DIR` set to the checkout.
Installed consumption uses the installed toolchain and `CMAKE_PREFIX_PATH`, with
`MCUBSP_SOURCE_DIR` empty. The target is `system_demo` in both cases. See the
[application guide](../../doc/getting-started/application.md) and
[native runtime contracts](../../doc/reference/nrf52840/README.md).
