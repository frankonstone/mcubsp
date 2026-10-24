# Core behavioral contracts and the reporter-failure fixture.
add_executable(mcubsp.test.core
    test_execution_policy.cpp
    test_metadata.cpp
    test_ownership.cpp
    test_protected_access.cpp
    test_recording_mmio.cpp
    test_register.cpp
    test_transfer.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.core core)

add_executable(mcubsp.test.fixture_failure "${MCUBSP_TEST_SOURCE_DIR}/support/test_fixture_failure.cpp")
add_dependencies(native-tests mcubsp.test.fixture_failure)
set_target_properties(mcubsp.test.fixture_failure PROPERTIES FOLDER "Tests/core/fixtures"
    RUNTIME_OUTPUT_DIRECTORY "${MCUBSP_TEST_BINARY_DIR}")
target_link_libraries(mcubsp.test.fixture_failure PRIVATE mcutest::mcutest)
mcubsp_apply_restricted_runtime(mcubsp.test.fixture_failure)
mcutest_configure_target(mcubsp.test.fixture_failure MAX_TESTS 4 SOURCE_LOCATION ON)
add_test(NAME mcubsp.fixture_failure COMMAND mcubsp.test.fixture_failure)
set_tests_properties(mcubsp.fixture_failure PROPERTIES WILL_FAIL TRUE LABELS "native;core")
