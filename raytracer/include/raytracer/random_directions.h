#pragma once

#include "common/aligned/vector.h"
#include "common/azimuth_elevation.h"
#include "common/conversions.h"

#include <random>

namespace raytracer {

aligned::vector<cl_float3> get_random_directions(size_t num);

}  // namespace raytracer
