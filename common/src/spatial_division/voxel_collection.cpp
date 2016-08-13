#include "common/scene_data.h"
#include "common/spatial_division/voxel_collection.h"

aligned::vector<cl_uint> get_flattened(const voxel_collection<3>& voxels) {
    const auto side = voxels.get_side();
    const auto dim = pow(side, 3);

    aligned::vector<cl_uint> ret(dim);

    const auto to_flat = [side](auto i) {
        return i.x * side * side + i.y * side + i.z;
    };

    for (auto x = 0u; x != side; ++x) {
        for (auto y = 0u; y != side; ++y) {
            for (auto z = 0u; z != side; ++z) {
                const glm::ivec3 ind(x, y, z);
                ret[to_flat(ind)] = ret.size();
                const auto& v = voxels.get_voxel(ind);
                ret.push_back(v.size());
                for (const auto& i : v) {
                    ret.push_back(i);
                }
            }
        }
    }

    return ret;
}

namespace {
glm::ivec3 get_starting_index(const voxel_collection<3>& voxels,
                              const glm::vec3& position) {
    //    return glm::floor((position - voxels.get_aabb().get_min()) /
    //                      voxel_dimensions(voxels));
    return (position - voxels.get_aabb().get_min()) / voxel_dimensions(voxels);
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
        ret[i] = !std::isnan(v[i]);
    }
    return ret;
}
}  // namespace

void traverse(const voxel_collection<3>& voxels,
              const geo::ray& ray,
              const traversal_callback& fun) {
    const auto side = voxels.get_side();

    auto ind = get_starting_index(voxels, ray.get_position());

    if (glm::any(glm::lessThan(ind, glm::ivec3{0})) ||
        glm::any(glm::lessThanEqual(glm::ivec3(side), ind))) {
        return;
    }

    const auto voxel_bounds = voxel_aabb(voxels, ind);

    const auto gt = nonnegative(ray.get_direction());
    const auto step = select(glm::ivec3(1), glm::ivec3(-1), gt);
    const auto just_out = select(glm::ivec3(side), glm::ivec3(-1), gt);
    const auto boundary =
            select(voxel_bounds.get_max(), voxel_bounds.get_min(), gt);

    const auto t_max_temp =
            glm::abs((boundary - ray.get_position()) / ray.get_direction());
    auto t_max = select(t_max_temp,
                        glm::vec3(std::numeric_limits<float>::infinity()),
                        is_not_nan(t_max_temp));
    const auto t_delta =
            glm::abs(dimensions(voxel_bounds) / ray.get_direction());

    float prev_max = 0;

    for (;;) {
        const auto min_i = min_component(t_max);

        const auto& tri = voxels.get_voxel(ind);
        if (!tri.empty()) {
            if (fun(ray, tri, prev_max, t_max[min_i])) {
                // callback has signalled that it should quit
                return;
            }
        }

        ind[min_i] += step[min_i];
        if (ind[min_i] == just_out[min_i]) {
            return;
        }
        prev_max = t_max[min_i];
        t_max[min_i] += t_delta[min_i];
    }
}