add_executable(mcubsp.test.descriptions
    test_three_description_gate.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.descriptions descriptions)
