#include <mcubsp/backend/nrf52840/io.hpp>
#include <mcubsp/backend/nrf52840/twis.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
using namespace mcubsp;
using Binding=backend::nrf52840::twis_binding<0,{0,6},{0,8}>;
using Invalid=backend::nrf52840::io_i2c_backend_selector<Binding,core::direct_mmio,execution::fake_policy>::type;
