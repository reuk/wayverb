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

namespace {
void compute_data(voxel_collection<2>::data_type& ret,
                  const ndim_tree<2>& tree,
                  const glm::ivec2& d) {
    if (!tree.has_nodes()) {
        ret[d.x][d.y] = detail::voxel<2>(tree);
    } else {
        auto count = 0u;
        for (const auto& i : tree.get_nodes()) {
            const auto x = (count & 1u) ? 1u : 0u;
            const auto y = (count & 2u) ? 1u : 0u;
            compute_data(ret,
                         i,
                         d +
                                 glm::ivec2(x, y) *
                                         static_cast<int>(tree.get_side()) / 2);
            count += 1;
        }
    }
}
}  // namespace

template <>
voxel_collection<2>::data_type voxel_collection<2>::compute_data(
        const ndim_tree<2>& tree) {
    auto ret = detail::voxel_collection_data<2>::get_blank(tree.get_side());
    ::compute_data(ret, tree, glm::ivec2(0));
    return ret;
}

namespace {
void compute_data(voxel_collection<3>::data_type& ret,
                  const ndim_tree<3>& tree,
                  const glm::ivec3& d) {
    if (!tree.has_nodes()) {
        ret[d.x][d.y][d.z] = detail::voxel<3>(tree);
    } else {
        auto count = 0u;
        for (const auto& i : tree.get_nodes()) {
            const auto x = (count & 1u) ? 1u : 0u;
            const auto y = (count & 2u) ? 1u : 0u;
            const auto z = (count & 4u) ? 1u : 0u;
            compute_data(ret,
                         i,
                         d +
                                 glm::ivec3(x, y, z) *
                                         static_cast<int>(tree.get_side()) / 2);
            count += 1;
        }
    }
}
}  // namespace

template <>
voxel_collection<3>::data_type voxel_collection<3>::compute_data(
        const ndim_tree<3>& tree) {
    auto ret = detail::voxel_collection_data<3>::get_blank(tree.get_side());
    ::compute_data(ret, tree, glm::ivec3(0));
    return ret;
}