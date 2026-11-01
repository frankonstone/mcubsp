#include <mcubsp/backend/nrf52840/spim.hpp>
using binding=mcubsp::backend::nrf52840::spim_binding<0,{0,6},{0,6},{0,8}>;
static_assert(binding::instance==0);
