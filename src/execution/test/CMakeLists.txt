add_executable(mcubsp.test.execution
    test_execution_policy.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.execution execution)
