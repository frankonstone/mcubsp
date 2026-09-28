add_executable(mcuhw.test.drivers
    test_sdio.cpp
    test_sdmmc.cpp
    test_timing_contracts.cpp "${MCUHW_TEST_SOURCE_DIR}/support/main.cpp")
mcuhw_configure_native_suite(mcuhw.test.drivers drivers)
