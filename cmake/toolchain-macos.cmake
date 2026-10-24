if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    message(FATAL_ERROR "The macOS host toolchain requires a macOS host")
endif()

if(NOT CMAKE_CXX_COMPILER)
    find_program(MCUBSP_MACOS_CXX NAMES clang++ REQUIRED NO_CACHE)
    set(CMAKE_CXX_COMPILER "${MCUBSP_MACOS_CXX}")
endif()
