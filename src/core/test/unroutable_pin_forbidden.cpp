#include <mcubsp/core/metadata.hpp>

constexpr mcubsp::core::pin_id available_pin{1, 0};
constexpr mcubsp::core::pin_id unavailable_pin{1, 1};
constexpr mcubsp::core::signal_id output_signal{{1, 0}, 1};

struct database {
    template<mcubsp::core::pin_id Pin, mcubsp::core::signal_id Signal,
             mcubsp::core::electrical_mode Mode>
    static constexpr bool valid =
        Pin == available_pin && Signal == output_signal
        && Mode == mcubsp::core::electrical_mode::push_pull;
};

using invalid_binding = mcubsp::core::pin_binding<
    database, unavailable_pin, output_signal,
    mcubsp::core::electrical_mode::push_pull>;

static_assert(sizeof(invalid_binding) != 0);
