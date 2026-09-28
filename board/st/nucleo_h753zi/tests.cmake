# @req REQ-0049 REQ-0052
set(board_test_directory "${CMAKE_CURRENT_LIST_DIR}/test")
# The first STM32H753 runtime slice deliberately has no dependency on the
# unfinished cross-target fixture. It proves reset/startup, the reset HSI,
# GPIO clocking and the built-in PE1 LED through the common GPIO driver.
# @req REQ-0003 REQ-0009 REQ-0021 REQ-0031 REQ-0033 REQ-0036
if(NOT MCUHW_ARCH STREQUAL "cortex-m7")
    message(FATAL_ERROR
        "MCUHW_BUILD_NUCLEO_H753ZI_TESTS requires the Cortex-M7 toolchain")
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
if(NOT TARGET mcujson::mcujson)
    add_subdirectory("${MCUHW_MCUJSON_SOURCE_DIR}"
                     "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcujson"
                     EXCLUDE_FROM_ALL)
endif()

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
mcuhw_apply_firmware_runtime(mcuip HOSTED NO_OPTIMIZE)
# Compiler-selected headers keep unrelated devices out of embedded source IDs.
get_target_property(_ip_sources mcuip SOURCES)
list(TRANSFORM _ip_sources PREPEND "${MCUHW_MCUIP_SOURCE_DIR}/")
mcuhw_embedded_source_identity(MCUHW_MCUIP_SOURCE_ID
    SOURCES ${_ip_sources}
    INCLUDES "${MCUHW_MCUIP_SOURCE_DIR}/include"
    FLAGS -std=c++23 ${MCUHW_ARCH_COMPILE_OPTIONS})
mcuhw_embedded_source_identity(MCUHW_SOURCE_ID
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
set(nucleo_platform_sources ${MCUHW_BOARD_STARTUP})
list(REMOVE_ITEM nucleo_platform_sources "${MCUHW_BOARD_EARLY_INIT}")
add_library(st.nucleo_h753zi.platform OBJECT ${nucleo_platform_sources})
mcuhw_configure_board_platform(st.nucleo_h753zi.platform NO_OPTIMIZE)
add_library(st.nucleo_h753zi.early_init_coverage OBJECT "${MCUHW_BOARD_EARLY_INIT}")
mcuhw_configure_board_platform(st.nucleo_h753zi.early_init_coverage NO_OPTIMIZE)
target_compile_options(st.nucleo_h753zi.platform PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
target_compile_options(st.nucleo_h753zi.early_init_coverage PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
target_compile_definitions(st.nucleo_h753zi.early_init_coverage PRIVATE
    MCUCOV_BYTE_PER_PROBE=1
    MCUCOV_COMPACT_RUNTIME=1
)
target_compile_options(st.nucleo_h753zi.early_init_coverage PRIVATE
    -include
    "${MCUHW_ROOT}/test/support/mcucov_compat.hpp"
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
    mcuhw_configure_board_firmware(${target} STARTUP supplied NO_OPTIMIZE HEX
        PLATFORM_TARGETS st.nucleo_h753zi.platform
            st.nucleo_h753zi.early_init_coverage ${options})
    target_compile_options(${target} PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${MCUHW_ARM_SIZE}" -A "$<TARGET_FILE:${target}>" VERBATIM)
endforeach()
target_link_libraries(st.nucleo_h753zi.test.native_peripherals PRIVATE
    mcujson::mcujson)
target_link_libraries(st.nucleo_h753zi.test.ethernet PRIVATE
    mcujson::mcujson mcuip::mcuip)
target_include_directories(st.nucleo_h753zi.test.native_peripherals PRIVATE
    "${MCUHW_MCUTEST_SOURCE_DIR}/include"
)
target_include_directories(st.nucleo_h753zi.test.ethernet PRIVATE
    "${MCUHW_MCUTEST_SOURCE_DIR}/include"
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
    MCUHW_SOURCE_ID="${MCUHW_SOURCE_ID}"
    MCUHW_MCUIP_SOURCE_ID="${MCUHW_MCUIP_SOURCE_ID}"
)
target_compile_options(st.nucleo_h753zi.test.native_peripherals PRIVATE
    -include
    "${MCUHW_ROOT}/test/support/mcucov_compat.hpp"
)
target_compile_options(st.nucleo_h753zi.test.ethernet PRIVATE
    -include
    "${MCUHW_ROOT}/test/support/mcucov_compat.hpp"
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
function(mcuhw_add_stm32h753_coverage_manifest_probe name definition)
    set(target "st.nucleo_h753zi.${name}.coverage_manifest")
    target_link_libraries(${target} PRIVATE mcuhw::mcuhw)
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_definitions(${target} PRIVATE
        ${definition}=1
        MCUCOV_BYTE_PER_PROBE=1
        MCUCOV_COMPACT_RUNTIME=1
    )
    mcuhw_apply_firmware_runtime(${target} HOSTED NO_OPTIMIZE)
    target_compile_options(${target} PRIVATE ${MCUHW_RESTRICTED_COMPILE_OPTIONS}
        -include "${MCUHW_ROOT}/test/support/mcucov_compat.hpp")
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
    test/compile/stm32h753_backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.memory.coverage_manifest OBJECT
    test/compile/stm32h753_backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.spi.coverage_manifest OBJECT
    test/compile/stm32h753_backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.timer.coverage_manifest OBJECT
    test/compile/stm32h753_backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.usart.coverage_manifest OBJECT
    test/compile/stm32h753_backend_coverage_probe.cpp)
add_library(st.nucleo_h753zi.ethernet.coverage_manifest OBJECT
    test/compile/stm32h753_backend_coverage_probe.cpp)

mcuhw_add_stm32h753_coverage_manifest_probe(
    i2c MCUHW_COVERAGE_PROBE_I2C)
mcuhw_add_stm32h753_coverage_manifest_probe(
    memory MCUHW_COVERAGE_PROBE_MEMORY)
mcuhw_add_stm32h753_coverage_manifest_probe(
    spi MCUHW_COVERAGE_PROBE_SPI)
mcuhw_add_stm32h753_coverage_manifest_probe(
    timer MCUHW_COVERAGE_PROBE_TIMER)
mcuhw_add_stm32h753_coverage_manifest_probe(
    usart MCUHW_COVERAGE_PROBE_USART)
mcuhw_add_stm32h753_coverage_manifest_probe(
    ethernet MCUHW_COVERAGE_PROBE_ETHERNET)
