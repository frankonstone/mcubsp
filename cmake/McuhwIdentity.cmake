include_guard(GLOBAL)

# A post-build evidence artifact uses Ninja's actual selected edges and compiler
# dependency records. It never scans unselected implementations.
# @req REQ-0052
function(mcuhw_add_firmware_identity target)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        message(STATUS "${target}: automatic identity report requires Ninja")
        return()
    endif()
    find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
    get_filename_component(root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    set(extras "${MCUHW_BOARD_ENTRY}" ${MCUHW_FIRMWARE_EXTRA_INPUTS})
    set(extra_args "")
    foreach(extra IN LISTS extras)
        if(extra)
            list(APPEND extra_args --extra "${extra}")
            set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${extra}")
        endif()
    endforeach()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${Python3_EXECUTABLE}" "${root}/tool/mcuhwlib/provenance.py"
            --root "${root}" --build "${CMAKE_BINARY_DIR}"
            --target "$<PATH:RELATIVE_PATH,$<TARGET_FILE:${target}>,${CMAKE_BINARY_DIR}>"
            --ninja "${CMAKE_MAKE_PROGRAM}"
            ${extra_args} --output "${CMAKE_CURRENT_BINARY_DIR}/${target}.identity.json"
        VERBATIM)
endfunction()

# Historical hardware-test images embed source IDs in their result protocol.
# Compute those from compiler preprocessing of the selected test/platform sources.
# @req REQ-0052
function(mcuhw_embedded_source_identity output)
    cmake_parse_arguments(ARG "" "" "SOURCES;INCLUDES;FLAGS" ${ARGN})
    find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
    set(args "")
    foreach(source IN LISTS ARG_SOURCES)
        list(APPEND args --source "${source}")
    endforeach()
    foreach(path IN LISTS ARG_INCLUDES)
        list(APPEND args --include "${path}")
    endforeach()
    foreach(flag IN LISTS ARG_FLAGS)
        list(APPEND args "--flag=${flag}")
    endforeach()
    execute_process(COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../tool/mcuhwlib/provenance.py"
        --root "${mcuhw_SOURCE_DIR}" --compiler "${CMAKE_CXX_COMPILER}"
        --output "${CMAKE_CURRENT_BINARY_DIR}/${output}.identity.json" ${args}
        RESULT_VARIABLE result OUTPUT_VARIABLE value ERROR_VARIABLE error
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(result)
        message(FATAL_ERROR "Selected-source identity failed: ${error}")
    endif()
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/${output}.identity.json" json)
    string(JSON count LENGTH "${json}" files)
    math(EXPR last "${count} - 1")
    foreach(index RANGE 0 ${last})
        string(JSON path GET "${json}" files ${index} path)
        string(REPLACE "mcuhw:" "${mcuhw_SOURCE_DIR}/" path "${path}")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${path}")
    endforeach()
    set(${output} "${value}" PARENT_SCOPE)
endfunction()

# The same artifact record is emitted by direct CMake/IDE and frontend builds.
function(mcuhw_firmware_artifact_command output artifact startup)
    cmake_parse_arguments(ARG "" "SDK_BUILD;OUTPUT" "" ${ARGN})
    find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
    set(command "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../tool/mcuhwlib/workflow.py" record
        --build "${CMAKE_CURRENT_BINARY_DIR}" --board "${MCUHW_BOARD}"
        --revision "${MCUHW_BOARD_REVISION}" --startup "${startup}"
        --root "${MCUHW_ROOT}" --artifact "${artifact}")
    if(ARG_OUTPUT)
        list(APPEND command --output "${ARG_OUTPUT}")
    endif()
    if(ARG_SDK_BUILD)
        list(APPEND command --sdk-build "${ARG_SDK_BUILD}")
    endif()
    if(MCUHW_ARTIFACT_POINTER)
        list(APPEND command --pointer "${MCUHW_ARTIFACT_POINTER}")
    endif()
    set(${output} "${command}" PARENT_SCOPE)
endfunction()

function(mcuhw_add_firmware_artifact_report target startup)
    mcuhw_firmware_artifact_command(command "$<TARGET_FILE:${target}>" "${startup}"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}.build.json")
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${command} VERBATIM)
endfunction()
