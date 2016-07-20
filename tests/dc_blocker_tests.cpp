#include "common/dc_blocker.h"
#include "common/write_audio_file.h"
#include "common/string_builder.h"
#include "common/dsp_vector_ops.h"

#include "gtest/gtest.h"

#include <random>
#include <algorithm>

TEST(dc_blocker, delay_line) {
    constexpr auto LENGTH = 4;

    filter::DelayLine dl(LENGTH);

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
    filter::MovingAverage ma(4);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);

    ASSERT_EQ(ma(1), 1 / 4.0);
    ASSERT_EQ(ma(0), 1 / 4.0);
    ASSERT_EQ(ma(0), 1 / 4.0);
    ASSERT_EQ(ma(0), 1 / 4.0);

    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
}

TEST(dc_blocker, two_moving_average) {
    filter::NMovingAverages<2> ma(4);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);

    ASSERT_EQ(ma(1), 1 / 16.0);
    ASSERT_EQ(ma(0), 2 / 16.0);
    ASSERT_EQ(ma(0), 3 / 16.0);
    ASSERT_EQ(ma(0), 4 / 16.0);

    ASSERT_EQ(ma(0), 3 / 16.0);
    ASSERT_EQ(ma(0), 2 / 16.0);
    ASSERT_EQ(ma(0), 1 / 16.0);
    ASSERT_EQ(ma(0), 0 / 16.0);

    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
}

TEST(dc_blocker, dc_blocker) {
    filter::LinearDCBlocker dc(4);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);

    ASSERT_EQ(dc(1), -1 / 16.0);
    ASSERT_EQ(dc(0), -2 / 16.0);
    ASSERT_EQ(dc(0), -3 / 16.0);
    ASSERT_EQ(dc(0), 12 / 16.0);

    ASSERT_EQ(dc(0), -3 / 16.0);
    ASSERT_EQ(dc(0), -2 / 16.0);
    ASSERT_EQ(dc(0), -1 / 16.0);
    ASSERT_EQ(dc(0), -0 / 16.0);

    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
}

TEST(dc_blocker, big_offset) {
    filter::LinearDCBlocker dc(4);
    ASSERT_EQ(dc(2), -2 / 16.0);
    ASSERT_EQ(dc(2), -6 / 16.0);
    ASSERT_EQ(dc(2), -12 / 16.0);
    ASSERT_EQ(dc(2), 12 / 16.0);
    ASSERT_EQ(dc(2), 6 / 16.0);
    ASSERT_EQ(dc(2), 2 / 16.0);
}

aligned::vector<float> generate_noise(size_t samples) {
    std::default_random_engine engine{std::random_device()()};
    std::uniform_real_distribution<float> dist{-1, 1};
    aligned::vector<float> ret(samples);
    std::generate(ret.begin(), ret.end(), [&engine, &dist] {
        return dist(engine);
    });
    return ret;
}

aligned::vector<float> generate_sweep(size_t samples) {
    double phase {0};
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

    for (const auto& i: signals) {
        snd::write(
                build_string(
                        "dc_test.input.", i.name, ".wav"),
                {i.kernel},
                44100,
                16);

        auto run = [&i](auto& filter, const auto& filter_name) {

            auto output = filter::run_two_pass(
                    filter, i.kernel.begin(), i.kernel.end());
            normalize(output);
            snd::write(build_string("dc_test.output.",
                                    filter_name,
                                    ".",
                                    i.name,
                                    ".wav"),
                       {output},
                       44100,
                       16);
        };

        {
            filter::LinearDCBlocker dc;
            run(dc, "normal");
        }
        {
            filter::ExtraLinearDCBlocker dc;
            run(dc, "super");
        }
    }
}
