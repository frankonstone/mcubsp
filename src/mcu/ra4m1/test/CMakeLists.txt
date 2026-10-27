add_executable(mcubsp.test.mcu.ra4m1
    test_description.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.ra4m1 mcu:renesas/ra4m1)
