#include <mcubsp/backend/nrf52840/spim.hpp>
using binding=mcubsp::backend::nrf52840::spim_binding<4,{0,6},{0,7},{0,8}>;
static_assert(binding::instance==4);
