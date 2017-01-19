#include "waveguide/config.h"

#include <cmath>

#include "samplerate.h"

namespace wayverb {
namespace waveguide {
namespace config {

namespace {
constexpr auto DIM = 3;
}  // namespace

double speed_of_sound(double time_step, double grid_spacing) {
    return grid_spacing / (time_step * std::sqrt(DIM));
}

double time_step(double speed_of_sound, double grid_spacing) {
    return grid_spacing / (speed_of_sound * std::sqrt(DIM));
}

double grid_spacing(double speed_of_sound, double time_step) {
    return speed_of_sound * time_step * std::sqrt(DIM);
}

}  // namespace config

util::aligned::vector<float> adjust_sampling_rate(const float* data,
                                                  size_t size,
                                                  double in_sr,
                                                  double out_sr) {
    if (!(in_sr && out_sr)) {
        throw std::runtime_error{
                "Sample rate of 0 gives few hints about how to proceed."};
    }
    const auto ratio = out_sr / in_sr;
    util::aligned::vector<float> out_signal(ratio * size);
    SRC_DATA sample_rate_info{data,
                              out_signal.data(),
                              static_cast<long>(size),
                              static_cast<long>(out_signal.size()),
                              0,
                              0,
                              0,
                              ratio};
    src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

    //  Correct output level.
    const auto volume_scale = 1 / ratio;
    for (auto& i : out_signal) {
        i *= volume_scale;
    }

    return out_signal;
}

}  // namespace waveguide
}  // namespace wayverb
