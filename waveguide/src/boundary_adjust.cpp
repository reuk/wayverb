#include "waveguide/boundary_adjust.h"

#include <cassert>

namespace waveguide {

box compute_adjusted_boundary(const box& min_boundary,
                              const glm::vec3& anchor,
                              float cube_side) {
    auto dif = anchor - min_boundary.get_c0();
    glm::ivec3 ceiled(glm::ceil(dif / cube_side));
    auto extra = 1;
    auto c0 = anchor - (glm::vec3(ceiled + 1 + extra) * cube_side);
    assert(glm::all(glm::lessThan(c0, min_boundary.get_c0() - cube_side)));
    auto dim = glm::ivec3(glm::ceil((min_boundary.get_c1() - c0) / cube_side)) +
               extra + 1;
    auto c1 = c0 + glm::vec3(dim) * cube_side;
    assert(glm::all(glm::lessThan(min_boundary.get_c1() + cube_side, c1)));
    return box(c0, c1);
}

}  // namespace waveguide
