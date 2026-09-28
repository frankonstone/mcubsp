cmake_minimum_required(VERSION 3.25)
project(mcuhw VERSION 0.1.0
    DESCRIPTION "Source-traceable MCU descriptions and bare-metal drivers"
    LANGUAGES CXX)

# Library consumers do not inherit maintainer targets or global testing policy.
# @req REQ-0003 REQ-0047
option(MCUHW_BUILD_TESTS "Build native behavioral and compile tests" OFF)
option(MCUHW_BUILD_PROBES "Build architecture verification probes" OFF)
option(MCUHW_CHECK_HEADERS "Compile every public header independently" OFF)
option(MCUHW_BUILD_MCUIP_CONTRACTS "Verify the optional MCUIP integration" OFF)
option(MCUHW_WRITE_TOOLCHAIN_REPORT "Write maintainer compiler identity report" OFF)
option(MCUHW_BUILD_TOOLING_TESTS "Test packaging and build tooling" OFF)
option(MCUHW_BUILD_IO_COST_REPORT "Build paired Cortex-M4 driver/io facade RAM and flash probes" OFF)
option(MCUHW_BUILD_IO_EXAMPLES "Build standalone UNO R4 bare-metal io examples" OFF)
option(MCUHW_INSTALL "Install library and reusable board support" ${PROJECT_IS_TOP_LEVEL})
option(MCUHW_WORKSPACE "Expose task groups for the active boards of this architecture" OFF)
set(MCUHW_ACTIVE_BOARDS "" CACHE STRING "Active vendor/board[@revision] IDs, separated by semicolons; empty for none, all for maintainer builds")
set(MCUHW_BOARD "" CACHE STRING "Selected board vendor/name; exposes firmware, tests and examples")
set(MCUHW_BOARD_REVISION "" CACHE STRING "Board revision; empty selects the board default")
set(MCUHW_BOARD_TESTS "" CACHE STRING "Board ID whose physical-test images to build")
# An existing BSP may select its own cross-toolchain without mcuhw variables.
# Its architecture is resolved from the selected board only when unspecified.
if(NOT DEFINED MCUHW_ARCH)
    if(CMAKE_CROSSCOMPILING)
        set(MCUHW_ARCH "" CACHE STRING "Architecture probed by this build")
    else()
        set(MCUHW_ARCH "native" CACHE STRING "Architecture probed by this build")
    endif()
endif()
set(MCUHW_EXAMPLE "" CACHE STRING "Firmware example for the selected board; empty selects blink")
set(MCUHW_TEST_SUITES "all" CACHE STRING "Native suite selection for contributor/maintainer builds")
if(PROJECT_IS_TOP_LEVEL)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

add_library(mcuhw INTERFACE)
add_library(mcuhw::mcuhw ALIAS mcuhw)
target_include_directories(mcuhw INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:share/mcuhw/include>)
target_compile_features(mcuhw INTERFACE cxx_std_23)

# @req REQ-0045 REQ-0046 REQ-0047
add_library(mcuhw_io INTERFACE)
add_library(mcuhw::io ALIAS mcuhw_io)
set_target_properties(mcuhw_io PROPERTIES EXPORT_NAME io)
target_link_libraries(mcuhw_io INTERFACE mcuhw::mcuhw)
add_library(mcuhw_device INTERFACE)
add_library(mcuhw::device ALIAS mcuhw_device)
set_target_properties(mcuhw_device PROPERTIES EXPORT_NAME device)
target_link_libraries(mcuhw_device INTERFACE mcuhw::mcuhw)

# add_subdirectory consumers use these functions to create their own firmware.
include(board/Support.cmake)

# Explicit maintainer workflows alone may resolve the development dependencies.
if(MCUHW_BUILD_TESTS OR MCUHW_BUILD_MCUIP_CONTRACTS OR MCUHW_BOARD_TESTS)
    foreach(component IN ITEMS MCUTEST MCUJSON MCUCOV MCUIP)
        string(TOLOWER "${component}" directory)
        set(MCUHW_${component}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${directory}"
            CACHE PATH "Optional local ${directory} checkout")
    endforeach()
endif()

if(MCUHW_BUILD_IO_EXAMPLES)
    if(NOT MCUHW_ARCH STREQUAL "cortex-m4")
        message(FATAL_ERROR "IO examples use the UNO R4 Cortex-M4 toolchain")
    endif()
    if(MCUHW_BOARD AND NOT MCUHW_BOARD STREQUAL "arduino/uno_r4_minima")
        message(FATAL_ERROR "IO examples currently support only arduino/uno_r4_minima")
    endif()
    if(NOT MCUHW_BOARD)
        set(MCUHW_BOARD arduino/uno_r4_minima)
    endif()
endif()

# Board tasks follow an explicit board or workspace selection.
# @req REQ-0038 REQ-0047 REQ-0049 REQ-0052
if(MCUHW_BOARD_TESTS OR (PROJECT_IS_TOP_LEVEL AND MCUHW_BOARD) OR
        (MCUHW_WORKSPACE AND NOT MCUHW_BOARD))
    include(board/Workspace.cmake)
    if(MCUHW_BOARD_TESTS)
        include(test/support/ProjectOptions.cmake)
        mcuhw_add_board_test_images()
    elseif(MCUHW_BOARD)
        mcuhw_add_selected_board()
    else()
        if(MCUHW_EXAMPLE)
            message(FATAL_ERROR "Set MCUHW_BOARD for one firmware application, or clear MCUHW_EXAMPLE when using MCUHW_ACTIVE_BOARDS.")
        endif()
        mcuhw_add_active_boards()
    endif()
endif()

if(MCUHW_BUILD_MCUIP_CONTRACTS AND NOT MCUHW_BUILD_PROBES)
    message(FATAL_ERROR "MCUHW_BUILD_MCUIP_CONTRACTS requires MCUHW_BUILD_PROBES")
endif()
if(MCUHW_BUILD_TESTS OR MCUHW_BUILD_TOOLING_TESTS)
    enable_testing()
endif()
if(MCUHW_BUILD_TESTS)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "MCUHW_BUILD_TESTS requires a native compiler")
    endif()
    if(NOT TARGET mcutest::mcutest)
        if(NOT EXISTS "${MCUHW_MCUTEST_SOURCE_DIR}/CMakeLists.txt")
            message(FATAL_ERROR "Native tests requested; set MCUHW_MCUTEST_SOURCE_DIR")
        endif()
        set(MCUTEST_BUILD_TESTS OFF)
        set(MCUTEST_INSTALL OFF)
        add_subdirectory("${MCUHW_MCUTEST_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/_deps/mcutest")
    endif()
endif()
if(MCUHW_BUILD_TESTS OR MCUHW_BUILD_TOOLING_TESTS OR MCUHW_BUILD_PROBES OR
        MCUHW_CHECK_HEADERS OR MCUHW_WRITE_TOOLCHAIN_REPORT OR MCUHW_BUILD_IO_COST_REPORT)
    include(test/support/ProjectOptions.cmake)
    add_subdirectory(test)
endif()

if(MCUHW_INSTALL)
    include(Install.cmake)
endif()
