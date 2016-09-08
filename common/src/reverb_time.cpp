#include "common/reverb_time.h"
#include "common/geo/triangle_vec.h"
#include "common/hrtf.h"
#include "common/stl_wrappers.h"

float area(const scene_data& scene, size_t material_index) {
    //  TODO this is dumb because of the linear search.
    //  If this becomes a bottleneck, maybe scene_data could store surfaces
    //  pre-sorted by material.
    return proc::accumulate(
            scene.get_triangles(), 0.0f, [&](auto running_total, auto tri) {
                return running_total +
                       (tri.surface == material_index
                                ? geo::area(geo::get_triangle_vec3(
                                          tri, scene.get_vertices()))
                                : 0.0f);
            });
}

float area(const scene_data& scene) {
    //  This is OK - we have to look at every triangle anyway.
    return proc::accumulate(
            scene.get_triangles(), 0.0f, [&](auto running_total, auto tri) {
                return running_total + geo::area(geo::get_triangle_vec3(
                                               tri, scene.get_vertices()));
            });
}

volume_type absorption_area(const scene_data& scene, size_t material_index) {
    return scene.get_materials()[material_index].surface.specular_absorption *
           area(scene, material_index);
}

volume_type equivalent_absorption_area(const scene_data& scene) {
    const auto num_surfaces{scene.get_materials().size()};
    volume_type running_total{};
    for (auto i{0u}; i != num_surfaces; ++i) {
        running_total += absorption_area(scene, i);
    }
    return running_total;
}

//----------------------------------------------------------------------------//

std::array<std::pair<cl_uint, cl_uint>, 3> get_index_pairs(const triangle& t) {
    return {{std::make_pair(t.v0, t.v1),
             std::make_pair(t.v1, t.v2),
             std::make_pair(t.v2, t.v0)}};
}

float six_times_tetrahedron_volume(const geo::triangle_vec3& t) {
    /// From Efficient Feature Extraction for 2d/3d Objects in Mesh
    /// Representation, Cha Zhang and Tsuhan Chen
    const auto volume{(t[1].x * t[2].y * t[0].z) - (t[2].x * t[1].y * t[0].z) +
                      (t[2].x * t[0].y * t[1].z) - (t[0].x * t[2].y * t[1].z) +
                      (t[0].x * t[1].y * t[2].z) - (t[1].x * t[0].y * t[2].z)};
    const auto sign{glm::dot(glm::normalize(t[0]), geo::normal(t))};
    return std::copysign(volume, sign);
}

float estimate_room_volume(const scene_data& scene) {
    return std::abs(proc::accumulate(
                   scene.get_triangles(),
                   0.0f,
                   [&](auto running_total, auto tri) {
                       return running_total +
                              six_times_tetrahedron_volume(
                                      geo::get_triangle_vec3(
                                              tri, scene.get_vertices()));
                   })) /
           6;
}

//----------------------------------------------------------------------------//

float estimate_air_intensity_absorption(float frequency, float humidity) {
    return (0.0275 / humidity) * std::pow(frequency / 1000, 1.7);
}

volume_type estimate_air_intensity_absorption(float humidity) {
    volume_type ret{};
    for (auto i{0u}; i != 8; ++i) {
        const auto frequency{(hrtf_data::edges[i] + hrtf_data::edges[i + 1]) *
                             0.5};
        ret.s[i] = estimate_air_intensity_absorption(frequency, humidity);
    }
    return ret;
}

//----------------------------------------------------------------------------//

volume_type sabine_reverb_time(const scene_data& scene,
                               volume_type air_coefficient) {
    const auto room_volume{estimate_room_volume(scene)};
    const auto absorption_area{equivalent_absorption_area(scene)};
    return (0.161f * room_volume) /
           (absorption_area + (4 * room_volume * air_coefficient));
}

//----------------------------------------------------------------------------//

volume_type eyring_reverb_time(const scene_data& scene,
                               volume_type air_coefficient) {
    const auto room_volume{estimate_room_volume(scene)};
    const auto absorption_area{equivalent_absorption_area(scene)};
    const auto full_area{area(scene)};
    return (0.161f * room_volume) /
           (-full_area * log(1 - (absorption_area / full_area)) +
            (4 * room_volume * air_coefficient));
}
