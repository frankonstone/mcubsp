# @req REQ-0001 REQ-0002 REQ-0006 REQ-0008 REQ-0038 REQ-0039 REQ-0040
add_executable(mcuhw_tests
    test_description_suite.cpp
    test_esp32p4_description.cpp
    test_esp32p4_ethernet.cpp
    test_esp32p4_gpio.cpp
    test_esp32p4_i2c.cpp
    test_esp32p4_i2s.cpp
    test_esp32p4_sdmmc.cpp
    test_esp32p4_spi.cpp
    test_esp32p4_uart.cpp
    test_sdmmc.cpp
    test_sdio.cpp
    test_metadata.cpp
    test_ownership.cpp
    test_protected_access.cpp
    test_ra4m1_description.cpp
    test_ra4m1_clock.cpp
    test_ra4m1_gpio.cpp
    test_uart.cpp
    test_i2c.cpp
    test_spi.cpp
    test_adc.cpp
    test_dmac.cpp
    test_pwm.cpp
    test_stm32h753_clock.cpp
    test_stm32h753_description.cpp
    test_stm32h753_gpio.cpp
    test_stm32h753_ethernet.cpp
    test_w5100.cpp
    test_w5100_io.cpp
    test_stm32h753_drivers.cpp
    test_three_description_gate.cpp
    test_register.cpp
    test_recording_mmio.cpp
    test_execution_policy.cpp
    test_transfer.cpp
)
target_link_libraries(mcuhw_tests PRIVATE mcuhw::mcuhw mcutest::mcutest)
if(EXISTS "${MCUHW_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
    target_include_directories(mcuhw_tests PRIVATE
        "${MCUHW_MCUIP_SOURCE_DIR}/include")
endif()
mcuhw_apply_restricted_runtime(mcuhw_tests)
mcutest_configure_target(mcuhw_tests MAX_TESTS 256 SOURCE_LOCATION ON)

add_test(
    NAME mcuhw.unit
    COMMAND "${CMAKE_COMMAND}"
        -DTEST_PROGRAM=$<TARGET_FILE:mcuhw_tests>
        -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/artifacts/mcutest.json
        -P "${CMAKE_SOURCE_DIR}/cmake/CaptureMcutest.cmake"
)

add_executable(mcuhw_fixture_failure test_fixture_failure.cpp)
target_link_libraries(mcuhw_fixture_failure PRIVATE mcutest::mcutest)
mcuhw_apply_restricted_runtime(mcuhw_fixture_failure)
mcutest_configure_target(mcuhw_fixture_failure MAX_TESTS 4 SOURCE_LOCATION ON)
add_test(NAME mcuhw.fixture_failure COMMAND mcuhw_fixture_failure)
set_tests_properties(mcuhw.fixture_failure PROPERTIES WILL_FAIL TRUE)

# @req REQ-0045
add_executable(mcuhw_io_tests
    test_io.cpp test_io_boards.cpp test_io_peripherals.cpp
    test_io_spi_devices.cpp test_spi_device_backends.cpp
    test_io_i2c_devices.cpp test_tmp102.cpp)
target_link_libraries(mcuhw_io_tests PRIVATE mcuhw::io mcutest::mcutest)
mcuhw_apply_restricted_runtime(mcuhw_io_tests)
mcutest_configure_target(mcuhw_io_tests MAX_TESTS 64 SOURCE_LOCATION ON)
add_test(
    NAME mcuhw.io
    COMMAND "${CMAKE_COMMAND}"
        -DTEST_PROGRAM=$<TARGET_FILE:mcuhw_io_tests>
        -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/artifacts/mcutest-io.json
        -P "${CMAKE_SOURCE_DIR}/cmake/CaptureMcutest.cmake"
)

function(mcuhw_add_compile_case name source expect_success)
    add_test(
        NAME "compile.${name}"
        COMMAND "${CMAKE_COMMAND}"
            -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            "-DCOMPILE_OPTIONS=${MCUHW_RESTRICTED_COMPILE_OPTIONS}"
            -DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/compile/${source}
            -DEXPECT_SUCCESS=${expect_success}
            -P "${CMAKE_SOURCE_DIR}/cmake/CompileCase.cmake"
    )
endfunction()

mcuhw_add_compile_case(cxx23 cxx23_features.cpp TRUE)
mcuhw_add_compile_case(io_example io_example.cpp TRUE)
mcuhw_add_compile_case(io_devices_example io_devices_example.cpp TRUE)
mcuhw_add_compile_case(io_disabled_peripheral io_disabled_peripheral_forbidden.cpp FALSE)
mcuhw_add_compile_case(io_unsupported_dma io_unsupported_dma_forbidden.cpp FALSE)
mcuhw_add_compile_case(io_move_runtime io_move_runtime_forbidden.cpp FALSE)
mcuhw_add_compile_case(exceptions_rejected exceptions_forbidden.cpp FALSE)
mcuhw_add_compile_case(rtti_rejected rtti_forbidden.cpp FALSE)
mcuhw_add_compile_case(command_read_rejected command_read_forbidden.cpp FALSE)
mcuhw_add_compile_case(unsafe_rmw_rejected unsafe_rmw_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_field_rejected invalid_field_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_enum_rejected invalid_enum_forbidden.cpp FALSE)
mcuhw_add_compile_case(duplicate_pin_rejected duplicate_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(unroutable_pin_rejected unroutable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(unsupported_capability_rejected unsupported_capability_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_clock_rejected invalid_clock_forbidden.cpp FALSE)
mcuhw_add_compile_case(interrupt_conflict_rejected interrupt_conflict_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_dma_rejected invalid_dma_forbidden.cpp FALSE)
mcuhw_add_compile_case(reserved_write_rejected reserved_write_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_unavailable_pin_rejected ra4m1_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_third_gpt_output_rejected ra4m1_third_gpt_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_invalid_gpt_output_rejected ra4m1_invalid_gpt_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(stm32h753_unavailable_pin_rejected stm32h753_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(stm32h753_advanced_output_rejected stm32h753_advanced_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(esp32p4_unavailable_pin_rejected esp32p4_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(esp32p4_i3c_capability_rejected esp32p4_i3c_capability_forbidden.cpp FALSE)

# The all-target gate exists now but is only built after all three real,
# source-backed description headers exist. No placeholder MCU types are used.
# @req REQ-0040
set(description_headers
    "${CMAKE_SOURCE_DIR}/include/mcuhw/mcu/ra4m1/description.hpp"
    "${CMAKE_SOURCE_DIR}/include/mcuhw/mcu/stm32h753/description.hpp"
    "${CMAKE_SOURCE_DIR}/include/mcuhw/mcu/esp32p4/description.hpp"
)
set(all_descriptions_available TRUE)
foreach(header IN LISTS description_headers)
    if(NOT EXISTS "${header}")
        set(all_descriptions_available FALSE)
    endif()
endforeach()
if(all_descriptions_available)
    add_library(mcuhw_all_descriptions OBJECT description_all.cpp)
    target_link_libraries(mcuhw_all_descriptions PRIVATE mcuhw::mcuhw)
    mcuhw_apply_restricted_runtime(mcuhw_all_descriptions)
else()
    message(STATUS
        "All-three description gate deferred until the three real description headers exist")
endif()
