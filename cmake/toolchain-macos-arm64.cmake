if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" OR
        NOT CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
    message(FATAL_ERROR "The macOS arm64 host toolchain requires an Apple Silicon macOS host")
endif()

if(NOT CMAKE_CXX_COMPILER)
    find_program(MCUBSP_MACOS_CXX NAMES clang++ REQUIRED NO_CACHE)
    set(CMAKE_CXX_COMPILER "${MCUBSP_MACOS_CXX}")
endif()
