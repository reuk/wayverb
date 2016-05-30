#pragma once

#include "common/boundaries.h"

CuboidBoundary compute_adjusted_boundary(const CuboidBoundary& min_boundary,
                                         const glm::vec3& anchor,
                                         float cube_side);
