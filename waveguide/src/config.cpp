#include "waveguide/config.h"

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

float Waveguide::get_max_frequency() const {
    return filter_frequency * oversample_ratio;
}

float Waveguide::get_waveguide_sample_rate() const {
    return get_max_frequency() * 4;
}

float Waveguide::get_divisions() const {
    return grid_spacing(SPEED_OF_SOUND, 1 / get_waveguide_sample_rate());
}
}

std::vector<float> adjust_sampling_rate(std::vector<float>& w_results,
                                        const config::Waveguide& cc) {
    std::vector<float> out_signal(cc.sample_rate * w_results.size() /
                                  cc.get_waveguide_sample_rate());

    SRC_DATA sample_rate_info{
            w_results.data(),
            out_signal.data(),
            long(w_results.size()),
            long(out_signal.size()),
            0,
            0,
            0,
            cc.sample_rate / double(cc.get_waveguide_sample_rate())};

    src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);
    return out_signal;
}
