#pragma once

#include "utilities/aligned/vector.h"

#include <vector>

namespace wayverb {
namespace waveguide {
namespace config {

double speed_of_sound(double time_step, double grid_spacing);
double time_step(double speed_of_sound, double grid_spacing);
double grid_spacing(double speed_of_sound, double time_step);

}  // namespace config

util::aligned::vector<float> adjust_sampling_rate(const float* data,
                                                  size_t size,
                                                  double in_sr,
                                                  double out_sr);

}  // namespace waveguide
}  // namespace wayverb
