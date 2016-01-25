#include "boundary_adjust.h"

CuboidBoundary get_adjusted_boundary(const CuboidBoundary& min_boundary,
                                     const Vec3f& anchor,
                                     float cube_side) {
    auto dif = anchor - min_boundary.get_c0();
    Vec3i ceiled = (dif / cube_side).map([](auto i) { return ceil(i); });
    auto extra = 1;
    auto c0 = anchor - ((ceiled + 1 + extra) * cube_side);
    Vec3i dim = ((min_boundary.get_c1() - c0) / cube_side)
                    .map([](auto i) { return ceil(i); }) +
                extra;
    auto c1 = c0 + dim * cube_side;
    return CuboidBoundary(c0, c1);
}
