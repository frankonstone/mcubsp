#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/driver/sdmmc.hpp>
#include <mcuhw/mcu/esp32p4/peripherals.hpp>

namespace mcuhw::backend::esp32p4 {

namespace detail {
struct sdmmc_owner { static inline bool claimed{}; };
} // namespace detail

struct sdmmc_slot0_binding {
    static constexpr std::uintptr_t base = 0x5008'3000U;
    static constexpr std::uintptr_t hp_clock_base = 0x500E'6000U;
    static constexpr std::uintptr_t lp_clock_base = 0x5011'1000U;
    static constexpr std::uintptr_t pmu_base = 0x5011'5000U;
    static constexpr std::uint8_t d0 = 39U;
    static constexpr std::uint8_t d1 = 40U;
    static constexpr std::uint8_t d2 = 41U;
    static constexpr std::uint8_t d3 = 42U;
    static constexpr std::uint8_t clock = 43U;
    static constexpr std::uint8_t command = 44U;
    static constexpr std::uint8_t slot = 0U;
    static constexpr bool uses_gpio_matrix = false;
    static constexpr bool requires_io_power = true;
};

// Slot 1 is routed through the GPIO matrix to the ESP32-C5 wireless
// coprocessor on ESP32-P4-NANO-WIFI6-DB. Signal indices are the ESP32-P4
// SDMMC slot-1 matrix signals published by Espressif.
struct sdmmc_slot1_wifi_binding {
    static constexpr std::uintptr_t base = sdmmc_slot0_binding::base;
    static constexpr std::uintptr_t hp_clock_base =
        sdmmc_slot0_binding::hp_clock_base;
    static constexpr std::uintptr_t lp_clock_base =
        sdmmc_slot0_binding::lp_clock_base;
    static constexpr std::uintptr_t pmu_base = sdmmc_slot0_binding::pmu_base;
    static constexpr std::uint8_t d0 = 14U;
    static constexpr std::uint8_t d1 = 15U;
    static constexpr std::uint8_t d2 = 16U;
    static constexpr std::uint8_t d3 = 17U;
    static constexpr std::uint8_t clock = 18U;
    static constexpr std::uint8_t command = 19U;
    static constexpr std::uint8_t slot = 1U;
    static constexpr bool uses_gpio_matrix = true;
    static constexpr bool requires_io_power = false;
    static constexpr std::uint16_t clock_signal = 0U;
    static constexpr std::uint16_t command_signal = 1U;
    static constexpr std::uint16_t d0_signal = 2U;
    static constexpr std::uint16_t d1_signal = 3U;
    static constexpr std::uint16_t d2_signal = 4U;
    static constexpr std::uint16_t d3_signal = 5U;
};

// ESP32-P4 native SDMMC host. Slot bindings select dedicated IO-MUX or GPIO
// matrix routing. A single fixed 64-byte IDMAC descriptor is used. Commands and
// one-sector transfers are synchronously polled with finite limits; no
// scheduler or allocator is used.
template<typename MmioPolicy, typename MemoryPolicy,
         typename Binding = sdmmc_slot0_binding>
// @req REQ-0003 REQ-0009 REQ-0010 REQ-0011 REQ-0013 REQ-0014 REQ-0016 REQ-0019 REQ-0034 REQ-0037
class sdmmc_host_backend {
public:
    enum class initialization_stage : std::uint8_t {
        none,
        ownership,
        io_power,
        bus_clock,
        module_reset,
        pins,
        source_clock,
        controller_reset,
        interrupts,
        dma_reset,
        card_clock,
        fifo_reset,
        complete,
    };

    sdmmc_host_backend(MmioPolicy& mmio, MemoryPolicy& memory) noexcept
        : mmio_(mmio), memory_(memory) {}

