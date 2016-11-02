#include "waveguide/pcs.h"

#include "gtest/gtest.h"

TEST(pcs, factdbl) {
    ASSERT_EQ(waveguide::factdbl(-7), 1);
    ASSERT_EQ(waveguide::factdbl(-6), 1);
    ASSERT_EQ(waveguide::factdbl(-5), 1);
    ASSERT_EQ(waveguide::factdbl(-4), 1);
    ASSERT_EQ(waveguide::factdbl(-3), 1);
    ASSERT_EQ(waveguide::factdbl(-2), 1);
    ASSERT_EQ(waveguide::factdbl(-1), 1);
    ASSERT_EQ(waveguide::factdbl(0), 1);
    ASSERT_EQ(waveguide::factdbl(1), 1);
    ASSERT_EQ(waveguide::factdbl(2), 2);
    ASSERT_EQ(waveguide::factdbl(3), 3);
    ASSERT_EQ(waveguide::factdbl(4), 8);
    ASSERT_EQ(waveguide::factdbl(5), 15);
    ASSERT_EQ(waveguide::factdbl(6), 48);
    ASSERT_EQ(waveguide::factdbl(7), 105);
}

TEST(pcs, maxflat) {
    for (const auto& pair :
         util::aligned::vector<std::tuple<waveguide::offset_signal,
                                          util::aligned::vector<double>>>{
                 {waveguide::maxflat(0.1, 4, 1, 1024),
                  util::aligned::vector<double>{-0.00580,
                                                -0.01272,
                                                0.00000,
                                                0.08268,
                                                0.28443,
                                                0.58864,
                                                0.87895,
                                                1.00000,
                                                0.87895,
                                                0.58864,
                                                0.28443,
                                                0.08268,
                                                0.00000,
                                                -0.01272,
                                                -0.00580}},

                 {waveguide::maxflat(0.1, 8, 1, 1024),
                  util::aligned::vector<double>{
                          0.00000,  0.00004,  0.00026,  0.00081,  0.00135,
                          -0.00000, -0.00712, -0.02341, -0.04446, -0.04931,
                          0.00000,  0.14119,  0.38036,  0.66779,  0.90673,
                          1.00000,  0.90673,  0.66779,  0.38036,  0.14119,
                          0.00000,  -0.04931, -0.04446, -0.02341, -0.007123,
                          -0.00000, 0.00135,  0.00081,  0.00026,  0.00004,
                          0.00000}},

                 {waveguide::maxflat(0.01, 16, 1, 1024),
                  util::aligned::vector<double>{
                          0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000,
                          0.00001, 0.00003, 0.00007, 0.00018, 0.00041, 0.00090,
                          0.00188, 0.00373, 0.00707, 0.01279, 0.02218, 0.03689,
                          0.05894, 0.09056, 0.13399, 0.19107, 0.26282, 0.34896,
                          0.44752, 0.55463, 0.66458, 0.77019, 0.86354, 0.93692,
                          0.98385, 1.00000, 0.98385, 0.93692, 0.86354, 0.77019,
                          0.66458, 0.55463, 0.44752, 0.34896, 0.26282, 0.19107,
                          0.13399, 0.09056, 0.05894, 0.03689, 0.02218, 0.01279,
                          0.00707, 0.00373, 0.00188, 0.00090, 0.00041, 0.00018,
                          0.00007, 0.00003, 0.00001, 0.00000, 0.00000, 0.00000,
                          0.00000, 0.00000, 0.00000}},

         }) {
        for (auto i{0ul},
             end{std::min(std::get<0>(pair).signal.size(),
                          std::get<1>(pair).size())};
             i != end;
             ++i) {
            ASSERT_NEAR(
                    std::get<0>(pair).signal[i], std::get<1>(pair)[i], 0.00001);
        }
    }
}

TEST(pcs, g0) {
    ASSERT_NEAR(
            waveguide::compute_g0(400, 340, 44100, 0.05), 0.92259, 0.000001);
}

TEST(pcs, mech_sphere) {
    for (const auto& pair :
         util::aligned::vector<std::tuple<core::filter::biquad::coefficients,
                                          core::filter::biquad::coefficients>>{

                 {waveguide::mech_sphere(0.025, 0.003, 0.7, 1.0 / 16000),
                  core::filter::biquad::coefficients{
                          0.0012333, 0.00000, -0.0012333, -1.9731, 0.97343}},

                 {waveguide::mech_sphere(0.025, 0.003, 0.7, 1.0 / 10000),
                  core::filter::biquad::coefficients{
                          0.00197, 0.00000, -0.00197, -1.97308, 0.97343}},

                 {waveguide::mech_sphere(0.0025, 0.006, 1.5, 1.0 / 10000),
                  core::filter::biquad::coefficients{
                          0.01975, 0.00000, -0.01975, -1.97378, 0.97518}},

                 {waveguide::mech_sphere(0.03, 0.01, 2, 1.0 / 10000),
                  core::filter::biquad::coefficients{
                          0.00164, 0.00000, -0.00164, -1.96520, 0.96909}},

         }) {
        ASSERT_NEAR(std::get<0>(pair).b0, std::get<1>(pair).b0, 0.0001);
        ASSERT_NEAR(std::get<0>(pair).b1, std::get<1>(pair).b1, 0.0001);
        ASSERT_NEAR(std::get<0>(pair).b2, std::get<1>(pair).b2, 0.0001);
        ASSERT_NEAR(std::get<0>(pair).a1, std::get<1>(pair).a1, 0.0001);
        ASSERT_NEAR(std::get<0>(pair).a2, std::get<1>(pair).a2, 0.0001);
    }
}
