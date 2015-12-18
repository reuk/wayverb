#include "octree.h"

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

Octree::Octree(const SceneData & scene_data, int max_depth)
        : Octree(scene_data,
                 max_depth,
                 scene_data.get_triangle_indices(),
                 scene_data.get_aabb()) {
}

Octree::Octree(const SceneData & scene_data,
               int max_depth,
               const std::vector<int> to_test,
               const CuboidBoundary & aabb)
        : scene_data(scene_data)
        , aabb(aabb) {
    for (const auto i : to_test) {
        if (get_aabb().overlaps(get_triangle_verts(scene_data.triangles[i],
                                                   scene_data.vertices))) {
            triangles.push_back(i);
        }
    }

    if (max_depth == 0 || to_test.empty()) {
        return;
    }

    auto next = next_boundaries(get_aabb());
    for (const auto & i : next) {
        Octree test(scene_data, max_depth - 1, get_triangles(), i);
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

void fill_static_octree(std::vector<int> & ret, const Octree & o) {
    //  int     n triangles
    //  int[]   triangle indices
    //  int     n nodes
    //  int[]   node offsets

    const auto & nodes = o.get_nodes();
    const auto & triangles = o.get_triangles();

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

std::vector<int> get_static_octree(const Octree & o) {
    std::vector<int> ret;
    fill_static_octree(ret, o);
    return ret;
}