    [[nodiscard]] bool initialize() noexcept {
        init_stage_ = initialization_stage::ownership;
        if (initialized_ || detail::sdmmc_owner::claimed) return false;
        detail::sdmmc_owner::claimed = true;
        owned_ = true;
        init_stage_ = initialization_stage::io_power;
        if (!enable_io_power()) return init_fail();
        init_stage_ = initialization_stage::bus_clock;
        if (!modify(hp_soc_clock_ctrl1, 1U << 14U, 0U)) return init_fail();
        init_stage_ = initialization_stage::module_reset;
        if (!pulse_module_reset()) return init_fail();
        init_stage_ = initialization_stage::pins;
        if (!configure_pins()) return init_fail();
        init_stage_ = initialization_stage::source_clock;
        if (!modify(hp_ref_clock_ctrl2, 1U, 0U)
            || !configure_source_clock(10U)) return init_fail();
        init_stage_ = initialization_stage::controller_reset;
        if (!reset_controller()) return init_fail();
        init_stage_ = initialization_stage::interrupts;
        if (!write(reg_intmask, 0U) || !write(reg_rintsts, 0xFFFF'FFFFU)
            || !write(reg_idsts, 0xFFFF'FFFFU)
            || !write(reg_idinten, (1U << 8U) | dma_receive_done
                                      | dma_transmit_done)
            || !modify(reg_ctrl, 0U, (1U << 4U) | (1U << 5U)
                                     | (1U << 25U))) return init_fail();
        init_stage_ = initialization_stage::dma_reset;
        if (!write(reg_bmod, 1U) || !wait_clear(reg_bmod, 1U)) {
            return init_fail();
        }
        init_stage_ = initialization_stage::card_clock;
        if (!set_initial_card_clock(20U)) return init_fail();
        init_stage_ = initialization_stage::fifo_reset;
        if (!modify(reg_ctrl, 1U << 1U, 0U)
            || !wait_clear(reg_ctrl, 1U << 1U)) return init_fail();
        initialized_ = true;
        init_stage_ = initialization_stage::complete;
        return true;
    }

    // Slot 0's dedicated UHS-capable pads are supplied by ESP32-P4 LDO VO4.
    // The Nano routes that rail to its microSD socket, so board code may call
    // this before pulsing GPIO45 to reproduce the vendor power-reset sequence.
    [[nodiscard]] bool enable_io_power() noexcept {
        if constexpr (!Binding::requires_io_power) return true;
        if (io_power_enabled_) return true;
        constexpr std::uint32_t current_limit = 1U << 27U;
        constexpr std::uint32_t ripple_suppression = 1U << 26U;
        constexpr std::uint32_t mul_mask = 0x7U << 23U;
        constexpr std::uint32_t dref_mask = 0xFU << 28U;
        constexpr std::uint32_t use_3v3_rail = 1U << 14U;
        constexpr std::uint32_t software_owner = 1U << 7U;
        constexpr std::uint32_t source_mask = 0x7U << 9U;
        constexpr std::uint32_t enable = 1U << 8U;

        if (!modify(pmu_ldo4_ana, current_limit, 0U)
            || !modify(pmu_ldo4_control, use_3v3_rail, 0U)
            || !modify(pmu_ldo4_ana, (0x7U << 23U) | (0xBU << 28U),
                       (mul_mask | dref_mask)
                           & ~((0x7U << 23U) | (0xBU << 28U)))
            || !modify(pmu_ldo4_control, software_owner, source_mask)
            || !modify(pmu_ldo4_ana, ripple_suppression, 0U)
            || !modify(pmu_ldo4_control, enable, 0U)
            || !modify(pmu_ldo4_ana, 0U, current_limit)) {
            return false;
        }
        io_power_enabled_ = true;
        return true;
    }

    [[nodiscard]] initialization_stage last_initialization_stage()
        const noexcept {
        return init_stage_;
    }

    [[nodiscard]] std::uint8_t last_command_index() const noexcept {
        return last_command_index_;
    }

    [[nodiscard]] std::uint32_t last_interrupt_status() const noexcept {
        return last_interrupt_status_;
    }

    [[nodiscard]] std::uint32_t last_dma_status() const noexcept {
        return last_dma_status_;
    }

    [[nodiscard]] std::expected<driver::sdmmc_response,
                                driver::sdmmc_error>
    command(driver::sdmmc_command command) noexcept {
        if (!initialized_ && !owned_) {
            return std::unexpected(driver::sdmmc_error::not_initialized);
        }
        return transact(command);
    }

    [[nodiscard]] std::expected<driver::sdmmc_response,
                                driver::sdmmc_error>
    read_data(driver::sdmmc_command command,
              std::span<std::byte, 512> data) noexcept {
        if (!initialized_) {
            return std::unexpected(driver::sdmmc_error::not_initialized);
        }
        const auto buffer = memory_.address(data.data());
        const auto descriptor = memory_.address(&descriptor_);
        if (!buffer || !descriptor) {
            return std::unexpected(driver::sdmmc_error::dma);
        }
        descriptor_ = {
            .control = descriptor_owned | descriptor_first | descriptor_last
                | descriptor_chained,
            .size = 512U,
            .buffer = *buffer,
            .next = 0U,
        };
        if (!memory_.prepare_for_device(&descriptor_, sizeof(descriptor_))
            || !memory_.prepare_for_device(data.data(), data.size())
            || !write(reg_bytcnt, 512U) || !write(reg_blksiz, 512U)
            || !write(reg_dbaddr, *descriptor)
            || !modify(reg_ctrl, (1U << 5U) | (1U << 25U), 0U)
            || !modify(reg_bmod, (1U << 1U) | (1U << 7U), 0U)
            || !write(reg_pldmnd, 1U)) {
            return std::unexpected(driver::sdmmc_error::dma);
        }
        auto response = transact(command, transfer_direction::read);
        if (!response) return response;
        if (!memory_.complete_from_device(data.data(), data.size())) {
            return std::unexpected(driver::sdmmc_error::dma);
        }
        return response;
    }

    [[nodiscard]] std::expected<driver::sdmmc_response,
                                driver::sdmmc_error>
    write_data(driver::sdmmc_command command,
               std::span<const std::byte, 512> data) noexcept {
        if (!initialized_) {
            return std::unexpected(driver::sdmmc_error::not_initialized);
        }
        const auto buffer = memory_.address(data.data());
        const auto descriptor = memory_.address(&descriptor_);
        if (!buffer || !descriptor) {
            return std::unexpected(driver::sdmmc_error::dma);
        }
        descriptor_ = {
            .control = descriptor_owned | descriptor_first | descriptor_last
                | descriptor_chained,
            .size = 512U,
            .buffer = *buffer,
            .next = 0U,
        };
        if (!memory_.prepare_for_device(&descriptor_, sizeof(descriptor_))
            || !memory_.prepare_for_device(data.data(), data.size())
            || !write(reg_bytcnt, 512U) || !write(reg_blksiz, 512U)
            || !write(reg_dbaddr, *descriptor)
            || !modify(reg_ctrl, (1U << 5U) | (1U << 25U), 0U)
            || !modify(reg_bmod, (1U << 1U) | (1U << 7U), 0U)
            || !write(reg_pldmnd, 1U)) {
            return std::unexpected(driver::sdmmc_error::dma);
        }
        return transact(command, transfer_direction::write);
    }

    [[nodiscard]] bool set_bus_width_4() noexcept {
        return initialized_
            && modify(reg_ctype, 1U << Binding::slot,
                      1U << (16U + Binding::slot));
    }

    [[nodiscard]] bool set_transfer_clock() noexcept {
        if (!initialized_) return false;
        return disable_card_clock() && configure_source_clock(8U)
            && write(reg_clkdiv, 0U) && clock_update()
            && enable_card_clock() && write(reg_tmout, 0x1E84'80FFU);
    }

    [[nodiscard]] std::expected<void, driver::sdmmc_error>
    shutdown() noexcept {
        if (!owned_) {
            return std::unexpected(driver::sdmmc_error::not_initialized);
        }
        bool ok = true;
        if (initialized_) {
            ok = disable_card_clock();
        }
        ok = modify(hp_soc_clock_ctrl1, 0U, 1U << 14U) && ok;
        ok = disable_io_power() && ok;
        initialized_ = false;
        release();
        return ok ? std::expected<void, driver::sdmmc_error>{}
                  : std::unexpected(driver::sdmmc_error::mmio);
    }

private:
    struct alignas(64) dma_descriptor {
        std::uint32_t control{};
        std::uint32_t size{};
        std::uint32_t buffer{};
        std::uint32_t next{};
        std::uint32_t reserved[12]{};
    };
    static_assert(sizeof(dma_descriptor) == 64U);

    static constexpr std::uintptr_t reg_ctrl = Binding::base + 0x00U;
    static constexpr std::uintptr_t reg_clkdiv = Binding::base + 0x08U;
    static constexpr std::uintptr_t reg_clksrc = Binding::base + 0x0CU;
    static constexpr std::uintptr_t reg_clkena = Binding::base + 0x10U;
    static constexpr std::uintptr_t reg_tmout = Binding::base + 0x14U;
    static constexpr std::uintptr_t reg_ctype = Binding::base + 0x18U;
    static constexpr std::uintptr_t reg_blksiz = Binding::base + 0x1CU;
    static constexpr std::uintptr_t reg_bytcnt = Binding::base + 0x20U;
    static constexpr std::uintptr_t reg_intmask = Binding::base + 0x24U;
    static constexpr std::uintptr_t reg_cmdarg = Binding::base + 0x28U;
    static constexpr std::uintptr_t reg_cmd = Binding::base + 0x2CU;
    static constexpr std::uintptr_t reg_resp0 = Binding::base + 0x30U;
    static constexpr std::uintptr_t reg_rintsts = Binding::base + 0x44U;
    static constexpr std::uintptr_t reg_bmod = Binding::base + 0x80U;
    static constexpr std::uintptr_t reg_pldmnd = Binding::base + 0x84U;
    static constexpr std::uintptr_t reg_dbaddr = Binding::base + 0x88U;
    static constexpr std::uintptr_t reg_idsts = Binding::base + 0x8CU;
    static constexpr std::uintptr_t reg_idinten = Binding::base + 0x90U;
    static constexpr std::uintptr_t hp_soc_clock_ctrl1 =
        Binding::hp_clock_base + 0x18U;
    static constexpr std::uintptr_t hp_peri_clock_ctrl1 =
        Binding::hp_clock_base + 0x34U;
    static constexpr std::uintptr_t hp_peri_clock_ctrl2 =
        Binding::hp_clock_base + 0x38U;
    static constexpr std::uintptr_t hp_ref_clock_ctrl2 =
        Binding::hp_clock_base + 0x2CU;
    static constexpr std::uintptr_t lp_sdmmc_reset =
        Binding::lp_clock_base + 0x4CU;
    // LDO channel 4 maps to PMU ext_ldo index 4 (P1 0.2 A).
    static constexpr std::uintptr_t pmu_ldo4_control =
        Binding::pmu_base + 0x1D8U;
    static constexpr std::uintptr_t pmu_ldo4_ana =
        Binding::pmu_base + 0x1DCU;

    static constexpr std::uint32_t command_done = 1U << 2U;
    static constexpr std::uint32_t data_over = 1U << 3U;
    static constexpr std::uint32_t response_error = 1U << 1U;
    static constexpr std::uint32_t response_crc_error = 1U << 6U;
    static constexpr std::uint32_t response_timeout = 1U << 8U;
    static constexpr std::uint32_t hardware_locked = 1U << 12U;
    static constexpr std::uint32_t data_crc_error = 1U << 7U;
    static constexpr std::uint32_t data_timeout = 1U << 9U;
    static constexpr std::uint32_t data_errors = data_crc_error | data_timeout
        | (1U << 10U) | (1U << 11U) | (1U << 13U) | (1U << 15U);
    static constexpr std::uint32_t command_errors = response_error
        | response_crc_error | response_timeout | hardware_locked;
    static constexpr std::uint32_t dma_receive_done = 1U << 1U;
    static constexpr std::uint32_t dma_transmit_done = 1U << 0U;
    static constexpr std::uint32_t dma_errors = (1U << 2U) | (1U << 4U)
        | (1U << 5U) | (1U << 9U);
    static constexpr std::uint32_t descriptor_last = 1U << 2U;
    static constexpr std::uint32_t descriptor_first = 1U << 3U;
    static constexpr std::uint32_t descriptor_chained = 1U << 4U;
    static constexpr std::uint32_t descriptor_owned = 1U << 31U;
    static constexpr std::size_t poll_limit = 2'000'000U;

    enum class transfer_direction : std::uint8_t { none, read, write };

    [[nodiscard]] std::expected<driver::sdmmc_response,
                                driver::sdmmc_error>
    transact(driver::sdmmc_command request,
             transfer_direction direction = transfer_direction::none) noexcept {
        last_command_index_ = request.index;
        last_interrupt_status_ = 0U;
        last_dma_status_ = 0U;
        if (!wait_clear(reg_cmd, 1U << 31U)
            || !write(reg_rintsts, 0xFFFF'FFFFU)
            || !write(reg_idsts, 0xFFFF'FFFFU)
            || !write(reg_cmdarg, request.argument)) {
            return std::unexpected(driver::sdmmc_error::mmio);
        }
        auto encoded = static_cast<std::uint32_t>(request.index) | (1U << 29U)
            | (static_cast<std::uint32_t>(Binding::slot) << 16U)
            | (1U << 31U);
        if (request.index != 0U) encoded |= 1U << 13U;
        if (request.send_initial_clocks) encoded |= 1U << 15U;
        if (request.response != driver::sdmmc_response_type::none) {
            encoded |= 1U << 6U;
        }
        if (request.response == driver::sdmmc_response_type::short_crc
            || request.response == driver::sdmmc_response_type::long_crc) {
            encoded |= 1U << 8U;
        }
        if (request.response == driver::sdmmc_response_type::long_crc) {
            encoded |= 1U << 7U;
        }
        if (direction != transfer_direction::none) encoded |= 1U << 9U;
        if (direction == transfer_direction::write) encoded |= 1U << 10U;
        if (!write(reg_cmd, encoded) || !wait_clear(reg_cmd, 1U << 31U)) {
            return std::unexpected(driver::sdmmc_error::command_timeout);
        }

        bool command_complete{};
        bool data_complete = direction == transfer_direction::none;
        bool dma_complete = direction == transfer_direction::none;
        for (std::size_t count = 0U; count < poll_limit; ++count) {
            const auto interrupt = read(reg_rintsts);
            const auto dma = read(reg_idsts);
            if (!interrupt || !dma) {
                return std::unexpected(driver::sdmmc_error::mmio);
            }
            last_interrupt_status_ = *interrupt;
            last_dma_status_ = *dma;
            if ((*interrupt & response_timeout) != 0U) {
                return clear_and_fail(driver::sdmmc_error::command_timeout,
                                      *interrupt, *dma);
            }
            if ((*interrupt & response_crc_error) != 0U) {
                return clear_and_fail(driver::sdmmc_error::response_crc,
                                      *interrupt, *dma);
            }
            if ((*interrupt & (response_error | hardware_locked)) != 0U) {
                return clear_and_fail(driver::sdmmc_error::invalid_response,
                                      *interrupt, *dma);
            }
            if ((*interrupt & data_timeout) != 0U) {
                return clear_and_fail(driver::sdmmc_error::data_timeout,
                                      *interrupt, *dma);
            }
            if ((*interrupt & (data_errors & ~data_timeout)) != 0U) {
                return clear_and_fail(driver::sdmmc_error::data_crc,
                                      *interrupt, *dma);
            }
            if ((*dma & dma_errors) != 0U) {
                return clear_and_fail(driver::sdmmc_error::dma,
                                      *interrupt, *dma);
            }
            command_complete = command_complete
                || ((*interrupt & command_done) != 0U);
            data_complete = data_complete || ((*interrupt & data_over) != 0U);
            const auto dma_done = direction == transfer_direction::write
                ? dma_transmit_done : dma_receive_done;
            dma_complete = dma_complete || ((*dma & dma_done) != 0U);
            if (command_complete && data_complete && dma_complete) {
                static_cast<void>(write(reg_rintsts, *interrupt));
                static_cast<void>(write(reg_idsts, *dma));
                driver::sdmmc_response response{};
                if (request.response != driver::sdmmc_response_type::none) {
                    const auto words = request.response
                            == driver::sdmmc_response_type::long_crc
                        ? 4U : 1U;
                    for (std::size_t index = 0U; index < words; ++index) {
                        const auto word = read(reg_resp0 + index * 4U);
                        if (!word) {
                            return std::unexpected(driver::sdmmc_error::mmio);
                        }
                        response.words[index] = *word;
                    }
                }
                return response;
            }
        }
        return std::unexpected(direction != transfer_direction::none
                ? driver::sdmmc_error::data_timeout
                : driver::sdmmc_error::command_timeout);
    }

    [[nodiscard]] std::expected<driver::sdmmc_response,
                                driver::sdmmc_error>
    clear_and_fail(driver::sdmmc_error error, std::uint32_t interrupts,
                   std::uint32_t dma) noexcept {
        static_cast<void>(write(reg_rintsts, interrupts));
        static_cast<void>(write(reg_idsts, dma));
        return std::unexpected(error);
    }

    [[nodiscard]] bool pulse_module_reset() noexcept {
        return modify(lp_sdmmc_reset, 1U << 28U, 0U)
            && modify(lp_sdmmc_reset, 0U, 1U << 28U);
    }

    [[nodiscard]] bool reset_controller() noexcept {
        constexpr std::uint32_t controller_and_dma_reset =
            (1U << 0U) | (1U << 2U);
        return modify(reg_ctrl, controller_and_dma_reset, 0U)
            && wait_clear(reg_ctrl, controller_and_dma_reset);
    }

    [[nodiscard]] bool configure_source_clock(std::uint32_t divider) noexcept {
        if (divider < 2U || divider > 16U) return false;
        if (!modify(hp_peri_clock_ctrl1, 1U << 24U,
                    (1U << 22U) | (1U << 23U))) return false;
        constexpr std::uint32_t controlled = (0xFU << 9U) | (0xFU << 13U)
            | (0xFU << 17U) | (0x3U << 21U) | (0x3U << 23U)
            | (0x3U << 25U) | (0x7U << 27U);
        const auto value = ((divider - 1U) << 9U)
            | ((divider / 2U - 1U) << 13U)
            | ((divider - 1U) << 17U) | (1U << 23U)
            | (0x7U << 27U);
        if (!modify(hp_peri_clock_ctrl2, value, controlled & ~value)
            || !modify(hp_peri_clock_ctrl2, 1U << 8U, 0U)
            || !modify(hp_peri_clock_ctrl2, 0U, 1U << 8U)) return false;
        return true;
    }

    [[nodiscard]] bool set_card_clock(std::uint32_t divider) noexcept {
        return disable_card_clock() && write(reg_clksrc, 0U)
            && write(reg_clkdiv, divider) && clock_update()
            && enable_card_clock() && write(reg_tmout, 0x00FA'00FFU);
    }

    [[nodiscard]] bool set_initial_card_clock(
        std::uint32_t divider) noexcept {
        return write(reg_clkena, 0U) && write(reg_clksrc, 0U)
            && write(reg_clkdiv, divider)
            && write(reg_clkena, card_clock_mask())
            && clock_update() && write(reg_tmout, 0x00FA'00FFU);
    }

    [[nodiscard]] bool disable_card_clock() noexcept {
        return modify(reg_clkena, 0U, card_clock_mask())
            && clock_update();
    }

    [[nodiscard]] bool enable_card_clock() noexcept {
        return modify(reg_clkena, card_clock_mask(), 0U)
            && clock_update();
    }

    [[nodiscard]] bool clock_update() noexcept {
        if (!wait_clear(reg_cmd, 1U << 31U)) return false;
        constexpr std::uint32_t update = (1U << 13U) | (1U << 21U)
            | (1U << 29U) | (1U << 31U);
        return write(reg_cmdarg, 0U) && write(reg_cmd, update)
            && wait_clear(reg_cmd, 1U << 31U);
    }

    [[nodiscard]] bool configure_pins() noexcept {
        if constexpr (Binding::uses_gpio_matrix) {
            return configure_matrix_pin(Binding::clock,
                                        Binding::clock_signal, true, false,
                                        false)
                && configure_matrix_pin(Binding::command,
                                        Binding::command_signal, true, true,
                                        true)
                && configure_matrix_pin(Binding::d0, Binding::d0_signal,
                                        true, true, true)
                && configure_matrix_pin(Binding::d1, Binding::d1_signal,
                                        true, true, true)
                && configure_matrix_pin(Binding::d2, Binding::d2_signal,
                                        true, true, true)
                && configure_matrix_pin(Binding::d3, Binding::d3_signal,
                                        true, true, true);
        }
        return configure_pin(Binding::clock, false)
            && configure_pin(Binding::command, true)
            && configure_pin(Binding::d0, true)
            && configure_pin(Binding::d1, true)
            && configure_pin(Binding::d2, true)
            && configure_pin(Binding::d3, true);
    }

    [[nodiscard]] bool configure_pin(std::uint8_t pin, bool pull_up) noexcept {
        const auto address = mcu::esp32p4::io_mux_base + 0x04U + pin * 4U;
        const auto current = read(address);
        if (!current) return false;
        constexpr std::uint32_t controlled = (1U << 7U) | (1U << 8U)
            | (1U << 9U) | (0x3U << 10U) | (0x7U << 12U);
        const auto next = (*current & ~controlled) | (1U << 9U)
            | (0x3U << 10U) | (pull_up ? 1U << 8U : 0U);
        return write(address, next);
    }

    [[nodiscard]] bool configure_matrix_pin(std::uint8_t pin,
                                            std::uint16_t signal,
                                            bool output, bool input,
                                            bool pull_up) noexcept {
        const auto pad = mcu::esp32p4::io_mux_base + 0x04U + pin * 4U;
        const auto output_select = mcu::esp32p4::gpio_matrix_base + 0x558U
            + static_cast<std::uintptr_t>(pin) * 4U;
        const auto input_select = mcu::esp32p4::gpio_matrix_base + 0x158U
            + static_cast<std::uintptr_t>(signal) * 4U;
        const auto current_pad = read(pad);
        if (!current_pad) return false;
        constexpr std::uint32_t pad_controlled = (1U << 7U) | (1U << 8U)
            | (1U << 9U) | (0x3U << 10U) | (0x7U << 12U);
        auto next_pad = static_cast<std::uint32_t>(
            (*current_pad & ~pad_controlled) | (0x3U << 10U)
            | (1U << 12U));
        if (input) next_pad |= 1U << 9U;
        if (pull_up) next_pad |= 1U << 8U;
        if (output) {
            const auto current_output = read(output_select);
            if (!current_output
                || !write(output_select,
                          (*current_output & ~0x7FFU) | signal)) {
                return false;
            }
        }
        if (input) {
            const auto current_input = read(input_select);
            if (!current_input
                || !write(input_select,
                          (*current_input & ~0xFFU) | pin | (1U << 7U))) {
                return false;
            }
        }
        return write(pad, next_pad);
    }

    [[nodiscard]] static constexpr std::uint32_t
    card_clock_mask() noexcept {
        return (1U << Binding::slot) | (1U << (16U + Binding::slot));
    }

    [[nodiscard]] std::expected<std::uint32_t, core::mmio_error>
    read(std::uintptr_t address) noexcept {
        return mmio_.template read<std::uint32_t>(address);
    }

    [[nodiscard]] bool write(std::uintptr_t address,
                             std::uint32_t value) noexcept {
        const auto result = mmio_.template write<std::uint32_t>(address, value);
        if (!result) return false;
        arch::data_memory_barrier();
        return true;
    }

    [[nodiscard]] bool modify(std::uintptr_t address, std::uint32_t set,
                              std::uint32_t clear) noexcept {
        const auto current = read(address);
        return current && write(address, (*current | set) & ~clear);
    }

    [[nodiscard]] bool wait_clear(std::uintptr_t address,
                                  std::uint32_t mask) noexcept {
        for (std::size_t count = 0U; count < poll_limit; ++count) {
            const auto value = read(address);
            if (!value) return false;
            if ((*value & mask) == 0U) return true;
        }
        return false;
    }

    void release() noexcept {
        if (owned_) detail::sdmmc_owner::claimed = false;
        owned_ = false;
    }

    [[nodiscard]] bool init_fail() noexcept {
        static_cast<void>(disable_io_power());
        release();
        return false;
    }

    [[nodiscard]] bool disable_io_power() noexcept {
        if constexpr (!Binding::requires_io_power) return true;
        if (!io_power_enabled_) return true;
        const auto disabled = modify(pmu_ldo4_control, 0U, 1U << 8U);
        if (disabled) io_power_enabled_ = false;
        return disabled;
    }

    MmioPolicy& mmio_;
    MemoryPolicy& memory_;
    dma_descriptor descriptor_{};
    initialization_stage init_stage_{initialization_stage::none};
    std::uint32_t last_interrupt_status_{};
    std::uint32_t last_dma_status_{};
    std::uint8_t last_command_index_{};
    bool initialized_{};
    bool owned_{};
    bool io_power_enabled_{};
};

} // namespace mcuhw::backend::esp32p4
