#include "common/cl/iterator.h"
#include "common/cl/scene_structs.h"
#include "common/dsp_vector_ops.h"
#include "common/mixdown.h"

#include "utilities/aligned/vector.h"
#include "utilities/decibels.h"

#include "frequency_domain/filter.h"
#include "frequency_domain/multiband_filter.h"

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

    const auto sample_rate{44100.0};

    const range<double> nrange{20 / sample_rate, 20000 / sample_rate};

    constexpr auto bands{detail::components_v<volume_type>};

    const auto results{multiband_filter_and_mixdown<bands>(
            multiband.begin(),
            multiband.end(),
            nrange,
            [](auto it, auto index) {
                return make_cl_type_iterator(std::move(it), index);
            })};

    write("multiband_noise.wav",
          audio_file::make_audio_file(results, sample_rate),
          16);
}

