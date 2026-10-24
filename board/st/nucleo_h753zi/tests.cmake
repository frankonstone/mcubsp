# @req REQ-0049 REQ-0052
set(board_test_directory "${CMAKE_CURRENT_LIST_DIR}/test")
# The first STM32H753 runtime slice deliberately has no dependency on the
# unfinished cross-target fixture. It proves reset/startup, the reset HSI,
# GPIO clocking and the built-in PE1 LED through the common GPIO driver.
# @req REQ-0003 REQ-0009 REQ-0021 REQ-0031 REQ-0033 REQ-0036
if(NOT MCUBSP_ARCH STREQUAL "cortex-m7")
    message(FATAL_ERROR
        "MCUBSP_BUILD_NUCLEO_H753ZI_TESTS requires the Cortex-M7 toolchain")
endif()
if(NOT EXISTS "${MCUBSP_MCUTEST_SOURCE_DIR}/include/mcutest/mcutest.hpp")
    message(FATAL_ERROR "mcutest headers not found at ${MCUBSP_MCUTEST_SOURCE_DIR}")
endif()
if(NOT EXISTS "${MCUBSP_MCUJSON_SOURCE_DIR}/include/mcujson/mcujson.hpp")
    message(FATAL_ERROR "mcujson headers not found at ${MCUBSP_MCUJSON_SOURCE_DIR}")
endif()

