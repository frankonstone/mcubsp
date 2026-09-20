#pragma once

// GCC 15 advertises extended floating-point types that the Clang frontend
// used by MCUCOV does not parse for this RISC-V target. This forced include is
// inert for the real target compiler and adjusts only the analysis pass.
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
