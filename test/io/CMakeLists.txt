add_executable(mcuhw.test.io
    test_io.cpp
    test_io_boards.cpp
    test_io_i2c_devices.cpp
    test_io_peripherals.cpp
    test_io_spi_devices.cpp
    test_spi_device_backends.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
mcuhw_configure_native_suite(mcuhw.test.io io)
