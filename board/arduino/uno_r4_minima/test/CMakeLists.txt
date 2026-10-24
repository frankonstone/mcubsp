file(GLOB mcubsp_uno_tests CONFIGURE_DEPENDS "*.cpp")
foreach(source IN LISTS mcubsp_uno_tests)
    get_filename_component(name "${source}" NAME_WE)
    set(target "arduino.uno_r4_minima.test.${name}")
    mcubsp_uno_firmware(${target} "${source}")
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
        if(name STREQUAL "ethernet_mcuip")
            set(build_id 0x4d485702)
        else()
            set(build_id 0x4d485701)
        endif()
        mcucov_instrument_target(${target}
            PROFILE FULL
            BUILD_ID ${build_id}
            SOURCE_ROOT "${PROJECT_SOURCE_DIR}"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/mcucov/${name}")
    endif()
endforeach()

# The Ethernet image also uses the MCUIP implementation from the sibling checkout.
set(mcubsp_mcuip_root "${PROJECT_SOURCE_DIR}/../mcuip")
if(NOT EXISTS "${mcubsp_mcuip_root}/include/mcuip/target.hpp")
    message(FATAL_ERROR "The Ethernet board test requires ../mcuip")
endif()
file(GLOB mcubsp_mcuip_sources CONFIGURE_DEPENDS "${mcubsp_mcuip_root}/src/*.cpp")
add_library(mcubsp_uno_mcuip STATIC ${mcubsp_mcuip_sources})
target_include_directories(mcubsp_uno_mcuip PUBLIC "${mcubsp_mcuip_root}/include")
target_link_libraries(mcubsp_uno_mcuip PUBLIC mcubsp::board)
target_compile_options(mcubsp_uno_mcuip PRIVATE
    -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti)
target_link_libraries(arduino.uno_r4_minima.test.ethernet_mcuip PRIVATE mcubsp_uno_mcuip)
