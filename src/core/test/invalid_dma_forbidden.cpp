#include <mcubsp/core/metadata.hpp>

constexpr mcubsp::core::dma_constraints constraints{
    mcubsp::core::dma_engine_kind::ra_dmac,
    static_cast<std::uint8_t>(mcubsp::core::dma_direction::memory_to_peripheral),
    mcubsp::core::dma_width_bit(1),
    1,
    32,
};

using invalid_transfer = mcubsp::core::static_dma_transfer<
    constraints, 4, 64, mcubsp::core::dma_direction::memory_to_peripheral>;

static_assert(sizeof(invalid_transfer) != 0);
