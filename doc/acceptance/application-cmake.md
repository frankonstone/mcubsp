# Application CMake boundary

Software verification recorded on 26 October 2026 for PLN-0036. This work
addresses library users' integration responsibilities, independently of the
framework maintainer workflow. No new physical observations or hosted CI runs
were performed.

Board-name follow-up on the same date: the generated alias described in the
initial checks below was removed at the user's request. Application sources
now include and name their concrete boards explicitly, with one Blink source
per vendor/board identity. The public CMake selection/startup interface remains.
The follow-up rebuilt all three source starters and installed UNO/NUCLEO starters,
reran the nine-case CMake workflow check and validated the trace graph with zero
errors. Selected input graphs contain neither the removed header nor its include
directory. Native behavior/coverage suites retain their preceding results.

## Public contract

- Existing BSP/RTOS projects link the interface targets and retain their own
  executable, compiler policy, startup and tests. Source inclusion creates no
  repository firmware even when the parent defines `MCUBSP_BOARD`.
- Source inclusion, source-tree package discovery and installed `find_package`
  expose `mcubsp_add_firmware` directly. Loading helpers does not enable ASM or
  discover Python, a debugger or test dependencies.
- New board projects select `cmake/board-toolchain.cmake` and `MCUBSP_BOARD`
  before `project()`. The selected local board entry owns the architecture
  mapping. Unknown boards/revisions and reuse of another board's build directory
  fail with focused diagnostics.
- Explicit firmware creation prepares a target-private
  `<mcubsp/selected_board.hpp>` alias and defaults the function's board/revision
  from the configured selection. Startup ownership remains explicit. Supplied
  Arm startup requires the application to declare the ASM language.
- A copyable standalone Blink project ships in both checkout and installation.
  P4's outer target remains an explicit SDK subproject, with its component API
  limitation documented rather than presented as an ordinary executable target.

## Executed checks

| Check | Result |
| --- | --- |
| Ordinary GCC 15.1 host and AppleClang 21 native coverage host | 43/43 CTest entries each; 289 merged cases, zero failures |
| Existing-BSP consumption | Source/package consumers retain parent tests, compiler policy and languages; no unrequested firmware/check targets; public function available |
| Public configuration diagnostics | Invalid board/revision and changed build-directory owner rejected before compiler discovery; misspelled/empty firmware arguments rejected |
| Source starter | UNO R4, NUCLEO H753 and P4 Blink link with the public board toolchain and generated profile |
| Relocated-package starter | Same three boards link from copied installed examples; selected library headers resolve exclusively within the relocated prefix |
| Paths with spaces | Installed UNO/NUCLEO application source/build paths with spaces link and produce firmware input records |
| Existing repository path | Root UNO `firmware` target still links through its existing preset integration |
| Workflow definition | CI builds the public starter in its Arm/P4 jobs; YAML parses locally; hosted execution remains pending |

Source starter outputs use `build/consumer-uno`, `build/consumer-nucleo` and
`build/consumer-p4`. The installed P4 output uses `build/consumer-p4-installed`;
installed Arm starter checks used temporary directories. The relocated prefix
used `build/consumer-relocated`. These are local generated outputs, not committed
evidence snapshots. Native coverage retains its prior selected native-suite
scope; external target links do not count as physical execution.

See the [application guide](../getting-started/application.md) for the user-facing
contract and [starter](../../example/blink/README.md) for complete commands.
The older toolchain paths and explicit helper includes remain available; the
guide records the source-inclusion and ASM migration details. This milestone
does not close host setup/workspace or hardware qualification milestones.
