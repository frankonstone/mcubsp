set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(MCUHW_ARM_CXX_COMPILER arm-none-eabi-g++ REQUIRED)
set(CMAKE_CXX_COMPILER "${MCUHW_ARM_CXX_COMPILER}")
set(MCUHW_ARCH "cortex-m4" CACHE STRING "Architecture probed by this build" FORCE)
set(MCUHW_ARCH_COMPILE_OPTIONS -mcpu=cortex-m4 -mthumb -ffreestanding
    CACHE STRING "Architecture compile options" FORCE)
