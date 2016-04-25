#pragma once

#include "common/boundaries.h"
#include "common/vec.h"

CuboidBoundary compute_adjusted_boundary(const CuboidBoundary& min_boundary,
                                         const Vec3f& anchor,
                                         float cube_side);
