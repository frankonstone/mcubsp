# Adding an MCU and native backend

A new MCU needs an authoritative description before functional backend acceptance.
Use [`catalog/templates/mcu.json`](../../catalog/templates/mcu.json) and the current
`mcu/<vendor>/<part>` / `ip/<vendor>/<implementation>` entries as structural examples.

1. Select the exact family, parts/packages, silicon revisions, manuals, datasheets
   and errata. Record conflicts and unavailable material. Separate package/part
   variants from board revisions and from backend implementation versions.
2. Add `include/mcuhw/mcu/<mcu>/` using `model.hpp`, `system.hpp`, `peripherals.hpp`,
   `pins.hpp`, `interrupts_dma.hpp`, `coverage.hpp` and `description.hpp`. Keep
   native differences explicit instead of forcing another vendor's model onto it.
3. Create `mcu/<vendor>/<part>/component.json`, with variants anchored to reviewed
   C++ facts, description tests and maintained review documentation. List shared IP
   implementation dependencies. MCU metadata must not copy board connector wiring.
4. Reuse an existing `ip` component when the implementation is actually shared.
   Otherwise add `ip/<vendor>/<implementation>/component.json` and incrementally
   implement native backends under `include/mcuhw/backend/`. Register focused
   fake/recording-MMIO tests, including error cleanup. Peripheral operations retain
   existing storage, ownership, deadline and transport contracts.
5. Add or reuse shared architecture barriers/toolchain definitions and restricted
   runtime flags. A genuinely new platform may need a common helper/runner adapter;
   another part using an existing platform should require only local metadata and
   implementation. Verify emitted barrier instructions and package route negatives.
6. Add an exact board binding following [the board guide](adding-a-board.md). Keep
   supplied reset/clocks/vectors/linker integration board-owned and external startup
   explicitly selectable. Cross-build complete examples and run focused host checks.
7. Record physical procedures, exact identities, sizes, results and unexecuted scopes
   before claiming hardware support. Host description coverage and compile success
   alone do not establish register behavior or electrical accuracy.

When the target reveals a new capability difference, extend the metadata/functional
contract explicitly and rerun affected existing targets. Keep the catalog summary
anchored to authoritative C++ instead of creating a second register database.

See [catalog validation and compatibility rules](catalog.md). Full-maintainer
header, description, compile-fail and architecture checks remain distinct from
ordinary application builds.
