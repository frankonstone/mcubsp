add_executable(mcubsp.test.backend.shared
    test_spi_device_backends.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.shared backend)
