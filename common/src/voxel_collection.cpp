#include "common/voxel_collection.h"

#include "common/triangle.h"

#include <cmath>
#include <iostream>

VoxelCollection::Voxel::Voxel(const box& aabb,
                              const aligned::vector<size_t>& triangles)
        : aabb(aabb)
        , triangles(triangles) {}

VoxelCollection::Voxel::Voxel(const Octree& o)
        : Voxel(o.get_aabb(), o.get_triangles()) {}

box VoxelCollection::Voxel::get_aabb() const { return aabb; }
const aligned::vector<size_t>& VoxelCollection::Voxel::Voxel::get_triangles()
        const {
    return triangles;
}

VoxelCollection::VoxelCollection(const Octree& o)
        : aabb(o.get_aabb())
        , data(o.get_side(), YAxis(o.get_side(), ZAxis(o.get_side()))) {
    init(o);
}

VoxelCollection::VoxelCollection(const CopyableSceneData& scene_data,
                                 int depth,
                                 float padding)
        : VoxelCollection(Octree(scene_data, depth, padding)) {}

void VoxelCollection::init(const Octree& o, const glm::ivec3& d) {
    if (!o.has_nodes()) {
        data[d.x][d.y][d.z] = Voxel(o);
    } else {
        auto count = 0u;
        for (const auto& i : o.get_nodes()) {
            auto x = (count & 1u) ? 1u : 0u;
            auto y = (count & 2u) ? 1u : 0u;
            auto z = (count & 4u) ? 1u : 0u;
            init(i,
                 d + glm::ivec3(x, y, z) * static_cast<int>(o.get_side()) / 2);
            count += 1;
        }
    }
}

box VoxelCollection::get_aabb() const { return aabb; }

box VoxelCollection::get_voxel_aabb() const {
    return data.front().front().front().get_aabb();
}

const VoxelCollection::XAxis& VoxelCollection::get_data() const { return data; }

glm::ivec3 VoxelCollection::get_starting_index(
        const glm::vec3& position) const {
    return glm::floor((position - get_aabb().get_c0()) /
                      dimensions(get_voxel_aabb()));
}

glm::bvec3 nonnegative(const glm::vec3& t) {
    glm::bvec3 ret;
    for (auto i = 0u; i != t.length(); ++i) {
        ret[i] = 0 <= t[i];
    }
    return ret;
}

template <typename T, typename U>
T select(const T& a, const T& b, const U& selector) {
    T ret;
    for (auto i = 0u; i != selector.length(); ++i) {
        ret[i] = selector[i] ? a[i] : b[i];
    }
    return ret;
}

const VoxelCollection::Voxel& VoxelCollection::get_voxel(
        const glm::ivec3& i) const {
    return data[i.x][i.y][i.z];
}

auto min_component(const glm::vec3& v) {
    size_t ret{0};
    for (auto i = 1u; i != 3; ++i) {
        if (v[i] < v[ret]) {
            ret = i;
        }
    }
    return ret;
}

glm::bvec3 is_not_nan(const glm::vec3& v) {
    glm::bvec3 ret;
    for (auto i = 0u; i != v.length(); ++i) {
        ret[i] = ! std::isnan(v[i]);
    }
    return ret;
}

geo::Intersection VoxelCollection::traverse(
        const geo::Ray& ray, const TraversalCallback& fun) const {
    //  from http://www.cse.chalmers.se/edu/year/2010/course/TDA361/grid.pdf
    auto ind                = get_starting_index(ray.get_position());
    const auto voxel_bounds = get_voxel(ind).get_aabb();

    const auto gt       = nonnegative(ray.get_direction());
    const auto step     = select(glm::ivec3(1), glm::ivec3(-1), gt);
    const auto just_out = select(glm::ivec3(data.size()), glm::ivec3(-1), gt);
    const auto boundary =
            select(voxel_bounds.get_c1(), voxel_bounds.get_c0(), gt);

    const auto t_max_temp =
            glm::abs((boundary - ray.get_position()) / ray.get_direction());
    auto t_max = select(t_max_temp,
                   glm::vec3(std::numeric_limits<float>::infinity()),
                   is_not_nan(t_max_temp));
    const auto t_delta =
            glm::abs(dimensions(voxel_bounds) / ray.get_direction());

    for (;;) {
        const auto min_i = min_component(t_max);

        const auto& tri = get_voxel(ind).get_triangles();
        if (!tri.empty()) {
            const auto ret = fun(ray, tri);

            if (ret && ret->distance < t_max[min_i]) {
                return ret;
            }
        }

        ind[min_i] += step[min_i];
        if (ind[min_i] == just_out[min_i]) {
            return geo::Intersection();
        }
        t_max[min_i] += t_delta[min_i];
    }
}

VoxelCollection::TriangleTraversalCallback::TriangleTraversalCallback(
        const CopyableSceneData& scene_data)
        : tri(scene_data.get_triangles())
        , vertices(scene_data.get_converted_vertices()) {}

geo::Intersection VoxelCollection::TriangleTraversalCallback::operator()(
        const geo::Ray& ray, const aligned::vector<size_t>& triangles) const {
    return geo::ray_triangle_intersection(ray, triangles, tri, vertices);
}

aligned::vector<cl_uint> VoxelCollection::get_flattened() const {
    auto side = get_data().size();
    auto dim  = pow(side, 3);

    aligned::vector<cl_uint> ret(dim);

    auto to_flat = [side](auto i) {
        return i.x * side * side + i.y * side + i.z;
    };

    for (auto x = 0u; x != side; ++x) {
        for (auto y = 0u; y != side; ++y) {
            for (auto z = 0u; z != side; ++z) {
                glm::ivec3 ind(x, y, z);
                ret[to_flat(ind)] = ret.size();
                const auto& v     = get_voxel(ind);
                ret.push_back(v.get_triangles().size());
                for (const auto& i : v.get_triangles()) {
                    ret.push_back(i);
                }
            }
        }
    }

    return ret;
}

int VoxelCollection::get_side() const { return get_data().size(); }
