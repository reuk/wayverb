#include "common/scene_data.h"
#include "common/voxel_collection.h"

triangle_traversal_callback::triangle_traversal_callback(
        const copyable_scene_data& scene_data)
        : tri(scene_data.get_triangles())
        , vertices(scene_data.get_converted_vertices()) {}

geo::intersection triangle_traversal_callback::operator()(
        const geo::ray& ray, const aligned::vector<size_t>& triangles) const {
    return geo::ray_triangle_intersection(ray, triangles, tri, vertices);
}

template <>
aligned::vector<cl_uint> voxel_collection<3>::get_flattened() const {
    auto side = get_data().size();
    auto dim = pow(side, 3);

    aligned::vector<cl_uint> ret(dim);

    auto to_flat = [side](auto i) {
        return i.x * side * side + i.y * side + i.z;
    };

    for (auto x = 0u; x != side; ++x) {
        for (auto y = 0u; y != side; ++y) {
            for (auto z = 0u; z != side; ++z) {
                glm::ivec3 ind(x, y, z);
                ret[to_flat(ind)] = ret.size();
                const auto& v = get_voxel(ind);
                ret.push_back(v.get_items().size());
                for (const auto& i : v.get_items()) {
                    ret.push_back(i);
                }
            }
        }
    }

    return ret;
}
