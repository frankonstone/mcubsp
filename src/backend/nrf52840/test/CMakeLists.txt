# @req REQ-0053
add_executable(mcubsp.test.backend.nrf52840
    test_gpio.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840 ip:nordic/nrf52840_native)