set(MCUJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MCUJSON_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(MCUJSON_BUILD_FUZZING OFF CACHE BOOL "" FORCE)
set(MCUJSON_INSTALL OFF CACHE BOOL "" FORCE)
if(NOT TARGET mcujson::mcujson)
    add_subdirectory("${MCUBSP_MCUJSON_SOURCE_DIR}"
                     "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcujson"
                     EXCLUDE_FROM_ALL)
endif()

if(NOT EXISTS "${MCUBSP_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
    message(FATAL_ERROR "mcuip headers not found at ${MCUBSP_MCUIP_SOURCE_DIR}")
endif()
set(MCUIP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_NATIVE_ADAPTER OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_MACOS_ADAPTER OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_MCUNET_ADAPTER OFF CACHE BOOL "" FORCE)
set(MCUIP_BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
if(NOT TARGET mcuip::mcuip)
    add_subdirectory("${MCUBSP_MCUIP_SOURCE_DIR}"
                     "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcuip"
                     EXCLUDE_FROM_ALL)
endif()
mcubsp_apply_firmware_runtime(mcuip HOSTED NO_OPTIMIZE)
# Compiler-selected headers keep unrelated devices out of embedded source IDs.
get_target_property(_ip_sources mcuip SOURCES)
list(TRANSFORM _ip_sources PREPEND "${MCUBSP_MCUIP_SOURCE_DIR}/")
mcubsp_embedded_source_identity(MCUBSP_MCUIP_SOURCE_ID
    SOURCES ${_ip_sources}
    INCLUDES "${MCUBSP_MCUIP_SOURCE_DIR}/include"
    FLAGS -std=c++23 ${MCUBSP_ARCH_COMPILE_OPTIONS})
mcubsp_embedded_source_identity(MCUBSP_SOURCE_ID
    SOURCES
        "${board_test_directory}/ethernet_mcuip.cpp"
        ${MCUBSP_BOARD_STARTUP}
        "${MCUBSP_BOARD_LINKER}"
        "${CMAKE_CURRENT_LIST_FILE}"
    INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${MCUBSP_MCUIP_SOURCE_DIR}/include" "${MCUBSP_MCUTEST_SOURCE_DIR}/include"
        "${MCUBSP_MCUJSON_SOURCE_DIR}/include" "${MCUBSP_MCUCOV_SOURCE_DIR}/include"
    FLAGS -std=c++23 ${MCUBSP_ARCH_COMPILE_OPTIONS})

if(NOT mcucov_DIR)
    file(GLOB _mcubsp_mcucov_configs
        "${MCUBSP_MCUCOV_SOURCE_DIR}/output/install/*/lib/cmake/mcucov/mcucov-config.cmake")
    list(LENGTH _mcubsp_mcucov_configs _mcubsp_mcucov_config_count)
    if(_mcubsp_mcucov_config_count EQUAL 0)
        message(FATAL_ERROR
            "No installed mcucov package found below ${MCUBSP_MCUCOV_SOURCE_DIR}/output/install; "
            "run ../mcucov/make.sh all or set mcucov_DIR")
    endif()
    list(GET _mcubsp_mcucov_configs 0 _mcubsp_mcucov_config)
    get_filename_component(mcucov_DIR "${_mcubsp_mcucov_config}" DIRECTORY)
endif()
find_package(mcucov CONFIG REQUIRED)

find_program(MCUBSP_ARM_SIZE arm-none-eabi-size REQUIRED)
set(nucleo_platform_sources ${MCUBSP_BOARD_STARTUP})
list(REMOVE_ITEM nucleo_platform_sources "${MCUBSP_BOARD_EARLY_INIT}")
add_library(st.nucleo_h753zi.platform OBJECT ${nucleo_platform_sources})
mcubsp_configure_board_platform(st.nucleo_h753zi.platform NO_OPTIMIZE)
add_library(st.nucleo_h753zi.early_init_coverage OBJECT "${MCUBSP_BOARD_EARLY_INIT}")
mcubsp_configure_board_platform(st.nucleo_h753zi.early_init_coverage NO_OPTIMIZE)
target_compile_options(st.nucleo_h753zi.platform PRIVATE ${MCUBSP_RESTRICTED_COMPILE_OPTIONS})
target_compile_options(st.nucleo_h753zi.early_init_coverage PRIVATE ${MCUBSP_RESTRICTED_COMPILE_OPTIONS})
target_compile_definitions(st.nucleo_h753zi.early_init_coverage PRIVATE
    MCUCOV_BYTE_PER_PROBE=1
    MCUCOV_COMPACT_RUNTIME=1
)
target_compile_options(st.nucleo_h753zi.early_init_coverage PRIVATE
    -include
    "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp"
)
mcucov_instrument_target(
    st.nucleo_h753zi.early_init_coverage
    PROFILE FULL
    BUILD_ID 0x4d487501
    SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/mcucov/st.nucleo_h753zi.early_init_coverage"
)
add_executable(st.nucleo_h753zi.test.startup_gpio_smoke
    "${board_test_directory}/startup_gpio.cpp"
)
add_executable(st.nucleo_h753zi.test.native_peripherals
    "${board_test_directory}/native_peripherals.cpp"
)
add_executable(st.nucleo_h753zi.test.ethernet
    "${board_test_directory}/ethernet_mcuip.cpp"
)
foreach(target IN ITEMS st.nucleo_h753zi.test.startup_gpio_smoke
        st.nucleo_h753zi.test.native_peripherals st.nucleo_h753zi.test.ethernet)
    set(options "")
    if(NOT target STREQUAL "st.nucleo_h753zi.test.startup_gpio_smoke")
        list(APPEND options HOSTED)
    endif()
    mcubsp_configure_board_firmware(${target} STARTUP supplied NO_OPTIMIZE HEX
        PLATFORM_TARGETS st.nucleo_h753zi.platform
            st.nucleo_h753zi.early_init_coverage ${options})
    target_compile_options(${target} PRIVATE ${MCUBSP_RESTRICTED_COMPILE_OPTIONS})
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${MCUBSP_ARM_SIZE}" -A "$<TARGET_FILE:${target}>" VERBATIM)
endforeach()
target_link_libraries(st.nucleo_h753zi.test.native_peripherals PRIVATE
    mcujson::mcujson)
target_link_libraries(st.nucleo_h753zi.test.ethernet PRIVATE
    mcujson::mcujson mcuip::mcuip)
target_include_directories(st.nucleo_h753zi.test.native_peripherals PRIVATE
    "${MCUBSP_MCUTEST_SOURCE_DIR}/include"
)
target_include_directories(st.nucleo_h753zi.test.ethernet PRIVATE
    "${MCUBSP_MCUTEST_SOURCE_DIR}/include"
)
target_compile_definitions(st.nucleo_h753zi.test.native_peripherals PRIVATE
    MCUTEST_MAX_TESTS=8
    MCUTEST_SOURCE_LOCATION=1
    MCUCOV_BYTE_PER_PROBE=1
    MCUCOV_COMPACT_RUNTIME=1
)
target_compile_definitions(st.nucleo_h753zi.test.ethernet PRIVATE
    MCUTEST_MAX_TESTS=3
    MCUTEST_SOURCE_LOCATION=1
    MCUCOV_BYTE_PER_PROBE=1
    MCUCOV_COMPACT_RUNTIME=1
    MCUBSP_SOURCE_ID="${MCUBSP_SOURCE_ID}"
    MCUBSP_MCUIP_SOURCE_ID="${MCUBSP_MCUIP_SOURCE_ID}"
)
target_compile_options(st.nucleo_h753zi.test.native_peripherals PRIVATE
    -include
    "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp"
)
target_compile_options(st.nucleo_h753zi.test.ethernet PRIVATE
    -include
    "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp"
)
mcucov_instrument_target(
    st.nucleo_h753zi.test.native_peripherals
    PROFILE FULL
    BUILD_ID 0x4d487501
    SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/mcucov/st.nucleo_h753zi.test.native_peripherals"
)
mcucov_instrument_target(
    st.nucleo_h753zi.test.ethernet
    PROFILE FULL
    BUILD_ID 0x4d487502
    SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/mcucov/st.nucleo_h753zi.test.ethernet"
)

# The instrumented board translation unit can embed more modules than one
# mcucov manifest serializes. These analysis-only objects regenerate the
# identical source manifests for the remaining STM32 backend headers; they
# are not linked into the firmware.
function(mcubsp_add_stm32h753_coverage_manifest_probe name definition)
    set(target "st.nucleo_h753zi.${name}.coverage_manifest")
    target_link_libraries(${target} PRIVATE mcubsp::mcubsp)
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_definitions(${target} PRIVATE
        ${definition}=1
        MCUCOV_BYTE_PER_PROBE=1
        MCUCOV_COMPACT_RUNTIME=1
    )
    mcubsp_apply_firmware_runtime(${target} HOSTED NO_OPTIMIZE)
    target_compile_options(${target} PRIVATE ${MCUBSP_RESTRICTED_COMPILE_OPTIONS}
        -include "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp")
    add_dependencies(st.nucleo_h753zi.test.native_peripherals ${target})
    if(name STREQUAL "ethernet")
        add_dependencies(st.nucleo_h753zi.test.ethernet ${target})
    endif()
    mcucov_instrument_target(
        ${target}
        PROFILE FULL
        BUILD_ID 0x4d487501
        SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${target}"
    )
endfunction()

add_library(st.nucleo_h753zi.i2c.coverage_manifest OBJECT
    test/compile/stm32h753/backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.memory.coverage_manifest OBJECT
    test/compile/stm32h753/backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.spi.coverage_manifest OBJECT
    test/compile/stm32h753/backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.timer.coverage_manifest OBJECT
    test/compile/stm32h753/backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.usart.coverage_manifest OBJECT
    test/compile/stm32h753/backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.ethernet.coverage_manifest OBJECT
    test/compile/stm32h753/backend_coverage_probe.cpp)

mcubsp_add_stm32h753_coverage_manifest_probe(
    i2c MCUBSP_COVERAGE_PROBE_I2C)
mcubsp_add_stm32h753_coverage_manifest_probe(
    memory MCUBSP_COVERAGE_PROBE_MEMORY)
mcubsp_add_stm32h753_coverage_manifest_probe(
    spi MCUBSP_COVERAGE_PROBE_SPI)
mcubsp_add_stm32h753_coverage_manifest_probe(
    timer MCUBSP_COVERAGE_PROBE_TIMER)
mcubsp_add_stm32h753_coverage_manifest_probe(
    usart MCUBSP_COVERAGE_PROBE_USART)
mcubsp_add_stm32h753_coverage_manifest_probe(
    ethernet MCUBSP_COVERAGE_PROBE_ETHERNET)
