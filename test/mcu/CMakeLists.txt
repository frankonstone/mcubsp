add_executable(mcubsp.test.mcu.esp32p4
    esp32p4/test_description.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.esp32p4 mcu:espressif/esp32p4)

add_executable(mcubsp.test.mcu.ra4m1
    ra4m1/test_description.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.ra4m1 mcu:renesas/ra4m1)

add_executable(mcubsp.test.mcu.stm32h753
    stm32h753/test_description.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.stm32h753 mcu:st/stm32h753)

add_executable(mcubsp.test.descriptions
    test_description_suite.cpp
    test_three_description_gate.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.descriptions descriptions)
