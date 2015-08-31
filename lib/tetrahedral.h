#pragma once

#include "vec.h"

#include <vector>

struct Node {
    Node(Vec3f position)
            : position(position) {
    }
    Vec3f position;
    std::vector<int> ports;
};

struct Boundary {
    virtual bool inside(const Vec3f & v) const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(Vec3f c0, Vec3f c1);
    bool inside(const Vec3f & v) const override;
    Vec3f c0, c1;
};

void build_mesh(std::vector<Node> & ret,
                const Boundary & boundary,
                int parent_index,
                float spacing);
std::vector<Node> tetrahedral_mesh(const Boundary & boundary, Vec3f start, float spacing);
