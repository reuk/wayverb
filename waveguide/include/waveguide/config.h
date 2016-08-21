#pragma once

#include "common/aligned/vector.h"

#include <vector>

namespace waveguide {
namespace config {

double speed_of_sound(double time_step, double grid_spacing);
double time_step(double speed_of_sound, double grid_spacing);
double grid_spacing(double speed_of_sound, double time_step);

}  // namespace config

aligned::vector<float> adjust_sampling_rate(aligned::vector<float> &&w_results,
                                            double in_sr,
                                            double out_sr);

}  // namespace waveguide
