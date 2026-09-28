# @req REQ-0049 REQ-0052
set(board_test_directory "${CMAKE_CURRENT_LIST_DIR}/test")
# Board composition is explicit and opt-in: ordinary architecture probes do
# not acquire startup files, linker ownership, or a board dependency.
# @req REQ-0022 REQ-0031 REQ-0036 REQ-0037 REQ-0038
if(NOT MCUHW_ARCH STREQUAL "cortex-m4")
    message(FATAL_ERROR
        "MCUHW_BUILD_UNO_R4_MINIMA_TESTS requires the Cortex-M4 toolchain")
endif()
if(NOT EXISTS "${MCUHW_MCUTEST_SOURCE_DIR}/include/mcutest/mcutest.hpp")
    message(FATAL_ERROR "mcutest headers not found at ${MCUHW_MCUTEST_SOURCE_DIR}")
endif()
if(NOT EXISTS "${MCUHW_MCUJSON_SOURCE_DIR}/include/mcujson/mcujson.hpp")
    message(FATAL_ERROR "mcujson headers not found at ${MCUHW_MCUJSON_SOURCE_DIR}")
endif()

set(MCUJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MCUJSON_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(MCUJSON_BUILD_FUZZING OFF CACHE BOOL "" FORCE)
set(MCUJSON_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${MCUHW_MCUJSON_SOURCE_DIR}"
                 "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcujson"
                 EXCLUDE_FROM_ALL)

if(NOT EXISTS "${MCUHW_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
    message(FATAL_ERROR "mcuip headers not found at ${MCUHW_MCUIP_SOURCE_DIR}")
endif()
set(MCUIP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_NATIVE_ADAPTER OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_MACOS_ADAPTER OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_MCUNET_ADAPTER OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
if(NOT TARGET mcuip::mcuip)
    add_subdirectory("${MCUHW_MCUIP_SOURCE_DIR}"
                     "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcuip"
                     EXCLUDE_FROM_ALL)
endif()
mcuhw_apply_firmware_runtime(mcuip HOSTED)
target_compile_options(mcuip PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS})

# Compiler-selected headers keep unrelated devices out of embedded source IDs.
get_target_property(_ip_sources mcuip SOURCES)
list(TRANSFORM _ip_sources PREPEND "${MCUHW_MCUIP_SOURCE_DIR}/")
mcuhw_embedded_source_identity(MCUHW_UNO_MCUIP_SOURCE_ID
    SOURCES ${_ip_sources}
    INCLUDES "${MCUHW_MCUIP_SOURCE_DIR}/include"
    FLAGS -std=c++23 ${MCUHW_ARCH_COMPILE_OPTIONS})
mcuhw_embedded_source_identity(MCUHW_UNO_SOURCE_ID
    SOURCES
        "${board_test_directory}/ethernet_mcuip.cpp"
        ${MCUHW_BOARD_STARTUP}
        "${MCUHW_BOARD_LINKER}"
        "${CMAKE_CURRENT_LIST_FILE}"
    INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${MCUHW_MCUIP_SOURCE_DIR}/include" "${MCUHW_MCUTEST_SOURCE_DIR}/include"
        "${MCUHW_MCUJSON_SOURCE_DIR}/include" "${MCUHW_MCUCOV_SOURCE_DIR}/include"
    FLAGS -std=c++23 ${MCUHW_ARCH_COMPILE_OPTIONS})

if(NOT mcucov_DIR)
    file(GLOB _mcuhw_mcucov_configs
        "${MCUHW_MCUCOV_SOURCE_DIR}/output/install/*/lib/cmake/mcucov/mcucov-config.cmake")
    list(LENGTH _mcuhw_mcucov_configs _mcuhw_mcucov_config_count)
    if(_mcuhw_mcucov_config_count EQUAL 0)
        message(FATAL_ERROR
            "No installed mcucov package found below ${MCUHW_MCUCOV_SOURCE_DIR}/output/install; "
            "run ../mcucov/make.sh all or set mcucov_DIR")
    endif()
    list(GET _mcuhw_mcucov_configs 0 _mcuhw_mcucov_config)
    get_filename_component(mcucov_DIR "${_mcuhw_mcucov_config}" DIRECTORY)
endif()
find_package(mcucov CONFIG REQUIRED)

find_program(MCUHW_ARM_SIZE arm-none-eabi-size REQUIRED)

function(mcuhw_add_uno_r4_minima_test target source build_id)
    set(enable_coverage TRUE)
    if(ARGC GREATER 3 AND ARGV3 STREQUAL "NO_COVERAGE")
        set(enable_coverage FALSE)
    endif()
    set(platform_target "${target}.platform")
    set(options "")
    if(enable_coverage)
        list(APPEND options NO_OPTIMIZE)
    endif()
    mcuhw_configure_board_platform(${platform_target} ${options})
    mcuhw_configure_board_firmware(${target} STARTUP supplied
        PLATFORM_TARGETS ${platform_target} HOSTED HEX ${options})
    target_link_libraries(${target} PRIVATE mcujson::mcujson)
    target_compile_options(${platform_target} PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
    target_compile_options(${target} PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
    target_include_directories(${target} PRIVATE
        "${MCUHW_MCUTEST_SOURCE_DIR}/include"
    )
    target_compile_definitions(${target} PRIVATE
        MCUTEST_MAX_TESTS=8
        MCUTEST_SOURCE_LOCATION=1
    )
    if(enable_coverage)
        target_compile_definitions(${target} PRIVATE
            MCUCOV_BYTE_PER_PROBE=1
            MCUCOV_COMPACT_RUNTIME=1
        )
    else()
        target_compile_definitions(${target} PRIVATE
            MCUHW_BOARD_TEST_NO_COVERAGE=1
        )
    endif()
    if(enable_coverage)
        target_compile_options(${target} PRIVATE
            -include
            "${MCUHW_ROOT}/test/support/mcucov_compat.hpp"
        )
    endif()
    if(enable_coverage)
        mcucov_instrument_target(
            ${target}
            PROFILE FULL
            BUILD_ID ${build_id}
            SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${target}"
        )
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${MCUHW_ARM_SIZE}" -A $<TARGET_FILE:${target}>
        VERBATIM
    )
endfunction()

# Every physical-test image has an explicit executable and source.
add_library(arduino.uno_r4_minima.test.startup_gpio.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.startup_gpio "${board_test_directory}/startup_gpio.cpp")
add_library(arduino.uno_r4_minima.test.gpio_loopback.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.gpio_loopback "${board_test_directory}/gpio_loopback.cpp")
add_library(arduino.uno_r4_minima.test.pwm.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.pwm "${board_test_directory}/pwm.cpp")
add_library(arduino.uno_r4_minima.test.uart.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.uart "${board_test_directory}/uart.cpp")
add_library(arduino.uno_r4_minima.test.i2c.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.i2c "${board_test_directory}/i2c.cpp")
add_library(arduino.uno_r4_minima.test.spi.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.spi "${board_test_directory}/spi.cpp")
add_library(arduino.uno_r4_minima.test.adc.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.adc "${board_test_directory}/adc.cpp")
add_library(arduino.uno_r4_minima.test.dmac.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.dmac "${board_test_directory}/dmac.cpp")
add_library(arduino.uno_r4_minima.test.ethernet_mcuip.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.ethernet_mcuip "${board_test_directory}/ethernet_mcuip.cpp")
add_library(arduino.uno_r4_minima.test.ethernet_metrics.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.ethernet_metrics "${board_test_directory}/ethernet_mcuip.cpp")
add_library(arduino.uno_r4_minima.test.ethernet_metrics_fast6.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.ethernet_metrics_fast6 "${board_test_directory}/ethernet_mcuip.cpp")
add_library(arduino.uno_r4_minima.test.ethernet_metrics_fast12.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.ethernet_metrics_fast12 "${board_test_directory}/ethernet_mcuip.cpp")
add_library(arduino.uno_r4_minima.test.ethernet_link_recovery.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.ethernet_link_recovery "${board_test_directory}/ethernet_mcuip.cpp")
add_library(arduino.uno_r4_minima.test.release_metrics.platform OBJECT ${MCUHW_BOARD_STARTUP})
add_executable(arduino.uno_r4_minima.test.release_metrics "${board_test_directory}/release_metrics.cpp")

mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.startup_gpio
    "${board_test_directory}/startup_gpio.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.gpio_loopback
    "${board_test_directory}/gpio_loopback.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.pwm
    "${board_test_directory}/pwm.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.uart
    "${board_test_directory}/uart.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.i2c
    "${board_test_directory}/i2c.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.spi
    "${board_test_directory}/spi.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.adc
    "${board_test_directory}/adc.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.dmac
    "${board_test_directory}/dmac.cpp"
    0x4d485701)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.ethernet_mcuip
    "${board_test_directory}/ethernet_mcuip.cpp"
    0x4d485702)
target_link_libraries(arduino.uno_r4_minima.test.ethernet_mcuip PRIVATE
    mcuip::mcuip)
target_compile_definitions(
    arduino.uno_r4_minima.test.ethernet_mcuip PRIVATE
    MCUHW_SOURCE_ID="${MCUHW_UNO_SOURCE_ID}"
    MCUHW_MCUIP_SOURCE_ID="${MCUHW_UNO_MCUIP_SOURCE_ID}"
)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.ethernet_metrics
    "${board_test_directory}/ethernet_mcuip.cpp"
    0
    NO_COVERAGE)
