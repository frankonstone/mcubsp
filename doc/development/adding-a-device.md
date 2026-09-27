# Adding an external chip

Use [`catalog/templates/device.json`](../../catalog/templates/device.json) to create
`device/<vendor>/<part>/component.json` and a local `README.md`. Existing hardware
facts in a copied template must all be replaced before validation/review.

1. Choose the exact chip/revision scope, datasheet baseline and implemented modes.
   Distinguish similar parts explicitly; W5100 support does not imply W5500 support.
2. Put protocol code under `include/mcuhw/device/<category>/`. Depend on a narrow
   transport contract, such as an owned I2C/SPI endpoint, rather than a board type,
   MMIO address or scheduler. The board/application owns pins, resets, power,
   clocks, addresses and chip selects.
3. Declare `requires` transport capabilities, `provides` functionality, authoritative
   source anchors, focused tests, examples, documentation and maintainers locally.
   Reference shared protocol/IP code when applicable instead of copying it. Do not
   register the chip in a central build list.
4. Test protocol sequencing, response validation, errors, deadlines, cancellation
   and cleanup using a fake transport. Add a compile check through a real endpoint
   contract. List those focused test sources in the manifest; source presence is
   distinct from recorded execution evidence.
5. Provide a complete application and document supply/logic levels, pull-ups, address
   straps, wiring/profile selection and any required delays. The device's README
   should state supported operations and known limitations directly.
6. Validate the manifest and application composition. Physically execute one named
   fixture before adding passed physical evidence. Compatibility with other boards
   remains a transport-contract statement until their compositions are tested.

For example, with `PYTHONPATH=tools`,
`catalog.compose(root, 'st/nucleo_h753zi', ['ti/tmp102'])` checks the board's exposed
I2C capability. It does not verify the sensor is wired, powered or observed.

Run `python3 -m unittest discover -s tests/tooling -p test_catalog.py -v` for catalog
and local-contribution regression coverage. See [catalog rules](catalog.md) for
validation, evidence and API migration policy.
