#include <mcubsp/core/metadata.hpp>

constexpr mcubsp::core::source_ref source{"trial", "1", "interrupt"};
constexpr mcubsp::core::interrupt_route first{
    1, 5, mcubsp::core::interrupt_route_kind::selectable_line, 1, true, source};
constexpr mcubsp::core::interrupt_route second{
    2, 5, mcubsp::core::interrupt_route_kind::selectable_line, 2, true, source};

static_assert(!mcubsp::core::interrupt_routes_conflict(first, second),
              "exclusive interrupt-line conflict must be rejected");
