add_executable(mcubsp.test.backend.stm32h753
    test_clock.cpp
    test_drivers.cpp
    test_ethernet.cpp
    test_gpio.cpp
    "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.backend.stm32h753 ip:st/stm32h753_native)
