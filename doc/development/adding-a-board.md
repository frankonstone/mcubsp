# Adding a board on an existing MCU

Use this path when the MCU description and required native implementations exist.
Start with [`catalog/templates/board.json`](../../catalog/templates/board.json)
and an existing local `board.cmake` for the same platform.

1. Create `board/<vendor>/<name>/component.json`, `board.cmake` and `README.md`.
   Choose a stable ID and exact supported board revision. Pin the schematic,
   connector pinout, oscillators, memory, bootloader, jumper and solder-bridge
   assumptions. Do not infer a PCB revision from the MCU silicon revision.
2. Reference the shared `mcu:<vendor>/<part>` entry. Add board-level pin aliases
   under `include/mcuhw/board/` and an application profile under
   `include/mcuhw/io/boards/`; retain compile-time route/ownership checks. Register
   the actual public headers and source anchors in the manifest.
3. Bind shared build helpers from local `board.cmake`. Set `build.entry` and
   `build.toolchain`, `build.architecture` (matching `MCUHW_ARCH`), supplied
   `build.profile_header`, `build.profile_type`, startup/linker/clock bindings and
   shared runner configuration. External-BSP startup remains explicitly selectable.
   Reuse a runner adapter; do not add a root CMake branch, board option or preset.
4. Keep startup sources in `bsp/`, physical fixtures and their scripts in `tests/`,
   and wiring/procedure documents in `docs/`. Put complete example entry points, troubleshooting,
   flash regions and recovery instructions in the component metadata/docs. Flashing
   remains separate from building and names a probe/port explicitly.
5. Validate the catalog and cross-build Blink and any claimed device example with
   this board selected. Check external-startup composition has only one owner of
   vectors/reset/clocks. Run only applicable focused tests before broader maintainer
   checks. No sibling development checkout is an application prerequisite.
6. Execute physical fixtures with their documented wiring before claiming hardware
   results. Preserve the exact board/silicon, firmware, toolchain, fixture and
   observation method. Register actual artifacts with mcutrace and keep unexecuted
   scopes `missing`; component existence never proves physical support.

Run catalog validation and inspect only this closure:

```sh
PYTHONPATH=tools python3 -c 'from mcuhwlib.catalog import validate; errors=validate("."); print("\n".join(errors)); raise SystemExit(bool(errors))'
python3 tools/mcuhw.py list boards
```

The tooling tests create an additional local board with shared infrastructure and
verify discovery without central edits. See [catalog rules](catalog.md) for schema,
revision/evidence semantics and compatibility policy. Recheck all copied values
against the new board's own sources before publishing its entry.

For an architecture workspace, the board's `examples` array lists complete
`examples/.../CMakeLists.txt` entry points. Each creates the executable named by
`MCUHW_FIRMWARE_TARGET`; see the shared Blink example. The workspace supplies the
board/revision, supplied startup, unique target name and separate binary directory.
Add `board/<vendor>/<name>/tests.cmake` for a root-style instrumented test build,
or `build.test_project` for a standalone ESP-IDF test project. The `.tests` group
configures that build only when requested. A list of test source files alone does
not create an executable test group.

Set `MCUHW_BOARD` to the new board's ID, reconfigure the matching
architecture preset and build `firmware`, `tests` or `examples`. No central registration
is needed. See [board selection](verification.md#board-selection).

A board binding can set `MCUHW_BOARD_IO_EXAMPLES` in `board.cmake` to the
compatible standalone source names from `examples/io/` (without `.cpp`). These
appear automatically as `example.io.<name>` alongside `example.app.<name>`.
Only declare examples compatible with the board's profile and startup.
