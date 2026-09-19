#include <mcuhw/core/metadata.hpp>

constexpr mcuhw::core::pin_id pin{1, 2};
using invalid_composition = mcuhw::core::pin_composition<pin, pin>;

static_assert(sizeof(invalid_composition) != 0);
