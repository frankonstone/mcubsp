# Adding an MCU and native backend

Start from the existing `include/mcuhw/mcu/` descriptions and
`include/mcuhw/backend/` implementations. Pin the exact part, package, silicon
revision, manuals and errata before adding register and route facts. Keep
package differences explicit in C++ types.

Add focused description tests under `tests/mcu/<part>/` and backend tests under
`tests/backend/<part>/`. Add or reuse shared architecture definitions and
restricted runtime flags, then create a board binding through
[the board guide](adding-a-board.md). Cross-build complete examples and run
negative route and ownership checks. A successful host test or cross-build does
not establish physical behavior; record fixture results separately with mcutrace.
