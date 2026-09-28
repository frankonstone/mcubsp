# Core behavioral contracts and the reporter-failure fixture.
add_executable(mcuhw.test.core
    test_execution_policy.cpp
    test_metadata.cpp
    test_ownership.cpp
    test_protected_access.cpp
    test_recording_mmio.cpp
    test_register.cpp
    test_transfer.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
mcuhw_configure_native_suite(mcuhw.test.core core)

if("all" IN_LIST MCUHW_TEST_SUITES OR "core" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.fixture_failure "${MCUHW_TEST_SOURCE_DIR}/support/test_fixture_failure.cpp")
    add_dependencies(native-tests mcuhw.test.fixture_failure)
    set_target_properties(mcuhw.test.fixture_failure PROPERTIES FOLDER "Tests/core/fixtures"
        RUNTIME_OUTPUT_DIRECTORY "${MCUHW_TEST_BINARY_DIR}")
    target_link_libraries(mcuhw.test.fixture_failure PRIVATE mcutest::mcutest)
    mcuhw_apply_restricted_runtime(mcuhw.test.fixture_failure)
    mcutest_configure_target(mcuhw.test.fixture_failure MAX_TESTS 4 SOURCE_LOCATION ON)
    add_test(NAME mcuhw.fixture_failure COMMAND mcuhw.test.fixture_failure)
    set_tests_properties(mcuhw.fixture_failure PROPERTIES WILL_FAIL TRUE LABELS "native;core")
endif()
