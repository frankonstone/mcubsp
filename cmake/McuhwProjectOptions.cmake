include_guard(GLOBAL)

set(MCUHW_RESTRICTED_COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
    -fno-use-cxa-atexit
)

# @req REQ-0002
function(mcuhw_apply_restricted_runtime target)
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_options(${target} PRIVATE
        ${MCUHW_RESTRICTED_COMPILE_OPTIONS}
        ${MCUHW_ARCH_COMPILE_OPTIONS}
    )
endfunction()

# @req REQ-0003 REQ-0038
function(mcuhw_add_header_self_containment_checks prefix)
    cmake_parse_arguments(ARG "" "ROOT" "" ${ARGN})
    if(NOT ARG_ROOT)
        message(FATAL_ERROR "mcuhw_add_header_self_containment_checks requires ROOT")
    endif()

    file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${ARG_ROOT}/mcuhw/*.hpp")
    foreach(header IN LISTS headers)
        file(RELATIVE_PATH relative_header "${ARG_ROOT}" "${header}")
        string(MAKE_C_IDENTIFIER "${relative_header}" header_id)
        set(generated_source
            "${CMAKE_CURRENT_BINARY_DIR}/header-checks/${header_id}.cpp")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/header-checks")
        file(WRITE "${generated_source}" "#include <${relative_header}>\n")

        add_library(${prefix}_${header_id} OBJECT "${generated_source}")
        target_link_libraries(${prefix}_${header_id} PRIVATE mcuhw::mcuhw)
        mcuhw_apply_restricted_runtime(${prefix}_${header_id})
    endforeach()
endfunction()
