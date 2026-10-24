if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" OR
        NOT CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
    message(FATAL_ERROR "The Linux x86_64 host toolchain requires a Linux x86_64 host")
endif()

if(NOT CMAKE_CXX_COMPILER)
    find_program(MCUBSP_LINUX_CXX NAMES g++-15 g++-14 g++ clang++ REQUIRED NO_CACHE)
    set(CMAKE_CXX_COMPILER "${MCUBSP_LINUX_CXX}")
endif()
