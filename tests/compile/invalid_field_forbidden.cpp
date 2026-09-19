#include <cstdint>

#include <mcuhw/core/register.hpp>

using byte_register = mcuhw::core::register_descriptor<
    std::uint8_t, 0, mcuhw::core::access_semantics::read_write>;
using invalid_field = mcuhw::core::field_descriptor<byte_register, 7, 2>;

static_assert(sizeof(invalid_field) != 0);