target_link_libraries(arduino.uno_r4_minima.test.ethernet_metrics PRIVATE
    mcuip::mcuip)
target_compile_definitions(
    arduino.uno_r4_minima.test.ethernet_metrics PRIVATE
    MCUHW_ETHERNET_METRICS=1
    MCUHW_SOURCE_ID="${MCUHW_UNO_SOURCE_ID}"
    MCUHW_MCUIP_SOURCE_ID="${MCUHW_UNO_MCUIP_SOURCE_ID}"
)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.ethernet_metrics_fast6
    "${board_test_directory}/ethernet_mcuip.cpp"
    0
    NO_COVERAGE)
target_link_libraries(
    arduino.uno_r4_minima.test.ethernet_metrics_fast6 PRIVATE
    mcuip::mcuip)
target_compile_definitions(
    arduino.uno_r4_minima.test.ethernet_metrics_fast6 PRIVATE
    MCUHW_ETHERNET_METRICS=1
    MCUHW_ETHERNET_FAST_SPI=1
    MCUHW_ETHERNET_SPI_HZ=6000000
    MCUHW_SOURCE_ID="${MCUHW_UNO_SOURCE_ID}"
    MCUHW_MCUIP_SOURCE_ID="${MCUHW_UNO_MCUIP_SOURCE_ID}"
)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.ethernet_metrics_fast12
    "${board_test_directory}/ethernet_mcuip.cpp"
    0
    NO_COVERAGE)
