# @req REQ-0055
add_executable(mcubsp.test.mcu.nrf52840 test_description.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.mcu.nrf52840 mcu:nrf52840)
