# Firmware input identity

Ninja firmware builds use Python 3.10+ to record inputs after successful linking.
The adjacent `.elf.inputs.json` contains selected source/header/generated-file
hashes, transitive static-library and linker-script hashes, compiler specification
files, build commands and compiler/tool fingerprints. GNU ld dependency files
include the implicit runtime libraries, which are absent from ordinary compiler
dependency logs. Ninja response-file contents are expanded even after the
temporary response files have been deleted.

The producer follows the selected target graph and its compiler dependency log;
it does not scan every source file in the repository. P4 records include the
SDK application and separate bootloader graphs, the generated SDK configuration
header and partition-table inputs. The test project explicitly selects mcubsp's
factory-only boot component through the SDK's bootloader component property.
SDK projects set `MCUBSP_APPLICATION_VERSION` explicitly (default `0.1.0`), so
the SDK does not inject the whole-repository Git description into selected
compile flags. Change that setting when choosing an application release version.

`identity_sha256` covers those selected file bytes, commands, tools and
board/revision/startup identity. Source and build root paths in commands and
file names use stable labels; external toolchain paths are retained. This is an
input identity, not a guarantee of identical binaries across relocated toolchains
or hosts. Tool fingerprints cover the compiler driver and discoverable GCC
compiler, assembler and linker programs. The record also lists selected runtime
libraries and explicit compiler specifications.

Repository revision and tracked dirty state are recorded separately and are
excluded from the selected-input identity. An unrelated file or repository-only
commit therefore leaves that identity unchanged when the actual selected inputs
and commands are unchanged. A selected comment/header change still alters the
identity even if optimization produces the same machine code.

The `.elf.build.json` record binds the ELF hash and the input sidecar's checksum
and identity. Keep all three files together when archiving firmware. Runner
planning validates the sidecar checksum before allowing a flash operation. No
source checkout is required to flash an archived, completed bundle; its record
describes the inputs used to produce that image, not the current working tree.

The producer fails on missing/stale compiler dependencies, missing linker
dependency files or missing selected files. A failed artifact producer removes
the previous completed build record. Rebuild/relink with the current helper to
repair it. Shared-producer changes trigger relinking for Arm applications.

Current automatic tracking supports the single-configuration Ninja generator
and the supplied GNU Arm/P4 linkers. Other generators retain the ELF/build record
and explicitly report input tracking as unavailable. This does not qualify new
host/compiler combinations, replace physical observation or complete the broader
repository setup/workspace qualification. Source identity strings embedded in
older board test JSON remain `unavailable`; use the verified adjacent sidecar
instead of interpreting those placeholders as a hash.

`mcubsp.test.provenance` requires Git and tests a real Ninja/CMake transitive library build,
selected header and generated configuration changes, compile flags, linker
inputs, unrelated commits, response-file expansion and stale dependency
rejection. `mcubsp.test.runner` covers missing/modified sidecars and failed
producer invalidation. See [verification](verification.md) and
[hardware planning](../getting-started/hardware.md).
