#pragma once

#include "common/boundaries.h"

box compute_adjusted_boundary(const box& min_boundary,
                              const glm::vec3& anchor,
                              float cube_side);
