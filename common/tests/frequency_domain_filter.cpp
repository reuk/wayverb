#include "common/frequency_domain_filter.h"
#include "common/aligned/vector.h"
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
        multiband.emplace_back(volume_type{{distribution(engine) * 8 / 8.0f,
                                            distribution(engine) * 7 / 8.0f,
                                            distribution(engine) * 6 / 8.0f,
                                            distribution(engine) * 5 / 8.0f,
                                            distribution(engine) * 4 / 8.0f,
                                            distribution(engine) * 3 / 8.0f,
                                            distribution(engine) * 2 / 8.0f,
                                            distribution(engine) * 1 / 8.0f}});
    }

    const auto results{multiband_filter_and_mixdown(multiband, 44100)};

    snd::write("multiband_noise.wav", {results}, 44100, 16);
}
