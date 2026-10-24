#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string_view>
#include <type_traits>

#include <mcubsp/core/units.hpp>

namespace mcubsp::core {

// @req REQ-0005 REQ-0007
struct source_ref {
    std::string_view document;
    std::string_view revision;
    std::string_view location;
};

// Source-unit coverage is intentionally separate from driver support.  A
// completed source unit accounts for register, field, and routing material,
// including an explicit not-applicable decision when a category does not occur
// in that unit.  Unavailable material is retained in the denominator and must
// never be reported as verified.
// @req REQ-0005 REQ-0006 REQ-0040
enum class description_coverage_state : std::uint8_t {
    unreviewed,
    described,
    verified,
    deferred,
    unavailable_in_source,
    not_applicable,
};

// @req REQ-0006 REQ-0040
enum class driver_support_state : std::uint8_t {
    planned,
    deferred,
    not_applicable,
};

// @req REQ-0005 REQ-0006 REQ-0040
struct source_unit_coverage {
    std::string_view unit;
    std::string_view title;
    std::uint16_t first_pdf_page;
    description_coverage_state description;
    driver_support_state driver;
    bool registers_accounted;
    bool fields_accounted;
    bool routes_accounted;
    std::string_view decision;
    source_ref provenance;

};

// @req REQ-0006 REQ-0040
template<std::size_t Extent>
[[nodiscard]] constexpr bool complete_source_inventory(
    std::span<const source_unit_coverage, Extent> inventory,
    std::size_t expected_denominator,
    std::size_t expected_unavailable = 0U) noexcept {
    if (inventory.size() != expected_denominator) {
        return false;
    }

    std::size_t unavailable{};
    for (std::size_t index = 0; index < inventory.size(); ++index) {
        const auto& unit = inventory[index];
        if (unit.unit.empty() || unit.title.empty()
            || unit.provenance.document.empty()
            || unit.provenance.revision.empty()) {
            return false;
        }
        for (std::size_t other = index + 1U; other < inventory.size(); ++other) {
            if (unit.unit == inventory[other].unit) {
                return false;
            }
        }

        if (unit.description
            == description_coverage_state::unavailable_in_source) {
            ++unavailable;
            if (unit.registers_accounted || unit.fields_accounted
                || unit.routes_accounted || unit.decision.empty()) {
                return false;
            }
            continue;
        }

        if (unit.description != description_coverage_state::verified
            && unit.description != description_coverage_state::not_applicable) {
            return false;
        }
        if (!unit.registers_accounted || !unit.fields_accounted
            || !unit.routes_accounted) {
            return false;
        }
    }
    return unavailable == expected_unavailable;
}

// @req REQ-0005 REQ-0007
struct device_identity {
    std::string_view vendor;
    std::string_view family;
    std::string_view device;
    std::string_view package;
    std::string_view silicon_revision;
    source_ref provenance;
};

// @req REQ-0004 REQ-0010 REQ-0011 REQ-0012 REQ-0013
enum class metadata_error : std::uint8_t {
    unsupported,
    unavailable_pin,
    unroutable_signal,
    conflicting_assignment,
    invalid_clock,
    invalid_divider,
    inaccessible_memory,
    invalid_alignment,
    invalid_count,
    invalid_width,
};

template<typename Enum, Enum... Available>
    requires std::is_enum_v<Enum>
// @req REQ-0004
struct capability_set {
    template<Enum Requested>
    static constexpr bool contains = ((Requested == Available) || ...);

    [[nodiscard]] static constexpr bool contains_runtime(Enum requested) noexcept {
        return ((requested == Available) || ...);
    }

    [[nodiscard]] static constexpr std::expected<void, metadata_error>
    require(Enum requested) noexcept {
        if (!contains_runtime(requested)) {
            return std::unexpected(metadata_error::unsupported);
        }
        return {};
    }
};

template<typename Capabilities, auto Requested>
concept has_capability = Capabilities::template contains<Requested>;

template<typename Capabilities, auto Requested>
    requires has_capability<Capabilities, Requested>
// @req REQ-0004
struct capability_request {};

// @req REQ-0007
struct peripheral_id {
    std::uint16_t kind;
    std::uint16_t instance;

    friend constexpr bool operator==(peripheral_id, peripheral_id) = default;
};

// The owning description retains the arrays referenced by these spans.
// @req REQ-0007
struct peripheral_metadata {
    peripheral_id id;
    std::span<const std::uint16_t> clock_inputs;
    std::span<const std::uint16_t> reset_controls;
    std::span<const std::uint16_t> event_sources;
    std::span<const std::uint16_t> dma_requests;
    source_ref provenance;
};

// @req REQ-0007 REQ-0009
struct protection_requirement {
    std::uint16_t group;
    std::size_t control_register_offset;
    bool relock_required;
    source_ref provenance;
};

// @req REQ-0010
struct pin_id {
    std::uint16_t port;
    std::uint16_t index;

