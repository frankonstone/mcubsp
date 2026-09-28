# Adding a board on an existing MCU

Create `board/<vendor>/<name>/board.cmake` and a local README. Use an existing
board entry for the same architecture as a starting point. The entry declares the
supported revision, architecture, toolchain, C++ profile, examples, runner and
startup files. Board IDs are the directory's `<vendor>/<name>` path.

Add the public pin aliases under `include/mcuhw/board/` and the application
profile under `include/mcuhw/io/boards/`. Keep register facts in the MCU
headers and controller sequencing in the reusable backend. Put startup sources
in `bsp/`, physical fixtures in `tests/`, and wiring and recovery procedures in
`docs/`.

`MCUHW_BOARD_EXAMPLES` lists complete `examples/.../CMakeLists.txt` projects.
Each project creates the requested `MCUHW_FIRMWARE_TARGET`. A board may also
set `MCUHW_BOARD_IO_EXAMPLES` to source names from `examples/io/`. Add
`tests.cmake` for a root style instrumented test build, or set
`MCUHW_BOARD_TEST_PROJECT` for a standalone SDK test project.

Configure a matching architecture preset with `-DMCUHW_BOARD=<vendor>/<name>`.
Cross-build declared applications, check external startup for duplicate reset
ownership, and run focused host tests. Execute physical fixtures with the
recorded wiring before claiming hardware results. Register actual results with
mcutrace and document untested scopes beside the board.
