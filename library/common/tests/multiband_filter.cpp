#include "common/dsp_vector_ops.h"
#include "common/hrtf_utils.h"

#include "utilities/aligned/vector.h"
#include "utilities/decibels.h"

#include "frequency_domain/filter.h"

#include "audio_file/audio_file.h"

#include "gtest/gtest.h"

#include <random>

TEST(multiband_filter, multiband_filter) {
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

    write("multiband_noise.wav",
          audio_file::make_audio_file(results, 44100),
          16);
}

