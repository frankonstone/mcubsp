# Installation and dependencies

mcubsp provides C++23 header-only interfaces. Applications can consume a source
checkout, a versioned CMake package, or the public headers directly. Ordinary
library use needs no board selection or the Python frontend,
test framework or sibling checkout.

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
cmake --preset arduino-uno-r4-minima
cmake --install build/arduino-uno-r4-minima --prefix /path/to/install
```

An application consumes the package with:

```cmake
find_package(mcubsp 0.1 CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE mcubsp::device)
```

Configure the application with `-DCMAKE_PREFIX_PATH=/path/to/install`. The
package exports `mcubsp::mcubsp`, `mcubsp::io` and `mcubsp::device`; version matching
uses the same minor release. It can be relocated as one installation prefix.

The layout is deliberately mirrored so board settings and resources keep
their relative paths:

```text
<prefix>/
  lib/cmake/mcubsp/       CMake configuration, version and imported targets
  share/mcubsp/
    include/mcubsp/      Public headers
    cmake/              Toolchains and remaining CMake files
    board/              Board definitions, local examples and documentation
    doc/                Project documentation
    test/               Test sources
```

`find_package` exposes `MCUBSP_ROOT` as `<prefix>/share/mcubsp`. Imported targets
supply `<prefix>/share/mcubsp/include` automatically. SDKs, toolchains, private
vendor documents, development checkouts and build products are not bundled.

## Direct headers

For a build system other than CMake, add `<checkout>/include` or
`<prefix>/share/mcubsp/include` to the compiler's include search path and enable
C++23. The public include names stay the same, such as
`#include <mcubsp/device/sensor/tmp102.hpp>`. An existing firmware remains
responsible for its BSP, startup and linker integration.

## Contributor and maintainer dependencies

The top-level CMake project has no optional test or probe switches. The
previous maintainer build targets depended on removed CMake helpers. The
header-only library and installed package need no sibling checkouts.
