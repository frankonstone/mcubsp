add_executable(mcubsp.test.mcu.stm32h753
    test_description.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.stm32h753 mcu:st/stm32h753)
