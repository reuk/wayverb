#include "boundaries.h"

#include <cmath>
#include <algorithm>

using namespace std;

CuboidBoundary::CuboidBoundary(const Vec3f & c0, const Vec3f & c1)
        : c0(c0)
        , c1(c1) {
}

bool CuboidBoundary::inside(const Vec3f & v) const {
    return (c0 < v).all() && (v < c1).all();
}

SphereBoundary::SphereBoundary(const Vec3f & c, float radius)
        : c(c)
        , radius(radius) {
}

bool SphereBoundary::inside(const Vec3f & v) const {
    return (v - c).mag() < radius;
}
