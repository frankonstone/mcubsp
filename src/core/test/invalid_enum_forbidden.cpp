#include <cstdint>

#include <mcubsp/core/register.hpp>

using register_type = mcubsp::core::register_descriptor<
    std::uint8_t, 0, mcubsp::core::access_semantics::read_write>;
using raw_field = mcubsp::core::field_descriptor<register_type, 0, 2>;
enum class mode : std::uint8_t { zero, one, prohibited };
using mode_field = mcubsp::core::enum_field_descriptor<
    raw_field, mode, mode::zero, mode::one>;

constexpr auto invalid = mode_field::encode_constant<mode::prohibited>();
