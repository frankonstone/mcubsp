# Installation and dependencies

mcubsp provides C++23 header-only interfaces. Applications can consume a source
checkout, a versioned CMake package, or the public headers directly. Ordinary
library use needs no board selection or the Python frontend,
test framework or sibling checkout.

Choose [existing-BSP consumption or a new board application](application.md)
before selecting tools. An application uses either the source checkout or an
installed dependency; it does not need to run mcubsp's maintainer workflow.

## Use a source checkout

Add mcubsp to your application and link only the interface you use:

```cmake
add_subdirectory(external/mcubsp EXCLUDE_FROM_ALL)
target_link_libraries(my_firmware PRIVATE mcubsp::mcubsp)
```

The interfaces `mcubsp::io` and `mcubsp::device` are also available. They include
the core interface transitively. Source inclusion defines only library
interfaces by default; it preserves the application's compiler options and
`BUILD_TESTING` setting. No `BUILD_TESTING=OFF` workaround is required.

Use CMake 3.25 or newer and a compiler with a matching C++23 standard library.
For complete selected-board applications, the
[first-build guide](building.md) records actual tested tool versions and
provides setup, diagnostics, build and explicit flash commands.

## Install a relocatable package

Configure and install the library from a checkout:

```sh
cmake --preset library
cmake --build --preset library
cmake --install output/build/library
```

An application consumes the package with:

```cmake
find_package(mcubsp 0.1 CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE mcubsp::device)
```

The default installation is `output/install`. Configure a dependent application
with `-DCMAKE_PREFIX_PATH=/absolute/path/to/mcubsp/output/install`, or set
`mcubsp_DIR` to that directory's `lib/cmake/mcubsp` subdirectory. An explicit
`cmake --install output/build/library --prefix /path/to/install` selects another prefix.
The
package exports `mcubsp::mcubsp`, `mcubsp::io` and `mcubsp::device`; version matching
uses the same minor release. It can be relocated as one installation prefix.

The layout is deliberately mirrored so board settings and resources keep
their relative paths:

```text
<prefix>/
  lib/cmake/mcubsp/       CMake configuration, version and imported targets
  share/mcubsp/
    include/mcubsp/      Public headers
    cmake/              Public toolchains and firmware helpers
    src/board/          Board metadata, startup, linker and SDK boot resources
    example/            Standalone example applications, grouped by topic
    doc/package.md      Self-contained application package guide
    tool/               Firmware records and the board runner
```

Both source inclusion and `find_package` expose the public `mcubsp_add_firmware`
function when a new board application requests it. Ordinary library inclusion
does not select a board or create a firmware target. The installed
`share/mcubsp/cmake/board-toolchain.cmake` is the public pre-project board selector;
the copyable starter is under `share/mcubsp/example/blink/`.

`find_package` exposes `MCUBSP_ROOT` as `<prefix>/share/mcubsp`. Imported targets
supply `<prefix>/share/mcubsp/include` automatically. SDKs, toolchains, private
vendor documents, development checkouts and build products are not bundled.
Root and module tests, test-only CMake files, captured evidence and internal
plans stay in the checkout. The package includes only application resources;
the full development documentation remains with the source tree.

## Direct headers

For a build system other than CMake, add `<checkout>/include` or
`<prefix>/share/mcubsp/include` to the compiler's include search path and enable
C++23. The public include names stay the same, such as
`#include <mcubsp/device/sensor/tmp102.hpp>`. An existing firmware remains
responsible for its BSP, startup and linker integration.

## Contributor and maintainer dependencies

Native verification uses `MCUBSP_BUILD_HOST_TESTS`; isolated headers,
Cross checks and board-test images have independent options. MCUIP adapters and
contracts belong to the sibling [integration project](../../../integration/README.md),
which is not included in the mcubsp package.
See [build and verification](build-and-test.md). Ordinary top-level configuration
builds and installs the application package without running tests.
