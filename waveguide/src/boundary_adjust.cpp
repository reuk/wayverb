#include "waveguide/boundary_adjust.h"

#include <cassert>

CuboidBoundary compute_adjusted_boundary(const CuboidBoundary& min_boundary,
                                         const Vec3f& anchor,
                                         float cube_side) {
    auto dif = anchor - min_boundary.get_c0();
    Vec3i ceiled((dif / cube_side).ceil());
    auto extra = 1;
    auto c0 = anchor - ((ceiled + 1 + extra) * cube_side);
    assert((c0 < (min_boundary.get_c0() - cube_side)).all());
    Vec3i dim(((min_boundary.get_c1() - c0) / cube_side).ceil() + extra + 1);
    auto c1 = c0 + dim * cube_side;
    assert((c1 > (min_boundary.get_c1() + cube_side)).all());
    return CuboidBoundary(c0, c1);
}
