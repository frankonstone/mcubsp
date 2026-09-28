# Positive/negative compiler contracts and the all-description object gate.
function(mcuhw_add_compile_case name source expect_success)
    add_test(
        NAME "compile.${name}"
        COMMAND "${CMAKE_COMMAND}"
            -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            "-DCOMPILE_OPTIONS=${MCUHW_RESTRICTED_COMPILE_OPTIONS}"
            -DINCLUDE_DIR=${mcuhw_SOURCE_DIR}/include
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${source}
            -DEXPECT_SUCCESS=${expect_success}
            -P "${mcuhw_SOURCE_DIR}/cmake/CompileCase.cmake"
    )
    set_tests_properties("compile.${name}" PROPERTIES LABELS "compile")
endfunction()

mcuhw_add_compile_case(cxx23 cxx23_features.cpp TRUE)
mcuhw_add_compile_case(io_example io_example.cpp TRUE)
mcuhw_add_compile_case(io_devices_example io_devices_example.cpp TRUE)
mcuhw_add_compile_case(io_disabled_peripheral io_disabled_peripheral_forbidden.cpp FALSE)
mcuhw_add_compile_case(io_unsupported_dma io_unsupported_dma_forbidden.cpp FALSE)
mcuhw_add_compile_case(io_move_runtime io_move_runtime_forbidden.cpp FALSE)
mcuhw_add_compile_case(exceptions_rejected exceptions_forbidden.cpp FALSE)
mcuhw_add_compile_case(rtti_rejected rtti_forbidden.cpp FALSE)
mcuhw_add_compile_case(command_read_rejected command_read_forbidden.cpp FALSE)
mcuhw_add_compile_case(unsafe_rmw_rejected unsafe_rmw_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_field_rejected invalid_field_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_enum_rejected invalid_enum_forbidden.cpp FALSE)
mcuhw_add_compile_case(duplicate_pin_rejected duplicate_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(unroutable_pin_rejected unroutable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(unsupported_capability_rejected unsupported_capability_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_clock_rejected invalid_clock_forbidden.cpp FALSE)
mcuhw_add_compile_case(interrupt_conflict_rejected interrupt_conflict_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_dma_rejected invalid_dma_forbidden.cpp FALSE)
mcuhw_add_compile_case(reserved_write_rejected reserved_write_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_unavailable_pin_rejected ra4m1_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_third_gpt_output_rejected ra4m1_third_gpt_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_invalid_gpt_output_rejected ra4m1_invalid_gpt_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(stm32h753_unavailable_pin_rejected stm32h753_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(stm32h753_advanced_output_rejected stm32h753_advanced_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(esp32p4_unavailable_pin_rejected esp32p4_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(esp32p4_i3c_capability_rejected esp32p4_i3c_capability_forbidden.cpp FALSE)

# The all-target gate exists now but is only built after all three real,
# source-backed description headers exist. No placeholder MCU types are used.
# @req REQ-0040
set(description_headers
    "${mcuhw_SOURCE_DIR}/include/mcuhw/mcu/ra4m1/description.hpp"
    "${mcuhw_SOURCE_DIR}/include/mcuhw/mcu/stm32h753/description.hpp"
    "${mcuhw_SOURCE_DIR}/include/mcuhw/mcu/esp32p4/description.hpp"
)
set(all_descriptions_available TRUE)
foreach(header IN LISTS description_headers)
    if(NOT EXISTS "${header}")
        set(all_descriptions_available FALSE)
    endif()
endforeach()
if(all_descriptions_available)
    add_library(mcuhw_all_descriptions OBJECT description_all.cpp)
    add_dependencies(native-tests mcuhw_all_descriptions)
    set_target_properties(mcuhw_all_descriptions PROPERTIES FOLDER "Tests/compile")
    target_link_libraries(mcuhw_all_descriptions PRIVATE mcuhw::mcuhw)
    mcuhw_apply_restricted_runtime(mcuhw_all_descriptions)
else()
    message(STATUS
        "All-three description gate deferred until the three real description headers exist")
endif()
