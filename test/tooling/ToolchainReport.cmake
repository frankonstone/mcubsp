include_guard(GLOBAL)

# @req REQ-0002 REQ-0038
function(mcuhw_write_toolchain_report output_path)
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" ${MCUHW_ARCH_COMPILE_OPTIONS}
                -std=c++23 -dM -E
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/stdlib_probe.cpp"
        RESULT_VARIABLE preprocess_result
        OUTPUT_VARIABLE predefined_macros
        ERROR_VARIABLE preprocess_error
    )

    if(NOT preprocess_result EQUAL 0)
        message(FATAL_ERROR
            "Could not inspect the C++ standard library: ${preprocess_error}")
    endif()

    set(stdlib "unknown")
    if(predefined_macros MATCHES "#define _LIBCPP_VERSION ([0-9]+)")
        set(stdlib "libc++ ${CMAKE_MATCH_1}")
    elseif(predefined_macros MATCHES "#define __GLIBCXX__ ([0-9]+)")
        set(stdlib "libstdc++ ${CMAKE_MATCH_1}")
        if(predefined_macros MATCHES "#define _GLIBCXX_RELEASE ([0-9]+)")
            string(APPEND stdlib " (release ${CMAKE_MATCH_1})")
        endif()
    endif()

    string(JOIN " " restricted_flags ${MCUHW_RESTRICTED_COMPILE_OPTIONS})
    string(JOIN " " architecture_flags ${MCUHW_ARCH_COMPILE_OPTIONS})
    file(WRITE "${output_path}"
        "mcuhw toolchain report\n"
        "architecture: ${MCUHW_ARCH}\n"
        "system: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}\n"
        "generator: ${CMAKE_GENERATOR}\n"
        "cmake: ${CMAKE_VERSION}\n"
        "compiler: ${CMAKE_CXX_COMPILER}\n"
        "compiler-id: ${CMAKE_CXX_COMPILER_ID}\n"
        "compiler-version: ${CMAKE_CXX_COMPILER_VERSION}\n"
        "compiler-target: ${CMAKE_CXX_COMPILER_TARGET}\n"
        "language-standard: C++23\n"
        "standard-library: ${stdlib}\n"
        "restricted-flags: ${restricted_flags}\n"
        "architecture-flags: ${architecture_flags}\n"
    )
    message(STATUS "Wrote toolchain identity to ${output_path}")
endfunction()
