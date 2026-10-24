set(mcubsp_uno_toolchain "${PROJECT_SOURCE_DIR}/cmake/toolchain-arm-cortex-m4.cmake")
get_filename_component(mcubsp_uno_toolchain "${mcubsp_uno_toolchain}" REALPATH)
get_filename_component(mcubsp_selected_toolchain "${CMAKE_TOOLCHAIN_FILE}" REALPATH)
if(NOT mcubsp_selected_toolchain STREQUAL mcubsp_uno_toolchain)
    message(FATAL_ERROR "arduino/uno_r4_minima requires the Cortex-M4 toolchain preset")
endif()

add_library(mcubsp_board INTERFACE)
add_library(mcubsp::board ALIAS mcubsp_board)
target_link_libraries(mcubsp_board INTERFACE mcubsp::io)
target_compile_options(mcubsp_board INTERFACE ${MCUBSP_ARCH_COMPILE_OPTIONS})
set_target_properties(mcubsp_board PROPERTIES
    MCUBSP_BOARD_NAME "${MCUBSP_BOARD}"
    MCUBSP_BOARD_ARCHITECTURE "cortex-m4")

enable_language(ASM)

set(mcubsp_uno_bsp "${CMAKE_CURRENT_SOURCE_DIR}/bsp")
add_library(mcubsp_uno_platform OBJECT
    "${mcubsp_uno_bsp}/startup.S"
    "${mcubsp_uno_bsp}/early_init.cpp"
    "${mcubsp_uno_bsp}/restricted_runtime.cpp")
target_link_libraries(mcubsp_uno_platform PRIVATE mcubsp::board)

function(mcubsp_uno_firmware target source)
    add_executable(${target} "${source}" $<TARGET_OBJECTS:mcubsp_uno_platform>)
    target_link_libraries(${target} PRIVATE mcubsp::board)
    target_compile_options(${target} PRIVATE
        -Os -ffunction-sections -fdata-sections
        -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit)
    target_link_options(${target} PRIVATE
        ${MCUBSP_ARCH_COMPILE_OPTIONS}
        "-T${mcubsp_uno_bsp}/linker.ld"
        -Wl,--gc-sections -nostartfiles -specs=nano.specs -specs=nosys.specs)
    set_target_properties(${target} PROPERTIES SUFFIX ".elf")
endfunction()

add_subdirectory(example)
add_subdirectory(test)

message(STATUS "Board: ${MCUBSP_BOARD} (cortex-m4)")
