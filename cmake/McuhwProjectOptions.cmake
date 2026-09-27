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
        "$<$<BOOL:${CMAKE_CROSSCOMPILING}>:-ffreestanding>"
    )
endfunction()

# One selectable target, with one independent translation unit per public header.
# @req REQ-0003 REQ-0038 REQ-0047 REQ-0052
function(mcuhw_add_header_self_containment_checks target)
    cmake_parse_arguments(ARG "" "ROOT" "" ${ARGN})
    if(NOT ARG_ROOT)
        message(FATAL_ERROR "mcuhw_add_header_self_containment_checks requires ROOT")
    endif()

    file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${ARG_ROOT}/mcuhw/*.hpp")
    set(sources "")
    foreach(header IN LISTS headers)
        file(RELATIVE_PATH relative_header "${ARG_ROOT}" "${header}")
        string(MAKE_C_IDENTIFIER "${relative_header}" header_id)
        set(generated_source
            "${CMAKE_CURRENT_BINARY_DIR}/header-checks/${header_id}.cpp")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/header-checks")
        file(CONFIGURE OUTPUT "${generated_source}" CONTENT "#include <${relative_header}>\n")
        list(APPEND sources "${generated_source}")
    endforeach()
    add_library(${target} OBJECT ${sources})
    target_link_libraries(${target} PRIVATE mcuhw::mcuhw)
    mcuhw_apply_restricted_runtime(${target})
    # Neither unity builds nor precompiled headers may mask missing includes.
    set_target_properties(${target} PROPERTIES UNITY_BUILD OFF
        DISABLE_PRECOMPILE_HEADERS ON FOLDER "Verification")
endfunction()
