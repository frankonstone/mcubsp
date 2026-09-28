cmake_minimum_required(VERSION 3.22)

set(BOOTLOADER_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../bsp/bootloader_components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(mcuhw_esp32p4_baremetal_smoke)
