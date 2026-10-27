add_executable(mcubsp.test.backend.ra4m1
    test_adc.cpp
    test_clock.cpp
    test_dmac.cpp
    test_gpio.cpp
    test_i2c.cpp
    test_pwm.cpp
    test_spi.cpp
    test_uart.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.ra4m1 ip:renesas/ra4m1_native)
