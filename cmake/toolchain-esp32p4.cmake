set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(esp32p4_toolchain_hints "")
if(DEFINED ENV{IDF_TOOLS_PATH})
    file(GLOB esp32p4_idf_toolchains
        "$ENV{IDF_TOOLS_PATH}/tools/riscv32-esp-elf/*/riscv32-esp-elf")
    list(APPEND esp32p4_toolchain_hints ${esp32p4_idf_toolchains})
endif()
if(DEFINED ENV{HOME})
    file(GLOB esp32p4_home_toolchains
        "$ENV{HOME}/.espressif/tools/riscv32-esp-elf/*/riscv32-esp-elf")
    list(APPEND esp32p4_toolchain_hints ${esp32p4_home_toolchains})
endif()

find_program(MCUBSP_ESP32P4_CXX_COMPILER riscv32-esp-elf-g++
    HINTS ${esp32p4_toolchain_hints}
    PATH_SUFFIXES bin NO_CACHE)
if(NOT MCUBSP_ESP32P4_CXX_COMPILER)
    message(FATAL_ERROR
        "riscv32-esp-elf-g++ was not found. Install the ESP-IDF toolchain "
        "selected for ESP32-P4 and activate its environment.")
endif()

set(CMAKE_CXX_COMPILER "${MCUBSP_ESP32P4_CXX_COMPILER}")
set(MCUBSP_ARCH "esp32p4-riscv")
include("${CMAKE_CURRENT_LIST_DIR}/arch/esp32p4-riscv.cmake")
