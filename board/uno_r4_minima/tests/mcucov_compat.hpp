#pragma once

// GCC 15 advertises the C++23 stdfloat types in its predefined macro dump,
// while the Clang frontend used by MCUCOV does not yet parse _Float32 and
// _Float64 for arm-none-eabi. Keep the real GCC compile unchanged and hide
// only those two feature macros from MCUCOV's analysis pass.
#ifdef __MCUCOV_ANALYZER__
#undef __STDCPP_FLOAT32_T__
#undef __STDCPP_FLOAT64_T__
#undef __FLT32_DIG__
#undef __FLT64_DIG__
#endif
