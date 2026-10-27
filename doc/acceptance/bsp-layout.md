# Completed BSP directory layout

Software verification recorded on 26 October 2026 for PLN-0038. This completes
the support-tree move following the [public board consolidation](board-layout.md).

The former root `board/` directory is now `bsp/`. Each vendor/board package owns
`board.cmake`, `startup/`, examples, physical tests and documentation. Arm
`linker.ld` files sit beside `board.cmake`; the nested `bsp/` startup directories
are gone. Public C++ headers remain under `include/mcubsp/board/`.

CMake selection, default application paths, SDK projects, runners, package
installation, CI artifact paths, trace globs and current authored documentation
use the completed layout. Cached repository application defaults migrate from
the old support path when the matching new source exists. External application
paths and board identities remain unchanged. The standalone P4 SDK test project
uses a fresh `build/bsp-p4-tests` directory because its source directory moved.

## Executed checks

| Check | Result |
| --- | --- |
| GCC 15.1 and AppleClang 21 native coverage host | 43/43 CTest entries each; 290 merged cases, zero failures |
| Repository firmware | UNO, NUCLEO and P4 link to the new `bsp/firmware.elf` output location |
| Existing build caches | UNO/NUCLEO repository application defaults migrated to `bsp/` and rebuilt |
| Standalone applications | Source starters and relocated-package starters link for all three boards |
| Installed resources | Fresh relocated package has `bsp/`, board-local `startup/` and Arm root linker scripts; no former root `board/` tree |
| Board-test images | All ten UNO, three NUCLEO and one P4 SDK images build from moved sources |
| Runner planning | All 14 new image/input bundles validate against moved board entries; no device command sent |
| Resource report | UNO static ELF resource reporting succeeds for nine images using current paths |
| Capture preservation | Eleven historical P4 capture files moved byte for byte |
| CI definition | Updated artifact paths; YAML parses locally; hosted CI execution remains separate |

Native reports remain under `build/host/artifact/` and
`build/host-coverage-clang/artifact/`. Repository firmware uses the `bsp/`
subdirectory of `build/review-uno`, `build/review-nucleo` and `build/review-p4`.
Board-test outputs use the `bsp/` subdirectories of `build/uno-coverage` and
`build/nucleo-tests`, plus the fresh `build/bsp-p4-tests` SDK build. Installed
checks use `build/bsp-package-relocated`, temporary Arm application directories
and `build/bsp-p4-installed`. These are local generated outputs, not committed
evidence snapshots.

Historical records retain their original source/build identities. No hardware
was flashed or physical observation refreshed. See the current
[directory layout and migration](../development/structure.md),
[application guide](../getting-started/application.md) and
[hardware procedures](../getting-started/hardware.md).
