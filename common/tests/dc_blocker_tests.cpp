#include "common/dc_blocker.h"
#include "common/dsp_vector_ops.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <random>

TEST(dc_blocker, delay_line) {
    constexpr auto LENGTH = 4;

    filter::delay_line dl(LENGTH);

    for (auto i = 0; i != LENGTH; ++i) {
        ASSERT_EQ(dl[i], 0);
    }

    dl.push(1);
    ASSERT_EQ(dl[0], 1);

    dl.push(2);
    ASSERT_EQ(dl[0], 2);
    ASSERT_EQ(dl[1], 1);

    dl.push(3);
    ASSERT_EQ(dl[0], 3);
    ASSERT_EQ(dl[1], 2);
    ASSERT_EQ(dl[2], 1);

    dl.push(0);
    dl.push(0);
    dl.push(0);
    dl.push(0);
    ASSERT_EQ(dl[0], 0);
    ASSERT_EQ(dl[1], 0);
    ASSERT_EQ(dl[2], 0);
}

TEST(dc_blocker, moving_average) {
    filter::moving_average ma(4);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);

    ASSERT_EQ(ma.filter(1), 1 / 4.0);
    ASSERT_EQ(ma.filter(0), 1 / 4.0);
    ASSERT_EQ(ma.filter(0), 1 / 4.0);
    ASSERT_EQ(ma.filter(0), 1 / 4.0);

    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
}

TEST(dc_blocker, two_moving_average) {
    filter::n_moving_averages<2> ma(4);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);

    ASSERT_EQ(ma.filter(1), 1 / 16.0);
    ASSERT_EQ(ma.filter(0), 2 / 16.0);
    ASSERT_EQ(ma.filter(0), 3 / 16.0);
    ASSERT_EQ(ma.filter(0), 4 / 16.0);

    ASSERT_EQ(ma.filter(0), 3 / 16.0);
    ASSERT_EQ(ma.filter(0), 2 / 16.0);
    ASSERT_EQ(ma.filter(0), 1 / 16.0);
    ASSERT_EQ(ma.filter(0), 0 / 16.0);

    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
    ASSERT_EQ(ma.filter(0), 0);
}

TEST(dc_blocker, dc_blocker) {
    filter::linear_dc_blocker dc(4);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);

    ASSERT_EQ(dc.filter(1), -1 / 16.0);
    ASSERT_EQ(dc.filter(0), -2 / 16.0);
    ASSERT_EQ(dc.filter(0), -3 / 16.0);
    ASSERT_EQ(dc.filter(0), 12 / 16.0);

    ASSERT_EQ(dc.filter(0), -3 / 16.0);
    ASSERT_EQ(dc.filter(0), -2 / 16.0);
    ASSERT_EQ(dc.filter(0), -1 / 16.0);
    ASSERT_EQ(dc.filter(0), -0 / 16.0);

    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
    ASSERT_EQ(dc.filter(0), 0);
}

TEST(dc_blocker, big_offset) {
    filter::linear_dc_blocker dc(4);
    ASSERT_EQ(dc.filter(2), -2 / 16.0);
    ASSERT_EQ(dc.filter(2), -6 / 16.0);
    ASSERT_EQ(dc.filter(2), -12 / 16.0);
    ASSERT_EQ(dc.filter(2), 12 / 16.0);
    ASSERT_EQ(dc.filter(2), 6 / 16.0);
    ASSERT_EQ(dc.filter(2), 2 / 16.0);
}

aligned::vector<float> generate_noise(size_t samples) {
    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist{-1, 1};
    aligned::vector<float> ret(samples);
    std::generate(
            ret.begin(), ret.end(), [&engine, &dist] { return dist(engine); });
    return ret;
}

aligned::vector<float> generate_sweep(size_t samples) {
    double phase{0};
    aligned::vector<float> ret(samples);
    for (auto i = 0u; i != samples; ++i) {
        ret[i] = std::sin(phase * 2 * M_PI);
        phase += (i * 0.5) / samples;
        phase = std::fmod(phase, 1);
    }
    return ret;
}

aligned::vector<float> generate_impulse(size_t samples) {
    aligned::vector<float> ret(samples, 0);
    ret[ret.size() / 2] = 1;
    return ret;
}

TEST(dc_blocker, io) {
    struct signal {
        std::string name;
        aligned::vector<float> kernel;
    };

    constexpr auto samples = 1000000;

    aligned::vector<signal> signals{
            signal{"noise", generate_noise(samples)},
            signal{"sweep", generate_sweep(samples)},
            signal{"impulse", generate_impulse(samples)},
    };

    for (const auto& i : signals) {
        snd::write(build_string("dc_test.input.", i.name, ".wav"),
                   {i.kernel},
                   44100,
                   16);

        auto run{[&i](auto& filter, const auto& filter_name, auto i) {
            filter::run_two_pass(filter, i.kernel.begin(), i.kernel.end());
            normalize(i.kernel);
            snd::write(build_string("dc_test.output.",
                                    filter_name,
                                    ".",
                                    i.name,
                                    ".wav"),
                       {i.kernel},
                       44100,
                       16);
        }};

        {
            filter::linear_dc_blocker dc;
            run(dc, "normal", i);
        }
        {
            filter::extra_linear_dc_blocker dc;
            run(dc, "super", i);
        }
    }
}
