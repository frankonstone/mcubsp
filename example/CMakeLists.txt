# Repository example targets use the same sources as standalone applications.
# @req REQ-0038 REQ-0049
string(REPLACE "/" "." board_target "${MCUBSP_BOARD}")
add_custom_target(examples)
add_custom_target("${board_target}.examples")
add_dependencies(examples "${board_target}.examples")
file(GLOB example_directories LIST_DIRECTORIES TRUE CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(directory IN LISTS example_directories)
    set(source "${directory}/${MCUBSP_BOARD}.cpp")
    if(EXISTS "${source}")
        get_filename_component(name "${directory}" NAME)
        set(target "${board_target}.example.${name}")
        mcubsp_add_firmware("${target}" BOARD "${MCUBSP_BOARD}"
            REVISION "${MCUBSP_BOARD_REVISION}" STARTUP "${MCUBSP_STARTUP}"
            SOURCES "${source}")
        add_dependencies("${board_target}.examples" "${target}")
    endif()
endforeach()
