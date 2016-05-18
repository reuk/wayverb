#pragma once

#include "common/config.h"

namespace config {

double speed_of_sound(double time_step, double grid_spacing);
double time_step(double speed_of_sound, double grid_spacing);
double grid_spacing(double speed_of_sound, double time_step);

class Waveguide : public virtual App {
public:
    float get_max_frequency() const;
    float get_waveguide_sample_rate() const;
    float get_divisions() const;

    float filter_frequency{500};
    float oversample_ratio{2};
};

}  // namespace config

std::vector<float> adjust_sampling_rate(std::vector<float> &w_results,
                                        const config::Waveguide &cc);
