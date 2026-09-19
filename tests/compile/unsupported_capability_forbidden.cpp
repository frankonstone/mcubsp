#include <cstdint>

#include <mcuhw/core/metadata.hpp>

enum class output : std::uint8_t { a, b, c };
using supported_outputs = mcuhw::core::capability_set<
    output, output::a, output::b>;
using invalid_request = mcuhw::core::capability_request<
    supported_outputs, output::c>;

static_assert(sizeof(invalid_request) != 0);
