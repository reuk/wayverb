#pragma once

#include "common/boundaries.h"

namespace waveguide {

box<3> compute_adjusted_boundary(const box<3>& min_boundary,
                                 const glm::vec3& anchor,
                                 float cube_side);

}  // namespace waveguide
