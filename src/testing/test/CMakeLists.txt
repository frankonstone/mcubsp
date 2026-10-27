add_executable(mcubsp.test.testing
    test_recording_mmio.cpp
    test_description_suite.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.testing testing)
