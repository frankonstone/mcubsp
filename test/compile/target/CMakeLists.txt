cmake_minimum_required(VERSION 3.25)
project(mcubsp_target_probes LANGUAGES CXX)

if(NOT CMAKE_CROSSCOMPILING OR NOT MCUBSP_ARCH)
    message(FATAL_ERROR "Target probes require an mcubsp cross toolchain")
endif()

get_filename_component(mcubsp_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

add_library(mcubsp_headers INTERFACE)
add_library(mcubsp::mcubsp ALIAS mcubsp_headers)
target_include_directories(mcubsp_headers INTERFACE "${mcubsp_SOURCE_DIR}/include")
target_compile_features(mcubsp_headers INTERFACE cxx_std_23)

include("${mcubsp_SOURCE_DIR}/test/support/ProjectOptions.cmake")
set(MCUBSP_BUILD_MCUIP_CONTRACTS ON)
set(MCUBSP_MCUIP_SOURCE_DIR "${mcubsp_SOURCE_DIR}/../mcuip" CACHE PATH
    "MCUIP source checkout for target compile probes")
include("${mcubsp_SOURCE_DIR}/test/compile/Probes.cmake")
