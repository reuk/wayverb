#include "bsp.h"

#include "tri_cube_intersection.h"

std::vector<CuboidBoundary> next_boundaries(const CuboidBoundary & parent) {
    auto centre = parent.get_centre();
    return std::vector<CuboidBoundary>{
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
        : Octree(mesh_boundary,
                 max_depth,
                 mesh_boundary.get_triangle_indices(),
                 mesh_boundary.get_aabb()) {
}

Octree::Octree(const MeshBoundary & mesh_boundary,
               int max_depth,
               const std::vector<int> to_test,
               const CuboidBoundary & aabb)
        : mesh_boundary(mesh_boundary)
        , aabb(aabb) {
    for (const auto i : to_test) {
        if (get_aabb().overlaps(get_triangle_verts(mesh_boundary.triangles[i],
                                                   mesh_boundary.vertices))) {
            triangles.push_back(i);
        }
    }

    if (max_depth == 0 || to_test.empty()) {
        return;
    }

    auto next = next_boundaries(get_aabb());
    for (const auto & i : next) {
        Octree test(mesh_boundary, max_depth - 1, get_triangles(), i);
        if (!test.get_triangles().empty()) {
            nodes.push_back(test);
        }
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

//  TODO hahahahahahaha help
void fill_static_octree(std::vector<int> & ret, const Octree & o) {
    ret.push_back(o.get_nodes().size());  //  some nodes
    for (const auto & i : o.get_nodes()) {
        ret.push_back(0);  //  TODO node offsets
    }
    ret.push_back(o.get_triangles().size());  //  some triangles
    for (const auto & i : o.get_triangles()) {
        ret.push_back(i);  //  triangle offsets
    }
}

std::vector<int> get_static_octree(const Octree & o) {
    //  int     n nodes
    //  int[]   node offsets
    //  int     n triangles
    //  int[]   triangle indices

    std::vector<int> ret;
    fill_static_octree(ret, o);
    return ret;
}
