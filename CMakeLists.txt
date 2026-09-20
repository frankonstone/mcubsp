cmake_minimum_required(VERSION 3.25)

project(
    mcuhw
    VERSION 0.1.0
    DESCRIPTION "Source-traceable MCU descriptions and bare-metal drivers"
    LANGUAGES CXX ASM
)

include(CTest)
include(cmake/McuhwProjectOptions.cmake)
include(cmake/ToolchainReport.cmake)

set(MCUHW_ARCH "native" CACHE STRING "Architecture probed by this build")
set(MCUHW_MCUTEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mcutest"
    CACHE PATH "Path to the sibling mcutest checkout")
set(MCUHW_MCUJSON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mcujson"
    CACHE PATH "Path to the sibling mcujson checkout")
set(MCUHW_MCUCOV_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mcucov"
    CACHE PATH "Path to the sibling mcucov checkout")
set(MCUHW_MCUIP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mcuip"
    CACHE PATH "Path to the sibling mcuip checkout")
option(MCUHW_BUILD_UNO_R4_MINIMA_TESTS
    "Build the bootloader-preserving UNO R4 Minima mcutest firmware" OFF)
option(MCUHW_BUILD_NUCLEO_H753ZI_TESTS
    "Build the NUCLEO-H753ZI bare-metal smoke-test firmware" OFF)

