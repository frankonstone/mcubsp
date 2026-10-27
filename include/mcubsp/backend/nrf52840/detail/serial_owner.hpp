#pragma once

// @req-file REQ-0054
namespace mcubsp::backend::nrf52840::detail {
// TWI/TWIM/TWIS and SPI/SPIM/SPIS share serial blocks 0 and 1. All future
// backends for those blocks must use this identity, independent of pin binding.
template<unsigned Instance> struct serial_owner { static inline bool claimed{}; };
} // namespace mcubsp::backend::nrf52840::detail
