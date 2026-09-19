if(NOT DEFINED NM OR NOT DEFINED OBJDUMP OR NOT DEFINED OBJECT_FILE
   OR NOT DEFINED REPORT)
    message(FATAL_ERROR "NM, OBJDUMP, OBJECT_FILE, and REPORT are required")
endif()

execute_process(
    COMMAND "${NM}" -S --size-sort --demangle "${OBJECT_FILE}"
    RESULT_VARIABLE nm_result
    OUTPUT_VARIABLE symbols
    ERROR_VARIABLE nm_error
)
if(NOT nm_result EQUAL 0)
    message(FATAL_ERROR "nm failed: ${nm_error}")
endif()

execute_process(
    COMMAND "${OBJDUMP}" -d --demangle "${OBJECT_FILE}"
    RESULT_VARIABLE objdump_result
    OUTPUT_VARIABLE disassembly
    ERROR_VARIABLE objdump_error
)
if(NOT objdump_result EQUAL 0)
    message(FATAL_ERROR "objdump failed: ${objdump_error}")
endif()

string(REGEX MATCH "[^\n]*direct_gpio_set_high_baseline[^\n]*" baseline "${symbols}")
string(REGEX MATCH "[^\n]*mcuhw_gpio_set_high_probe[^\n]*" abstraction "${symbols}")
file(WRITE "${REPORT}"
    "RA4M1 GPIO code-generation comparison\n"
    "Compiler flags: -Os, Cortex-M4, Thumb, hard-float ABI\n"
    "Size rows are hexadecimal and include each exported probe body.\n\n"
    "Direct register baseline:\n${baseline}\n\n"
    "mcuhw functional-driver call:\n${abstraction}\n\n"
    "No zero-overhead claim is made; the driver intentionally validates state.\n\n"
    "Disassembly:\n${disassembly}")
