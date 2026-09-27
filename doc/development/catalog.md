# Local component catalog

The catalog describes support and selects build entry points. Public C++ headers
remain the source of register definitions, pin routes and compile-time constraints.
No catalog generation or Python package is needed for ordinary header/CMake library
consumption. The optional tools use Python 3.10 or newer and its standard library.

## Identities and ownership

Each component owns `<kind>/<vendor>/<name>/component.json`. Kinds are `board`,
`device`, `mcu` and `ip`. Examples are `board:st/nucleo_h753zi`,
`device:ti/tmp102`, `mcu:st/stm32h753` and `ip:st/stm32h753_native`.
IDs are stable lowercase vendor-qualified names. References use
`kind:vendor/name` or `kind:vendor/name@revision`. Revision selection defaults
only to the explicitly declared `default_revision`.

- IP entries own reusable backend implementations. The initial three native
  backend collections reference existing headers; they do not duplicate them.
- MCU entries own family/part/package/silicon descriptions and reference IP
  implementations. The `variants` records summarize named C++ source facts.
- Board entries own wiring, startup/build rules and runner configuration. Their
  dependencies select the shared MCU and any permanently mounted devices.
- Device entries own chip protocols and declare transport requirements. A chip
  does not depend on one board merely because its first fixture used that board.

The UNO `r4` binding identifies the documented UNO R4 Minima model; an exact PCB
revision was not recorded in historical evidence. NUCLEO uses `mb1364-rev-e`.
ESP32-P4-NANO `development-v1.0` identifies observed silicon v1.0, not an inferred
PCB revision. Its development observations do not qualify production-v3.x silicon.
These limits are stored in manifests and survive generated catalog output.

## Manifest contract

[`catalog/schema.json`](../../catalog/schema.json) defines component schema version 2.
The [migration guide](structure.md#component-schema-1-to-2) describes changed fields.
The standard-library validator implements precisely the vocabulary used by that
schema. Unsupported versions, unknown top-level fields, duplicate JSON keys/IDs,
missing references, cycles, unsafe paths and incompatible selections fail with
component-specific diagnostics.

Every entry declares name/category/summary; supported revisions; dependencies;
`headers`, `sources`, `tests`, `docs` and `examples`; responsible maintainers;
capabilities; limitations; and scoped evidence. Paths are repository-relative,
portable slash-separated file paths. They must remain inside the repository.
Legacy public header paths are stable; moving catalog ownership does not require
moving those headers. Board-private fixtures, BSPs and procedures live beside the
manifest under `tests/`, `bsp/` and `docs/`.

`provides` means implemented interfaces exposed by that component. `requires`
names transport contracts supplied by its composition, such as TMP102's `i2c`.
`described` records described-only hardware separately. `capability_sources` must
cover every provided or described capability with a reviewed C++ file and literal
symbol/anchor. The validator rejects deleted or renamed anchors. This check guards
against a disconnected summary; normal C++ contract and negative compilation tests
still enforce routes, ownership and configuration semantics.

MCU `variants` identify family, part, package, silicon scope and an authoritative
source anchor. Board `build.entry` selects local CMake rules, and `build.toolchain`
is resolved before language configuration. `build.profile_header` and
`build.profile_type` are the authoritative C++ profile selection. Other metadata names
required tool identities and shared runner adapter. A board using an existing
platform adds local rules without editing a root board list or adding a preset.

Architecture workspaces match `build.architecture` against `MCUHW_ARCH` and read
only the manifests named by `MCUHW_ACTIVE_BOARDS`. Their `examples` entries are
complete CMake application entry points supporting `MCUHW_FIRMWARE_TARGET`.
An optional `build.test_project` names a repository-relative ESP-IDF test project
directory; the catalog validates its `CMakeLists.txt`. Root-style board test
rules instead live at `board/<vendor>/<name>/tests.cmake`. Test configuration is
deferred until the board's `.tests` group is explicitly built.

## Discover, select and generate

From the checkout root, the optional frontend exposes these workflows:

```sh
python3 tools/mcuhw.py list boards
python3 tools/mcuhw.py list devices
python3 tools/catalog.py validate
python3 tools/catalog.py docs --output doc/reference/catalog.md
python3 tools/catalog.py index --output hardware-catalog.json
```

The Python API is also usable independently:

```python
from pathlib import Path
from mcuhwlib import catalog  # PYTHONPATH=tools

root = Path('.')
assert catalog.validate(root) == []
board = catalog.load(root, 'board', 'st/nucleo_h753zi', 'mb1364-rev-e')
selected = catalog.closure(root, 'board', board['id'])
compatible = catalog.compose(root, board['id'], ['ti/tmp102'])
temperature = catalog.search(root, kind='device', category='temperature', capability='i2c')
ethernet = catalog.search(root, kind='board', capability='ethernet')
flashable = catalog.search(root, kind='board', runner='openocd')
observations = catalog.search(root, kind='board', revision='mb1364-rev-e',
                              stage='physical', state='passed')
Path('hardware-catalog.json').write_text(catalog.render_index(root))
Path('hardware-catalog.md').write_text(catalog.render_markdown(root))
```

`load` and `closure` directly address known manifest paths. They never enumerate
other components. `discover`, `search`, `validate` and generators intentionally
scan the catalog: use them as discovery/contributor/maintainer operations, not as
mandatory application configuration. Output order is deterministic; timestamps
and host-specific paths are not embedded in generated indexes.

Capability compatibility is a preliminary composition check. It does not establish
pin availability, electrical compatibility, arbitrary mixed-device concurrency or
physical acceptance. Those remain the typed C++ checks and the exact fixture's job.

## Evidence has a scope and a state

Each evidence record has a stage (`host-test`, `cross-build`, `physical`), a state
(`passed`, `failed`, `stale`, `missing`), revision and narrative scope. A record may
reference an existing artifact or an explicitly historical observation report.
Listed test source files are not themselves passed test results. Build-only results
never create physical evidence.

Passed records with a missing referenced file display `missing`. A SHA-256 mismatch
displays `stale`; explicit `failed`, `stale` and `missing` states remain distinct.
The digest identifies the referenced record, not a new test execution or a proof
that current sources match a historical image. Historical observations explicitly
name that limitation. Evidence filters for stage/state/revision must match the same
record, so one host pass cannot combine with a different physical failure to produce
a physical pass. Generated tables retain all scopes and limitations.

Do not register unexecuted build outputs as evidence, infer physical support from
capability compatibility or silently upgrade stale evidence. Continue to register
real result artifacts with mcutrace under the repository's existing evidence policy.

## Contributions and compatibility

Use the focused [board](adding-a-board.md), [chip](adding-a-device.md), or
[MCU/backend](adding-an-mcu.md) guide and the matching
[`catalog/templates`](../../catalog/templates) JSON template. Templates are examples,
not catalog entries or supported hardware. Update every copied hardware-specific
value. Keep narrative wiring, limitations and troubleshooting beside the manifest.

Validate the component closure, run its focused software tests, and cross-build its
published application. Record physical acceptance only after executing the named
fixture. The catalog tooling test demonstrates a new local board and device using
shared infrastructure without central registration:

```sh
python3 -m unittest discover -s tests/tooling -p test_catalog.py -v
```

Keep public header paths and component IDs stable within a compatible release.
Adding optional manifest fields requires an explicit schema/tool change; tools
reject fields they cannot interpret. Incompatible schema semantics require a new
schema version and a migration guide. Renaming a public header, component ID,
revision or transport contract requires a documented source migration and a public
API compatibility decision; a catalog revision is not permission to break C++ APIs.
