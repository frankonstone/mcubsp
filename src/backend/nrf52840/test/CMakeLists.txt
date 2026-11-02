# @req REQ-0053 REQ-0054 REQ-0056 REQ-0057 REQ-0058 REQ-0059 REQ-0060 REQ-0061 REQ-0062 REQ-0064 REQ-0065 REQ-0071 REQ-0072 REQ-0073
add_executable(mcubsp.test.backend.nrf52840
    test_gpio.cpp test_i2c.cpp test_system.cpp test_resources.cpp test_uart_config.cpp test_uart.cpp test_uarte.cpp test_serial_io.cpp test_spi.cpp test_spis.cpp test_twis.cpp test_spim.cpp test_twim.cpp test_temperature_rng.cpp test_qdec.cpp test_nvmc.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840 ip:nordic/nrf52840_native)

# @req REQ-0063 REQ-0064 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.analog
    test_pwm.cpp test_saadc.cpp test_comparator.cpp test_analog_io.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.analog ip:nordic/nrf52840_analog)

# @req REQ-0072 REQ-0066 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.memory
    test_memory.cpp test_qspi.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.memory ip:nordic/nrf52840_memory)

# @req REQ-0067 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.audio
    test_audio.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.audio ip:nordic/nrf52840_audio)

# @req REQ-0071 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.crypto
    test_crypto.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.crypto ip:nordic/nrf52840_crypto)

# @req REQ-0070 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.nfct
    test_nfct.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.nfct ip:nordic/nrf52840_nfct)

# @req REQ-0069 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.radio
    test_radio.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.radio ip:nordic/nrf52840_radio)

# @req REQ-0068 REQ-0062
add_executable(mcubsp.test.backend.nrf52840.usb
    test_usb.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.nrf52840.usb ip:nordic/nrf52840_usb)
