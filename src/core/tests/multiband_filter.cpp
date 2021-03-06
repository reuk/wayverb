#include "core/cl/iterator.h"
#include "core/cl/scene_structs.h"
#include "core/dsp_vector_ops.h"
#include "core/mixdown.h"

#include "utilities/aligned/vector.h"
#include "utilities/decibels.h"

#include "frequency_domain/filter.h"
#include "frequency_domain/multiband_filter.h"

#include "audio_file/audio_file.h"

#include "gtest/gtest.h"

#include <random>

using namespace wayverb::core;

template <typename Engine, size_t... Ix>
auto multiband_sample(Engine& engine, std::index_sequence<Ix...>) {
    std::uniform_real_distribution<float> distribution(-1, 1);
    return bands_type{{distribution(engine) *
                       util::decibels::db2a(-60.0f * Ix / 8.0f)...}};
}

template <typename Engine>
auto multiband_sample(Engine& engine) {
    return multiband_sample(engine,
                            std::make_index_sequence<simulation_bands>{});
}

TEST(multiband_filter, multiband_filter) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distribution(-1, 1);

    util::aligned::vector<bands_type> multiband;
    for (auto i{0ul}; i != 44100 * 10; ++i) {
        multiband.emplace_back(multiband_sample(engine));
    }

    multiband[44100] = make_bands_type(1);
    multiband[88200] = make_bands_type(1);
    multiband[132300] = make_bands_type(1);

    const auto sample_rate{44100.0};

    const auto results = multiband_filter_and_mixdown(
            multiband.begin(),
            multiband.end(),
            sample_rate,
            [](auto it, auto index) {
                return make_cl_type_iterator(std::move(it), index);
            });

    write("multiband_noise.wav",
          results,
          sample_rate,
          audio_file::format::wav,
          audio_file::bit_depth::pcm16);
}

