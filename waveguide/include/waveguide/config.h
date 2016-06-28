#pragma once

#include "common/config.h"

#include <vector>

namespace config {

double speed_of_sound(double time_step, double grid_spacing);
double time_step(double speed_of_sound, double grid_spacing);
double grid_spacing(double speed_of_sound, double time_step);

}  // namespace config

std::vector<float> adjust_sampling_rate(std::vector<float> &&w_results,
                                        double in_sr,
                                        double out_sr);
