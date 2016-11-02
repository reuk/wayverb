#include "waveguide/config.h"

#include <cmath>

#include "samplerate.h"

namespace {
constexpr auto DIM = 3;
}  // namespace

namespace waveguide {
namespace config {

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

util::aligned::vector<float> adjust_sampling_rate(const float* begin,
                                                  const float* end,
                                                  double in_sr,
                                                  double out_sr) {
    const auto input_size = std::distance(begin, end);
    util::aligned::vector<float> out_signal(out_sr * input_size / in_sr);
    SRC_DATA sample_rate_info{begin,
                              out_signal.data(),
                              input_size,
                              static_cast<long>(out_signal.size()),
                              0,
                              0,
                              0,
                              out_sr / in_sr};
    src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);
    return out_signal;
}

}  // namespace waveguide
