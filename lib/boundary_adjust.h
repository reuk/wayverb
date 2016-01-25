#pragma once

#include "vec.h"
#include "boundaries.h"

CuboidBoundary get_adjusted_boundary(const CuboidBoundary& min_boundary,
                                     const Vec3f& anchor,
                                     float cube_side);
