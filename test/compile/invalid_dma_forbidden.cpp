#include <mcuhw/core/metadata.hpp>

constexpr mcuhw::core::dma_constraints constraints{
    mcuhw::core::dma_engine_kind::ra_dmac,
    static_cast<std::uint8_t>(mcuhw::core::dma_direction::memory_to_peripheral),
    mcuhw::core::dma_width_bit(1),
    1,
    32,
};

using invalid_transfer = mcuhw::core::static_dma_transfer<
    constraints, 4, 64, mcuhw::core::dma_direction::memory_to_peripheral>;

static_assert(sizeof(invalid_transfer) != 0);
