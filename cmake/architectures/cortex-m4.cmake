# Compiler target and ABI shared by supplied and external BSP toolchains.
set(MCUHW_COMPILER_PATTERN "arm-none-eabi")
set(MCUHW_ARCH_COMPILE_OPTIONS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
