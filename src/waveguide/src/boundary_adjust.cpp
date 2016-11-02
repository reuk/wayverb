#include "waveguide/boundary_adjust.h"

#include <cassert>

namespace waveguide {

core::geo::box compute_adjusted_boundary(const core::geo::box& min_boundary,
                                         const glm::vec3& anchor,
                                         float cube_side) {
    const auto dif = anchor - min_boundary.get_min();
    const glm::ivec3 ceiled{glm::ceil(dif / cube_side)};
    const auto extra = 1;
    const auto c0 = anchor - (glm::vec3{ceiled + 1 + extra} * cube_side);
    assert(glm::all(glm::lessThan(c0, min_boundary.get_min() - cube_side)));
    const auto dim =
            glm::ivec3{glm::ceil((min_boundary.get_max() - c0) / cube_side)} +
            extra + 1;
    const auto c1 = c0 + glm::vec3{dim} * cube_side;
    assert(glm::all(glm::lessThan(min_boundary.get_max() + cube_side, c1)));
    return {c0, c1};
}

}  // namespace waveguide
