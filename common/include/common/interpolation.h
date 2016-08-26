#pragma once

#include "common/cl/scene_structs.h"
#include <array>

/// This is rubbish but I don't have time to write a proper cubic spline
/// interpolation.
/// y0 and y1 are interpolation values, x is between 0 and 1
float cosine_interpolation(float y0, float y1, float x);
std::array<float, 128> smooth_energy_bands(const volume_type& v,
                                           double sample_rate);
