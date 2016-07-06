#include "waveguide/config.h"

#include <cmath>

#include "samplerate.h"

#define DIM 3

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

std::vector<float> adjust_sampling_rate(std::vector<float>&& w_results,
                                        double in_sr,
                                        double out_sr) {
    std::vector<float> out_signal(out_sr * w_results.size() / in_sr);

    SRC_DATA sample_rate_info{w_results.data(),
                              out_signal.data(),
                              long(w_results.size()),
                              long(out_signal.size()),
                              0,
                              0,
                              0,
                              out_sr / in_sr};

    src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);
    return out_signal;
}