target_link_libraries(
    arduino.uno_r4_minima.test.ethernet_metrics_fast12 PRIVATE
    mcuip::mcuip)
target_compile_definitions(
    arduino.uno_r4_minima.test.ethernet_metrics_fast12 PRIVATE
    MCUHW_ETHERNET_METRICS=1
    MCUHW_ETHERNET_FAST_SPI=1
    MCUHW_ETHERNET_SPI_HZ=12000000
    MCUHW_SOURCE_ID="${MCUHW_UNO_SOURCE_ID}"
    MCUHW_MCUIP_SOURCE_ID="${MCUHW_UNO_MCUIP_SOURCE_ID}"
)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.ethernet_link_recovery
    "${board_test_directory}/ethernet_mcuip.cpp"
    0
    NO_COVERAGE)
target_link_libraries(
    arduino.uno_r4_minima.test.ethernet_link_recovery PRIVATE
    mcuip::mcuip)
target_compile_definitions(
    arduino.uno_r4_minima.test.ethernet_link_recovery PRIVATE
    MCUHW_ETHERNET_LINK_RECOVERY=1
    MCUHW_SOURCE_ID="${MCUHW_UNO_SOURCE_ID}"
    MCUHW_MCUIP_SOURCE_ID="${MCUHW_UNO_MCUIP_SOURCE_ID}"
)
mcuhw_add_uno_r4_minima_test(
    arduino.uno_r4_minima.test.release_metrics
    "${board_test_directory}/release_metrics.cpp"
    0
    NO_COVERAGE)
