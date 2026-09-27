# Build a selected-board application

The optional `tools/mcuhw.py` frontend creates an ordinary external CMake
application and selects one board, revision, startup owner and toolchain. Use
Python 3.10 or newer. The commands below use `python`; use `python3` on hosts
where that is the Python 3 command, or `py -3` on Windows.

For a board-independent application, see [installation](installation.md) and
[library use](using-mcuhw.md). It does not need board discovery or this frontend.

## Discover and create

Run these commands from the mcuhw checkout or installed `share/mcuhw` directory:

```sh
python tools/mcuhw.py list boards
python tools/mcuhw.py search devices temperature
python tools/mcuhw.py list boards --capability i2c --json
python tools/mcuhw.py new ../my-blink --board arduino/uno_r4_minima
```

`new` copies [Blink](../../examples/apps/blink/README.md) into an external
directory. Choose `--example tmp102` for a complete
[TMP102 application](../../examples/apps/tmp102/README.md), and read its
wiring instructions before connecting hardware. The selected revision is
written explicitly into `mcuhw.json`; `--revision` selects another revision
declared by that board. An unknown board or revision fails before creation.
Existing nonempty directories are never overwritten.

| Board | Selected binding revision | Toolchain | Supplied startup |
| --- | --- | --- | --- |
| `arduino/uno_r4_minima` | `r4` | Arm GNU 15.2.Rel1 | RA4M1, application starts at `0x4000`; retained DFU bootloader |
| `st/nucleo_h753zi` | `mb1364-rev-e` | Arm GNU 15.2.Rel1 | STM32H753, HSI 64 MHz; vectors at `0x08000000` |
| `waveshare/esp32p4_nano` | `development-v1.0` | Espressif GCC 15.2.0 and ESP-IDF v6.1 | Custom SDK second-stage bootloader and explicit pre-scheduler application entry |

The UNO `r4` name identifies the supported binding, not an audited PCB revision.
The ESP binding is scoped to the observed development silicon; build success
does not establish production-silicon or physical TMP102 acceptance. The
[generated catalog](../reference/catalog.md) preserves each evidence scope.

## Import installed prerequisites explicitly

Install the versions in [the tool lock](../../tools/toolchains.lock.json)
using the tool vendors' distribution mechanisms, then import those installed
tools. ARM builds need CMake, Ninja and the complete Arm GNU compiler installation,
including its matching C++23 libstdc++ headers and binutils. Debuggers are only
required for the separate `flash` operation.

