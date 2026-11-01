cmake_minimum_required(VERSION 3.25)
project(mcubsp VERSION 0.1.0
    DESCRIPTION "Source-traceable MCU descriptions and bare-metal drivers"
    LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/output/install" CACHE PATH
        "Installation prefix" FORCE)
endif()

option(MCUBSP_COVERAGE "Instrument selected board test images with MCUCOV" OFF)
option(MCUBSP_HOST_COVERAGE "Instrument native suites and export coverage" OFF)
option(MCUBSP_BUILD_HOST_TESTS "Build and run native host tests" OFF)
option(MCUBSP_BUILD_BOARD_TESTS "Build physical-test images (never run or flash)" OFF)
option(MCUBSP_BUILD_TARGET_PROBES "Include cross-compiler probes in the default host build" OFF)
option(MCUBSP_CHECK_HEADERS "Compile each public header independently" OFF)
option(MCUBSP_INSTALL "Generate install rules" ${PROJECT_IS_TOP_LEVEL})
set(MCUBSP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(mcubsp INTERFACE)
add_library(mcubsp::mcubsp ALIAS mcubsp)
target_include_directories(mcubsp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:share/mcubsp/include>)
target_compile_features(mcubsp INTERFACE cxx_std_23)

add_library(mcubsp_io INTERFACE)
add_library(mcubsp::io ALIAS mcubsp_io)
set_target_properties(mcubsp_io PROPERTIES EXPORT_NAME io)
target_link_libraries(mcubsp_io INTERFACE mcubsp::mcubsp)

add_library(mcubsp_device INTERFACE)
add_library(mcubsp::device ALIAS mcubsp_device)
set_target_properties(mcubsp_device PROPERTIES EXPORT_NAME device)
target_link_libraries(mcubsp_device INTERFACE mcubsp::mcubsp)

# Public application helpers define functions only; dependencies are discovered
# when an application explicitly requests a firmware target.
include(cmake/Firmware.cmake)

if(MCUBSP_HOST_COVERAGE AND NOT MCUBSP_BUILD_HOST_TESTS)
    message(FATAL_ERROR "MCUBSP_HOST_COVERAGE requires MCUBSP_BUILD_HOST_TESTS=ON")
endif()
if(MCUBSP_BUILD_HOST_TESTS)
    if(NOT PROJECT_IS_TOP_LEVEL)
        message(FATAL_ERROR "Host tests require a top-level mcubsp build")
    endif()
    if(MCUBSP_BOARD)
        message(FATAL_ERROR "Host tests use a native build; clear MCUBSP_BOARD")
    endif()
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Host tests require a native compiler")
    endif()
    enable_testing()
    include(test/support/TestDependencies.cmake)
    mcubsp_test_dependency(mcutest)
    if(MCUBSP_HOST_COVERAGE)
        if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            message(FATAL_ERROR "Native MCUCOV analysis with GCC/macOS system headers is not qualified; use AppleClang in a fresh coverage build directory")
        endif()
        find_package(mcucov CONFIG REQUIRED)
        find_program(MCUBSP_MCUCOV_EXECUTABLE NAMES mcucov
            HINTS "${mcucov_DIR}/../../../bin" REQUIRED)
    endif()
    include(test/support/ProjectOptions.cmake)
    add_subdirectory(test)
    find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
    add_custom_target(host-tests ALL
        COMMAND "${CMAKE_COMMAND}" -E rm -f "${CMAKE_BINARY_DIR}/artifact/native.json"
            "${PROJECT_SOURCE_DIR}/output/artifact/native.json"
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CMAKE_BINARY_DIR}/artifact/native-trace"
            "${PROJECT_SOURCE_DIR}/output/artifact/native"
        COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
            --output-on-failure
        COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/test/support/MergeResults.py"
            --registry "${CMAKE_BINARY_DIR}/artifact/native-results.txt"
            --output "${CMAKE_BINARY_DIR}/artifact/native.json"
            --shard-dir "${CMAKE_BINARY_DIR}/artifact/native-trace"
            --publish-dir "${PROJECT_SOURCE_DIR}/output/artifact/native"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_SOURCE_DIR}/output/artifact"
        COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/artifact/native.json"
            "${PROJECT_SOURCE_DIR}/output/artifact/native.json.tmp"
        COMMAND "${CMAKE_COMMAND}" -E rename "${PROJECT_SOURCE_DIR}/output/artifact/native.json.tmp"
            "${PROJECT_SOURCE_DIR}/output/artifact/native.json"
        DEPENDS compile-probes native-tests
        USES_TERMINAL)
    if(MCUBSP_HOST_COVERAGE)
        add_custom_target(host-coverage ALL
            COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/test/support/ExportCoverage.py"
                --registry "${CMAKE_BINARY_DIR}/artifact/coverage-results.txt"
                --tool "${MCUBSP_MCUCOV_EXECUTABLE}" --source-root "${PROJECT_SOURCE_DIR}"
                --output "${CMAKE_BINARY_DIR}/artifact/coverage"
            DEPENDS host-tests USES_TERMINAL VERBATIM)
    endif()
endif()

if(MCUBSP_CHECK_HEADERS)
    include(test/support/ProjectOptions.cmake)
    mcubsp_add_header_self_containment_checks(check-headers ROOT "${MCUBSP_ROOT}/include")
endif()
if(MCUBSP_BUILD_HOST_TESTS AND MCUBSP_BUILD_TARGET_PROBES)
    add_dependencies(host-tests target-compile-probes)
endif()
if(MCUBSP_BOARD AND PROJECT_IS_TOP_LEVEL)
    if(MCUBSP_ARCH MATCHES "^cortex-m")
        enable_language(ASM)
    endif()
    add_subdirectory(src/board)
    add_subdirectory(example)
endif()
if(MCUBSP_INSTALL)
    include(Install.cmake)
endif()
