add_executable(mcubsp.test.mcu.esp32p4
    test_description.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.esp32p4 mcu:espressif/esp32p4)
