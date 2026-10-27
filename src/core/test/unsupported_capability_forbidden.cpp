#include <cstdint>

#include <mcubsp/core/metadata.hpp>

enum class output : std::uint8_t { a, b, c };
using supported_outputs = mcubsp::core::capability_set<
    output, output::a, output::b>;
using invalid_request = mcubsp::core::capability_request<
    supported_outputs, output::c>;

static_assert(sizeof(invalid_request) != 0);
