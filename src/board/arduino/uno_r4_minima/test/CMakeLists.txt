file(GLOB mcubsp_uno_tests CONFIGURE_DEPENDS "*.cpp")
foreach(source IN LISTS mcubsp_uno_tests)
    get_filename_component(name "${source}" NAME_WE)
    set(target "arduino.uno_r4_minima.test.${name}")
    mcubsp_uno_firmware(${target} "${source}" HOSTED)
    target_link_libraries(${target} PRIVATE
        mcutest::mcutest mcujson::mcujson mcucov::runtime)
    target_compile_definitions(${target} PRIVATE
        MCUTEST_MAX_TESTS=8 MCUTEST_SOURCE_LOCATION=1
        MCUBSP_COVERAGE=$<BOOL:${MCUBSP_COVERAGE}>)
    if(MCUBSP_COVERAGE)
        target_compile_definitions(${target} PRIVATE
            MCUCOV_BYTE_PER_PROBE=1 MCUCOV_COMPACT_RUNTIME=1)
        target_compile_options(${target} PRIVATE
            -include "${PROJECT_SOURCE_DIR}/test/support/mcucov_compat.hpp")
        set(build_id 0x4d485701)
        mcucov_instrument_target(${target}
            PROFILE FULL
            BUILD_ID ${build_id}
            SOURCE_ROOT "${PROJECT_SOURCE_DIR}"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${name}")
    endif()
endforeach()

