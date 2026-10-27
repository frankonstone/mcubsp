add_executable(mcubsp.test.backend.esp32p4
    test_ethernet.cpp
    test_gpio.cpp
    test_i2c.cpp
    test_i2s.cpp
    test_sdmmc.cpp
    test_spi.cpp
    test_uart.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.esp32p4 ip:espressif/esp32p4_native)
