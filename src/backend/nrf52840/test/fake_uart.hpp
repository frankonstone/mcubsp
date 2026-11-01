#pragma once
#include "fake_system.hpp"
#include <mcubsp/execution/fake_policy.hpp>
#include <span>
namespace nrf52840_test {
class fake_uart : public fake_system {
  public:
    static constexpr std::uintptr_t base=0x40002000;
    bool automatic_tx{},loopback{},cts_active{true},stop_responds{true},bad_rx_read_order{};
    bool error_during_ack{};
    mcubsp::execution::fake_policy* clock{};
    template<class T> std::expected<T,mcubsp::core::mmio_error> read(std::uintptr_t address) noexcept {
        if (clock) clock->advance();
        auto result=fake_system::read<T>(address);
        if (!result) return result;
        if (address==base+0x518) {
            if (peek(base+0x108)) bad_rx_read_order=true;
            if (head_<tail_) {
                result=static_cast<T>(fifo_[head_++]);
                if (head_<tail_) poke(base+0x108,1);
            }
        }
        return result;
    }
    template<class T> std::expected<void,mcubsp::core::mmio_error> write(std::uintptr_t address,T value) noexcept {
        const auto before=peek(address);
        auto result=fake_system::write<T>(address,value);
        if (!result && !fail_after_commit) return result;
        if (address==base+0x480) poke(address,before&~static_cast<std::uint32_t>(value)); // ERRORSRC W1C
        if (address==base+0x124 && value==0 && error_during_ack) {
            error_during_ack=false;poke(base+0x480,peek(base+0x480)|4U);poke(base+0x124,1);
        }
        if (address==base && value) receiving_=true;
        if (address==base+4 && value && stop_responds) {receiving_=false;poke(base+0x144,1);}
        if (address==base+0x51C) {
            if (automatic_tx && cts_active) poke(base+0x11C,1);
            if (loopback && receiving_) inject(static_cast<std::uint8_t>(value));
        }
        return result;
    }
    void inject(std::uint8_t value) noexcept {
        if (tail_==fifo_.size() && head_==tail_) head_=tail_=0;
        if (tail_<fifo_.size()) {fifo_[tail_++]=value;poke(base+0x108,1);}
    }
    void inject(std::span<const std::uint8_t> bytes) noexcept {for(auto value:bytes) inject(value);}
  private:
    std::array<std::uint8_t,32> fifo_{};
    std::size_t head_{},tail_{};bool receiving_{};
};
} // namespace nrf52840_test
