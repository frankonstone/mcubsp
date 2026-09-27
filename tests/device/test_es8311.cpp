#include <mcutest/mcutest.hpp>
#include <mcuhw/device/audio/es8311.hpp>

namespace {

TEST(es8311, exposes_bounded_48khz_analog_mic_and_dac_sequence,
     "REQ-0011", "REQ-0024", "REQ-0034") {
    constexpr auto sequence = mcuhw::device::es8311::startup_sequence_48khz_16bit();
    static_assert(sequence.size() == 38U);
    EXPECT_TRUE((sequence.front()
                 == mcuhw::device::es8311::register_write{0x0DU, 0xFAU}));
    EXPECT_TRUE((sequence.back()
                 == mcuhw::device::es8311::register_write{0x45U, 0x00U}));
    EXPECT_EQ(mcuhw::device::es8311::conservative_output_volume.address,
              0x32U);
    EXPECT_TRUE(mcuhw::device::es8311::conservative_output_volume.value
                < 0xBFU);
    EXPECT_TRUE((mcuhw::device::es8311::unity_output_volume
                 == mcuhw::device::es8311::register_write{0x32U, 0xBFU}));
}

} // namespace
