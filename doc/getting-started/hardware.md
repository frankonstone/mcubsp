# Plan flashing and collect board evidence

`tool/hardware.py` supplies the shared J-Link, OpenOCD and ESP-IDF adapters.
It reads only the selected board's `board.cmake` entry. Python 3.10+, CMake and
the selected debugger/SDK tools are required; ordinary application builds need
no debugger installation. A flash operation needs a completed `.elf.build.json`
record, matching board/revision and checksum, plus an explicit probe or port.

## Inspect an operation

Build first, then inspect a structured command plan. Replace the probe serial
with your selected probe. Planning sends no debugger or serial commands and
creates no command file:

```sh
python3 tool/hardware.py flash --board arduino/uno_r4_minima --elf output/build/arduino-uno-r4-minima/src/board/firmware.elf --probe 123456789 --dry-run
```

Ninja builds also produce `.elf.inputs.json`, bound by checksum to the completed
build record. The plan shows its selected-input identity. Keep that sidecar with
the ELF and build record; a missing or modified sidecar is rejected. A legacy or
non-Ninja record explicitly shows input tracking as unavailable. See
[firmware input identity](../development/firmware-inputs.md) for producer scope.

The same command without `--dry-run` flashes and starts the image. J-Link uses
SWD at 4 MHz and preserves the UNO bootloader below `0x4000`. OpenOCD selects
the recorded ST-LINK interface and H7 target; NUCLEO flash starts at `0x08000000`.
ELF load segments must fit the declared application regions and match the Arm
architecture. Probe selection is mandatory; connected probes are not selected
implicitly. `MCUBSP_PROBE_SERIAL` supplies the serial to board capture scripts.

For P4, specify both the completed SDK build and the chosen serial port:

```sh
python3 tool/hardware.py flash --board waveshare/esp32p4_nano --elf output/build/waveshare-esp32p4-nano/src/board/firmware.elf --sdk-build output/build/waveshare-esp32p4-nano/src/board/firmware-idf-build --port /dev/selected-port --dry-run
```

The runner verifies the recorded manifest, SDK ELF and every flash-image hash,
then checks the bootloader (`0x2000`), partition table (`0x8000`) and application
(`0x10000`) regions. A modified or incomplete bundle is rejected. Rebuild to
produce a new record. The supplied P4 startup remains development-v1.0-only.

Tool overrides are full executable paths: `JLINK_EXE`, `OPENOCD_EXE`,
`IDF_PYTHON`, `JLINK_RTT_LOGGER`, `ARM_NM`, and `MCUCOV_EXE`. Paths containing
spaces remain individual arguments. A missing explicit override produces an
error rather than silently using another installation. `MCUBSP_PYTHON` selects
the Python interpreter used by shell capture helpers.

## Build physical-test images

UNO and NUCLEO use the same explicit board-test options:

```sh
cmake --preset arduino-uno-r4-minima -DMCUBSP_BUILD_BOARD_TESTS=ON -DMCUBSP_COVERAGE=ON -Dmcucov_DIR=/path/to/host/mcucov/lib/cmake/mcucov
cmake --build --preset arduino-uno-r4-minima

cmake --preset st-nucleo-h753zi -DMCUBSP_BUILD_BOARD_TESTS=ON -DMCUBSP_COVERAGE=ON -Dmcucov_DIR=/path/to/host/mcucov/lib/cmake/mcucov
cmake --build --preset st-nucleo-h753zi
```

Test images are under `src/board/<vendor>/<name>/test/` for UNO and
`src/board/st/nucleo_h753zi/` for NUCLEO within the selected build directory.
The test/coverage dependencies are resolved only when requested. Set
`MCUBSP_MCUTEST_SOURCE_DIR` and `MCUBSP_MCUJSON_SOURCE_DIR` for explicit test
dependency locations. MCUIP Ethernet applications and their runners belong to
the sibling [integration project](../../../integration/README.md).

P4 keeps its separate SDK test project. Activate the installed SDK/Python
environment and build its instrumented application:

```sh
python "$IDF_PATH/tools/idf.py" -C src/board/waveshare/esp32p4_nano/test/firmware -B "$PWD/output/build/bsp-p4-tests" -DIDF_TARGET=esp32p4 -Dmcucov_DIR=/path/to/host/mcucov/lib/cmake/mcucov build
```

Its completed bundle receives the same ELF/image hash record as ordinary P4
applications. `MCUBSP_SDK_BUILD_DIR` selects that build for the capture script.

## Run scoped physical procedures

Read the board wiring and recovery notes before running its procedure. These
POSIX shell scripts require the named debugger tools, `jq`, and, for RTT or SDK
monitor capture, `expect`. Build success does not run any procedure.

- UNO: `sh src/board/arduino/uno_r4_minima/test/run-tests.sh --dry-run /absolute/path/to/test.elf`
  inspects flashing. With the recorded wiring and selected probe, invoke a named
  suite such as `startup-gpio` or `spi` to flash and capture RTT.
  `MCUBSP_BUILD_DIR` selects the directory containing the test ELFs.
- NUCLEO: `sh src/board/st/nucleo_h753zi/test/run-native.sh --dry-run /absolute/path/to/test.elf`
  inspects flashing. Set `NUCLEO_VCP` to the exact ST-LINK VCP before running
  without `--dry-run`. The native procedure collects register, JSON, coverage
  and UART-banner observations; waveform/analog-accuracy claims remain separate.
  `run-smoke.sh` covers the startup image. The Ethernet runner is in the upper
  integration project.
- P4: `MCUBSP_SDK_BUILD_DIR="$PWD/output/build/bsp-p4-tests" sh src/board/waveshare/esp32p4_nano/test/run-smoke.sh --dry-run /dev/selected-port`
  inspects the selected SDK bundle. The actual suite plays an audible tone and
  writes, reads back and restores the SD card's last sector. Use the intended
  test card and board wiring. `MCUBSP_SDK_BUILD_DIR` selects the completed build.

Captures are written under the selected build's `artifact/` directory. Coverage
manifests follow that image's build. Current helpers and command planning have
software tests; physical qualification requires executing the procedure and
recording its firmware identity, wiring, tools and observed result. Historical
records retain their original scope. See the [support matrix](../reference/support-matrix.md).
