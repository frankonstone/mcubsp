#include <mcubsp/core/metadata.hpp>

constexpr mcubsp::core::source_ref source{"trial", "1", "clock"};
constexpr mcubsp::core::clock_node destination{
    2, mcubsp::core::hertz{1}, mcubsp::core::hertz{100'000'000}, source};
constexpr mcubsp::core::clock_edge integer_only{
    1, 2, 1, 16, false, mcubsp::core::clock_route_kind::selectable, 0, source};

static_assert(mcubsp::core::derive_frequency(
                  mcubsp::core::hertz{80'000'000}, integer_only, {2, 1, 2},
                  destination)
                  .has_value(),
              "fractional divisor must be rejected on an integer-only edge");
