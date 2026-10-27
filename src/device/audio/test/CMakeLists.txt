add_executable(mcubsp.test.device.es8311
    test_es8311.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.device.es8311 device:everest/es8311)
