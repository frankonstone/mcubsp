if("all" IN_LIST MCUHW_TEST_SUITES OR "mcu" IN_LIST MCUHW_TEST_SUITES OR "mcu:espressif/esp32p4" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.mcu.esp32p4
        esp32p4/test_description.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.mcu.esp32p4 mcu:espressif/esp32p4)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "mcu" IN_LIST MCUHW_TEST_SUITES OR "mcu:renesas/ra4m1" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.mcu.ra4m1
        ra4m1/test_description.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.mcu.ra4m1 mcu:renesas/ra4m1)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "mcu" IN_LIST MCUHW_TEST_SUITES OR "mcu:st/stm32h753" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.mcu.stm32h753
        stm32h753/test_description.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.mcu.stm32h753 mcu:st/stm32h753)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "descriptions" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.descriptions
        test_description_suite.cpp
        test_three_description_gate.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.descriptions descriptions)
endif()
