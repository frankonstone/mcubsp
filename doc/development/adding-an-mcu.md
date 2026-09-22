# Adding a microcontroller

A new MCU needs an authoritative description before it receives functional
backends.

1. Select the exact family, parts/packages, silicon revisions, manuals,
   datasheets, and errata. Record conflicts and unavailable material.
2. Add `include/mcuhw/mcu/<mcu>/` using the existing split: `model.hpp`,
   `system.hpp`, `peripherals.hpp`, `pins.hpp`, `interrupts_dma.hpp`,
   `coverage.hpp`, and `description.hpp`.
3. Add the MCU to shared metadata tests, header self-containment checks,
   coverage inventory, route tests, and compile-fail cases. Preserve native
   differences rather than forcing another vendor's model onto it.
4. Add or reuse the architecture barriers, toolchain file, CMake preset, and
   restricted-runtime flags. Verify emitted barrier instructions.
5. Implement `include/mcuhw/backend/<mcu>/` incrementally. Reuse functional
   drivers, fixed storage, typed errors, ownership, deadlines, and transfer
   contracts. Test cleanup and failure paths with recording/fake MMIO first.
6. Add at least one exact board binding with startup, linker, interrupts,
   execution hooks, and a scoped physical procedure.
7. Run native tests, negative compile tests, the architecture build, linked
   firmware, and physical tests. Record identities, sizes, bounds, results,
   and non-claims before marking support complete.

When a target reveals a real capability difference, extend the metadata or
functional contract explicitly and re-run existing targets. Do not hide it in
a board macro or duplicate an otherwise portable driver.
