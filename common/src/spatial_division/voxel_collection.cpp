#include "common/scene_data.h"
#include "common/spatial_division/voxel_collection.h"

aligned::vector<cl_uint> get_flattened(const voxel_collection<3>& voxels) {
    const auto side{voxels.get_side()};
    const auto dim{pow(side, 3)};

    aligned::vector<cl_uint> ret(dim);

    const auto to_flat{
            [side](auto i) { return i.x * side * side + i.y * side + i.z; }};

    for (auto x{0u}; x != side; ++x) {
        for (auto y{0u}; y != side; ++y) {
            for (auto z{0u}; z != side; ++z) {
                const glm::ivec3 ind(x, y, z);
                ret[to_flat(ind)] = ret.size();
                const auto& v{voxels.get_voxel(ind)};
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
std::experimental::optional<glm::ivec3> get_starting_index(
        const voxel_collection<3>& voxels, const geo::ray& ray) {
    const auto aabb{voxels.get_aabb()};

    const auto to_index{[&](const auto& i) {
        const glm::ivec3 ret{(i - aabb.get_min()) / voxel_dimensions(voxels)};
        return glm::max(
                glm::ivec3{0},
                glm::min(glm::ivec3{static_cast<int>(voxels.get_side() - 1)},
                         ret));
    }};

    //  If the ray starts inside the voxel collection there's no problem.
    if (inside(aabb, ray.get_position())) {
        return to_index(ray.get_position());
    }

    //  Otherwise, we need to check that the ray intersects with the collection.
    if (const auto i{geo::intersects(aabb, ray)}) {
        return to_index(ray.get_position() + ray.get_direction() * *i);
    }

    //  The ray never intersects with the voxel collection.
    return std::experimental::nullopt;
}

auto min_component(const glm::vec3& v) {
    size_t ret{0};
    for (auto i{1u}; i != 3; ++i) {
        if (v[i] < v[ret]) {
            ret = i;
        }
    }
    return ret;
}
}  // namespace

void traverse(const voxel_collection<3>& voxels,
              const geo::ray& ray,
              const traversal_callback& fun) {
    /// From A Fast Voxel Traversal Algorithm for Ray Tracing by John Amanatides
    /// and Andrew Woo.
    const auto side{voxels.get_side()};

    auto ind{get_starting_index(voxels, ray)};
    if (!ind) {
        return;
    }

    const auto voxel_bounds{voxel_aabb(voxels, *ind)};

    const auto gt{glm::lessThanEqual(glm::vec3{0}, ray.get_direction())};
    const auto step{glm::mix(glm::ivec3{-1}, glm::ivec3{1}, gt)};
    const auto just_out{
            glm::mix(glm::ivec3{-1}, glm::ivec3{static_cast<int>(side)}, gt)};
    const auto boundary{
            glm::mix(voxel_bounds.get_min(), voxel_bounds.get_max(), gt)};

    const auto t_max_temp{
            glm::abs((boundary - ray.get_position()) / ray.get_direction())};
    auto t_max{glm::mix(t_max_temp,
                        glm::vec3(std::numeric_limits<float>::infinity()),
                        glm::isnan(t_max_temp))};
    const auto t_delta{
            glm::abs(dimensions(voxel_bounds) / ray.get_direction())};

    auto prev_max{0.0f};

    for (;;) {
        const auto min_i{min_component(t_max)};

        const auto& tri{voxels.get_voxel(*ind)};
        if (fun(ray, tri, prev_max, t_max[min_i])) {
            // callback has signalled that it should quit
            return;
        }

        (*ind)[min_i] += step[min_i];
        if ((*ind)[min_i] == just_out[min_i]) {
            return;
        }
        prev_max = t_max[min_i];
        t_max[min_i] += t_delta[min_i];
    }
}
