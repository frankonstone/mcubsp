include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/McuhwSelection.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/McuhwIdentity.cmake")

# Board entries own BSP-specific sources, startup policy and profile selection;
# architecture definitions own compiler/ABI options.
macro(mcuhw_load_board)
    mcuhw_resolve_board()
    unset(MCUHW_BOARD_SDK)
    unset(MCUHW_BOARD_IO_EXAMPLES)
    unset(MCUHW_BOARD_STARTUP)
    unset(MCUHW_BOARD_LINKER)
    include("${MCUHW_BOARD_ENTRY}")
    # Read only architecture facts, never a toolchain file after project(). An
    # existing BSP retains its own compiler, system and try-compile settings.
    if(NOT MCUHW_BOARD_ARCHITECTURE MATCHES "^[a-z0-9][a-z0-9_-]*$" OR
            NOT EXISTS "${MCUHW_ROOT}/cmake/architectures/${MCUHW_BOARD_ARCHITECTURE}.cmake")
        message(FATAL_ERROR "Missing architecture definition for ${MCUHW_BOARD_ARCHITECTURE}.")
    endif()
    include("${MCUHW_ROOT}/cmake/architectures/${MCUHW_BOARD_ARCHITECTURE}.cmake")
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -dumpmachine
        OUTPUT_VARIABLE _mcuhw_compiler_target OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _mcuhw_compiler_result)
    if(_mcuhw_compiler_result OR NOT MCUHW_COMPILER_PATTERN OR
            NOT _mcuhw_compiler_target MATCHES "${MCUHW_COMPILER_PATTERN}")
        message(FATAL_ERROR "Incompatible compiler '${CMAKE_CXX_COMPILER}' (${_mcuhw_compiler_target}) for ${MCUHW_BOARD}. Configure a fresh directory with -DCMAKE_TOOLCHAIN_FILE=${MCUHW_ROOT}/cmake/McuhwToolchain.cmake before project().")
    endif()
    if(NOT DEFINED MCUHW_ARCH OR MCUHW_ARCH STREQUAL "")
        set(MCUHW_ARCH "${MCUHW_BOARD_ARCHITECTURE}" CACHE STRING "Architecture probed by this build" FORCE)
    elseif(NOT MCUHW_BOARD_ARCHITECTURE STREQUAL MCUHW_ARCH)
        message(FATAL_ERROR "Board ${MCUHW_BOARD} requires architecture '${MCUHW_BOARD_ARCHITECTURE}', but the configured toolchain uses '${MCUHW_ARCH}'.")
    endif()
endmacro()

# Common code-generation policy for application, BSP and instrumented test TUs.
# HOSTED only exposes the hosted standard-library declarations needed by test
# reporters; exceptions, RTTI and dynamic local-static guards remain disabled.
function(mcuhw_apply_firmware_runtime target)
    cmake_parse_arguments(ARG "HOSTED;NO_OPTIMIZE" "" "" ${ARGN})
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_options(${target} PRIVATE ${MCUHW_ARCH_COMPILE_OPTIONS}
        -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables
        "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti;-fno-threadsafe-statics;-fno-use-cxa-atexit>")
    if(NOT ARG_HOSTED)
        target_compile_options(${target} PRIVATE -ffreestanding)
    endif()
    if(NOT ARG_NO_OPTIMIZE)
        target_compile_options(${target} PRIVATE -Os)
    endif()
endfunction()

# This macro deliberately enables ASM in the caller's directory scope. CMake's
# language initialization cannot be confined to a function's local variables.
macro(mcuhw_add_board_platform target)
    if(NOT CMAKE_ASM_COMPILER_LOADED)
        enable_language(ASM)
    endif()
    _mcuhw_add_board_platform(${target} ${ARGN})
endmacro()

function(_mcuhw_add_board_platform target)
    cmake_parse_arguments(ARG "NO_OPTIMIZE" "" "SOURCES;EXCLUDE_SOURCES" ${ARGN})
    mcuhw_load_board()
    if(NOT ARG_SOURCES)
        set(ARG_SOURCES ${MCUHW_BOARD_STARTUP})
    endif()
    list(REMOVE_ITEM ARG_SOURCES ${ARG_EXCLUDE_SOURCES})
    add_library(${target} OBJECT ${ARG_SOURCES})
    target_link_libraries(${target} PRIVATE mcuhw::mcuhw)
    set(options "")
    if(ARG_NO_OPTIMIZE)
        list(APPEND options NO_OPTIMIZE)
    endif()
    mcuhw_apply_firmware_runtime(${target} ${options})
endfunction()

