#pragma once

#include "common/boundaries.h"

namespace waveguide {

box compute_adjusted_boundary(const box& min_boundary,
                              const glm::vec3& anchor,
                              float cube_side);

}  // namespace waveguide
