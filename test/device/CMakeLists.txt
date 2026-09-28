if("all" IN_LIST MCUHW_TEST_SUITES OR "device" IN_LIST MCUHW_TEST_SUITES OR "device:everest/es8311" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.device.es8311
        test_es8311.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.device.es8311 device:everest/es8311)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "device" IN_LIST MCUHW_TEST_SUITES OR "device:ti/tmp102" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.device.tmp102
        test_tmp102.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.device.tmp102 device:ti/tmp102)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "device" IN_LIST MCUHW_TEST_SUITES OR "device:wiznet/w5100" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.device.w5100
        test_w5100.cpp test_w5100_io.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.device.w5100 device:wiznet/w5100)
endif()
