#pragma once

#include "vec.h"
#include "cl_structs.h"

struct Boundary {
    virtual bool inside(const Vec3f & v) const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(const Vec3f & c0, const Vec3f & c1);
    bool inside(const Vec3f & v) const override;
    Vec3f c0, c1;
};

struct SphereBoundary : public Boundary {
    SphereBoundary(const Vec3f & c, float radius);
    bool inside(const Vec3f & v) const override;
    Vec3f c;
    float radius;
};
