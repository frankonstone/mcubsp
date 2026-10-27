add_executable(mcubsp.test.device.w5100
    test_w5100.cpp
    test_w5100_io.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.device.w5100 device:wiznet/w5100)
