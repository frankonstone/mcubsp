# Pass this before project(): -DCMAKE_TOOLCHAIN_FILE=<root>/cmake/McuhwToolchain.cmake
# @req REQ-0049
include("${CMAKE_CURRENT_LIST_DIR}/McuhwSelection.cmake")
mcuhw_resolve_board()
include("${MCUHW_BOARD_TOOLCHAIN}")
get_filename_component(_mcuhw_selected_compiler "${CMAKE_CXX_COMPILER}" REALPATH)
if(DEFINED MCUHW_CONFIGURED_COMPILER AND
        NOT MCUHW_CONFIGURED_COMPILER STREQUAL _mcuhw_selected_compiler)
    message(FATAL_ERROR "This build directory uses ${MCUHW_CONFIGURED_COMPILER}. Select a separate build directory for compiler ${_mcuhw_selected_compiler}.")
endif()
set(MCUHW_CONFIGURED_COMPILER "${_mcuhw_selected_compiler}" CACHE INTERNAL
    "Immutable selected compiler installation")
get_filename_component(_mcuhw_compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
get_filename_component(_mcuhw_compiler_name "${CMAKE_CXX_COMPILER}" NAME)
string(REPLACE "g++" "gcc" _mcuhw_c_compiler_name "${_mcuhw_compiler_name}")
set(CMAKE_C_COMPILER "${_mcuhw_compiler_directory}/${_mcuhw_c_compiler_name}")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    MCUHW_ROOT MCUHW_BOARD MCUHW_BOARD_REVISION MCUHW_ARM_CXX_COMPILER
    MCUHW_ESP32P4_CXX_COMPILER MCUHW_ESP32P4_TOOLCHAIN_ROOT)
