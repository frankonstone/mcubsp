if(NOT MCUBSP_BOARD MATCHES "^[a-z0-9_-]+/[a-z0-9_-]+$")
    message(FATAL_ERROR "Select a board with a CMake configure preset (MCUBSP_BOARD=vendor/name).")
endif()

set(mcubsp_board_directory "${CMAKE_CURRENT_SOURCE_DIR}/${MCUBSP_BOARD}")
if(EXISTS "${mcubsp_board_directory}/CMakeLists.txt")
    add_subdirectory("${MCUBSP_BOARD}")
    return()
endif()

set(mcubsp_board_file "${mcubsp_board_directory}/board.cmake")
if(NOT EXISTS "${mcubsp_board_file}")
    message(FATAL_ERROR "Unknown board: ${MCUBSP_BOARD}")
endif()
include("${mcubsp_board_file}")

get_filename_component(mcubsp_expected_toolchain
    "${PROJECT_SOURCE_DIR}/${MCUBSP_BOARD_TOOLCHAIN}" REALPATH)
get_filename_component(mcubsp_selected_toolchain "${CMAKE_TOOLCHAIN_FILE}" REALPATH)
if(NOT mcubsp_selected_toolchain STREQUAL mcubsp_expected_toolchain)
    message(FATAL_ERROR
        "${MCUBSP_BOARD} requires toolchain ${MCUBSP_BOARD_TOOLCHAIN}; select its preset.")
endif()

set(mcubsp_board_profile "${PROJECT_SOURCE_DIR}/${MCUBSP_BOARD_PROFILE_HEADER}")
if(NOT EXISTS "${mcubsp_board_profile}")
    message(FATAL_ERROR "${MCUBSP_BOARD} profile header not found: ${MCUBSP_BOARD_PROFILE_HEADER}")
endif()

add_library(mcubsp_board INTERFACE)
add_library(mcubsp::board ALIAS mcubsp_board)
target_link_libraries(mcubsp_board INTERFACE mcubsp::io)
target_sources(mcubsp_board INTERFACE "${mcubsp_board_profile}")
target_compile_options(mcubsp_board INTERFACE ${MCUBSP_ARCH_COMPILE_OPTIONS})
set_target_properties(mcubsp_board PROPERTIES
    MCUBSP_BOARD_NAME "${MCUBSP_BOARD}"
    MCUBSP_BOARD_ARCHITECTURE "${MCUBSP_BOARD_ARCHITECTURE}")

message(STATUS "Board: ${MCUBSP_BOARD} (${MCUBSP_BOARD_ARCHITECTURE})")
