#include <cstdint>

#include <mcubsp/core/register.hpp>

using byte_register = mcubsp::core::register_descriptor<
    std::uint8_t, 0, mcubsp::core::access_semantics::read_write>;
using invalid_field = mcubsp::core::field_descriptor<byte_register, 7, 2>;

static_assert(sizeof(invalid_field) != 0);
