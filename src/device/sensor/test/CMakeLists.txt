add_executable(mcubsp.test.device.tmp102
    test_tmp102.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.device.tmp102 device:ti/tmp102)
