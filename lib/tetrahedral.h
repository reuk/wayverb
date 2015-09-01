#pragma once

#include "vec.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <vector>

typedef struct {
    cl_int ports[4];
    cl_float3 position;
} _Node_unalign;

typedef _Node_unalign __attribute__((aligned(8))) Node;

struct Boundary {
    virtual bool inside(const Vec3f & v) const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(Vec3f c0, Vec3f c1);
    bool inside(const Vec3f & v) const override;
    Vec3f c0, c1;
};

std::vector<Node> tetrahedral_mesh(const Boundary & boundary,
                                   Vec3f start,
                                   float spacing);