Use the official distributions for your host: [Python](https://www.python.org/downloads/),
[CMake](https://cmake.org/download/), [Ninja 1.12.1](https://github.com/ninja-build/ninja/releases/tag/v1.12.1),
and [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
Choose each recorded version rather than an unqualified latest release, install
or unpack the complete distribution, and add its executable directory to
`PATH` or pass the explicit path below. The installation source is selected by
you; the frontend checks and records what was actually imported.

```sh
python tools/mcuhw.py setup --project ../my-blink --offline
python tools/mcuhw.py doctor --project ../my-blink
python tools/mcuhw.py build --project ../my-blink
```

Setup imports tools from `PATH` unless overridden. Paths containing spaces are
supported; quote the complete `NAME=PATH` argument:

```sh
python tools/mcuhw.py setup --project ../my-blink --offline --tool "arm-gcc=/opt/Arm Toolchain/bin/arm-none-eabi-g++"
```

On Windows the equivalent override is, for example,
`--tool "arm-gcc=C:/Arm Toolchain/bin/arm-none-eabi-g++.exe"`. Other executable
override names are `cmake`, `ninja`, `riscv-gcc`, `idf-python`, `jlink` and
`openocd`. `esp-idf` and `idf-tools` name directories.

Setup records the imported tools and exposes one configuration to the frontend,
CMake and the IDE:

- `.mcuhw/toolchains.lock.json` records exact expected versions and optional
  immutable archive sources. Commit this file with the application.
- `.mcuhw/environment.json` records the host, actual installation paths,
  executable SHA-256 values, compiler/assembler/linker/library identities,
  frontend Python identity, versions, explicit version overrides, SDK commit
  and submodules, Python package versions, and SDK constraints checksum. This
  machine-local file is ignored by the generated `.gitignore`.
- `.mcuhw/CMakePresets.json` provides the `mcuhw` configure and build presets,
  including the selected board, tool paths and SDK environment. Setup adds its
  include to `CMakeUserPresets.json`, preserving existing user presets.
- `.mcuhw/verify-environment.cmake` checks the recorded installation during
  direct CMake and IDE configuration as well as frontend builds.

From the generated application directory, the equivalent CMake commands are:

```sh
cmake --preset mcuhw
cmake --build --preset mcuhw
```

Select the `mcuhw` configure/build preset in the IDE. Use the CMake executable
recorded by setup; the preset supplies that path to IDEs. The frontend invokes
these same presets. Change board/revision/startup selection in `mcuhw.json`
and rerun setup to regenerate the shared configuration. Generated files and
machine-local presets are ignored by version control.

Build and doctor verify recorded installations before use. If a tool changes,
rerun explicit setup to accept its new identity. An intentional alternative
version requires `--allow-version-override`; the resulting report preserves
that fact. Version overrides do not bypass the C++23 standard-library probe.
Modified SDK sources must be committed before importing an alternate SDK
revision; this keeps source overrides reproducible. Setup and doctor check the
SDK and every nested submodule worktree, including untracked source files,
independently of Git's submodule-ignore settings. An explicit version override
accepts a different clean commit, not uncommitted dependency changes.

The selected toolchain, SDK, board revision and startup owner determine an
isolated directory below `build/`. Adding a debugger does not change the build
directory. Each successful CMake build writes a target-specific
`firmware.build.json` beside `firmware.elf`, identifying the board, revision,
startup owner, actual artifact path and checksum. Frontend-created projects
also update `.mcuhw/last-build.json`; this works when the build is invoked
directly by CMake or an IDE. Source and toolchain provenance remains in the
adjacent `firmware.identity.json` report.

## ESP32-P4 SDK setup

Use a complete ESP-IDF v6.1 installation at commit
`fff9895c82d744c7237be8847347bdd1b07c6643`, including its pinned submodules,
`esp-15.2.0_20251204` RISC-V compiler and installed Python environment. Run the
SDK's host-specific installation workflow first, then import the paths it
created. For example, with the example installation paths below replaced by
your own:

```sh
python tools/mcuhw.py new ../p4-blink --board waveshare/esp32p4_nano
python tools/mcuhw.py setup --project ../p4-blink --offline --tool "esp-idf=/sdk/esp-idf" --tool "riscv-gcc=/sdk/tools/riscv32-esp-elf/bin/riscv32-esp-elf-g++" --tool "idf-python=/sdk/python/venv/bin/python" --tool "idf-tools=/sdk/tools"
python tools/mcuhw.py doctor --project ../p4-blink
python tools/mcuhw.py build --project ../p4-blink
```

On Windows, use the SDK environment's `Scripts/python.exe`. `idf-tools` must
contain `espidf.constraints.v6.1.txt`; setup checks its locked SHA-256. The
frontend supplies the recorded `IDF_PATH`, `IDF_TOOLS_PATH`, Python interpreter
and compiler to CMake, and doctor runs the SDK's dependency checker without
installing or upgrading packages. Build includes only the selected board's SDK
project; it does not require mcucov, mcutest, an adjacent checkout or a connected
device. The SDK may warn about missing optional ROM GDB symbols when
`ESP_ROM_ELF_DIR` is absent; that does not require debugger installation to build.

The [ESP-IDF v6.1 setup instructions](https://github.com/espressif/esp-idf/tree/v6.1#setting-up-esp-idf)
provide the platform prerequisites and installer entry points: `install.sh` on
Linux/macOS and `install.bat` or `install.ps1` on Windows. Keep the installed
compiler, SDK, Python packages and constraints file together after setup.

## Cached archive setup and offline builds

The bundled lock intentionally contains no unverified download URL or checksum.
It supports import of existing installations on each host. To distribute an
approved tool archive to a team, copy the lock and add a host-specific `archives`
entry to a tool. For example, this schematic entry must be populated with the
actual immutable URL, digest and path before use:

```json
{
  "archives": {
    "linux-x86_64": {
      "url": "https://your-tool-mirror.example/tool-version.zip",
      "sha256": "<actual 64-character lowercase SHA-256>",
      "format": "zip",
      "path": "tool-version/bin/tool"
    }
  }
}
```

Supported host keys include `linux-x86_64`, `linux-arm64`, `darwin-arm64`,
`darwin-x86_64`, `windows-x86_64` and `windows-arm64`. An archive entry is part of
the selected tool definition, next to `kind`, `version` and `version_args`.
Supported formats are `zip` and `tar` (including compressed tar files).
Archives may contain regular files and directories; links, device entries and
paths escaping the extraction root are rejected. Import an existing install
for vendor bundles requiring symbolic links. Local `file://` archives work too.

```sh
python tools/mcuhw.py setup --project ../my-blink --lock team-toolchains.lock.json --cache ../tool-cache
python tools/mcuhw.py setup --project ../my-blink --offline --cache ../tool-cache
python tools/mcuhw.py build --project ../my-blink
```

Only explicit `setup` can fetch an archive. Downloaded bytes are checked before
extraction and cached by SHA-256. Offline setup requires an existing verified
cache entry, and reports the missing entry with remediation otherwise. Reusing
the cache rechecks archive integrity and extracts fresh contents. Explicit
`--tool` imports take precedence over archives.

Build does not perform setup or resolve a newer tool version. Imported installs
and all selected SDK sources/packages must remain available locally. Ordinary
ARM builds and the selected minimal SDK application can be rebuilt with network
access unavailable after setup; no board SDK/component download step is added.

## Inspect and flash separately

Compilation never flashes. Import the debugger for the requested operation,
inspect its command, then run the same command without `--dry-run`:

```sh
python tools/mcuhw.py setup --project ../my-blink --operation flash --offline
python tools/mcuhw.py doctor --project ../my-blink --operation flash
python tools/mcuhw.py flash --project ../my-blink --probe 123456789 --dry-run
python tools/mcuhw.py flash --project ../my-blink --probe 123456789
```

Replace `123456789` with the selected physical probe's serial number. UNO uses
J-Link with `R7FA4M1AB`, SWD and 4 MHz; NUCLEO uses OpenOCD with ST-LINK and
`stm32h7x.cfg`. Flashing checks that ELF load segments fit the board's declared
application flash regions. The UNO application region starts at `0x4000` to
preserve its DFU bootloader. Reusable adapters read these settings from the
board manifest; they do not choose a connected probe implicitly.

Flash reads the last completed artifact record; changing a compiler or importing
a debugger does not redirect it to an unbuilt output directory. The record must
match the selected board/revision/startup and the firmware checksum. A dry-run
also requires a completed image. To select another completed CMake build,
provide its exact record (or a directory containing a single record):

```sh
python tools/mcuhw.py flash --project ../my-blink --build /path/to/build/firmware.build.json --probe 123456789 --dry-run
```

For ESP, the SDK Python environment already imported for the build supplies
esptool. Select the serial port explicitly (`COM4` on Windows, for example):

```sh
python tools/mcuhw.py flash --project ../p4-blink --port /dev/ttyACM0 --dry-run
python tools/mcuhw.py flash --project ../p4-blink --port /dev/ttyACM0
```

The runner reads the generated SDK `flasher_args.json`, verifies its chip and
flash regions, and flashes the bootloader at `0x2000`, partition table at
`0x8000` and application at `0x10000`. Existing contents in those regions are
replaced. See each board's [catalog documentation](../reference/catalog.md) for
boot policy, preserved regions and recovery. A flash dry-run prints structured
argument arrays and any proposed command-file contents; it sends nothing to a
probe or serial device and writes no runner files.

Physical acceptance fixtures live under each board's `tests/` directory. The
existing `tools/run-*.sh` commands remain compatibility wrappers. Their board
runner, tool resolution and artifact locations use the same shared host helpers
as application flashing; fixture-specific observation and evidence parsing stay
with the board. Set `MCUHW_PROBE_SERIAL` for J-Link/ST-LINK fixtures, and pass the
ESP serial port or set `MCUHW_SERIAL_PORT`. `MCUHW_ENVIRONMENT_JSON` can point to
an application's recorded environment; explicit `JLINK_EXE`, `OPENOCD_EXE`,
`IDF_PYTHON`, `ARM_NM` and `MCUCOV_EXE` overrides remain supported. An explicit
ELF argument makes captures and coverage lookup follow that image's build
directory. For the UNO named suites, `MCUHW_BUILD_DIR` selects an architecture
workspace's test build instead of the legacy preset directory.

## Active-board workspace

Choose an architecture and save the boards you are currently working on:

```sh
cmake --preset arm-cortex-m4 -DMCUHW_ACTIVE_BOARDS=arduino/uno_r4_minima
cmake --build --preset arm-cortex-m4 --target arduino.uno_r4_minima.examples
```

`MCUHW_ACTIVE_BOARDS` can be edited in CMake Tools, supplied as a quoted
semicolon-separated list, or stored in a local `CMakeUserPresets.json`. The cache
selection survives subsequent configure calls. Only active boards matching the
architecture contribute targets; reconfigure to apply edits. A fresh workspace
starts with no boards selected.

The examples aggregate produces independent Blink and TMP102 firmware images.
Select `arduino.uno_r4_minima.examples.blink` to build only Blink, or
`arduino.uno_r4_minima.tests` to build the instrumented board-test images.
Output is isolated under `build/<preset>/boards/<vendor>/<board>/<revision>/`.
The test build is configured only when requested, so example configuration does
not require test or coverage dependencies. See
[active-board workspaces](../development/verification.md#active-board-workspaces)
for target structure, persistent user presets, other architectures and SDK setup.

## Direct CMake and existing startup

For an example built directly from the repository, the application presets
expose one `firmware` target:

```sh
cmake --preset uno-r4-minima-app
cmake --build --preset uno-r4-minima-app
```

The default is Blink; configure with `-DMCUHW_EXAMPLE=tmp102` to select TMP102.
Use `nucleo-h753zi-app` or `esp32p4-nano-app` for the other boards. Supply the
ESP SDK paths described below when configuring its preset. The output is
`build/<preset>/application/firmware.elf`, accompanied by its binary and identity
reports. These presets do not enable tests, header checks, coverage or probes.
Their tools must already be installed; the frontend's explicit setup workflow
remains available for creating and building an external application.

The existing `uno-r4-minima` and `nucleo-h753zi` presets build instrumented
board-test images for the documented physical procedures. Select the `*-app`
presets for application work. New board manifests can use a local configure
preset inheriting `application-base`, setting `MCUHW_BOARD` and a unique
`binaryDir`; its build preset references that configuration and targets
`firmware`. Adding a board does not require editing the shared preset list.

The generated application is a normal CMake project. The equivalent ARM build
is, with paths and board selected for your host:

```sh
cmake -S ../my-blink -B ../my-blink/build/manual-uno -G Ninja -DMCUHW_SOURCE_DIR=/path/to/mcuhw -DMCUHW_BOARD=arduino/uno_r4_minima -DMCUHW_BOARD_REVISION=r4 -DCMAKE_TOOLCHAIN_FILE=/path/to/mcuhw/cmake/McuhwToolchain.cmake -DMCUHW_STARTUP=supplied
cmake --build ../my-blink/build/manual-uno --target firmware
```

For ESP also supply `MCUHW_IDF_PATH`, `MCUHW_IDF_TOOLS_PATH` and `PYTHON`, and
activate or explicitly select the SDK compiler. `build --dry-run` prints the
exact equivalent configure/build argument arrays for your recorded environment.

An existing BSP/RTOS application chooses `MCUHW_STARTUP=external` and supplies
its own startup sources, linker script and reset/clock services through the
documented [firmware helper](../../cmake/McuhwBoard.cmake). In this mode mcuhw
does not add its board vectors or reset owner. `new --startup external` records
that choice; the generated application must then be adapted to provide its BSP
before it can link. Use a separate build directory when switching startup.

## Qualification and diagnostics

The initial implementation was exercised on macOS arm64. The lock records
observed versions rather than claiming every host/tool combination is qualified:

| Input | Recorded baseline | Scope |
| --- | --- | --- |
| Frontend Python | 3.14.7 (frontend supports 3.10+) | Local CLI/unit checks |
| CMake / Ninja | 4.4.0 / 1.12.1 | Local external builds |
| Arm compiler and libstdc++ | Arm GNU 15.2.Rel1, GCC 15.2.1 | C++23 probe and UNO/NUCLEO applications |
| RISC-V compiler and libstdc++ | esp-15.2.0_20251204, GCC 15.2.0 | C++23 probe and ESP32-P4 SDK applications |
| ESP-IDF / SDK Python | v6.1 at the commit above / 3.12.2 | Complete local SDK application build and package checks |
| J-Link / OpenOCD | 9.76 / 0.12.0 | Version inspection and command planning; no new physical flash qualification |
| Linux / Windows | Portable tests and CI workflow supplied | Not executed on those hosts during this implementation; results must be recorded before claiming qualification |

The frontend tests exercise paths containing spaces, exact subprocess arguments,
setup imports, explicit overrides, missing tools, changed identities, C++ library
diagnostics, cache misses, checksum failures, offline reuse and each flash
adapter without touching hardware:

```sh
python -m unittest discover -s tests/tooling -p test_frontend.py -v
```

`doctor --operation build` checks only build prerequisites. `doctor --operation
flash` checks only the selected runner. Missing tools name their import option
and expected version; changed tool identities require explicit setup, and a
C++23 header failure names the matching standard-library installation. Setup
reports are machine-local artifacts, not physical hardware evidence.
