# Compiler target/ABI and SDK identity for the selected ESP32-P4 architecture.
set(MCUHW_COMPILER_PATTERN "riscv32-esp-elf")
set(MCUHW_ARCH_COMPILE_OPTIONS -march=rv32imafc_zicsr_zifencei -mabi=ilp32f)
set(MCUHW_IDF_TARGET "esp32p4")
