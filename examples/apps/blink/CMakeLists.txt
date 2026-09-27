cmake_minimum_required(VERSION 3.25)
project(mcuhw_blink LANGUAGES CXX ASM)

if(TARGET mcuhw::mcuhw)
    # Also usable as the single application selected by a repository preset.
elseif(MCUHW_SOURCE_DIR AND EXISTS "${MCUHW_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${MCUHW_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/mcuhw-library")
elseif(MCUHW_SOURCE_DIR)
    # The installed bundle retains its root-relative board/tool layout.
    find_package(mcuhw CONFIG REQUIRED
        PATHS "${MCUHW_SOURCE_DIR}/../../lib/cmake/mcuhw" NO_DEFAULT_PATH)
else()
    find_package(mcuhw CONFIG REQUIRED)
endif()

set(MCUHW_STARTUP supplied CACHE STRING "Startup owner: supplied or external")
if(NOT MCUHW_FIRMWARE_TARGET)
    set(MCUHW_FIRMWARE_TARGET firmware)
endif()
mcuhw_add_firmware(${MCUHW_FIRMWARE_TARGET} SOURCES main.cpp STARTUP "${MCUHW_STARTUP}")
