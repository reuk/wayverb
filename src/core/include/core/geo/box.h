#pragma once

#include "core/geo/geometric.h"
#include "core/geo/triangle_vec.h"
#include "core/scene_data.h"

#include "utilities/mapping_iterator_adapter.h"
#include "utilities/range.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace core {
namespace geo {

using box = util::range<glm::vec3>;

enum class wall { nx, px, ny, py, nz, pz };
enum class direction { x, y, z };

bool overlaps(const box& b, const triangle_vec3& t);

std::experimental::optional<std::pair<float, float>> intersection_distances(
        const box& b, const ray& ray);

/// Returns the shortest positive distance to the box from the ray.
/// If the ray is inside the box, returns the positive distance to the
/// nearest wall.
std::experimental::optional<float> intersects(const box& b, const ray& ray);

/// Returns true if there is an intersection with the box within the interval
/// (t0, t1) along the ray.
bool intersects(const box& b, const ray& ray, float t0, float t1);

template <typename Surface>
auto get_scene_data(const box& b, Surface s) {
    return make_scene_data(
            util::aligned::vector<triangle>{{0, 0, 1, 5},
                                            {0, 0, 5, 4},
                                            {0, 1, 0, 3},
                                            {0, 0, 2, 3},
                                            {0, 2, 0, 6},
                                            {0, 0, 4, 6},
                                            {0, 5, 1, 7},
                                            {0, 1, 3, 7},
                                            {0, 3, 2, 7},
                                            {0, 2, 6, 7},
                                            {0, 4, 5, 7},
                                            {0, 6, 4, 7}},
            util::aligned::vector<cl_float3>{
                    {{b.get_min().x, b.get_min().y, b.get_min().z}},
                    {{b.get_max().x, b.get_min().y, b.get_min().z}},
                    {{b.get_min().x, b.get_max().y, b.get_min().z}},
                    {{b.get_max().x, b.get_max().y, b.get_min().z}},
                    {{b.get_min().x, b.get_min().y, b.get_max().z}},
                    {{b.get_max().x, b.get_min().y, b.get_max().z}},
                    {{b.get_min().x, b.get_max().y, b.get_max().z}},
                    {{b.get_max().x, b.get_max().y, b.get_max().z}}},
            util::aligned::vector<Surface>{std::move(s)});
}

constexpr glm::vec3 mirror_on_axis(const glm::vec3& v,
                                   const glm::vec3& pt,
                                   direction d) {
    switch (d) {
        case direction::x: return glm::vec3(2 * pt.x - v.x, v.y, v.z);
        case direction::y: return glm::vec3(v.x, 2 * pt.y - v.y, v.z);
        case direction::z: return glm::vec3(v.x, v.y, 2 * pt.z - v.z);
    }
}

constexpr glm::vec3 mirror(const box& b, const glm::vec3& v, wall w) {
    switch (w) {
        case wall::nx: return mirror_on_axis(v, b.get_min(), direction::x);
        case wall::px: return mirror_on_axis(v, b.get_max(), direction::x);
        case wall::ny: return mirror_on_axis(v, b.get_min(), direction::y);
        case wall::py: return mirror_on_axis(v, b.get_max(), direction::y);
        case wall::nz: return mirror_on_axis(v, b.get_min(), direction::z);
        case wall::pz: return mirror_on_axis(v, b.get_max(), direction::z);
    }
}

template <typename Surface>
auto compute_aabb(const generic_scene_data<glm::vec3, Surface>& scene) {
    return enclosing_range(scene.get_vertices().begin(),
                           scene.get_vertices().end());
}

template <typename Surface>
auto compute_aabb(const generic_scene_data<cl_float3, Surface>& scene) {
    const auto make_iterator = [](auto i) {
        return util::make_mapping_iterator_adapter(
                std::move(i), [](auto i) { return to_vec3(i); });
    };
    return util::enclosing_range(make_iterator(scene.get_vertices().begin()),
                                 make_iterator(scene.get_vertices().end()));
}

glm::vec3 mirror_inside(const box& b, const glm::vec3& v, direction d);
box mirror(const box& b, wall w);

}  // namespace geo

inline auto inside(const geo::box& a, const glm::vec3& b) {
    return glm::all(glm::lessThan(a.get_min(), b)) &&
           glm::all(glm::lessThan(b, a.get_max()));
}

}  // namespace core
}  // namespace wayverb
