#include "frequency_domain/multiband_filter.h"

#include "utilities/map_to_vector.h"
#include "utilities/reduce.h"
#include "utilities/string_builder.h"

#include "audio_file/audio_file.h"

#include "gtest/gtest.h"

#include <random>

TEST(frequency_domain, widths) {
    constexpr auto sample_rate{44100.0};
    constexpr range<double> audible_range{20, 20000};

    const auto params{frequency_domain::band_edges_and_widths<8>(
            audible_range / sample_rate, 1)};

    for (auto i{0ul}; i != params.size() - 1; ++i) {
        const auto diff = (params[i].edge + params[i].width) -
                          (params[i + 1].edge - params[i + 1].width);
        ASSERT_NEAR(diff, 0, 0.000000001);
    }
}

TEST(frequency_domain, reconstruction) {
    constexpr auto sample_rate{5000.0};
    constexpr range<double> audible_range{20, 20000};

    aligned::vector<float> noise{};
    {
        std::default_random_engine engine{std::random_device{}()};
        constexpr auto amp{0.5};
        std::uniform_real_distribution<float> dist{-amp, amp};

        for (auto i{0ul}; i != sample_rate * 2; ++i) {
            noise.emplace_back(dist(engine));
        }
    }

    write("input_noise.wav",
          audio_file::make_audio_file(noise, sample_rate),
          16);

    auto multiband_noise{
            frequency_domain::make_multiband<8>(begin(noise), end(noise))};

    const auto params{frequency_domain::band_edges_and_widths<8>(
            audible_range / sample_rate, 1.0)};

    frequency_domain::multiband_filter(
            begin(multiband_noise),
            end(multiband_noise),
            params,
            frequency_domain::make_indexer_iterator{});

    for (auto band{0ul}; band != 8; ++band) {
        const auto audio{map_to_vector(begin(multiband_noise),
                                       end(multiband_noise),
                                       frequency_domain::indexer{band})};
        write(build_string("band_", band, ".wav"),
              audio_file::make_audio_file(audio, sample_rate),
              16);
    }

    auto mixed{map_to_vector(begin(multiband_noise),
                             end(multiband_noise),
                             [](auto i) { return reduce(std::plus<>{}, i); })};

    write("multiband_filtered_noise.wav",
          audio_file::make_audio_file(mixed, sample_rate),
          16);
}
