#include "octree.h"

#include "tri_cube_intersection.h"

std::vector<CuboidBoundary> next_boundaries(const CuboidBoundary& parent) {
    auto centre = parent.get_centre();

    auto x0 = parent.c0.x;
    auto y0 = parent.c0.y;
    auto z0 = parent.c0.z;
    auto xc = centre.x;
    auto yc = centre.y;
    auto zc = centre.z;
    auto x1 = parent.c1.x;
    auto y1 = parent.c1.y;
    auto z1 = parent.c1.z;

    return std::vector<CuboidBoundary>{
        CuboidBoundary(Vec3f(x0, y0, z0), Vec3f(xc, yc, zc)),
        CuboidBoundary(Vec3f(x0, y0, zc), Vec3f(xc, yc, z1)),
        CuboidBoundary(Vec3f(xc, y0, z0), Vec3f(x1, yc, zc)),
        CuboidBoundary(Vec3f(xc, y0, zc), Vec3f(x1, yc, z1)),
        CuboidBoundary(Vec3f(x0, yc, z0), Vec3f(xc, y1, zc)),
        CuboidBoundary(Vec3f(x0, yc, zc), Vec3f(xc, y1, z1)),
        CuboidBoundary(Vec3f(xc, yc, z0), Vec3f(x1, y1, zc)),
        CuboidBoundary(Vec3f(xc, yc, zc), Vec3f(x1, y1, z1)),
    };
}

std::vector<int> get_triangles(const SceneData& sd,
                               const std::vector<int>& to_test,
                               const CuboidBoundary& aabb) {
    std::vector<int> ret;
    for (const auto i : to_test) {
        if (aabb.overlaps(get_triangle_verts(sd.triangles[i], sd.vertices))) {
            ret.push_back(i);
        }
    }
    return ret;
}

std::vector<Octree> get_nodes(const SceneData& sd,
                              int md,
                              const std::vector<int> to_test,
                              const CuboidBoundary& ab) {
    std::vector<Octree> ret;
    if (md == 0) {
        return ret;
    }
    for (const auto& i : next_boundaries(ab)) {
        ret.emplace_back(sd, md - 1, to_test, i);
    }
    return ret;
}

Octree::Octree(const SceneData& sd, int md, float padding)
        : Octree(sd,
                 md,
                 sd.get_triangle_indices(),
                 sd.get_aabb().get_padded(padding)) {
}

Octree::Octree(const SceneData& sd,
               int md,
               const std::vector<int> to_test,
               const CuboidBoundary& ab)
        : aabb(ab)
        , triangles(::get_triangles(sd, to_test, ab))
        , nodes(::get_nodes(sd, md, triangles, ab)) {
}

CuboidBoundary Octree::get_aabb() const {
    return aabb;
}

const std::vector<Octree>& Octree::get_nodes() const {
    return nodes;
}

const std::vector<int>& Octree::get_triangles() const {
    return triangles;
}

void fill_static_octree(std::vector<int>& ret, const Octree& o) {
    //  int     n triangles
    //  int[]   triangle indices
    //  int     n nodes
    //  int[]   node offsets

    const auto& nodes = o.get_nodes();
    const auto& triangles = o.get_triangles();

    if (nodes.empty()) {
        std::vector<int> t(triangles.size() + 1);
        t[0] = triangles.size();
        for (auto i = 0u; i != triangles.size(); ++i)
            t[i + 1] = triangles[i];
        ret.insert(ret.end(), t.begin(), t.end());
    } else {
        ret.push_back(0);  //  no triangles

        ret.push_back(nodes.size());  //  some nodes
        auto node_table_start = ret.size();
        std::for_each(
            nodes.begin(), nodes.end(), [&ret](auto) { ret.push_back(0); });

        auto counter = node_table_start;
        std::for_each(nodes.begin(),
                      nodes.end(),
                      [&ret, &counter](auto i) {
                          ret[counter++] = ret.size();
                          fill_static_octree(ret, i);
                      });
    }
}

std::vector<int> get_static_octree(const Octree& o) {
    std::vector<int> ret;
    fill_static_octree(ret, o);
    return ret;
}
