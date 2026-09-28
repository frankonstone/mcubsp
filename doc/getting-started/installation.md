# Installation and dependencies

mcuhw provides C++23 header-only interfaces. Applications can consume a source
checkout, a versioned CMake package, or the public headers directly. Ordinary
library use needs no board selection or the Python frontend,
test framework or sibling checkout.

## Use a source checkout

Add mcuhw to your application and link only the interface you use:

```cmake
add_subdirectory(external/mcuhw EXCLUDE_FROM_ALL)
target_link_libraries(my_firmware PRIVATE mcuhw::mcuhw)
```

The interfaces `mcuhw::io` and `mcuhw::device` are also available. They include
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
cmake -S . -B build/package -DMCUHW_INSTALL=ON -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --install build/package
```

An application consumes the package with:

```cmake
find_package(mcuhw 0.1 CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE mcuhw::device)
```

Configure the application with `-DCMAKE_PREFIX_PATH=/path/to/install`. The
package exports `mcuhw::mcuhw`, `mcuhw::io` and `mcuhw::device`; version matching
uses the same minor release. It can be relocated as one installation prefix.

The layout is deliberately mirrored so board settings and resources keep
their relative paths:

```text
<prefix>/
  lib/cmake/mcuhw/       CMake configuration, version and imported targets
  share/mcuhw/
    include/mcuhw/      Public headers
    cmake/              Board/toolchain and firmware helpers
    board/ device/      Component definitions and local documentation
    example/ tool/    Complete applications and optional frontend
```

`find_package` exposes `MCUHW_ROOT` as `<prefix>/share/mcuhw`. Imported targets
supply `<prefix>/share/mcuhw/include` automatically. SDKs, toolchains, private
vendor documents, development checkouts and build products are not bundled.

## Direct headers

For a build system other than CMake, add `<checkout>/include` or
`<prefix>/share/mcuhw/include` to the compiler's include search path and enable
C++23. The public include names stay the same, such as
`#include <mcuhw/device/sensor/tmp102.hpp>`. An existing firmware remains
responsible for its BSP, startup and linker integration.

## Contributor and maintainer dependencies

Tests, architecture probes, complete header checks, hardware test images and
coverage are explicit workflows. A native contributor can enable
`MCUHW_BUILD_TESTS=ON` and provide `MCUHW_MCUTEST_SOURCE_DIR` for mcutest; a
library consumer does not need it. `MCUHW_BUILD_TOOLING_TESTS=ON` enables the
Python packaging and workflow checks. `MCUHW_BUILD_PROBES=ON`,
`MCUHW_CHECK_HEADERS=ON` and `MCUHW_WRITE_TOOLCHAIN_REPORT=ON` enable their
corresponding maintainer checks.

Optional integrations have explicit source overrides including
`MCUHW_MCUTEST_SOURCE_DIR`, `MCUHW_MCUJSON_SOURCE_DIR`,
`MCUHW_MCUCOV_SOURCE_DIR` and `MCUHW_MCUIP_SOURCE_DIR`. Sibling development
checkouts are convenient defaults only when an associated maintainer feature
is requested; their presence does not select application features. Existing
maintainer presets retain their explicit verification settings.

Continue with [library use](using-mcuhw.md),
[selected-board applications](building.md), or
[build and verification](build-and-test.md).
