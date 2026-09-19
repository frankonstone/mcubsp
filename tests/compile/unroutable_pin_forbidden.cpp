#include <mcuhw/core/metadata.hpp>

constexpr mcuhw::core::pin_id available_pin{1, 0};
constexpr mcuhw::core::pin_id unavailable_pin{1, 1};
constexpr mcuhw::core::signal_id output_signal{{1, 0}, 1};

struct database {
    template<mcuhw::core::pin_id Pin, mcuhw::core::signal_id Signal,
             mcuhw::core::electrical_mode Mode>
    static constexpr bool valid =
        Pin == available_pin && Signal == output_signal
        && Mode == mcuhw::core::electrical_mode::push_pull;
};

using invalid_binding = mcuhw::core::pin_binding<
    database, unavailable_pin, output_signal,
    mcuhw::core::electrical_mode::push_pull>;

static_assert(sizeof(invalid_binding) != 0);
