add_executable(mcubsp.test.drivers
    test_sdio.cpp
    test_sdmmc.cpp
    test_timing_contracts.cpp "${MCUBSP_TEST_SOURCE_DIR}/support/main.cpp")
mcubsp_configure_native_suite(mcubsp.test.drivers drivers)
