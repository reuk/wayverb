#pragma once

#include "vec.h"
#include "cl_structs.h"

struct Boundary {
    virtual bool inside(const Vec3f & v) const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(const Vec3f & c0 = Vec3f(), const Vec3f & c1 = Vec3f());
    bool inside(const Vec3f & v) const override;
    Vec3f c0, c1;
};

struct SphereBoundary : public Boundary {
    SphereBoundary(const Vec3f & c = Vec3f(), float radius = 0);
    bool inside(const Vec3f & v) const override;
    Vec3f c;
    float radius;
};
