# Target-toolchain compile probes. # @req REQ-0047 REQ-0052
# @req REQ-0002 REQ-0015 REQ-0016 REQ-0017 REQ-0018 REQ-0019 REQ-0035 REQ-0038 REQ-0040 REQ-0041
add_library(mcubsp_arch_probe OBJECT
    "${CMAKE_CURRENT_LIST_DIR}/all_descriptions_probe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/arch_probe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cxx23_features.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/execution_probe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/io_example.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/io_devices_example.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/uart_i2c_example.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/spi_adc_example.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dmac_example.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/esp32p4/metadata_probe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/stm32h753/metadata_probe.cpp"
)
target_link_libraries(mcubsp_arch_probe PRIVATE mcubsp::mcubsp)
target_compile_definitions(mcubsp_arch_probe PRIVATE
    MCUBSP_ARCH_NAME="${MCUBSP_ARCH}"
)
mcubsp_apply_restricted_runtime(mcubsp_arch_probe)
set(mcubsp_probe_targets mcubsp_arch_probe)

# Optional by installation, mandatory for the sibling-development contract:
# these TUs prove that both fixed-ring boundaries satisfy the current mcuip
# replacement for lwIP on every configured architecture.
# @req REQ-0043 REQ-0044
if(MCUBSP_BUILD_MCUIP_CONTRACTS)
    if(NOT EXISTS "${MCUBSP_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
        message(FATAL_ERROR "MCUIP contract checks requested; set MCUBSP_MCUIP_SOURCE_DIR")
    endif()
    add_library(mcubsp_mcuip_contract_probe OBJECT
        "${CMAKE_CURRENT_LIST_DIR}/stm32h753/mcuip_contract.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/esp32p4/mcuip_contract.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/uno_r4_minima/w5100_mcuip_contract.cpp")
    target_link_libraries(mcubsp_mcuip_contract_probe PRIVATE mcubsp::mcubsp)
    target_include_directories(mcubsp_mcuip_contract_probe PRIVATE
        "${MCUBSP_MCUIP_SOURCE_DIR}/include")
    mcubsp_apply_restricted_runtime(mcubsp_mcuip_contract_probe)
    list(APPEND mcubsp_probe_targets mcubsp_mcuip_contract_probe)
endif()

# @req REQ-0009 REQ-0038
add_library(mcubsp_barrier_probe OBJECT "${CMAKE_CURRENT_LIST_DIR}/barrier_probe.cpp")
target_link_libraries(mcubsp_barrier_probe PRIVATE mcubsp::mcubsp)
mcubsp_apply_restricted_runtime(mcubsp_barrier_probe)
add_custom_target(mcubsp_verify_barriers
    COMMAND "${CMAKE_COMMAND}"
        -DOBJDUMP=${CMAKE_OBJDUMP}
        -DOBJECT_FILE=$<TARGET_OBJECTS:mcubsp_barrier_probe>
        -DARCH=${MCUBSP_ARCH}
        -DREPORT=${CMAKE_CURRENT_BINARY_DIR}/barrier-report.txt
        -P "${CMAKE_CURRENT_LIST_DIR}/VerifyBarrierInstructions.cmake"
    DEPENDS mcubsp_barrier_probe
    VERBATIM)
list(APPEND mcubsp_probe_targets mcubsp_barrier_probe mcubsp_verify_barriers)

add_custom_target(check-compile DEPENDS ${mcubsp_probe_targets})
set_target_properties(check-compile PROPERTIES FOLDER "Verification")
foreach(target IN LISTS mcubsp_probe_targets)
    set_target_properties(${target} PROPERTIES FOLDER "Verification/Compile")
endforeach()
