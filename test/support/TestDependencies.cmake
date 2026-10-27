include_guard(GLOBAL)
# Resolve optional test dependencies only inside an explicitly requested build.
# @req REQ-0038 REQ-0049
function(mcubsp_test_dependency dependency)
    if(TARGET ${dependency}::${dependency})
        return()
    endif()
    string(TOUPPER "${dependency}" upper)
    set(MCUBSP_${upper}_SOURCE_DIR "${MCUBSP_ROOT}/../${dependency}" CACHE PATH
        "${dependency} checkout for test builds")
    if(EXISTS "${MCUBSP_${upper}_SOURCE_DIR}/CMakeLists.txt")
        set(${upper}_BUILD_TESTS OFF)
        set(${upper}_INSTALL OFF)
        set(${upper}_FETCH_DEPENDENCIES OFF)
        add_subdirectory("${MCUBSP_${upper}_SOURCE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/${dependency}")
    else()
        find_package(${dependency} CONFIG REQUIRED)
    endif()
endfunction()
