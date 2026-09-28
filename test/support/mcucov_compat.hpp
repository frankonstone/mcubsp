#pragma once

// GCC 15 advertises extended floating-point types that MCUCOV's Clang analysis
// frontend cannot parse for the supported Arm and RISC-V targets. Leave the
// real compiler unchanged and hide those declarations only during analysis.
#ifdef __MCUCOV_ANALYZER__
#undef __STDCPP_FLOAT32_T__
#undef __STDCPP_FLOAT64_T__
#undef __STDCPP_FLOAT128_T__
#undef __STDCPP_BFLOAT16_T__
#undef __FLT32_DIG__
#undef __FLT64_DIG__
#undef __FLT128_DIG__
#undef __BFLT16_DIG__
#endif
