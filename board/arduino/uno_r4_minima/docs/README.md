# Arduino UNO R4 Minima bare-metal board tests

This is the first physical mcuhw composition. It uses the RA4M1 description,
the RA-native clock/GPIO/GPT/SCI backends, the target-independent drivers,
mcutest, and an RTT output channel. It has no Arduino core, FSP, MCUOS, heap,
hosted startup, or external RTT source dependency. Each physical fixture is a
separate executable; fixture selection is never hidden behind a compile define.

## Startup and flash ownership

The selected part is `R7FA4M1AB3CFM#AA0`. ArduinoCore-renesas 1.6.0 identifies
D13/`LED_BUILTIN` as P111 and configures the board for a 48 MHz HOCO. Its
`variants/MINIMA/memory_regions.ld` starts the application at `0x00004000`.
The shipped `dfu_minima.hex` occupies `0x00000000..0x00003087`; therefore the
mcuhw linker owns `0x00004000..0x0003FFFF` and preserves the DFU bootloader.
The `minima.upload.address=0x00010000` property in `boards.txt` is not used by
that package's DFU upload command and is not the linked sketch address.

The board startup owns:

- the vector table at `0x00004000` and VTOR update;
- FPU enable, `.data` copy, `.bss` clear, and C++ constructor traversal;
- the 4 KiB main-stack reservation at the top of 32 KiB SRAM;
- a checked 48 MHz HOCO transition, flash wait state, clock divisors, protected
  register unlock/relock, and transfer to `main`.

It verifies that option-setting memory selected the 48 MHz HOCO trim. It does
not rewrite option memory. The resulting clock contract is ICLK/PCLKA/PCLKC/
PCLKD at 48 MHz and PCLKB/FCLK at 24 MHz.

## Build and automatic execution

Build the four ELF, BIN, HEX, and map sets plus the shared barrier and GPIO
code-generation reports:

```sh
cmake --preset arm-cortex-m4 -B build/uno-r4-minima -DMCUHW_BOARD_TESTS=arduino/uno_r4_minima
cmake --build build/uno-r4-minima --target board-tests
```

Connect a J-Link Compact Plus to the Minima SWD header using VTref (3.3 V),
GND, SWDIO, SWCLK, and RESET. Do not connect competing power sources. Then run:

```sh
./tools/run-uno-r4-minima-tests.sh startup-gpio
```

The script programs the ELF's application sectors, resets through the retained
bootloader, captures mcutest output with J-Link RTT, and returns success only
after `MCUHW_RESULT:PASS`. Its log identifies the board, firmware plan,
toolchain, clock frequencies, and debug interface. The runner accepts
`startup-gpio`, `gpio-loopback`, `pwm`, `uart`, `all`, or an explicit ELF path.
Each suite writes a distinct JSON artifact named after its executable.

The `startup-gpio` image checks startup/clock registers, initial low output
on the built-in LED, set/read/clear behavior, and adjacent-pin preservation.
For the physical loopback test, connect D2 directly to D3 and run:

```sh
./tools/run-uno-r4-minima-tests.sh gpio-loopback
```

The PWM fixture uses the same D2-to-D3 jumper, now driving D3 from GPT1B and
observing D2 as GPIO. Because it is a separate image, it does not share global
driver state or conditional code with the GPIO test. Run it with:

```sh
./tools/run-uno-r4-minima-tests.sh pwm
```

The firmware solves the 20 kHz request from the 48 MHz PCLKD at compile time
(`2400` timer counts, `GTPR=2399`, zero solver error). It measures the physical
D3 waveform independently through D2 and the Cortex-M4 cycle counter, checks
25% and buffered 75% duty within a declared ±5% observation tolerance, then
verifies that stopping the timer drives D3 low.

The UART fixture connects D1/SCI2 TX directly to D0/SCI2 RX and runs with:

```sh
./tools/run-uno-r4-minima-tests.sh uart
```

It sends eight finite-buffer bytes at an achieved 115385 baud for the 115200
target, completes reception through RXI, and completes transmission through
TEI only after the final stop bit. With both D2-D3 and D1-D0 jumpers installed,
`./tools/run-uno-r4-minima-tests.sh all` runs every image in sequence.
It also merges the four mcucov captures and writes the aggregate LCOV report to
`build/uno-r4-minima/artifacts/coverage.lcov`. The mcucov VS Code extension
discovers that conventional filename automatically and publishes it in the
Test Coverage view and source-editor coverage gutter.

## Current measurements and evidence boundary

With Arm GNU Toolchain 15.2.1, the standalone startup/GPIO, GPIO-loopback, PWM,
and UART images use respectively 25,808, 22,320, 28,408, and 44,304 bytes of
flash-region content. Their RAM use is 4,552, 4,560, 4,656, and 4,624 bytes.
The fixed 4 KiB RTT up-buffer and 16-byte down-buffer are included in BSS.
These are measurements, not yet release budgets.

On 16 September 2026, the J-Link/RTT suites ran on an Arduino UNO R4 Minima.
The default image reported four tests with zero failures: vector/48 MHz
startup, initial low output on D13, set/read/clear, and adjacent-pin
preservation. With D2 connected directly to D3, the loopback-enabled image
reported those four tests plus the physical low/high/low loopback test, for
five tests with zero failures.

On the same date, the PWM fixture image used 34,904 bytes of flash-region
content and 4,688 bytes of RAM. RTT reported `2418` observed CPU cycles per
period, `600` high cycles at the 25% setting, and `1816` high cycles after the
buffered 75% update. The expected ideal values are `2400`, `600`, and `1800`;
the edge-polling overhead is included in the observed values. All five board
tests passed, including start, buffered update, and stopped-low behavior.
The runner stores each live result independently at
`build/uno-r4-minima/artifacts/mcuhw_uno_r4_minima_<suite>_tests.json`, so
running one fixture cannot overwrite another fixture's evidence. Add a suite's
artifact to `mcutrace.toml` only after that standalone image has actually run;
missing configured artifact paths are configuration errors, not placeholder
evidence. The startup/GPIO and UART artifacts are currently configured because
those standalone images were recaptured on 17 September 2026. The standalone
GPIO-loopback and PWM images were then recaptured with D2-D3, so all four
artifacts are now configured independently.

The standalone PWM capture observed `2426` CPU cycles per period, `584` high
cycles at 25% duty, and `1842` high cycles at 75% duty. The expected ideal
values remain `2400`, `600`, and `1800`; all measurements passed the declared
5% tolerance and stop drove the observed pin low.

At `-Os`, `gpio-codegen-report.txt` records 20 bytes for the direct P111 POSR
write plus DMB baseline and 60 bytes for the checked functional-driver call.
No zero-overhead claim is made: the latter deliberately validates initialization,
direction, package bounds, and ownership before issuing the same atomic write.

Host traces and cross-linking remain software evidence. Captured RTT results
verify only the image and physical wiring named by that artifact. Running a
loopback fixture without its jumper is expected to fail.
