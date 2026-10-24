cmake_minimum_required(VERSION 3.25)
project(mcubsp VERSION 0.1.0
    DESCRIPTION "Source-traceable MCU descriptions and bare-metal drivers"
    LANGUAGES CXX)

option(MCUBSP_COVERAGE "Instrument Arduino UNO R4 Minima test images with MCUCOV" OFF)
option(MCUBSP_BUILD_HOST_TESTS "Build and run native host tests" OFF)

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
    set(MCUBSP_MCUTEST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mcutest" CACHE PATH
        "mcutest source checkout for host suites")
    if(EXISTS "${MCUBSP_MCUTEST_SOURCE_DIR}/CMakeLists.txt")
        set(MCUTEST_BUILD_TESTS OFF)
        set(MCUTEST_INSTALL OFF)
        add_subdirectory("${MCUBSP_MCUTEST_SOURCE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcutest")
    else()
        find_package(mcutest CONFIG REQUIRED)
    endif()
    include(test/support/ProjectOptions.cmake)
    add_subdirectory(test)
    add_custom_target(host-tests ALL
        COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
            --output-on-failure
        DEPENDS compile-probes native-tests target-compile-probes
        USES_TERMINAL)
endif()

if(MCUBSP_BOARD OR (PROJECT_IS_TOP_LEVEL AND NOT MCUBSP_BUILD_HOST_TESTS))
    find_package(mcutest CONFIG REQUIRED)
    find_package(mcucov CONFIG REQUIRED)
    find_package(mcujson CONFIG REQUIRED)
    add_subdirectory(board)
endif()
if(PROJECT_IS_TOP_LEVEL)
    include(Install.cmake)
endif()
