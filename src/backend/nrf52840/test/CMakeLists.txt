# @req REQ-0053 REQ-0054 REQ-0056 REQ-0057 REQ-0058 REQ-0059 REQ-0060 REQ-0061 REQ-0062 REQ-0064 REQ-0065 REQ-0071 REQ-0072 REQ-0073
add_executable(mcubsp.test.backend.nrf52840
    test_gpio.cpp test_i2c.cpp test_system.cpp test_resources.cpp test_uart_config.cpp test_uart.cpp test_uarte.cpp test_spim.cpp test_twim.cpp test_temperature_rng.cpp test_qdec.cpp test_nvmc.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840 ip:nordic/nrf52840_native)
