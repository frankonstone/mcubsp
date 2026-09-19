#include <mcuhw/core/metadata.hpp>

constexpr mcuhw::core::source_ref source{"trial", "1", "interrupt"};
constexpr mcuhw::core::interrupt_route first{
    1, 5, mcuhw::core::interrupt_route_kind::selectable_line, 1, true, source};
constexpr mcuhw::core::interrupt_route second{
    2, 5, mcuhw::core::interrupt_route_kind::selectable_line, 2, true, source};

static_assert(!mcuhw::core::interrupt_routes_conflict(first, second),
              "exclusive interrupt-line conflict must be rejected");
