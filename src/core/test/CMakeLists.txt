add_executable(mcubsp.test.core
    test_metadata.cpp
    test_ownership.cpp
    test_protected_access.cpp
    test_register.cpp
    test_transfer.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.core core)
