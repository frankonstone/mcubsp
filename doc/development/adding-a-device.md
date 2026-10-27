# Adding an external chip

Add its public protocol implementation under `include/mcubsp/device/<category>/`
and a local `doc/device/<vendor>/<part>/README.md`. Record the exact chip and datasheet
scope, supported modes, wiring needs and limitations in that README.

Depend on a narrow I2C or SPI endpoint rather than a board type. The application
owns pins, power, reset, chip select and addresses. Add focused protocol tests
under the matching `src/device/<category>/test/` and a complete example when useful. Test sequencing,
errors, deadlines and cleanup with a fake transport, then cross-build the example
with a compatible board. Bus compatibility alone does not establish physical
acceptance; record physical results only after running a named fixture.
