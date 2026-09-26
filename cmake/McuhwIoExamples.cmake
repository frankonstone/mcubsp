include_guard(GLOBAL)
option(MCUHW_BUILD_IO_EXAMPLES "Build standalone UNO R4 bare-metal io examples" OFF)
if(MCUHW_BUILD_IO_EXAMPLES)
    if(NOT MCUHW_ARCH STREQUAL "cortex-m4")
        message(FATAL_ERROR "IO examples use the UNO R4 Cortex-M4 toolchain")
    endif()
    set(_io_linker "${CMAKE_CURRENT_SOURCE_DIR}/board/uno_r4_minima/linker.ld")
    foreach(_example IN ITEMS blink serial_echo spi_dma tmp102 w5100)
        set(_target "mcuhw_io_${_example}")
        add_executable(${_target}
            "examples/io/${_example}.cpp"
            board/uno_r4_minima/startup.S
            board/uno_r4_minima/early_init.cpp
            board/uno_r4_minima/restricted_runtime.cpp)
        target_link_libraries(${_target} PRIVATE mcuhw::io)
        mcuhw_apply_restricted_runtime(${_target})
        target_compile_options(${_target} PRIVATE
            -Os -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
            -ffunction-sections -fdata-sections -fno-unwind-tables
            -fno-asynchronous-unwind-tables)
        target_link_options(${_target} PRIVATE
            -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
            -nostartfiles --specs=nosys.specs "-T${_io_linker}"
            -Wl,--gc-sections -Wl,--print-memory-usage
            "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${_target}.map")
        set_target_properties(${_target} PROPERTIES
            SUFFIX ".elf" LINK_DEPENDS "${_io_linker}")
    endforeach()
endif()
