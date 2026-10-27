add_executable(mcubsp.test.io
    test_io.cpp
    test_io_boards.cpp
    test_io_composition.cpp
    test_io_i2c_devices.cpp
    test_io_peripherals.cpp
    test_io_spi_devices.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.io io)

# The workflow checks invoke this fixture and inspect its contract diagnostic.
add_executable(mcubsp.test.contract_fixture test_contract_diagnostic.cpp)
target_link_libraries(mcubsp.test.contract_fixture PRIVATE mcubsp::io)
mcubsp_apply_restricted_runtime(mcubsp.test.contract_fixture)
add_dependencies(native-tests mcubsp.test.contract_fixture)
