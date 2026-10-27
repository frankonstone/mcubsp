# Cross-library integration extraction

Verified locally on 27 October 2026. This supersedes earlier mcubsp build
instructions that enabled MCUIP integration options or built Ethernet/MCUIP
applications from the mcubsp BSP tree. Historical physical records retain their
original source and observation scope.

The sibling [integration project](../../../integration/README.md) owns three
MCUIP adapter headers, three contract probes, UNO/NUCLEO Ethernet applications,
their test report harnesses and capture runners. Its first module is
`mcubsp-mcuip`, with target/namespace `mcuintegration::mcubsp_mcuip` and public
headers under `mcuintegration/mcubsp_mcuip/`.

`include/mcubsp/integration/` is removed without compatibility headers. mcubsp
has no MCUIP source lookup, integration option or integration target. Native
hardware/device tests include their hardware headers directly. Optional mcubsp
board tests no longer load MCUIP. Protocol-neutral Ethernet backends, W5100
protocol/transport, raw examples and board startup stay in mcubsp.

| Check | Result |
| --- | --- |
| Native GCC 15.1 and AppleClang 21 configurations | 43 CTest entries; 290 cases pass in each |
| Native coverage | Fresh AppleClang coverage exported successfully |
| Public headers | 110 headers compile independently; three removed adapters are outside the package |
| mcubsp architecture probes | Cortex-M4, Cortex-M7 and ESP32-P4 checks pass |
| mcubsp UNO/NUCLEO board tests | Nine UNO and two NUCLEO images link with coverage, without MCUIP |
| Source and relocated-package consumers | Workflow regression checks pass; package contains no integration headers or integration targets |
| Extracted contracts | Host and all three target architectures compile against the explicit MCUIP checkout |
| Extracted board applications | Both images link with and without integration coverage |
| Runner planning | Both extracted runners validate the exact ELF and selected-input record in dry runs; no device command executed |

Integration coverage instruments that project's adapters, composition and report
harness. mcubsp owns coverage/physical qualification of peripheral code and
startup; MCUIP owns protocol coverage. Splitting the instrumented composition
from supplied startup avoids treating external library sources as integration
sources. Compiler and linked-input identities still include the selected
libraries and BSP startup. Original integration adapter/report headers remain explicit Ninja inputs, so
their changes rebuild the composition and invalidate the image record. Native
and compiler checks do not establish physical Ethernet acceptance.

The core trace graph continues to cover mcubsp inputs only. Moving composition
sources out does not make their physical acceptance complete or import their
builds as current physical evidence. No new board traffic, timings, link-cycle
or cache/descriptor recovery observation was captured during this extraction.

Core trace validation has zero errors and 40 warnings: missing BSP coverage
evidence and the existing motor-PWM, RTOS and physical-test evidence gaps.
