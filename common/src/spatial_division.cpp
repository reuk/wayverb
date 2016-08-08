#include "common/spatial_division.h"
#include "common/scene_data.h"

template <>
std::array<box<2>, 4> ndim_tree<2>::next_boundaries(const box<2>& parent) {
    const auto c = ::centre(parent);

    const auto x0 = parent.get_c0().x;
    const auto y0 = parent.get_c0().y;

    const auto xc = c.x;
    const auto yc = c.y;

    const auto x1 = parent.get_c1().x;
    const auto y1 = parent.get_c1().y;

    return std::array<box<2>, 4>{{
            box<2>(glm::vec2(x0, y0), glm::vec2(xc, yc)),
            box<2>(glm::vec2(xc, y0), glm::vec2(x1, yc)),
            box<2>(glm::vec2(x0, yc), glm::vec2(xc, y1)),
            box<2>(glm::vec2(xc, yc), glm::vec2(x1, y1)),
    }};
}

template <>
std::array<box<3>, 8> ndim_tree<3>::next_boundaries(const box<3>& parent) {
    const auto c = ::centre(parent);

    const auto x0 = parent.get_c0().x;
    const auto y0 = parent.get_c0().y;
    const auto z0 = parent.get_c0().z;

    const auto xc = c.x;
    const auto yc = c.y;
    const auto zc = c.z;

    const auto x1 = parent.get_c1().x;
    const auto y1 = parent.get_c1().y;
    const auto z1 = parent.get_c1().z;

    return std::array<box<3>, 8>{{
            box<3>(glm::vec3(x0, y0, z0), glm::vec3(xc, yc, zc)),
            box<3>(glm::vec3(xc, y0, z0), glm::vec3(x1, yc, zc)),
            box<3>(glm::vec3(x0, yc, z0), glm::vec3(xc, y1, zc)),
            box<3>(glm::vec3(xc, yc, z0), glm::vec3(x1, y1, zc)),
            box<3>(glm::vec3(x0, y0, zc), glm::vec3(xc, yc, z1)),
            box<3>(glm::vec3(xc, y0, zc), glm::vec3(x1, yc, z1)),
            box<3>(glm::vec3(x0, yc, zc), glm::vec3(xc, y1, z1)),
            box<3>(glm::vec3(xc, yc, zc), glm::vec3(x1, y1, z1)),
    }};
}

octree octree_from_scene_data(const copyable_scene_data& scene_data,
                              size_t depth,
                              float padding) {
    return ndim_tree<3>(
            depth,
            [&](size_t item, const box<3>& aabb) {
                return ::overlaps(
                        aabb,
                        ::get_triangle_vec3(scene_data.get_triangles()[item],
                                            scene_data.get_vertices()));
            },
            scene_data.get_triangle_indices(),
            ::padded(scene_data.get_aabb(), padding));
}
