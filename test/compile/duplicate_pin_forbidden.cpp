#include <mcubsp/core/metadata.hpp>

constexpr mcubsp::core::pin_id pin{1, 2};
using invalid_composition = mcubsp::core::pin_composition<pin, pin>;

static_assert(sizeof(invalid_composition) != 0);
