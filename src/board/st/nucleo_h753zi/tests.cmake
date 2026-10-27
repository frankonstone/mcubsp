# @req REQ-0036 REQ-0038 REQ-0049 REQ-0052
set(board_test_directory "${CMAKE_CURRENT_LIST_DIR}/test")
foreach(name IN ITEMS startup_gpio_smoke native_peripherals)
    if(name STREQUAL startup_gpio_smoke)
        set(source startup_gpio.cpp)
    else()
        set(source native_peripherals.cpp)
    endif()
    set(target "st.nucleo_h753zi.test.${name}")
    mcubsp_add_firmware(${target} BOARD st/nucleo_h753zi
        REVISION "${MCUBSP_BOARD_REVISION}" STARTUP supplied HOSTED
        SOURCES "${board_test_directory}/${source}")
    target_link_libraries(${target} PRIVATE mcutest::mcutest mcujson::mcujson mcucov::runtime)
    target_compile_definitions(${target} PRIVATE MCUTEST_MAX_TESTS=8 MCUTEST_SOURCE_LOCATION=1
        MCUCOV_BYTE_PER_PROBE=1 MCUCOV_COMPACT_RUNTIME=1)
    if(MCUBSP_COVERAGE AND NOT name STREQUAL startup_gpio_smoke)
        set(build_id 0x4d487501)
        target_compile_options(${target} PRIVATE -O0
            -include "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp")
        mcucov_instrument_target(${target} PROFILE FULL BUILD_ID ${build_id}
            SOURCE_ROOT "${MCUBSP_ROOT}"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${target}")
    endif()
endforeach()

