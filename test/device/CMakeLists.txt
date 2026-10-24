add_executable(mcubsp.test.device.es8311
    test_es8311.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.device.es8311 device:everest/es8311)

add_executable(mcubsp.test.device.tmp102
    test_tmp102.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.device.tmp102 device:ti/tmp102)

add_executable(mcubsp.test.device.w5100
    test_w5100.cpp test_w5100_io.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.device.w5100 device:wiznet/w5100)
