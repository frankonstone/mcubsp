cmake_minimum_required(VERSION 3.25)
project(my_w5100 LANGUAGES CXX ASM)

set(MCUBSP_SOURCE_DIR "" CACHE PATH "mcubsp checkout; leave empty for an installed package")
if(MCUBSP_SOURCE_DIR)
    add_subdirectory("${MCUBSP_SOURCE_DIR}" mcubsp EXCLUDE_FROM_ALL)
else()
    find_package(mcubsp 0.1 CONFIG REQUIRED)
endif()

# MCUBSP_BOARD and its toolchain were selected before project().
# Each source names its concrete C++ board explicitly.
set(w5100_source "${CMAKE_CURRENT_SOURCE_DIR}/${MCUBSP_BOARD}.cpp")
if(NOT EXISTS "${w5100_source}")
    message(FATAL_ERROR "This starter has no w5100 source for ${MCUBSP_BOARD}; add ${w5100_source}")
endif()
mcubsp_add_firmware(w5100 STARTUP supplied SOURCES "${w5100_source}")
