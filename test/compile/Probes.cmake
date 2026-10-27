# Target-toolchain compile probes. # @req REQ-0047 REQ-0052
# @req REQ-0002 REQ-0015 REQ-0016 REQ-0017 REQ-0018 REQ-0019 REQ-0035 REQ-0038 REQ-0040 REQ-0041
add_library(mcubsp_arch_probe OBJECT
    "${mcubsp_SOURCE_DIR}/src/mcu/test/all_descriptions_probe.cpp"
    "${mcubsp_SOURCE_DIR}/src/io/test/composition_example.cpp"
    "${mcubsp_SOURCE_DIR}/src/testing/test/arch_probe.cpp"
    "${mcubsp_SOURCE_DIR}/test/compile/cxx23_features.cpp"
    "${mcubsp_SOURCE_DIR}/src/execution/test/execution_probe.cpp"
    "${mcubsp_SOURCE_DIR}/src/io/test/io_example.cpp"
    "${mcubsp_SOURCE_DIR}/src/io/test/io_devices_example.cpp"
    "${mcubsp_SOURCE_DIR}/src/backend/ra4m1/test/uart_i2c_example.cpp"
    "${mcubsp_SOURCE_DIR}/src/backend/ra4m1/test/spi_adc_example.cpp"
    "${mcubsp_SOURCE_DIR}/src/backend/ra4m1/test/dmac_example.cpp"
    "${mcubsp_SOURCE_DIR}/src/mcu/esp32p4/test/metadata_probe.cpp"
    "${mcubsp_SOURCE_DIR}/src/mcu/stm32h753/test/metadata_probe.cpp"
)
target_link_libraries(mcubsp_arch_probe PRIVATE mcubsp::mcubsp)
target_compile_definitions(mcubsp_arch_probe PRIVATE
    MCUBSP_ARCH_NAME="${MCUBSP_ARCH}"
)
mcubsp_apply_restricted_runtime(mcubsp_arch_probe)
set(mcubsp_probe_targets mcubsp_arch_probe)

# @req REQ-0009 REQ-0038
add_library(mcubsp_barrier_probe OBJECT "${mcubsp_SOURCE_DIR}/src/arch/test/barrier_probe.cpp")
target_link_libraries(mcubsp_barrier_probe PRIVATE mcubsp::mcubsp)
mcubsp_apply_restricted_runtime(mcubsp_barrier_probe)
add_custom_target(mcubsp_verify_barriers
    COMMAND "${CMAKE_COMMAND}"
        -DOBJDUMP=${CMAKE_OBJDUMP}
        -DOBJECT_FILE=$<TARGET_OBJECTS:mcubsp_barrier_probe>
        -DARCH=${MCUBSP_ARCH}
        -DREPORT=${CMAKE_CURRENT_BINARY_DIR}/barrier-report.txt
        -P "${mcubsp_SOURCE_DIR}/src/arch/test/VerifyBarrierInstructions.cmake"
    DEPENDS mcubsp_barrier_probe
    VERBATIM)
list(APPEND mcubsp_probe_targets mcubsp_barrier_probe mcubsp_verify_barriers)

add_custom_target(check-compile DEPENDS ${mcubsp_probe_targets})
set_target_properties(check-compile PROPERTIES FOLDER "Verification")
foreach(target IN LISTS mcubsp_probe_targets)
    set_target_properties(${target} PROPERTIES FOLDER "Verification/Compile")
endforeach()
