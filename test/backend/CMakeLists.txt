if("all" IN_LIST MCUHW_TEST_SUITES OR "backend" IN_LIST MCUHW_TEST_SUITES OR "ip:espressif/esp32p4_native" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.backend.esp32p4
        esp32p4/test_ethernet.cpp esp32p4/test_gpio.cpp esp32p4/test_i2c.cpp
        esp32p4/test_i2s.cpp esp32p4/test_sdmmc.cpp esp32p4/test_spi.cpp
        esp32p4/test_uart.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.backend.esp32p4 ip:espressif/esp32p4_native)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "backend" IN_LIST MCUHW_TEST_SUITES OR "ip:renesas/ra4m1_native" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.backend.ra4m1
        ra4m1/test_adc.cpp ra4m1/test_clock.cpp ra4m1/test_dmac.cpp
        ra4m1/test_gpio.cpp ra4m1/test_i2c.cpp ra4m1/test_pwm.cpp
        ra4m1/test_spi.cpp ra4m1/test_uart.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.backend.ra4m1 ip:renesas/ra4m1_native)
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "backend" IN_LIST MCUHW_TEST_SUITES OR "ip:st/stm32h753_native" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw.test.backend.stm32h753
        stm32h753/test_clock.cpp stm32h753/test_drivers.cpp
        stm32h753/test_ethernet.cpp stm32h753/test_gpio.cpp
        "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
    mcuhw_configure_native_suite(mcuhw.test.backend.stm32h753 ip:st/stm32h753_native)
endif()
