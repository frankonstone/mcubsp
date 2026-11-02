#include <array>
#include <mcutest/mcutest.hpp>
#include <mcubsp/backend/nrf52840/dma_buffers.hpp>
#include <mcubsp/backend/nrf52840/i2c.hpp>
#include <mcubsp/backend/nrf52840/system_context.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_din.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_dout.hpp>
#include "fake_system.hpp"
#include <mcubsp/execution/fake_policy.hpp>
#include "fake_twi.hpp"

namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
using p = peripheral;
using constraints = mcu::nrf52840::dma_constraints;
constexpr auto bytes = mcu::nrf52840::serial_dma_constraints<p::uarte0>;
static auto quiet() noexcept -> std::expected<bool, coordination_error> { return true; }
static auto running() noexcept -> std::expected<bool, coordination_error> { return false; }
struct fixture {
    execution::fake_policy execution;
    resource_registry resources;
    unsigned first{}, second{};
};
TEST(nrf_resources, all_documented_serial_personalities_use_physical_identities, "REQ-0062") {
    fixture f;
    const std::array aliases{
        std::array{p::uart0,p::uarte0}, std::array{p::spi0,p::spim0},
        std::array{p::spi0,p::spis0}, std::array{p::spi0,p::twi0},
        std::array{p::spi0,p::twim0}, std::array{p::spi0,p::twis0},
        std::array{p::spi1,p::spim1}, std::array{p::spi1,p::spis1},
        std::array{p::spi1,p::twi1}, std::array{p::spi1,p::twim1}, std::array{p::spi1,p::twis1},
        std::array{p::spi2,p::spim2}, std::array{p::spi2,p::spis2},
        std::array{p::comp,p::lpcomp}, std::array{p::aar,p::ccm},
        std::array{p::egu0,p::swi0}, std::array{p::egu5,p::swi5}};
    for (auto pair : aliases) {
        auto a = f.resources.claim(f.execution,pair[0],&f.first); ASSERT_TRUE(a.has_value());
        auto b = f.resources.claim(f.execution,pair[1],&f.second); ASSERT_FALSE(b.has_value());
        EXPECT_TRUE(b.error() == coordination_error::busy);
        ASSERT_TRUE(f.resources.release(f.execution,*a).has_value());
        b = f.resources.claim(f.execution,pair[1],&f.second); ASSERT_TRUE(b.has_value());
        ASSERT_TRUE(f.resources.release(f.execution,*b).has_value());
    }
}
TEST(nrf_resources, same_address_controls_and_distinct_serial_instances_remain_independent, "REQ-0062") {
    fixture f;
    std::array<resource_token,11> tokens{};
    const std::array ids{p::power,p::clock,p::approtect,p::nvmc,p::acl,p::mwu,
                          p::spim0,p::spim1,p::spim2,p::spim3,p::uarte1};
    for (std::size_t i=0;i<ids.size();++i) {
        auto t=f.resources.claim(f.execution,ids[i],&f.first); ASSERT_TRUE(t.has_value());tokens[i]=*t;
    }
    auto guard=f.resources.claim(f.execution,p::flash_mutation,&f.first); ASSERT_TRUE(guard.has_value());
    EXPECT_FALSE(f.resources.claim(f.execution,p::flash_mutation,&f.second).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*guard).has_value());
    for(auto t:tokens) ASSERT_TRUE(f.resources.release(f.execution,t).has_value());
}
TEST(nrf_resources, pin_claims_are_atomic_across_independent_peripherals, "REQ-0062") {
    fixture f;
    const std::array pins{core::pin_id{0,6},core::pin_id{0,8}};
    auto a=f.resources.claim(f.execution,p::uarte0,&f.first,pins); ASSERT_TRUE(a.has_value());
    EXPECT_FALSE(f.resources.claim(f.execution,p::twim1,&f.second,pins).has_value());
    const std::array alternate{core::pin_id{0,11},core::pin_id{1,0}};
    auto b=f.resources.claim(f.execution,p::twim1,&f.second,alternate); ASSERT_TRUE(b.has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*b).has_value());
    const std::array duplicate{core::pin_id{0,12},core::pin_id{0,12}};
    EXPECT_FALSE(f.resources.claim(f.execution,p::twim1,&f.second,duplicate).has_value());
    const std::array reserved{core::pin_id{0,18}};
    EXPECT_FALSE(f.resources.claim(f.execution,p::twim1,&f.second,reserved).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*a).has_value());
    b=f.resources.claim(f.execution,p::twim1,&f.second,pins); ASSERT_TRUE(b.has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*b).has_value());
}
TEST(nrf_resources, stale_foreign_and_isr_claims_cannot_release_or_reconfigure_new_owner, "REQ-0062") {
    fixture f;resource_registry other;
    auto old=f.resources.claim(f.execution,p::uart0,&f.first);ASSERT_TRUE(old.has_value());
    EXPECT_FALSE(other.release(f.execution,*old).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*old).has_value());
    auto current=f.resources.claim(f.execution,p::uarte0,&f.second);ASSERT_TRUE(current.has_value());
    EXPECT_FALSE(f.resources.release(f.execution,*old).has_value());
    f.execution.set_isr(true);
    EXPECT_FALSE(f.resources.claim(f.execution,p::uarte1,&f.first).has_value());
    EXPECT_FALSE(f.resources.release(f.execution,*current).has_value());
    ASSERT_TRUE(f.resources.retain(f.execution,*current).has_value());
    ASSERT_TRUE(f.resources.drop(f.execution,*current).has_value());
    f.execution.set_isr(false);
    ASSERT_TRUE(f.resources.release(f.execution,*current).has_value());
}
TEST(nrf_dma, checks_data_ram_capacity_alignment_count_units_and_overflow_before_handoff, "REQ-0062") {
    EXPECT_TRUE(bytes.accepts(0x20000001,65535,65535));
    EXPECT_FALSE(bytes.accepts(0x20000000,65536,65536));
    EXPECT_FALSE(bytes.accepts(0x20000000,4,5));
    EXPECT_FALSE(bytes.accepts(0x20000000,4,0));
    EXPECT_FALSE(bytes.accepts(0x00800000,4,4));
    EXPECT_FALSE(bytes.accepts(0x26000,4,4));
    EXPECT_FALSE(bytes.accepts(0x2003FFFF,2,2));
    const constraints words{16383,4,4};
    EXPECT_TRUE(words.accepts(0x20000000,16,4));
    EXPECT_FALSE(words.accepts(0x20000001,16,4));
    EXPECT_FALSE(words.accepts(0x20000000,15,4));
    EXPECT_FALSE((constraints{10,0,1}.accepts(0x20000000,10,1)));
    EXPECT_FALSE((constraints{10,1,3}.accepts(0x20000000,10,1)));
    EXPECT_FALSE((constraints{std::size_t(-1),std::size_t(-1),1}.accepts(0x20000000,std::size_t(-1),2)));
}
TEST(nrf_dma, cryptocell_memory_dma_has_its_own_count_start_and_busy_contract, "REQ-0055", "REQ-0062") {
    using crypto=mcu::nrf52840::cryptocell_memory_dma;
    static_assert(!crypto::uses_easydma);
    static_assert(crypto::count_bits==30);
    namespace din=mcu::nrf52840::product::cc_din;
    namespace dout=mcu::nrf52840::product::cc_dout;
    static_assert(din::src_mem_size::access==core::access_semantics::command);
    EXPECT_EQ(din::src_mem_size::writable_mask,0x3FFFFFFFU);
    EXPECT_EQ(dout::dst_mem_size::writable_mask,0x3FFFFFFFU);
    const auto constraints=crypto::constraints(4);
    EXPECT_FALSE(constraints.accepts(0x20000000,65536,65536));
    EXPECT_TRUE(constraints.accepts(0x20000000,65535,65535));
    EXPECT_FALSE(bytes.accepts(0x20000000,65536,65536));
    EXPECT_FALSE(constraints.accepts(0x20000001,16,16));
    fixture f;nrf52840_test::fake_system mmio;
    dma_buffer_pool<execution::fake_policy,1> pool{f.resources,f.execution};
    auto owner=f.resources.claim(f.execution,p::cryptocell,&f.first);ASSERT_TRUE(owner.has_value());
    auto t=pool.prepare(*owner,constraints,0x20000000,16,16,dma_access::read);ASSERT_TRUE(t.has_value());
    ASSERT_TRUE(pool.arm(*t).has_value()); // BEFORE a SIZE write that may commit
    core::register_ref<din::src_mem_size,decltype(mmio)> start{mmio,din::base};
    EXPECT_FALSE(start.issue(0x80000010).has_value());
    ASSERT_TRUE(start.issue(16).has_value());
    mmio.poke(din::base+din::din_dma_mem_busy::offset,1);
    auto stopped=[&]()->std::expected<bool,coordination_error>{
        auto busy=mmio.read<std::uint32_t>(din::base+din::din_dma_mem_busy::offset);
        if (!busy) return std::unexpected(coordination_error::faulted);
        return (*busy&1)==0;
    };
    EXPECT_FALSE(pool.recover(*t,stopped).has_value());
    EXPECT_FALSE(f.resources.release(f.execution,*owner).has_value());
    mmio.poke(din::base+din::din_dma_mem_busy::offset,0);
    ASSERT_TRUE(pool.recover(*t,stopped).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_dma, overlaps_are_checked_and_readers_can_share_without_unbounded_queueing, "REQ-0062") {
    fixture f; dma_buffer_pool<execution::fake_policy,2> pool{f.resources,f.execution};
    auto a=f.resources.claim(f.execution,p::uarte0,&f.first);ASSERT_TRUE(a.has_value());
    auto b=f.resources.claim(f.execution,p::spim1,&f.second);ASSERT_TRUE(b.has_value());
    auto read=pool.prepare(*a,bytes,0x20000000,8,8,dma_access::read);ASSERT_TRUE(read.has_value());
    EXPECT_FALSE(pool.prepare(*b,bytes,0x20000004,8,8,dma_access::write).has_value());
    auto shared=pool.prepare(*b,bytes,0x20000004,8,8,dma_access::read);ASSERT_TRUE(shared.has_value());
    auto full=pool.prepare(*b,bytes,0x20000020,8,8,dma_access::read);ASSERT_FALSE(full.has_value());
    EXPECT_TRUE(full.error()==coordination_error::exhausted);
    EXPECT_FALSE(f.resources.release(f.execution,*a).has_value());
    ASSERT_TRUE(pool.discard_prepared(*read).has_value());
    ASSERT_TRUE(pool.discard_prepared(*shared).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*a).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*b).has_value());
}
TEST(nrf_dma, final_peripheral_quiescence_is_required_even_after_partial_dma_completion, "REQ-0062") {
    fixture f;dma_buffer_pool<execution::fake_policy,1> pool{f.resources,f.execution};
    auto owner=f.resources.claim(f.execution,p::uarte0,&f.first);ASSERT_TRUE(owner.has_value());
    auto t=pool.prepare(*owner,bytes,0x20000000,8,8,dma_access::write);ASSERT_TRUE(t.has_value());
    ASSERT_TRUE(pool.arm(*t).has_value());
    EXPECT_FALSE(pool.discard_prepared(*t).has_value());
    EXPECT_FALSE(pool.finish(*t,3,running).has_value());
    EXPECT_FALSE(f.resources.release(f.execution,*owner).has_value());
    ASSERT_TRUE(pool.finish(*t,3,quiet).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_dma, a_second_pool_cannot_bypass_the_mcus_buffer_overlap_domain, "REQ-0062") {
    fixture f;dma_buffer_pool<execution::fake_policy,1> second{f.resources,f.execution};
    auto owner=f.resources.claim(f.execution,p::uarte0,&f.first);ASSERT_TRUE(owner.has_value());
    {
        dma_buffer_pool<execution::fake_policy,1> first{f.resources,f.execution};
        auto a=first.prepare(*owner,bytes,0x20000000,8,8,dma_access::write);ASSERT_TRUE(a.has_value());
        EXPECT_FALSE(second.prepare(*owner,bytes,0x20000000,8,8,dma_access::write).has_value());
        ASSERT_TRUE(first.discard_prepared(*a).has_value());
    }
    auto b=second.prepare(*owner,bytes,0x20000000,8,8,dma_access::write);ASSERT_TRUE(b.has_value());
    ASSERT_TRUE(second.discard_prepared(*b).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_dma, over_count_and_failed_stop_retain_memory_until_explicit_recovery, "REQ-0062") {
    fixture f;dma_buffer_pool<execution::fake_policy,1> pool{f.resources,f.execution};
    auto owner=f.resources.claim(f.execution,p::twim0,&f.first);ASSERT_TRUE(owner.has_value());
    auto t=pool.prepare(*owner,bytes,0x20000000,8,8,dma_access::write);ASSERT_TRUE(t.has_value());
    ASSERT_TRUE(pool.arm(*t).has_value());
    EXPECT_FALSE(pool.finish(*t,9,quiet).has_value());
    EXPECT_FALSE(pool.finish(*t,8,quiet).has_value());
    EXPECT_FALSE(pool.recover(*t,[]()->std::expected<bool,coordination_error>{return std::unexpected(coordination_error::faulted);}).has_value());
    EXPECT_FALSE(pool.recover(*t,running).has_value());
    EXPECT_FALSE(f.resources.release(f.execution,*owner).has_value());
    ASSERT_TRUE(pool.recover(*t,quiet).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_dma, late_completion_cannot_retire_a_reused_buffer_slot, "REQ-0062") {
    fixture f;dma_buffer_pool<execution::fake_policy,1> pool{f.resources,f.execution};
    auto owner=f.resources.claim(f.execution,p::spim3,&f.first);ASSERT_TRUE(owner.has_value());
    auto old=pool.prepare(*owner,bytes,0x20000000,8,8,dma_access::read);ASSERT_TRUE(old.has_value());
    ASSERT_TRUE(pool.arm(*old).has_value());ASSERT_TRUE(pool.finish(*old,8,quiet).has_value());
    auto current=pool.prepare(*owner,bytes,0x20000020,8,8,dma_access::read);ASSERT_TRUE(current.has_value());
    ASSERT_TRUE(pool.arm(*current).has_value());
    bool called{};
    EXPECT_FALSE(pool.finish(*old,8,[&]()->std::expected<bool,coordination_error>{called=true;return true;}).has_value());
    EXPECT_FALSE(called);EXPECT_TRUE(pool.valid(*current));
    ASSERT_TRUE(pool.finish(*current,8,quiet).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_dma, abandoning_live_pool_reports_contract_failure_and_keeps_owner_claimed, "REQ-0062") {
    fixture f;auto owner=f.resources.claim(f.execution,p::uarte1,&f.first);ASSERT_TRUE(owner.has_value());
    {
        dma_buffer_pool<execution::fake_policy,1> pool{f.resources,f.execution};
        auto t=pool.prepare(*owner,bytes,0x20000000,8,8,dma_access::write);ASSERT_TRUE(t.has_value());
        ASSERT_TRUE(pool.arm(*t).has_value());
    }
    EXPECT_EQ(f.execution.destruction_violations(),1U);
    EXPECT_FALSE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_dma, reentrant_observation_cannot_retire_a_new_generation, "REQ-0062") {
    fixture f;dma_buffer_pool<execution::fake_policy,1> pool{f.resources,f.execution};
    auto owner=f.resources.claim(f.execution,p::spim0,&f.first);ASSERT_TRUE(owner.has_value());
    auto old=pool.prepare(*owner,bytes,0x20000000,8,8,dma_access::read);ASSERT_TRUE(old.has_value());
    ASSERT_TRUE(pool.arm(*old).has_value());dma_buffer_token current{};
    auto result=pool.finish(*old,8,[&]()->std::expected<bool,coordination_error>{
        if (!pool.recover(*old,quiet)) return std::unexpected(coordination_error::faulted);
        auto next=pool.prepare(*owner,bytes,0x20000020,8,8,dma_access::read);
        if (!next) return std::unexpected(next.error());
        current=*next;return true;
    });
    EXPECT_FALSE(result.has_value());EXPECT_TRUE(pool.valid(current));
    EXPECT_FALSE(f.resources.release(f.execution,*owner).has_value());
    ASSERT_TRUE(pool.discard_prepared(current).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*owner).has_value());
}
TEST(nrf_resources, legacy_twi_and_future_dma_personalities_share_the_same_registry, "REQ-0054", "REQ-0062") {
    fixture f;nrf52840_test::fake_twi mmio;
    using binding=backend::nrf52840::i2c_binding<0,{0,11},{1,0}>;
    i2c_controller_backend<binding,decltype(mmio),execution::fake_policy> bus{mmio,f.execution,f.resources};
    constexpr auto timing=solve_i2c_timing({16'000'000,100'000,0,300,100});
    auto serial=f.resources.claim(f.execution,p::spim0,&f.first);ASSERT_TRUE(serial.has_value());
    EXPECT_FALSE(bus.initialize(*timing).has_value());EXPECT_EQ(mmio.write_count,0U);
    ASSERT_TRUE(f.resources.release(f.execution,*serial).has_value());
    ASSERT_TRUE(bus.initialize(*timing).has_value());
    EXPECT_FALSE(f.resources.claim(f.execution,p::twis0,&f.first).has_value());
    ASSERT_TRUE(bus.shutdown().has_value());
    serial=f.resources.claim(f.execution,p::spim0,&f.first);ASSERT_TRUE(serial.has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*serial).has_value());
}
TEST(nrf_resources, native_gpio_and_serial_pin_claims_exclude_one_another, "REQ-0062") {
    fixture f;nrf52840_test::fake_system mmio;
    gpio_backend gpio{mmio,f.resources};
    ASSERT_TRUE(gpio.claim({0,6},driver::gpio_direction::output).has_value());
    const std::array pins{core::pin_id{0,6},core::pin_id{0,8}};
    EXPECT_FALSE(f.resources.claim(f.execution,p::uarte0,&f.first,pins).has_value());
    gpio.release({0,6});
    auto serial=f.resources.claim(f.execution,p::uarte0,&f.first,pins);ASSERT_TRUE(serial.has_value());
    EXPECT_FALSE(gpio.claim({0,8},driver::gpio_direction::input).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*serial).has_value());
    ASSERT_TRUE(gpio.claim({0,8},driver::gpio_direction::input).has_value());gpio.release({0,8});
}
TEST(nrf_resources, existing_system_context_bridges_native_personalities_and_clock_controls, "REQ-0062") {
    fixture f;nrf52840_test::fake_system mmio;gpio_backend gpio{mmio,f.resources};
    system_context context{mmio,f.execution,gpio};
    ASSERT_TRUE(context.claim(77,endpoint_domain::aar).has_value());
    EXPECT_FALSE(f.resources.claim(f.execution,p::ccm,&f.first).has_value());
    ASSERT_TRUE(context.release(77).has_value());
    auto ccm=f.resources.claim(f.execution,p::ccm,&f.first);ASSERT_TRUE(ccm.has_value());
    EXPECT_FALSE(context.claim(77,endpoint_domain::aar).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*ccm).has_value());
    ASSERT_TRUE(context.claim(61).has_value());
    auto power=f.resources.claim(f.execution,p::power,&f.first);ASSERT_TRUE(power.has_value());
    EXPECT_FALSE(f.resources.claim(f.execution,p::clock,&f.second).has_value());
    ASSERT_TRUE(f.resources.release(f.execution,*power).has_value());
    ASSERT_TRUE(context.release(61).has_value());
}
} // namespace