    friend constexpr bool operator==(pin_id, pin_id) = default;
};

// @req REQ-0010
enum class electrical_mode : std::uint16_t {
    input = 1U << 0U,
    push_pull = 1U << 1U,
    open_drain = 1U << 2U,
    pull_up = 1U << 3U,
    pull_down = 1U << 4U,
    analog = 1U << 5U,
};

[[nodiscard]] constexpr std::uint16_t mode_mask(electrical_mode mode) noexcept {
    return static_cast<std::uint16_t>(mode);
}

// @req REQ-0010
struct package_pin {
    pin_id pin;
    std::uint16_t package_position;
    std::uint16_t electrical_modes;
    source_ref provenance;
};

// @req REQ-0010
struct signal_id {
    peripheral_id peripheral;
    std::uint16_t signal;

    friend constexpr bool operator==(signal_id, signal_id) = default;
};

// @req REQ-0010
enum class pin_route_kind : std::uint8_t {
    fixed,
    alternate_function,
    programmable_matrix,
};

// @req REQ-0010
struct pin_route {
    pin_id pin;
    signal_id signal;
    pin_route_kind kind;
    std::uint32_t vendor_encoding;
    source_ref provenance;
};

[[nodiscard]] constexpr bool package_contains(
    std::span<const package_pin> pins, pin_id requested,
    electrical_mode mode) noexcept {
    for (const auto& pin : pins) {
        if (pin.pin == requested
            && (pin.electrical_modes & mode_mask(mode)) != 0) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] constexpr bool route_exists(
    std::span<const pin_route> routes, pin_id pin, signal_id signal) noexcept {
    for (const auto& route : routes) {
        if (route.pin == pin && route.signal == signal) {
            return true;
        }
    }
    return false;
}

template<pin_id... Pins>
[[nodiscard]] consteval bool pins_are_unique() noexcept {
    constexpr std::array values{Pins...};
    for (std::size_t left = 0; left < values.size(); ++left) {
        for (std::size_t right = left + 1; right < values.size(); ++right) {
            if (values[left] == values[right]) {
                return false;
            }
        }
    }
    return true;
}

template<pin_id... Pins>
// @req REQ-0010
struct pin_composition {
    static_assert(pins_are_unique<Pins...>(),
                  "exclusive pin assigned more than once in this composition");
};

template<typename Database, pin_id Pin, signal_id Signal, electrical_mode Mode>
concept valid_pin_binding = requires {
    { Database::template valid<Pin, Signal, Mode> } -> std::convertible_to<bool>;
} && Database::template valid<Pin, Signal, Mode>;

template<typename Database, pin_id Pin, signal_id Signal, electrical_mode Mode>
    requires valid_pin_binding<Database, Pin, Signal, Mode>
// @req REQ-0010
struct pin_binding {};

// @req REQ-0011
struct clock_node {
    std::uint16_t id;
    hertz minimum;
    hertz maximum;
    source_ref provenance;
};

// @req REQ-0011
enum class clock_route_kind : std::uint8_t {
    fixed,
    selectable,
};

// @req REQ-0011
struct clock_edge {
    std::uint16_t source;
    std::uint16_t destination;
    std::uint16_t minimum_divider;
    std::uint16_t maximum_divider;
    bool allows_fractional_divider;
    clock_route_kind kind;
    std::uint32_t vendor_encoding;
    source_ref provenance;
};

// @req REQ-0011
struct clock_divider {
    std::uint16_t whole;
    std::uint16_t numerator;
    std::uint16_t denominator{1};
};

[[nodiscard]] constexpr std::expected<hertz, metadata_error>
derive_frequency(hertz input, const clock_edge& edge,
                 clock_divider divider, const clock_node& destination) noexcept {
    if (divider.whole < edge.minimum_divider
        || divider.whole > edge.maximum_divider
        || divider.denominator == 0
        || divider.numerator >= divider.denominator) {
        return std::unexpected(metadata_error::invalid_divider);
    }
    if (!edge.allows_fractional_divider && divider.numerator != 0) {
        return std::unexpected(metadata_error::invalid_divider);
    }

    const auto scaled_whole =
        static_cast<std::uint64_t>(divider.whole) * divider.denominator;
    const auto divisor = scaled_whole + divider.numerator;
    if (divisor == 0) {
        return std::unexpected(metadata_error::invalid_divider);
    }
    if (input.count > std::numeric_limits<std::uint64_t>::max()
                          / divider.denominator) {
        return std::unexpected(metadata_error::invalid_clock);
    }
    const hertz output{input.count * divider.denominator / divisor};
    if (output < destination.minimum || output > destination.maximum) {
        return std::unexpected(metadata_error::invalid_clock);
    }
    return output;
}

// @req REQ-0011
struct reset_control {
    std::uint16_t id;
    std::uint16_t clock_domain;
    std::size_t register_offset;
    std::uint32_t mask;
    bool active_high;
    std::uint16_t protection_group;
    source_ref provenance;
};

// @req REQ-0012
enum class interrupt_route_kind : std::uint8_t {
    fixed_line,
    selectable_line,
    interrupt_matrix,
};

// @req REQ-0012
struct interrupt_route {
    std::uint16_t event_source;
    std::uint16_t cpu_line;
    interrupt_route_kind kind;
    std::uint32_t vendor_encoding;
    bool exclusive_line;
    source_ref provenance;
};

[[nodiscard]] constexpr bool interrupt_route_exists(
    std::span<const interrupt_route> routes, std::uint16_t event,
    std::uint16_t line) noexcept {
    for (const auto& route : routes) {
        if (route.event_source == event && route.cpu_line == line) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] constexpr bool interrupt_routes_conflict(
    interrupt_route left, interrupt_route right) noexcept {
    return left.cpu_line == right.cpu_line
        && left.event_source != right.event_source
        && (left.exclusive_line || right.exclusive_line);
}

// @req REQ-0013
enum class dma_engine_kind : std::uint8_t {
    ra_dmac,
    ra_dtc,
    stm32_dma,
    stm32_bdma,
    stm32_mdma,
    stm32_ethernet,
    esp_gdma_ahb,
    esp_gdma_axi,
    esp_vdma,
    esp_2d_dma,
    esp_ethernet,
};

// @req REQ-0013
enum class dma_direction : std::uint8_t {
    memory_to_memory = 1U << 0U,
    memory_to_peripheral = 1U << 1U,
    peripheral_to_memory = 1U << 2U,
};

// @req REQ-0013
enum class cache_policy : std::uint8_t {
    uncached,
    software_maintained,
    coherent,
};

// @req REQ-0013
struct dma_constraints {
    dma_engine_kind engine;
    std::uint8_t directions;
    std::uint8_t width_mask;
    std::uint16_t minimum_alignment;
    std::uint32_t maximum_count;

    friend constexpr bool operator==(dma_constraints, dma_constraints) = default;
};

// @req REQ-0013
struct dma_request {
    std::uint16_t request_id;
    dma_constraints constraints;
    source_ref provenance;
};

// @req REQ-0013
struct memory_region {
    std::uint16_t id;
    std::uintptr_t begin;
    std::size_t size;
    bool readable;
    bool writable;
    cache_policy cache;
    source_ref provenance;
};

// @req REQ-0013
struct dma_reachability {
    dma_engine_kind engine;
    std::uint16_t region_id;
    bool can_read;
    bool can_write;
};

[[nodiscard]] constexpr std::uint8_t dma_width_bit(std::size_t width) noexcept {
    return width == 1 ? 1U : width == 2 ? 2U : width == 4 ? 4U : 0U;
}

[[nodiscard]] constexpr std::uint8_t dma_direction_bit(
    dma_direction direction) noexcept {
    return static_cast<std::uint8_t>(direction);
}

[[nodiscard]] constexpr std::expected<void, metadata_error>
validate_dma_transfer(const dma_request& request, const memory_region& region,
                      const dma_reachability& reachability,
                      std::uintptr_t address, std::size_t count,
                      std::size_t width, dma_direction direction) noexcept {
    const auto& constraints = request.constraints;
    if (reachability.engine != constraints.engine
        || reachability.region_id != region.id) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if ((constraints.directions & dma_direction_bit(direction)) == 0) {
        return std::unexpected(metadata_error::unsupported);
    }
    const auto width_bit = dma_width_bit(width);
    if (width_bit == 0 || (constraints.width_mask & width_bit) == 0) {
        return std::unexpected(metadata_error::invalid_width);
    }
    if (count == 0 || count > constraints.maximum_count) {
        return std::unexpected(metadata_error::invalid_count);
    }
    const auto alignment = std::max<std::size_t>(
        constraints.minimum_alignment, width);
    if (address % alignment != 0) {
        return std::unexpected(metadata_error::invalid_alignment);
    }
    if (count > std::numeric_limits<std::size_t>::max() / width) {
        return std::unexpected(metadata_error::invalid_count);
    }
    const auto transfer_bytes = count * width;
    if (address < region.begin || transfer_bytes > region.size
        || address - region.begin > region.size - transfer_bytes) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if (direction == dma_direction::memory_to_peripheral && !reachability.can_read) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if (direction == dma_direction::memory_to_peripheral && !region.readable) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if (direction == dma_direction::peripheral_to_memory && !reachability.can_write) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if (direction == dma_direction::peripheral_to_memory && !region.writable) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if (direction == dma_direction::memory_to_memory
        && (!reachability.can_read || !reachability.can_write)) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    if (direction == dma_direction::memory_to_memory
        && (!region.readable || !region.writable)) {
        return std::unexpected(metadata_error::inaccessible_memory);
    }
    return {};
}

template<dma_constraints Constraints, std::size_t Width, std::size_t Count,
         dma_direction Direction>
concept valid_static_dma_transfer =
    Count > 0 && Count <= Constraints.maximum_count
    && (dma_width_bit(Width) & Constraints.width_mask) != 0
    && (dma_direction_bit(Direction) & Constraints.directions) != 0;

template<dma_constraints Constraints, std::size_t Width, std::size_t Count,
         dma_direction Direction>
    requires valid_static_dma_transfer<Constraints, Width, Count, Direction>
// @req REQ-0013
struct static_dma_transfer {};

} // namespace mcubsp::core
