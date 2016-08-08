#include "waveguide/boundary_adjust.h"

#include <cassert>

namespace waveguide {

box<3> compute_adjusted_boundary(const box<3>& min_boundary,
                                 const glm::vec3& anchor,
                                 float cube_side) {
    const auto dif = anchor - min_boundary.get_c0();
    const glm::ivec3 ceiled(glm::ceil(dif / cube_side));
    const auto extra = 1;
    const auto c0 = anchor - (glm::vec3(ceiled + 1 + extra) * cube_side);
    assert(glm::all(glm::lessThan(c0, min_boundary.get_c0() - cube_side)));
    const auto dim =
            glm::ivec3(glm::ceil((min_boundary.get_c1() - c0) / cube_side)) +
            extra + 1;
    const auto c1 = c0 + glm::vec3(dim) * cube_side;
    assert(glm::all(glm::lessThan(min_boundary.get_c1() + cube_side, c1)));
    return box<3>(c0, c1);
}

}  // namespace waveguide
