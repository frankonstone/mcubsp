add_executable(mcubsp.test.backend.esp32p4
    esp32p4/test_ethernet.cpp esp32p4/test_gpio.cpp esp32p4/test_i2c.cpp
    esp32p4/test_i2s.cpp esp32p4/test_sdmmc.cpp esp32p4/test_spi.cpp
    esp32p4/test_uart.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.esp32p4 ip:espressif/esp32p4_native)

add_executable(mcubsp.test.backend.ra4m1
    ra4m1/test_adc.cpp ra4m1/test_clock.cpp ra4m1/test_dmac.cpp
    ra4m1/test_gpio.cpp ra4m1/test_i2c.cpp ra4m1/test_pwm.cpp
    ra4m1/test_spi.cpp ra4m1/test_uart.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.ra4m1 ip:renesas/ra4m1_native)

add_executable(mcubsp.test.backend.stm32h753
    stm32h753/test_clock.cpp stm32h753/test_drivers.cpp
    stm32h753/test_ethernet.cpp stm32h753/test_gpio.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.stm32h753 ip:st/stm32h753_native)
