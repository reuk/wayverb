#pragma once

#include "core/geo/box.h"

namespace wayverb {
namespace waveguide {

core::geo::box compute_adjusted_boundary(const core::geo::box& min_boundary,
                                         const glm::vec3& anchor,
                                         float cube_side);

}  // namespace waveguide
}  // namespace wayverb
