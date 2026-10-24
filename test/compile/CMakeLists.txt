# Host compiler contracts and cross-compiler build checks.
set(mcubsp_compile_source_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(mcubsp_compile_options
    -Wall -Wextra -Wpedantic -Werror
    -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    message(FATAL_ERROR "Compile tests require a GCC- or Clang-compatible C++23 compiler")
endif()

function(mcubsp_add_compile_case name source expect_success)
    add_test(
        NAME "mcubsp.test.compile.${name}"
        COMMAND "${CMAKE_COMMAND}"
            "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DCOMPILE_OPTIONS=${mcubsp_compile_options}"
            "-DINCLUDE_DIR=${PROJECT_SOURCE_DIR}/include"
            "-DSOURCE=${mcubsp_compile_source_dir}/${source}"
            "-DEXPECT_SUCCESS=${expect_success}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/CompileCase.cmake"
    )
    set_tests_properties("mcubsp.test.compile.${name}" PROPERTIES LABELS "compile")
endfunction()

mcubsp_add_compile_case(cxx23 cxx23_features.cpp TRUE)
mcubsp_add_compile_case(io_example io_example.cpp TRUE)
mcubsp_add_compile_case(io_devices_example io_devices_example.cpp TRUE)
mcubsp_add_compile_case(io_disabled_peripheral io_disabled_peripheral_forbidden.cpp FALSE)
mcubsp_add_compile_case(io_unsupported_dma io_unsupported_dma_forbidden.cpp FALSE)
mcubsp_add_compile_case(io_move_runtime io_move_runtime_forbidden.cpp FALSE)
mcubsp_add_compile_case(exceptions_rejected exceptions_forbidden.cpp FALSE)
mcubsp_add_compile_case(rtti_rejected rtti_forbidden.cpp FALSE)
mcubsp_add_compile_case(command_read_rejected command_read_forbidden.cpp FALSE)
mcubsp_add_compile_case(unsafe_rmw_rejected unsafe_rmw_forbidden.cpp FALSE)
mcubsp_add_compile_case(invalid_field_rejected invalid_field_forbidden.cpp FALSE)
mcubsp_add_compile_case(invalid_enum_rejected invalid_enum_forbidden.cpp FALSE)
mcubsp_add_compile_case(duplicate_pin_rejected duplicate_pin_forbidden.cpp FALSE)
mcubsp_add_compile_case(unroutable_pin_rejected unroutable_pin_forbidden.cpp FALSE)
mcubsp_add_compile_case(unsupported_capability_rejected unsupported_capability_forbidden.cpp FALSE)
mcubsp_add_compile_case(invalid_clock_rejected invalid_clock_forbidden.cpp FALSE)
mcubsp_add_compile_case(interrupt_conflict_rejected interrupt_conflict_forbidden.cpp FALSE)
mcubsp_add_compile_case(invalid_dma_rejected invalid_dma_forbidden.cpp FALSE)
mcubsp_add_compile_case(reserved_write_rejected reserved_write_forbidden.cpp FALSE)
mcubsp_add_compile_case(ra4m1_unavailable_pin_rejected ra4m1/unavailable_pin_forbidden.cpp FALSE)
mcubsp_add_compile_case(ra4m1_third_gpt_output_rejected ra4m1/third_gpt_output_forbidden.cpp FALSE)
mcubsp_add_compile_case(ra4m1_invalid_gpt_output_rejected ra4m1/invalid_gpt_output_forbidden.cpp FALSE)
mcubsp_add_compile_case(stm32h753_unavailable_pin_rejected stm32h753/unavailable_pin_forbidden.cpp FALSE)
mcubsp_add_compile_case(stm32h753_advanced_output_rejected stm32h753/advanced_output_forbidden.cpp FALSE)
mcubsp_add_compile_case(esp32p4_unavailable_pin_rejected esp32p4/unavailable_pin_forbidden.cpp FALSE)
mcubsp_add_compile_case(esp32p4_i3c_capability_rejected esp32p4/i3c_capability_forbidden.cpp FALSE)

# Positive translation units are compiled as objects, so they require neither
# startup code nor a linkable test executable.
add_library(mcubsp_compile_probes OBJECT
    "${mcubsp_compile_source_dir}/all_descriptions_probe.cpp"
    "${mcubsp_compile_source_dir}/arch_probe.cpp"
    "${mcubsp_compile_source_dir}/barrier_probe.cpp"
    "${mcubsp_compile_source_dir}/cxx23_features.cpp"
    "${mcubsp_compile_source_dir}/description_all.cpp"
    "${mcubsp_compile_source_dir}/dmac_example.cpp"
    "${mcubsp_compile_source_dir}/esp32p4/metadata_probe.cpp"
    "${mcubsp_compile_source_dir}/execution_probe.cpp"
    "${mcubsp_compile_source_dir}/io_devices_example.cpp"
    "${mcubsp_compile_source_dir}/io_example.cpp"
    "${mcubsp_compile_source_dir}/spi_adc_example.cpp"
    "${mcubsp_compile_source_dir}/stm32h753/metadata_probe.cpp"
    "${mcubsp_compile_source_dir}/uart_i2c_example.cpp")
target_link_libraries(mcubsp_compile_probes PRIVATE mcubsp::mcubsp)
target_compile_features(mcubsp_compile_probes PRIVATE cxx_std_23)
target_compile_options(mcubsp_compile_probes PRIVATE ${mcubsp_compile_options})
target_compile_definitions(mcubsp_compile_probes PRIVATE MCUBSP_ARCH_NAME="native")

set(mcubsp_compile_targets mcubsp_compile_probes)
foreach(backend IN ITEMS I2C MEMORY SPI TIMER USART ETHERNET)
    string(TOLOWER "${backend}" backend_name)
    set(target "mcubsp_compile_stm32h753_${backend_name}")
    add_library(${target} OBJECT
        "${mcubsp_compile_source_dir}/stm32h753/backend_coverage_probe.cpp")
    target_link_libraries(${target} PRIVATE mcubsp::mcubsp)
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_options(${target} PRIVATE ${mcubsp_compile_options})
    target_compile_definitions(${target} PRIVATE MCUBSP_COVERAGE_PROBE_${backend}=1)
    list(APPEND mcubsp_compile_targets ${target})
endforeach()

option(MCUBSP_BUILD_MCUIP_CONTRACTS "Compile MCUIP integration contracts" ON)
if(MCUBSP_BUILD_MCUIP_CONTRACTS)
    set(MCUBSP_MCUIP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mcuip" CACHE PATH
        "MCUIP source checkout")
    if(NOT EXISTS "${MCUBSP_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
        message(FATAL_ERROR "MCUIP contracts require MCUBSP_MCUIP_SOURCE_DIR")
    endif()
    target_sources(mcubsp_compile_probes PRIVATE
        "${mcubsp_compile_source_dir}/esp32p4/mcuip_contract.cpp"
        "${mcubsp_compile_source_dir}/stm32h753/mcuip_contract.cpp"
        "${mcubsp_compile_source_dir}/uno_r4_minima/w5100_mcuip_contract.cpp")
    target_include_directories(mcubsp_compile_probes PRIVATE
        "${MCUBSP_MCUIP_SOURCE_DIR}/include")
endif()

add_custom_target(compile-probes DEPENDS ${mcubsp_compile_targets})

add_custom_target(check-compile
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
        -L compile --output-on-failure
    DEPENDS compile-probes
    USES_TERMINAL)

# Cross-compile the target probes from the host build. No image is executed.
add_custom_target(target-compile-probes)

function(mcubsp_add_target_probe name toolchain)
    set(probe_build "${CMAKE_CURRENT_BINARY_DIR}/target-probes/${name}")
    add_custom_target("target-probes-${name}"
        COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_SOURCE_DIR}/target"
            -B "${probe_build}" -G Ninja
            "-DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/${toolchain}"
            "-DMCUBSP_MCUIP_SOURCE_DIR=${MCUBSP_MCUIP_SOURCE_DIR}"
        COMMAND "${CMAKE_COMMAND}" --build "${probe_build}"
            --target check-compile --parallel 2
        USES_TERMINAL VERBATIM)
    add_dependencies(target-compile-probes "target-probes-${name}")
endfunction()

mcubsp_add_target_probe(cortex-m4 toolchain-arm-cortex-m4.cmake)
mcubsp_add_target_probe(cortex-m7 toolchain-arm-cortex-m7.cmake)
mcubsp_add_target_probe(esp32p4-riscv toolchain-esp32p4.cmake)
