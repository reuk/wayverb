#include "bsp.h"

#include "tri_cube_intersection.h"

// Rel t_c_intersection(const Triangle & t, const std::vector<Vec3f> & v);

std::vector<CuboidBoundary> next_boundaries(const CuboidBoundary & parent) {
    auto centre = parent.get_centre();
    return std::vector<CuboidBoundary> {
        CuboidBoundary(parent.c0, centre),

        CuboidBoundary(Vec3f(parent.c0.x, parent.c0.y, centre.z),
                       Vec3f(centre.x, centre.y, parent.c1.z)),
        CuboidBoundary(Vec3f(centre.x, parent.c0.y, parent.c0.z),
                       Vec3f(parent.c1.x, centre.y, centre.z)),
        CuboidBoundary(Vec3f(centre.x, parent.c0.y, centre.z),
                       Vec3f(parent.c1.x, centre.y, parent.c1.z)),

        CuboidBoundary(Vec3f(parent.c0.x, centre.y, centre.z),
                       Vec3f(centre.x, parent.c1.y, parent.c1.z)),
        CuboidBoundary(Vec3f(centre.x, centre.y, parent.c0.z),
                       Vec3f(parent.c1.x, parent.c1.y, centre.z)),
        CuboidBoundary(Vec3f(centre.x, centre.y, centre.z),
                       Vec3f(parent.c1.x, parent.c1.y, parent.c1.z)),

        CuboidBoundary(centre, parent.c1),
    };
}

Octree::Octree(const MeshBoundary & mesh_boundary, int max_depth)
        : mesh_boundary(mesh_boundary)
        , aabb(mesh_boundary.get_aabb()) {

    triangles = std::vector<int>(mesh_boundary.triangles.size());
    std::iota(triangles.begin(), triangles.end(), 0);

    if (max_depth == 0) {
        return;
    }

    auto next = next_boundaries(get_aabb());
    for (const auto & i : next) {
        nodes.emplace_back(mesh_boundary, max_depth - 1, i, *this);
    }
}

Octree::Octree(const MeshBoundary & mesh_boundary,
               int max_depth,
               const CuboidBoundary & aabb,
               const Octree & parent)
    : mesh_boundary(mesh_boundary)
    , aabb(aabb) {

    for (auto i : parent.triangles) {
        if (get_aabb().overlaps(mesh_boundary.triangles[i], mesh_boundary.vertices)) {
            triangles.push_back(i);
        }
    }

    if (max_depth == 0) {
        return;
    }

    auto next = next_boundaries(get_aabb());
    for (const auto & i : next) {
        nodes.emplace_back(mesh_boundary, max_depth - 1, i, *this);
    }
}

CuboidBoundary Octree::get_aabb() const {
    return aabb;
}

const std::vector<Octree> & Octree::get_nodes() const {
    return nodes;
}

const std::vector<int> & Octree::get_triangles() const {
    return triangles;
}