# Shared link/image recipe. Tests may provide instrumented BSP object targets;
# ordinary applications receive the exact same board-owned startup sources.
function(mcuhw_configure_board_firmware target)
    cmake_parse_arguments(ARG "HOSTED;NO_OPTIMIZE;HEX" "STARTUP;LINKER_SCRIPT" "PLATFORM_TARGETS" ${ARGN})
    mcuhw_load_board()
    set(options "")
    foreach(option IN ITEMS HOSTED NO_OPTIMIZE)
        if(ARG_${option})
            list(APPEND options ${option})
        endif()
    endforeach()
    mcuhw_apply_firmware_runtime(${target} ${options})
    target_link_libraries(${target} PRIVATE mcuhw::io mcuhw::device ${ARG_PLATFORM_TARGETS})
    target_link_options(${target} PRIVATE ${MCUHW_ARCH_COMPILE_OPTIONS})
    if(ARG_STARTUP STREQUAL "supplied")
        if(NOT ARG_PLATFORM_TARGETS)
            target_sources(${target} PRIVATE ${MCUHW_BOARD_STARTUP})
        endif()
        set(ARG_LINKER_SCRIPT "${MCUHW_BOARD_LINKER}")
        target_link_options(${target} PRIVATE -nostartfiles --specs=nosys.specs)
    endif()
    if(ARG_LINKER_SCRIPT)
        target_link_options(${target} PRIVATE "-T${ARG_LINKER_SCRIPT}")
        set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${ARG_LINKER_SCRIPT}")
    endif()
    target_link_options(${target} PRIVATE -Wl,--gc-sections -Wl,--print-memory-usage
        "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${target}.map")
    set_target_properties(${target} PROPERTIES SUFFIX ".elf")
    if(CMAKE_OBJCOPY)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:${target}>"
                "${CMAKE_CURRENT_BINARY_DIR}/${target}.bin" VERBATIM)
        if(ARG_HEX)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_OBJCOPY}" -O ihex "$<TARGET_FILE:${target}>"
                    "${CMAKE_CURRENT_BINARY_DIR}/${target}.hex" VERBATIM)
        endif()
    endif()
    mcuhw_add_firmware_identity(${target})
endfunction()

# @req REQ-0049
macro(mcuhw_add_firmware target)
    cmake_parse_arguments(_MCUHW_FIRMWARE "" "STARTUP" "" ${ARGN})
    if(_MCUHW_FIRMWARE_STARTUP STREQUAL "supplied" AND NOT CMAKE_ASM_COMPILER_LOADED)
        enable_language(ASM)
    endif()
    _mcuhw_add_firmware(${target} ${ARGN})
    unset(_MCUHW_FIRMWARE_STARTUP)
    unset(_MCUHW_FIRMWARE_UNPARSED_ARGUMENTS)
endmacro()

function(_mcuhw_add_firmware target)
    cmake_parse_arguments(ARG "" "STARTUP;LINKER_SCRIPT" "SOURCES;LIBRARIES" ${ARGN})
    if(ARG_UNPARSED_ARGUMENTS OR NOT ARG_SOURCES OR NOT ARG_STARTUP MATCHES "^(supplied|external)$")
        message(FATAL_ERROR "mcuhw_add_firmware(${target}) requires SOURCES and explicit STARTUP supplied|external; optional LINKER_SCRIPT and LIBRARIES.")
    endif()
    if(ARG_STARTUP STREQUAL "supplied" AND ARG_LINKER_SCRIPT)
        message(FATAL_ERROR "Supplied startup owns the board memory layout; LINKER_SCRIPT requires STARTUP external with your BSP startup sources.")
    endif()
    mcuhw_load_board()
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/mcuhw/${target}/selected_board.hpp")
    if(MCUHW_BOARD_SDK STREQUAL "esp-idf" AND ARG_STARTUP STREQUAL "supplied")
        if(ARG_LIBRARIES OR ARG_LINKER_SCRIPT)
            message(FATAL_ERROR "ESP-IDF supplied startup owns its SDK link. Use an external BSP project for custom LIBRARIES or LINKER_SCRIPT.")
        endif()
        include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/McuhwEspIdf.cmake")
        mcuhw_add_idf_firmware(${target} ${ARG_SOURCES})
        return()
    endif()
    add_executable(${target} ${ARG_SOURCES})
    set_target_properties(${target} PROPERTIES FOLDER "Application")
    target_link_libraries(${target} PRIVATE ${ARG_LIBRARIES})
    if(ARG_STARTUP STREQUAL "external")
        target_compile_definitions(${target} PRIVATE MCUHW_EXTERNAL_STARTUP=1)
    endif()
    mcuhw_configure_board_firmware(${target} STARTUP "${ARG_STARTUP}"
        LINKER_SCRIPT "${ARG_LINKER_SCRIPT}")
    mcuhw_add_firmware_artifact_report(${target} "${ARG_STARTUP}")
endfunction()
