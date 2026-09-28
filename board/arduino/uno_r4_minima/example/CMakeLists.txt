cmake_minimum_required(VERSION 3.25)
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(mcuhw_uno_examples LANGUAGES CXX ASM)
endif()
if(TARGET mcuhw::mcuhw AND NOT MCUHW_SOURCE_DIR)
    set(MCUHW_SOURCE_DIR "${MCUHW_ROOT}")
endif()
if(MCUHW_SOURCE_DIR AND EXISTS "${MCUHW_SOURCE_DIR}/CMakeLists.txt")
    find_package(mcuhw CONFIG REQUIRED
        PATHS "${MCUHW_SOURCE_DIR}/cmake" NO_DEFAULT_PATH)
elseif(MCUHW_SOURCE_DIR)
    find_package(mcuhw CONFIG REQUIRED
        PATHS "${MCUHW_SOURCE_DIR}/../../lib/cmake/mcuhw" NO_DEFAULT_PATH)
else()
    find_package(mcuhw CONFIG REQUIRED)
endif()
if(NOT MCUHW_BOARD STREQUAL "arduino/uno_r4_minima")
    message(FATAL_ERROR "The UNO examples require arduino/uno_r4_minima")
endif()
mcuhw_load_board()
if(NOT MCUHW_BOARD_PROFILE STREQUAL "mcuhw::io::boards::uno_r4_minima")
    message(FATAL_ERROR "The UNO examples require arduino/uno_r4_minima")
endif()

enable_language(ASM)

# Each source owns one stable executable, in both standalone and workspace builds.
add_executable(arduino.uno_r4_minima.example.blink blink.cpp)
add_executable(arduino.uno_r4_minima.example.serial_echo serial_echo.cpp)
add_executable(arduino.uno_r4_minima.example.spi_dma spi_dma.cpp)
add_executable(arduino.uno_r4_minima.example.tmp102 tmp102.cpp)
add_executable(arduino.uno_r4_minima.example.w5100 w5100.cpp)

set(uno_examples
    arduino.uno_r4_minima.example.blink
    arduino.uno_r4_minima.example.serial_echo
    arduino.uno_r4_minima.example.spi_dma
    arduino.uno_r4_minima.example.tmp102
    arduino.uno_r4_minima.example.w5100)
foreach(target IN LISTS uno_examples)
    mcuhw_configure_board_firmware(${target} STARTUP supplied)
    mcuhw_add_firmware_artifact_report(${target} supplied)
    set_target_properties(${target} PROPERTIES FOLDER "Boards/arduino/uno_r4_minima/Examples")
endforeach()

# A selected board builds its chosen example by default; the examples task
# explicitly builds all five. A workspace leaves every image opt-in.
if(MCUHW_SELECTED_EXAMPLE)
    if(NOT "arduino.uno_r4_minima.example.${MCUHW_SELECTED_EXAMPLE}" IN_LIST uno_examples)
        message(FATAL_ERROR "MCUHW_EXAMPLE '${MCUHW_SELECTED_EXAMPLE}' is not a declared example")
    endif()
    foreach(target IN LISTS uno_examples)
        if(NOT target STREQUAL "arduino.uno_r4_minima.example.${MCUHW_SELECTED_EXAMPLE}")
            set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()
    endforeach()
elseif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(MCUHW_EXAMPLE blink CACHE STRING "UNO example source name")
    if(NOT "arduino.uno_r4_minima.example.${MCUHW_EXAMPLE}" IN_LIST uno_examples)
        message(FATAL_ERROR "Unknown MCUHW_EXAMPLE '${MCUHW_EXAMPLE}'")
    endif()
    add_custom_target(firmware DEPENDS arduino.uno_r4_minima.example.${MCUHW_EXAMPLE})
    foreach(target IN LISTS uno_examples)
        if(NOT target STREQUAL "arduino.uno_r4_minima.example.${MCUHW_EXAMPLE}")
            set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()
    endforeach()
endif()
