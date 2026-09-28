#include <cstdint>

#include <mcuhw/core/register.hpp>

using register_type = mcuhw::core::register_descriptor<
    std::uint8_t, 0, mcuhw::core::access_semantics::read_write>;
using raw_field = mcuhw::core::field_descriptor<register_type, 0, 2>;
enum class mode : std::uint8_t { zero, one, prohibited };
using mode_field = mcuhw::core::enum_field_descriptor<
    raw_field, mode, mode::zero, mode::one>;

constexpr auto invalid = mode_field::encode_constant<mode::prohibited>();