# @req REQ-0003
add_library(mcuhw INTERFACE)
add_library(mcuhw::mcuhw ALIAS mcuhw)
target_include_directories(mcuhw INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(mcuhw INTERFACE cxx_std_23)

# @req REQ-0002 REQ-0015 REQ-0016 REQ-0017 REQ-0018 REQ-0019 REQ-0035 REQ-0038 REQ-0040 REQ-0041
add_library(mcuhw_arch_probe OBJECT
    tests/compile/all_descriptions_probe.cpp
    tests/compile/arch_probe.cpp
    tests/compile/cxx23_features.cpp
    tests/compile/execution_probe.cpp
    tests/compile/uart_i2c_example.cpp
    tests/compile/spi_adc_example.cpp
    tests/compile/esp32p4_metadata_probe.cpp
    tests/compile/stm32h753_metadata_probe.cpp
)
target_link_libraries(mcuhw_arch_probe PRIVATE mcuhw::mcuhw)
target_compile_definitions(mcuhw_arch_probe PRIVATE
    MCUHW_ARCH_NAME="${MCUHW_ARCH}"
)
mcuhw_apply_restricted_runtime(mcuhw_arch_probe)

# Optional by installation, mandatory for the sibling-development contract:
# this TU proves that the STM32 fixed-ring boundary still satisfies the current
# mcuip replacement for lwIP on every configured architecture.
# @req REQ-0043
if(EXISTS "${MCUHW_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
    add_library(mcuhw_mcuip_contract_probe OBJECT
        tests/compile/stm32h753_mcuip_contract.cpp)
    target_link_libraries(mcuhw_mcuip_contract_probe PRIVATE mcuhw::mcuhw)
    target_include_directories(mcuhw_mcuip_contract_probe PRIVATE
        "${MCUHW_MCUIP_SOURCE_DIR}/include")
    mcuhw_apply_restricted_runtime(mcuhw_mcuip_contract_probe)
else()
    message(WARNING
        "mcuip checkout not found at ${MCUHW_MCUIP_SOURCE_DIR}; REQ-0043 contract probe disabled")
endif()

# @req REQ-0009 REQ-0038
add_library(mcuhw_barrier_probe OBJECT tests/compile/barrier_probe.cpp)
target_link_libraries(mcuhw_barrier_probe PRIVATE mcuhw::mcuhw)
mcuhw_apply_restricted_runtime(mcuhw_barrier_probe)
if(CMAKE_CROSSCOMPILING)
    add_custom_target(mcuhw_verify_barriers ALL
        COMMAND "${CMAKE_COMMAND}"
            -DOBJDUMP=${CMAKE_OBJDUMP}
            -DOBJECT_FILE=$<TARGET_OBJECTS:mcuhw_barrier_probe>
            -DARCH=${MCUHW_ARCH}
            -DREPORT=${CMAKE_CURRENT_BINARY_DIR}/barrier-report.txt
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyBarrierInstructions.cmake"
        DEPENDS mcuhw_barrier_probe
        VERBATIM
    )
endif()

# The first STM32H753 runtime slice deliberately has no dependency on the
# unfinished cross-target fixture. It proves reset/startup, the reset HSI,
# GPIO clocking and the built-in PE1 LED through the common GPIO driver.
# @req REQ-0003 REQ-0009 REQ-0021 REQ-0031 REQ-0033 REQ-0036
if(MCUHW_BUILD_NUCLEO_H753ZI_TESTS)
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
    target_compile_options(mcuip PRIVATE
        -mcpu=cortex-m7
        -mthumb
        -mfpu=fpv5-d16
        -mfloat-abi=hard
        -ffunction-sections
        -fdata-sections
        -fno-unwind-tables
        -fno-asynchronous-unwind-tables
        -fno-threadsafe-statics
        -fno-use-cxa-atexit
    )
    file(GLOB_RECURSE _mcuhw_mcuip_identity_files CONFIGURE_DEPENDS
        "${MCUHW_MCUIP_SOURCE_DIR}/include/mcuip/*.hpp"
        "${MCUHW_MCUIP_SOURCE_DIR}/src/*.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${_mcuhw_mcuip_identity_files})
    list(SORT _mcuhw_mcuip_identity_files)
    set(_mcuhw_mcuip_identity_material "")
    foreach(_mcuhw_mcuip_file IN LISTS _mcuhw_mcuip_identity_files)
        file(SHA256 "${_mcuhw_mcuip_file}" _mcuhw_mcuip_file_hash)
        file(RELATIVE_PATH _mcuhw_mcuip_relative
            "${MCUHW_MCUIP_SOURCE_DIR}" "${_mcuhw_mcuip_file}")
        string(APPEND _mcuhw_mcuip_identity_material
            "${_mcuhw_mcuip_relative}:${_mcuhw_mcuip_file_hash}\n")
    endforeach()
    string(SHA256 MCUHW_MCUIP_SOURCE_ID
        "${_mcuhw_mcuip_identity_material}")
    file(GLOB_RECURSE _mcuhw_ethernet_identity_files CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/mcuhw/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/board/nucleo_h753zi/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/board/nucleo_h753zi/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/board/nucleo_h753zi/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/board/nucleo_h753zi/*.ld")
    list(APPEND _mcuhw_ethernet_identity_files
        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${_mcuhw_ethernet_identity_files})
    list(SORT _mcuhw_ethernet_identity_files)
    set(_mcuhw_ethernet_identity_material "")
    foreach(_mcuhw_file IN LISTS _mcuhw_ethernet_identity_files)
        file(SHA256 "${_mcuhw_file}" _mcuhw_file_hash)
        file(RELATIVE_PATH _mcuhw_relative
            "${CMAKE_CURRENT_SOURCE_DIR}" "${_mcuhw_file}")
        string(APPEND _mcuhw_ethernet_identity_material
            "${_mcuhw_relative}:${_mcuhw_file_hash}\n")
    endforeach()
    string(SHA256 MCUHW_SOURCE_ID "${_mcuhw_ethernet_identity_material}")

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

    set(nucleo_linker_script
        "${CMAKE_CURRENT_SOURCE_DIR}/board/nucleo_h753zi/linker.ld")
    find_program(MCUHW_ARM_SIZE arm-none-eabi-size REQUIRED)

    add_library(mcuhw_nucleo_h753zi_platform OBJECT
        board/nucleo_h753zi/startup.S
        board/nucleo_h753zi/restricted_runtime.cpp
    )
    add_library(mcuhw_nucleo_h753zi_early_init_coverage OBJECT
        board/nucleo_h753zi/early_init.cpp
    )
    target_compile_definitions(mcuhw_nucleo_h753zi_early_init_coverage PRIVATE
        MCUCOV_BYTE_PER_PROBE=1
        MCUCOV_COMPACT_RUNTIME=1
    )
    target_compile_options(mcuhw_nucleo_h753zi_early_init_coverage PRIVATE
        -include
        "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/tests/mcucov_compat.hpp"
    )
    mcucov_instrument_target(
        mcuhw_nucleo_h753zi_early_init_coverage
        PROFILE FULL
        BUILD_ID 0x4d487501
        SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_DIR
            "${CMAKE_CURRENT_BINARY_DIR}/mcucov/mcuhw_nucleo_h753zi_early_init_coverage"
    )
    add_executable(mcuhw_nucleo_h753zi_startup_gpio_smoke
        board/nucleo_h753zi/tests/startup_gpio.cpp
        $<TARGET_OBJECTS:mcuhw_nucleo_h753zi_platform>
        $<TARGET_OBJECTS:mcuhw_nucleo_h753zi_early_init_coverage>
    )
    add_executable(mcuhw_nucleo_h753zi_native_peripherals
        board/nucleo_h753zi/tests/native_peripherals.cpp
        $<TARGET_OBJECTS:mcuhw_nucleo_h753zi_platform>
        $<TARGET_OBJECTS:mcuhw_nucleo_h753zi_early_init_coverage>
    )
    add_executable(mcuhw_nucleo_h753zi_ethernet
        board/nucleo_h753zi/tests/ethernet_mcuip.cpp
        $<TARGET_OBJECTS:mcuhw_nucleo_h753zi_platform>
        $<TARGET_OBJECTS:mcuhw_nucleo_h753zi_early_init_coverage>
    )
    target_link_libraries(mcuhw_nucleo_h753zi_native_peripherals PRIVATE
        mcujson::mcujson)
    target_link_libraries(mcuhw_nucleo_h753zi_ethernet PRIVATE
        mcujson::mcujson mcuip::mcuip)
    target_include_directories(mcuhw_nucleo_h753zi_native_peripherals PRIVATE
        "${MCUHW_MCUTEST_SOURCE_DIR}/include"
    )
    target_include_directories(mcuhw_nucleo_h753zi_ethernet PRIVATE
        "${MCUHW_MCUTEST_SOURCE_DIR}/include"
    )
    target_compile_definitions(mcuhw_nucleo_h753zi_native_peripherals PRIVATE
        MCUTEST_MAX_TESTS=8
        MCUTEST_SOURCE_LOCATION=1
        MCUCOV_BYTE_PER_PROBE=1
        MCUCOV_COMPACT_RUNTIME=1
    )
    target_compile_definitions(mcuhw_nucleo_h753zi_ethernet PRIVATE
        MCUTEST_MAX_TESTS=3
        MCUTEST_SOURCE_LOCATION=1
        MCUCOV_BYTE_PER_PROBE=1
        MCUCOV_COMPACT_RUNTIME=1
        MCUHW_SOURCE_ID="${MCUHW_SOURCE_ID}"
        MCUHW_MCUIP_SOURCE_ID="${MCUHW_MCUIP_SOURCE_ID}"
    )
    target_compile_options(mcuhw_nucleo_h753zi_native_peripherals PRIVATE
        -include
        "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/tests/mcucov_compat.hpp"
    )
    target_compile_options(mcuhw_nucleo_h753zi_ethernet PRIVATE
        -include
        "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/tests/mcucov_compat.hpp"
    )
    mcucov_instrument_target(
        mcuhw_nucleo_h753zi_native_peripherals
        PROFILE FULL
        BUILD_ID 0x4d487501
        SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_DIR
            "${CMAKE_CURRENT_BINARY_DIR}/mcucov/mcuhw_nucleo_h753zi_native_peripherals"
    )
    mcucov_instrument_target(
        mcuhw_nucleo_h753zi_ethernet
        PROFILE FULL
        BUILD_ID 0x4d487502
        SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_DIR
            "${CMAKE_CURRENT_BINARY_DIR}/mcucov/mcuhw_nucleo_h753zi_ethernet"
    )

    # The instrumented board translation unit can embed more modules than one
    # mcucov manifest serializes. These analysis-only objects regenerate the
    # identical source manifests for the remaining STM32 backend headers; they
    # are not linked into the firmware.
    function(mcuhw_add_stm32h753_coverage_manifest_probe name definition)
        set(target "mcuhw_nucleo_h753zi_${name}_coverage_manifest")
        add_library(${target} OBJECT
            tests/compile/stm32h753_backend_coverage_probe.cpp)
        target_link_libraries(${target} PRIVATE mcuhw::mcuhw)
        target_compile_features(${target} PRIVATE cxx_std_23)
        target_compile_definitions(${target} PRIVATE
            ${definition}=1
            MCUCOV_BYTE_PER_PROBE=1
            MCUCOV_COMPACT_RUNTIME=1
        )
        target_compile_options(${target} PRIVATE
            ${MCUHW_RESTRICTED_COMPILE_OPTIONS}
            -mcpu=cortex-m7
            -mthumb
            -mfpu=fpv5-d16
            -mfloat-abi=hard
            -ffunction-sections
            -fdata-sections
            -fno-unwind-tables
            -fno-asynchronous-unwind-tables
            -include
            "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/tests/mcucov_compat.hpp"
        )
        mcucov_instrument_target(
            ${target}
            PROFILE FULL
            BUILD_ID 0x4d487501
            SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${target}"
        )
    endfunction()

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
    foreach(nucleo_target IN ITEMS
            mcuhw_nucleo_h753zi_startup_gpio_smoke
            mcuhw_nucleo_h753zi_native_peripherals
            mcuhw_nucleo_h753zi_ethernet)
        set_target_properties(${nucleo_target} PROPERTIES
            SUFFIX ".elf"
            LINK_DEPENDS "${nucleo_linker_script}"
        )
    endforeach()
    foreach(compiled_target IN ITEMS
            mcuhw_nucleo_h753zi_platform
            mcuhw_nucleo_h753zi_early_init_coverage
            mcuhw_nucleo_h753zi_startup_gpio_smoke
            mcuhw_nucleo_h753zi_native_peripherals
            mcuhw_nucleo_h753zi_ethernet)
        target_link_libraries(${compiled_target} PRIVATE mcuhw::mcuhw)
        if(compiled_target STREQUAL mcuhw_nucleo_h753zi_native_peripherals
           OR compiled_target STREQUAL mcuhw_nucleo_h753zi_ethernet)
            # mcujson uses hosted C++23 headers but remains allocation-free;
            # keep the restricted language options without -ffreestanding.
            target_compile_features(${compiled_target} PRIVATE cxx_std_23)
            target_compile_options(${compiled_target} PRIVATE
                ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
        else()
            mcuhw_apply_restricted_runtime(${compiled_target})
        endif()
        target_compile_options(${compiled_target} PRIVATE
            -mcpu=cortex-m7
            -mthumb
            -mfpu=fpv5-d16
            -mfloat-abi=hard
            -ffunction-sections
            -fdata-sections
            -fno-unwind-tables
            -fno-asynchronous-unwind-tables
        )
    endforeach()
    foreach(nucleo_target IN ITEMS
            mcuhw_nucleo_h753zi_startup_gpio_smoke
            mcuhw_nucleo_h753zi_native_peripherals
            mcuhw_nucleo_h753zi_ethernet)
        target_link_options(${nucleo_target} PRIVATE
            -mcpu=cortex-m7
            -mthumb
            -mfpu=fpv5-d16
            -mfloat-abi=hard
            -nostartfiles
            --specs=nosys.specs
            "-T${nucleo_linker_script}"
            -Wl,--gc-sections
            -Wl,--print-memory-usage
            "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${nucleo_target}.map"
        )
    endforeach()
    add_custom_command(TARGET mcuhw_nucleo_h753zi_startup_gpio_smoke POST_BUILD
        COMMAND "${CMAKE_OBJCOPY}" -O binary
            $<TARGET_FILE:mcuhw_nucleo_h753zi_startup_gpio_smoke>
            "${CMAKE_CURRENT_BINARY_DIR}/mcuhw_nucleo_h753zi_startup_gpio_smoke.bin"
        COMMAND "${CMAKE_OBJCOPY}" -O ihex
            $<TARGET_FILE:mcuhw_nucleo_h753zi_startup_gpio_smoke>
            "${CMAKE_CURRENT_BINARY_DIR}/mcuhw_nucleo_h753zi_startup_gpio_smoke.hex"
        COMMAND "${MCUHW_ARM_SIZE}" -A
            $<TARGET_FILE:mcuhw_nucleo_h753zi_startup_gpio_smoke>
        VERBATIM
    )
    add_custom_command(TARGET mcuhw_nucleo_h753zi_native_peripherals POST_BUILD
        COMMAND "${CMAKE_OBJCOPY}" -O binary
            $<TARGET_FILE:mcuhw_nucleo_h753zi_native_peripherals>
            "${CMAKE_CURRENT_BINARY_DIR}/mcuhw_nucleo_h753zi_native_peripherals.bin"
        COMMAND "${CMAKE_OBJCOPY}" -O ihex
            $<TARGET_FILE:mcuhw_nucleo_h753zi_native_peripherals>
            "${CMAKE_CURRENT_BINARY_DIR}/mcuhw_nucleo_h753zi_native_peripherals.hex"
        COMMAND "${MCUHW_ARM_SIZE}" -A
            $<TARGET_FILE:mcuhw_nucleo_h753zi_native_peripherals>
        VERBATIM
    )
    add_custom_command(TARGET mcuhw_nucleo_h753zi_ethernet POST_BUILD
        COMMAND "${CMAKE_OBJCOPY}" -O binary
            $<TARGET_FILE:mcuhw_nucleo_h753zi_ethernet>
            "${CMAKE_CURRENT_BINARY_DIR}/mcuhw_nucleo_h753zi_ethernet.bin"
        COMMAND "${CMAKE_OBJCOPY}" -O ihex
            $<TARGET_FILE:mcuhw_nucleo_h753zi_ethernet>
            "${CMAKE_CURRENT_BINARY_DIR}/mcuhw_nucleo_h753zi_ethernet.hex"
        COMMAND "${MCUHW_ARM_SIZE}" -A
            $<TARGET_FILE:mcuhw_nucleo_h753zi_ethernet>
        VERBATIM
    )
endif()

if(MCUHW_ARCH STREQUAL "cortex-m4")
    add_library(mcuhw_ra4m1_gpio_codegen OBJECT
        tests/compile/ra4m1_gpio_codegen_probe.cpp)
    target_link_libraries(mcuhw_ra4m1_gpio_codegen PRIVATE mcuhw::mcuhw)
    mcuhw_apply_restricted_runtime(mcuhw_ra4m1_gpio_codegen)
    target_compile_options(mcuhw_ra4m1_gpio_codegen PRIVATE -Os)
    add_custom_target(mcuhw_gpio_codegen_report ALL
        COMMAND "${CMAKE_COMMAND}"
            -DNM=${CMAKE_NM}
            -DOBJDUMP=${CMAKE_OBJDUMP}
            -DOBJECT_FILE=$<TARGET_OBJECTS:mcuhw_ra4m1_gpio_codegen>
            -DREPORT=${CMAKE_CURRENT_BINARY_DIR}/gpio-codegen-report.txt
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateGpioCodegenReport.cmake"
        DEPENDS mcuhw_ra4m1_gpio_codegen
        VERBATIM
    )
endif()

# Every public header is compiled in an otherwise-empty translation unit. This
# keeps accidental include-order dependencies out of the interface.
# @req REQ-0002 REQ-0003 REQ-0038
mcuhw_add_header_self_containment_checks(mcuhw_headers
    ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

mcuhw_write_toolchain_report(
    "${CMAKE_CURRENT_BINARY_DIR}/toolchain-report.txt"
)

# Board composition is explicit and opt-in: ordinary architecture probes do
# not acquire startup files, linker ownership, or a board dependency.
# @req REQ-0022 REQ-0031 REQ-0036 REQ-0037 REQ-0038
if(MCUHW_BUILD_UNO_R4_MINIMA_TESTS)
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

    set(uno_linker_script
        "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/linker.ld")
    find_program(MCUHW_ARM_SIZE arm-none-eabi-size REQUIRED)

    function(mcuhw_add_uno_r4_minima_test target source build_id)
        set(platform_target "${target}_platform")
        add_library(${platform_target} OBJECT
            board/uno_r4_minima/startup.S
            board/uno_r4_minima/early_init.cpp
            board/uno_r4_minima/restricted_runtime.cpp
        )
        add_executable(${target} "${source}" $<TARGET_OBJECTS:${platform_target}>)
        set_target_properties(${target} PROPERTIES
            SUFFIX ".elf"
            LINK_DEPENDS "${uno_linker_script}"
        )
        target_link_libraries(${platform_target} PRIVATE mcuhw::mcuhw)
        target_link_libraries(${target} PRIVATE mcuhw::mcuhw mcujson::mcujson)
        target_include_directories(${target} PRIVATE
            "${MCUHW_MCUTEST_SOURCE_DIR}/include"
        )
        target_compile_definitions(${target} PRIVATE
            MCUTEST_MAX_TESTS=8
            MCUTEST_SOURCE_LOCATION=1
            MCUCOV_BYTE_PER_PROBE=1
            MCUCOV_COMPACT_RUNTIME=1
        )
        mcuhw_apply_restricted_runtime(${platform_target})
        # mcujson's forward writer uses the hosted C++23 charconv header but
        # remains allocation-free. The final image still uses the restricted
        # runtime and nosys linker specs; compile this reporting TU in hosted
        # library mode so GCC exposes charconv.
        target_compile_features(${target} PRIVATE cxx_std_23)
        target_compile_options(${target} PRIVATE
            ${MCUHW_RESTRICTED_COMPILE_OPTIONS}
        )
        foreach(compiled_target IN ITEMS ${platform_target} ${target})
            target_compile_options(${compiled_target} PRIVATE
                -mcpu=cortex-m4
                -mthumb
                -mfpu=fpv4-sp-d16
                -mfloat-abi=hard
                -ffunction-sections
                -fdata-sections
                -fno-unwind-tables
                -fno-asynchronous-unwind-tables
            )
        endforeach()
        target_compile_options(${target} PRIVATE
            -include
            "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/tests/mcucov_compat.hpp"
        )
        target_link_options(${target} PRIVATE
            -mcpu=cortex-m4
            -mthumb
            -mfpu=fpv4-sp-d16
            -mfloat-abi=hard
            -nostartfiles
            --specs=nosys.specs
            "-T${uno_linker_script}"
            -Wl,--gc-sections
            -Wl,--print-memory-usage
            "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${target}.map"
        )
        mcucov_instrument_target(
            ${target}
            PROFILE FULL
            BUILD_ID ${build_id}
            SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${target}"
        )
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_OBJCOPY}" -O binary
                $<TARGET_FILE:${target}>
                "${CMAKE_CURRENT_BINARY_DIR}/${target}.bin"
            COMMAND "${CMAKE_OBJCOPY}" -O ihex
                $<TARGET_FILE:${target}>
                "${CMAKE_CURRENT_BINARY_DIR}/${target}.hex"
            COMMAND "${MCUHW_ARM_SIZE}" -A $<TARGET_FILE:${target}>
            VERBATIM
        )
    endfunction()

    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_startup_gpio_tests
        board/uno_r4_minima/tests/startup_gpio.cpp
        0x4d485701)
    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_gpio_loopback_tests
        board/uno_r4_minima/tests/gpio_loopback.cpp
        0x4d485701)
    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_pwm_tests
        board/uno_r4_minima/tests/pwm.cpp
        0x4d485701)
    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_uart_tests
        board/uno_r4_minima/tests/uart.cpp
        0x4d485701)
    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_i2c_tests
        board/uno_r4_minima/tests/i2c.cpp
        0x4d485701)
    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_spi_tests
        board/uno_r4_minima/tests/spi.cpp
        0x4d485701)
    mcuhw_add_uno_r4_minima_test(
        mcuhw_uno_r4_minima_adc_tests
        board/uno_r4_minima/tests/adc.cpp
        0x4d485701)
endif()

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    if(NOT EXISTS "${MCUHW_MCUTEST_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "mcutest was not found at ${MCUHW_MCUTEST_SOURCE_DIR}; "
            "set MCUHW_MCUTEST_SOURCE_DIR to the sibling mcutest checkout")
    endif()

    set(MCUTEST_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(MCUTEST_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory("${MCUHW_MCUTEST_SOURCE_DIR}"
                     "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcutest")
    add_subdirectory(tests)
endif()
