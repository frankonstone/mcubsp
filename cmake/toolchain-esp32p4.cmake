set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(MCUHW_ESP32P4_TOOLCHAIN_ROOT "" CACHE PATH
    "Root of an Espressif riscv32-esp-elf toolchain")

set(esp32p4_toolchain_hints "${MCUHW_ESP32P4_TOOLCHAIN_ROOT}")
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

find_program(MCUHW_ESP32P4_CXX_COMPILER riscv32-esp-elf-g++
    HINTS ${esp32p4_toolchain_hints}
    PATH_SUFFIXES bin)
if(NOT MCUHW_ESP32P4_CXX_COMPILER)
    message(FATAL_ERROR
        "riscv32-esp-elf-g++ was not found. Install the ESP-IDF toolchain "
        "selected for ESP32-P4, activate its environment, or set "
        "MCUHW_ESP32P4_TOOLCHAIN_ROOT and configure again.")
endif()

set(CMAKE_CXX_COMPILER "${MCUHW_ESP32P4_CXX_COMPILER}")
set(MCUHW_ARCH "esp32p4-riscv" CACHE STRING "Architecture probed by this build" FORCE)
include("${CMAKE_CURRENT_LIST_DIR}/../board/architecture/esp32p4-riscv.cmake")
