#pragma once

#include <concepts>
#include <utility>

#include <mcuhw/core/metadata.hpp>

namespace mcuhw::testing {

// Invoke one contract for every real description type supplied by a caller.
// This deliberately imposes no target fields: PLN-0003 defines those contracts
// from real, source-backed differences before this harness is instantiated.
// @req REQ-0040
template<typename... Descriptions, typename Contract>
constexpr bool for_each_description(Contract&& contract) {
    return (static_cast<bool>(
                std::forward<Contract>(contract).template operator()<Descriptions>())
            && ...);
}

// The common gate intentionally checks only shared description concepts.  It
// does not require vendor registers or native encodings to have one layout.
template<typename Description>
concept readable_description = requires {
    Description::device;
    Description::source_units;
    Description::memories;
    Description::pins;
    Description::routes;
    Description::clocks;
    Description::clock_relations;
    Description::resets;
    Description::irqs;
    Description::transfers;
    Description::reachability;
    Description::peripherals;
    Description::native_instances;
    typename Description::pin_database_type;
    typename Description::representative_register;
    typename Description::representative_field;
};

[[nodiscard]] constexpr bool source_is_identified(
    const core::source_ref& source) noexcept {
    return !source.document.empty() && !source.revision.empty()
        && !source.location.empty();
}

template<readable_description Description>
[[nodiscard]] constexpr bool register_and_field_are_valid() noexcept {
    using reg = typename Description::representative_register;
    using field = typename Description::representative_field;
    return reg::width == sizeof(typename reg::value_type)
        && field::width > 0U
        && field::position + field::width
               <= sizeof(typename reg::value_type) * 8U
        && field::fits(0U)
        && field::fits(field::unshifted_mask)
        && !field::fits(static_cast<std::uintmax_t>(field::unshifted_mask) + 1U);
}

template<readable_description Description>
[[nodiscard]] constexpr bool pins_and_routes_are_valid() noexcept {
    if (Description::pins.empty() || Description::routes.empty()) {
        return false;
    }
    for (const auto& route : Description::routes) {
        bool package_pin_found = false;
        for (const auto& pin : Description::pins) {
            if (pin.pin == route.pin) {
                package_pin_found = pin.electrical_modes != 0U
                    && source_is_identified(pin.provenance);
                break;
            }
        }
        if (!package_pin_found || !source_is_identified(route.provenance)) {
            return false;
        }
    }
    const auto& first = Description::routes.front();
    return core::route_exists(Description::routes, first.pin, first.signal)
        && !core::route_exists(Description::routes,
                              core::pin_id{0xFFFFU, 0xFFFFU}, first.signal);
}

template<readable_description Description>
[[nodiscard]] constexpr bool clocks_are_valid() noexcept {
    if (Description::clocks.empty() || Description::clock_relations.empty()) {
        return false;
    }
    for (const auto& node : Description::clocks) {
        if (node.minimum > node.maximum || !source_is_identified(node.provenance)) {
            return false;
        }
    }
    for (const auto& edge : Description::clock_relations) {
        bool source_found = false;
        bool destination_found = false;
        for (const auto& node : Description::clocks) {
            source_found = source_found || node.id == edge.source;
            destination_found = destination_found || node.id == edge.destination;
        }
        if (!source_found || !destination_found || edge.minimum_divider == 0U
            || edge.minimum_divider > edge.maximum_divider
            || !source_is_identified(edge.provenance)) {
            return false;
        }
    }
    const auto& first = Description::clock_relations.front();
    return first.maximum_divider != 0xFFFFU;
}

template<readable_description Description>
[[nodiscard]] constexpr bool interrupts_are_valid() noexcept {
    if (Description::irqs.empty()) {
        return false;
    }
    for (const auto& route : Description::irqs) {
        if (!source_is_identified(route.provenance)) {
            return false;
        }
    }
    const auto& first = Description::irqs.front();
    return core::interrupt_route_exists(
               Description::irqs, first.event_source, first.cpu_line)
        && !core::interrupt_route_exists(
               Description::irqs, 0xFFFFU, 0xFFFFU);
}

[[nodiscard]] constexpr std::size_t first_dma_width(
    std::uint8_t width_mask) noexcept {
    return (width_mask & core::dma_width_bit(1U)) != 0U ? 1U
        : (width_mask & core::dma_width_bit(2U)) != 0U ? 2U
        : (width_mask & core::dma_width_bit(4U)) != 0U ? 4U : 0U;
}

template<readable_description Description>
[[nodiscard]] constexpr bool dma_and_memory_are_valid() noexcept {
    if (Description::transfers.empty() || Description::reachability.empty()
        || Description::memories.empty()) {
        return false;
    }
    for (const auto& request : Description::transfers) {
        if (request.constraints.minimum_alignment == 0U
            || request.constraints.maximum_count == 0U
            || !source_is_identified(request.provenance)) {
            return false;
        }
        const auto width = first_dma_width(request.constraints.width_mask);
        if (width == 0U) {
            return false;
        }
        for (const auto& reach : Description::reachability) {
            if (reach.engine != request.constraints.engine) {
                continue;
            }
            for (const auto& region : Description::memories) {
                if (region.id != reach.region_id
                    || !source_is_identified(region.provenance)) {
                    continue;
                }
                core::dma_direction direction{};
                bool direction_found = false;
                const auto allowed = request.constraints.directions;
                if ((allowed & core::dma_direction_bit(
                                   core::dma_direction::memory_to_memory)) != 0U
                    && reach.can_read && reach.can_write
                    && region.readable && region.writable) {
                    direction = core::dma_direction::memory_to_memory;
                    direction_found = true;
                } else if ((allowed & core::dma_direction_bit(
                                          core::dma_direction::memory_to_peripheral)) != 0U
                           && reach.can_read && region.readable) {
                    direction = core::dma_direction::memory_to_peripheral;
                    direction_found = true;
                } else if ((allowed & core::dma_direction_bit(
                                          core::dma_direction::peripheral_to_memory)) != 0U
                           && reach.can_write && region.writable) {
                    direction = core::dma_direction::peripheral_to_memory;
                    direction_found = true;
                }
                if (!direction_found) {
                    continue;
                }
                const auto alignment = request.constraints.minimum_alignment > width
                    ? request.constraints.minimum_alignment : width;
                const auto remainder = region.begin % alignment;
                const auto address = remainder == 0U
                    ? region.begin : region.begin + alignment - remainder;
                if (address < region.begin || address - region.begin > region.size
                    || region.size - (address - region.begin) < width) {
                    continue;
                }
                const auto positive = core::validate_dma_transfer(
                    request, region, reach, address, 1U, width, direction);
                const auto negative = core::validate_dma_transfer(
                    request, region, reach, address, 0U, width, direction);
                if (positive && !negative
                    && negative.error() == core::metadata_error::invalid_count) {
                    return true;
                }
            }
        }
    }
    return false;
}

template<readable_description Description>
[[nodiscard]] constexpr bool capabilities_are_preserved() noexcept {
    bool native_capability_found = false;
    for (const auto& instance : Description::native_instances) {
        native_capability_found = native_capability_found
            || instance.capability_bits != 0U;
        if (!source_is_identified(instance.provenance)) {
            return false;
        }
    }
    for (const auto& peripheral : Description::peripherals) {
        if (!source_is_identified(peripheral.provenance)) {
            return false;
        }
    }
    return native_capability_found;
}

// @req REQ-0006 REQ-0040
template<readable_description Description>
[[nodiscard]] constexpr bool source_inventory_is_resolved() noexcept {
    if (Description::source_units.empty()) {
        return false;
    }
    for (const auto& unit : Description::source_units) {
        using enum core::description_coverage_state;
        if (unit.description == unreviewed || unit.description == described
            || unit.description == deferred) {
            return false;
        }
        if (unit.description == unavailable_in_source) {
            if (unit.registers_accounted || unit.fields_accounted
                || unit.routes_accounted || unit.decision.empty()) {
                return false;
            }
        } else if (!unit.registers_accounted || !unit.fields_accounted
                   || !unit.routes_accounted) {
            return false;
        }
    }
    return true;
}

// @req REQ-0003 REQ-0004 REQ-0006 REQ-0040
struct complete_description_contract {
    template<readable_description Description>
    [[nodiscard]] constexpr bool operator()() const noexcept {
        return !Description::device.vendor.empty()
            && !Description::source_units.empty()
            && source_inventory_is_resolved<Description>()
            && !Description::memories.empty()
            && !Description::pins.empty()
            && !Description::routes.empty()
            && !Description::clocks.empty()
            && !Description::irqs.empty()
            && !Description::transfers.empty()
            && !Description::peripherals.empty()
            && register_and_field_are_valid<Description>()
            && pins_and_routes_are_valid<Description>()
            && clocks_are_valid<Description>()
            && interrupts_are_valid<Description>()
            && dma_and_memory_are_valid<Description>()
            && capabilities_are_preserved<Description>();
    }
};

} // namespace mcuhw::testing
