if(NOT DEFINED OBJDUMP OR NOT DEFINED OBJECT_FILE OR NOT DEFINED ARCH)
    message(FATAL_ERROR "OBJDUMP, OBJECT_FILE and ARCH are required")
endif()

execute_process(
    COMMAND "${OBJDUMP}" -d "${OBJECT_FILE}"
    RESULT_VARIABLE disassembly_result
    OUTPUT_VARIABLE disassembly
    ERROR_VARIABLE disassembly_error
)
if(NOT disassembly_result EQUAL 0)
    message(FATAL_ERROR "objdump failed: ${disassembly_error}")
endif()

if(ARCH MATCHES "^cortex-m")
    foreach(instruction IN ITEMS dmb dsb isb)
        if(NOT disassembly MATCHES "${instruction}[	 ]+sy")
            message(FATAL_ERROR
                "${ARCH} barrier probe does not contain '${instruction} sy'")
        endif()
    endforeach()
elseif(ARCH STREQUAL "esp32p4-riscv")
    string(REGEX MATCHALL "fence" fence_matches "${disassembly}")
    list(LENGTH fence_matches fence_count)
    if(fence_count LESS 3 OR NOT disassembly MATCHES "fence\\.i")
        message(FATAL_ERROR
            "ESP32-P4 barrier probe does not contain two fences and fence.i")
    endif()
else()
    message(FATAL_ERROR "No barrier instruction contract for ${ARCH}")
endif()

file(WRITE "${REPORT}"
    "architecture: ${ARCH}\n"
    "objdump: ${OBJDUMP}\n"
    "status: required barrier instructions verified\n")
message(STATUS "Verified ${ARCH} barrier instructions")
