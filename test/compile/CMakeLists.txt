# Host compiler contracts and cross-compiler build checks.
set(mcubsp_compile_source_dir "${PROJECT_SOURCE_DIR}")
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
            "-DEXPECT_DIAGNOSTIC=${ARGN}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/CompileCase.cmake"
    )
    set_tests_properties("mcubsp.test.compile.${name}" PROPERTIES LABELS "compile")
endfunction()

mcubsp_add_compile_case(cxx23 test/compile/cxx23_features.cpp TRUE)
mcubsp_add_compile_case(io_example src/io/test/io_example.cpp TRUE)
mcubsp_add_compile_case(io_devices_example src/io/test/io_devices_example.cpp TRUE)
mcubsp_add_compile_case(io_disabled_peripheral src/io/test/io_disabled_peripheral_forbidden.cpp FALSE "deleted")
mcubsp_add_compile_case(io_unsupported_dma src/io/test/io_unsupported_dma_forbidden.cpp FALSE "selected DMA channels")
mcubsp_add_compile_case(io_move_runtime src/io/test/io_move_runtime_forbidden.cpp FALSE "deleted")
mcubsp_add_compile_case(nrf52840_serial_unavailable src/backend/nrf52840/test/serial_unavailable_forbidden.cpp FALSE "no supported serial backend")
mcubsp_add_compile_case(nrf52840_i2c_duplicate_pin src/backend/nrf52840/test/i2c_duplicate_pin_forbidden.cpp FALSE "distinct pins")
mcubsp_add_compile_case(nrf52840_i2c_reserved_pin src/backend/nrf52840/test/i2c_reserved_pin_forbidden.cpp FALSE "available GPIOs")
mcubsp_add_compile_case(nrf52840_i2c_invalid_instance src/backend/nrf52840/test/i2c_invalid_instance_forbidden.cpp FALSE "instance must be 0 or 1")
mcubsp_add_compile_case(nrf52840_timer_invalid_instance src/backend/nrf52840/test/timer_invalid_instance_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(nrf52840_rtc_invalid_instance src/backend/nrf52840/test/rtc_invalid_instance_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(nrf52840_gpiote_invalid_instance src/backend/nrf52840/test/gpiote_invalid_instance_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(nrf52840_fixed_ppi_invalid_instance src/backend/nrf52840/test/fixed_ppi_invalid_instance_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(side_effect_read_rejected src/core/test/side_effect_read_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied|no matching member function")
mcubsp_add_compile_case(exceptions_rejected test/compile/exceptions_forbidden.cpp FALSE "exception handling|exceptions disabled")
mcubsp_add_compile_case(rtti_rejected test/compile/rtti_forbidden.cpp FALSE "not permitted.*rtti|requires.*rtti|not permitted.*RTTI|requires.*RTTI|RTTI.*disabled")
mcubsp_add_compile_case(command_read_rejected src/core/test/command_read_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied|no matching member function")
mcubsp_add_compile_case(unsafe_rmw_rejected src/core/test/unsafe_rmw_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied|no matching member function")
mcubsp_add_compile_case(invalid_field_rejected src/core/test/invalid_field_forbidden.cpp FALSE "field extends past")
mcubsp_add_compile_case(invalid_enum_rejected src/core/test/invalid_enum_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(duplicate_pin_rejected src/core/test/duplicate_pin_forbidden.cpp FALSE "exclusive pin assigned")
mcubsp_add_compile_case(unroutable_pin_rejected src/core/test/unroutable_pin_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(unsupported_capability_rejected src/core/test/unsupported_capability_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(invalid_clock_rejected src/core/test/invalid_clock_forbidden.cpp FALSE "fractional divisor")
mcubsp_add_compile_case(interrupt_conflict_rejected src/core/test/interrupt_conflict_forbidden.cpp FALSE "exclusive interrupt-line conflict")
mcubsp_add_compile_case(invalid_dma_rejected src/core/test/invalid_dma_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(reserved_write_rejected src/core/test/reserved_write_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied|no matching member function")
mcubsp_add_compile_case(ra4m1_unavailable_pin_rejected src/mcu/ra4m1/test/unavailable_pin_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(ra4m1_third_gpt_output_rejected src/mcu/ra4m1/test/third_gpt_output_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(ra4m1_invalid_gpt_output_rejected src/backend/ra4m1/test/invalid_gpt_output_forbidden.cpp FALSE "GPT output must be A or B")
mcubsp_add_compile_case(stm32h753_unavailable_pin_rejected src/mcu/stm32h753/test/unavailable_pin_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(stm32h753_advanced_output_rejected src/mcu/stm32h753/test/advanced_output_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(esp32p4_unavailable_pin_rejected src/mcu/esp32p4/test/unavailable_pin_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")
mcubsp_add_compile_case(esp32p4_i3c_capability_rejected src/mcu/esp32p4/test/i3c_capability_forbidden.cpp FALSE "constraints not satisfied|constraints were not satisfied")

# Positive translation units are compiled as objects, so they require neither
# startup code nor a linkable test executable.
add_library(mcubsp_compile_probes OBJECT
    "${mcubsp_compile_source_dir}/src/backend/nrf52840/test/system_probe.cpp"
    "${mcubsp_compile_source_dir}/src/mcu/test/all_descriptions_probe.cpp"
    "${mcubsp_compile_source_dir}/src/io/test/composition_example.cpp"
    "${mcubsp_compile_source_dir}/src/testing/test/arch_probe.cpp"
    "${mcubsp_compile_source_dir}/src/arch/test/barrier_probe.cpp"
    "${mcubsp_compile_source_dir}/test/compile/cxx23_features.cpp"
    "${mcubsp_compile_source_dir}/src/mcu/test/description_all.cpp"
    "${mcubsp_compile_source_dir}/src/backend/ra4m1/test/dmac_example.cpp"
    "${mcubsp_compile_source_dir}/src/mcu/esp32p4/test/metadata_probe.cpp"
    "${mcubsp_compile_source_dir}/src/execution/test/execution_probe.cpp"
    "${mcubsp_compile_source_dir}/src/io/test/io_devices_example.cpp"
    "${mcubsp_compile_source_dir}/src/io/test/io_example.cpp"
    "${mcubsp_compile_source_dir}/src/backend/ra4m1/test/spi_adc_example.cpp"
    "${mcubsp_compile_source_dir}/src/mcu/stm32h753/test/metadata_probe.cpp"
    "${mcubsp_compile_source_dir}/src/mcu/nrf52840/test/metadata_probe.cpp"
    "${mcubsp_compile_source_dir}/src/backend/ra4m1/test/uart_i2c_example.cpp")
target_link_libraries(mcubsp_compile_probes PRIVATE mcubsp::mcubsp)
target_compile_features(mcubsp_compile_probes PRIVATE cxx_std_23)
target_compile_options(mcubsp_compile_probes PRIVATE ${mcubsp_compile_options})
target_compile_definitions(mcubsp_compile_probes PRIVATE MCUBSP_ARCH_NAME="native")

set(mcubsp_compile_targets mcubsp_compile_probes)
foreach(backend IN ITEMS I2C MEMORY SPI TIMER USART ETHERNET)
    string(TOLOWER "${backend}" backend_name)
    set(target "mcubsp_compile_stm32h753_${backend_name}")
    add_library(${target} OBJECT
        "${mcubsp_compile_source_dir}/src/backend/stm32h753/test/backend_coverage_probe.cpp")
    target_link_libraries(${target} PRIVATE mcubsp::mcubsp)
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_options(${target} PRIVATE ${mcubsp_compile_options})
    target_compile_definitions(${target} PRIVATE MCUBSP_COVERAGE_PROBE_${backend}=1)
    list(APPEND mcubsp_compile_targets ${target})
endforeach()

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
            "-DMCUBSP_CHECK_HEADERS=${MCUBSP_CHECK_HEADERS}"
        COMMAND "${CMAKE_COMMAND}" --build "${probe_build}"
            --target check-compile --parallel 2
        USES_TERMINAL VERBATIM)
    add_dependencies(target-compile-probes "target-probes-${name}")
endfunction()

mcubsp_add_target_probe(cortex-m4 toolchain-arm-cortex-m4.cmake)
mcubsp_add_target_probe(cortex-m7 toolchain-arm-cortex-m7.cmake)
mcubsp_add_target_probe(esp32p4-riscv toolchain-esp32p4.cmake)
