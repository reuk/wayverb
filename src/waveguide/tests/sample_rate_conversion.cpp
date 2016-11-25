#include "waveguide/config.h"

#include "audio_file/audio_file.h"

#include "utilities/string_builder.h"

#include "gtest/gtest.h"

namespace {

template <typename T>
void scale_and_write(double scale,
                     const char* fname,
                     const T& data,
                     double sample_rate) {
    auto copy{data};
    for (auto& i : copy) {
        i *= scale;
    }

    audio_file::write(
            util::build_string(sample_rate, '.', fname, ".wav").c_str(),
            copy,
            sample_rate,
            audio_file::format::wav,
            audio_file::bit_depth::pcm16);
}

}  // namespace

TEST(sample_rate_conversion, convert) {
    std::vector<float> input(1000, 0.0);
    input[200] = 1.0;
    const auto input_sr = 1000;
    const auto output_sr = 2000;

    const auto scale = 0.1;

    scale_and_write(scale, "impulse", input, input_sr);

    const auto output = wayverb::waveguide::adjust_sampling_rate(
            input, input_sr, output_sr);

    scale_and_write(scale, "impulse", output, output_sr);
}
