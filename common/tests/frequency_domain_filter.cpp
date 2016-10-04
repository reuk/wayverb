#include "common/frequency_domain_filter.h"
#include "common/aligned/vector.h"
#include "common/decibels.h"
#include "common/dsp_vector_ops.h"
#include "common/hrtf_utils.h"
#include "common/stl_wrappers.h"
#include "common/write_audio_file.h"

#include "gtest/gtest.h"

#include <random>

TEST(fast_filter, do_nothing) {
    for (const auto &sig :
         {aligned::vector<float>{0},
          aligned::vector<float>{1},
          aligned::vector<float>{1, 0},
          aligned::vector<float>{1, 0, 0, 0, 0, 0, 0, 0},
          aligned::vector<float>{1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1}}) {
        fast_filter filter{sig.size()};

        auto output{sig};
        proc::fill(output, 0);
        filter.filter(sig.begin(),
                      sig.end(),
                      output.begin(),
                      [](auto cplx, auto freq) { return cplx; });

        for (auto i{0ul}, end{sig.size()}; i != end; ++i) {
            ASSERT_NEAR(sig[i], output[i], 0.00001);
        }
    }
}

TEST(fast_filter, reduce_magnitude) {
    for (const auto &sig :
         {aligned::vector<float>{0},
          aligned::vector<float>{1},
          aligned::vector<float>{1, 0},
          aligned::vector<float>{1, 0, 0, 0, 0, 0, 0, 0},
          aligned::vector<float>{1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1}}) {
        fast_filter filter{sig.size()};

        const auto magnitude{0.3f};

        auto output{sig};
        proc::fill(output, 0);
        filter.filter(sig.begin(),
                      sig.end(),
                      output.begin(),
                      [=](auto cplx, auto freq) { return cplx * magnitude; });

        for (auto i{0ul}, end{sig.size()}; i != end; ++i) {
            ASSERT_NEAR(sig[i] * magnitude, output[i], 0.00001);
        }
    }
}

TEST(fast_filter, multiband_filter) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distribution(-1, 1);

    aligned::vector<volume_type> multiband;
    for (auto i{0ul}; i != 44100 * 10; ++i) {
        multiband.emplace_back(volume_type{
                {distribution(engine) * decibels::db2a(-60.0f * 0 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 1 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 2 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 3 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 4 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 5 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 6 / 8.0f),
                 distribution(engine) * decibels::db2a(-60.0f * 7 / 8.0f)}});
    }

    multiband[44100] = make_volume_type(1);
    multiband[88200] = make_volume_type(1);
    multiband[132300] = make_volume_type(1);

    const auto results{multiband_filter_and_mixdown(multiband, 44100)};

    snd::write("multiband_noise.wav", {results}, 44100, 16);
}

TEST(fast_filter, band_edges) {
    {
        const auto centre{0.25};
        const auto P{0.1};

        for (auto l{0u}; l != 4; ++l) {
            for (auto p{-P}, end{P}; p < end; p += 0.02) {
                const auto lower{lower_band_edge(centre, p, P, l)};
                const auto upper{upper_band_edge(centre, p, P, l)};
                std::cout << "lower: " << lower << ", upper: " << upper << '\n';
                ASSERT_NEAR(lower * lower + upper * upper, 1, 0.000001) << p;
            }
        }
    }
}

TEST(fast_filter, lopass) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distribution(-1, 1);

    aligned::vector<float> sig;
    for (auto i{0ul}; i != 44100 * 10; ++i) {
        sig.emplace_back(distribution(engine));
    }

    fast_filter filter{sig.size()};
    filter.filter(
            sig.begin(), sig.end(), sig.begin(), [](auto cplx, auto freq) {
                return cplx * static_cast<float>(compute_lopass_magnitude(
                                      freq, 0.25, 0.05, 0));
            });
    snd::write("lopass_noise.wav", {sig}, 44100, 16);
}

TEST(fast_filter, hipass) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distribution(-1, 1);

    aligned::vector<float> sig;
    for (auto i{0ul}; i != 44100 * 10; ++i) {
        sig.emplace_back(distribution(engine));
    }

    fast_filter filter{sig.size()};
    filter.filter(
            sig.begin(), sig.end(), sig.begin(), [](auto cplx, auto freq) {
                return cplx * static_cast<float>(compute_hipass_magnitude(
                                      freq, 0.25, 0.05, 0));
            });
    snd::write("hipass_noise.wav", {sig}, 44100, 16);
}

TEST(fast_filter, transients) {
    aligned::vector<float> sig(1 << 13);
    sig[1 << 12] = 1.0f;

    fast_filter filter{sig.size()};
    filter.filter(
            sig.begin(), sig.end(), sig.begin(), [](auto cplx, auto freq) {
                return cplx * static_cast<float>(compute_hipass_magnitude(
                                      freq, 0.25, 0.05, 0));
            });
    snd::write("transients.wav", {sig}, 44100, 16);
}
