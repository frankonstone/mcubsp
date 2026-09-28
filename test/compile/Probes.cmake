# Maintainer-only probes. # @req REQ-0047 REQ-0052
get_property(_mcuhw_targets_before_probes DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
# @req REQ-0002 REQ-0015 REQ-0016 REQ-0017 REQ-0018 REQ-0019 REQ-0035 REQ-0038 REQ-0040 REQ-0041
add_library(mcuhw_arch_probe OBJECT
    compile/all_descriptions_probe.cpp
    compile/arch_probe.cpp
    compile/cxx23_features.cpp
    compile/execution_probe.cpp
    compile/io_example.cpp
    compile/io_devices_example.cpp
    compile/uart_i2c_example.cpp
    compile/spi_adc_example.cpp
    compile/dmac_example.cpp
    compile/esp32p4_metadata_probe.cpp
    compile/stm32h753_metadata_probe.cpp
)
target_link_libraries(mcuhw_arch_probe PRIVATE mcuhw::mcuhw)
target_compile_definitions(mcuhw_arch_probe PRIVATE
    MCUHW_ARCH_NAME="${MCUHW_ARCH}"
)
mcuhw_apply_restricted_runtime(mcuhw_arch_probe)

# Optional by installation, mandatory for the sibling-development contract:
# these TUs prove that both fixed-ring boundaries satisfy the current mcuip
# replacement for lwIP on every configured architecture.
# @req REQ-0043 REQ-0044
if(MCUHW_BUILD_MCUIP_CONTRACTS)
    if(NOT EXISTS "${MCUHW_MCUIP_SOURCE_DIR}/include/mcuip/target.hpp")
        message(FATAL_ERROR "MCUIP contract checks requested; set MCUHW_MCUIP_SOURCE_DIR")
    endif()
    add_library(mcuhw_mcuip_contract_probe OBJECT
        compile/stm32h753_mcuip_contract.cpp
        compile/esp32p4_mcuip_contract.cpp
        compile/uno_r4_minima_w5100_mcuip_contract.cpp)
    target_link_libraries(mcuhw_mcuip_contract_probe PRIVATE mcuhw::mcuhw)
    target_include_directories(mcuhw_mcuip_contract_probe PRIVATE
        "${MCUHW_MCUIP_SOURCE_DIR}/include")
    mcuhw_apply_restricted_runtime(mcuhw_mcuip_contract_probe)
endif()

# @req REQ-0009 REQ-0038
add_library(mcuhw_barrier_probe OBJECT compile/barrier_probe.cpp)
target_link_libraries(mcuhw_barrier_probe PRIVATE mcuhw::mcuhw)
mcuhw_apply_restricted_runtime(mcuhw_barrier_probe)
if(CMAKE_CROSSCOMPILING)
    add_custom_target(mcuhw_verify_barriers ALL
        COMMAND "${CMAKE_COMMAND}"
            -DOBJDUMP=${CMAKE_OBJDUMP}
            -DOBJECT_FILE=$<TARGET_OBJECTS:mcuhw_barrier_probe>
            -DARCH=${MCUHW_ARCH}
            -DREPORT=${mcuhw_BINARY_DIR}/barrier-report.txt
            -P "${mcuhw_SOURCE_DIR}/test/compile/VerifyBarrierInstructions.cmake"
        DEPENDS mcuhw_barrier_probe
        VERBATIM
    )
endif()

if(MCUHW_ARCH STREQUAL "cortex-m4")
    add_library(mcuhw_ra4m1_gpio_codegen OBJECT
        compile/ra4m1_gpio_codegen_probe.cpp)
    target_link_libraries(mcuhw_ra4m1_gpio_codegen PRIVATE mcuhw::mcuhw)
    mcuhw_apply_restricted_runtime(mcuhw_ra4m1_gpio_codegen)
    target_compile_options(mcuhw_ra4m1_gpio_codegen PRIVATE -Os)
    add_custom_target(mcuhw_gpio_codegen_report ALL
        COMMAND "${CMAKE_COMMAND}"
            -DNM=${CMAKE_NM}
            -DOBJDUMP=${CMAKE_OBJDUMP}
            -DOBJECT_FILE=$<TARGET_OBJECTS:mcuhw_ra4m1_gpio_codegen>
            -DREPORT=${mcuhw_BINARY_DIR}/gpio-codegen-report.txt
            -P "${mcuhw_SOURCE_DIR}/test/compile/GenerateGpioCodegenReport.cmake"
        DEPENDS mcuhw_ra4m1_gpio_codegen
        VERBATIM
    )
endif()

# The build preset selects this complete compile gate; the individual probes
# remain available for focused investigation in the Verification/Compile group.
get_property(_mcuhw_probe_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
list(REMOVE_ITEM _mcuhw_probe_targets ${_mcuhw_targets_before_probes})
add_custom_target(check-compile DEPENDS ${_mcuhw_probe_targets})
set_target_properties(check-compile PROPERTIES FOLDER "Verification")
foreach(target IN LISTS _mcuhw_probe_targets)
    set_target_properties(${target} PROPERTIES FOLDER "Verification/Compile")
endforeach()
