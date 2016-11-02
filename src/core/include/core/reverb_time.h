#pragma once

#include "core/geo/triangle_vec.h"
#include "core/scene_data.h"

#include "utilities/aligned/set.h"

/// IMPORTANT
/// The following functions assume that the scene being modelled has the
/// following properties:
///     All triangle normals are oriented in the same direction.
///         So, no pathological geometry in the scene (moebius strips etc.)
///     The scene consists of a single, closed cavity.
///         No gaps or openings, and no uncoupled spaces.
/// If these critera are not met, the equations are not guaranteed to return
/// a result, let alone a *valid* result.

/// Find the area covered by a particular material.
template <typename Vertex, typename Surface>
double area(const generic_scene_data<Vertex, Surface>& scene,
            size_t surface_index) {
    //  TODO this is dumb because of the linear search.
    //  If this becomes a bottleneck, maybe scene_data could store surfaces
    //  pre-sorted by material.
    return std::accumulate(
            begin(scene.get_triangles()),
            end(scene.get_triangles()),
            0.0,
            [&](auto running_total, auto tri) {
                return running_total +
                       (tri.surface == surface_index
                                ? geo::area(geo::get_triangle_vec3(
                                          tri, scene.get_vertices()))
                                : 0.0);
            });
}

/// Find the total surface area of an object.
template <typename Vertex, typename Surface>
double area(const generic_scene_data<Vertex, Surface>& scene) {
    //  This is OK - we have to look at every triangle anyway.
    return std::accumulate(begin(scene.get_triangles()),
                           end(scene.get_triangles()),
                           0.0,
                           [&](auto running_total, auto tri) {
                               return running_total +
                                      geo::area(geo::get_triangle_vec3(
                                              tri, scene.get_vertices()));
                           });
}

/// The product of the area covered by a material with the absorption
/// coefficient of that material.
template <typename Vertex, typename Surface>
auto absorption_area(const generic_scene_data<Vertex, Surface>& scene,
                     size_t surface_index) {
    return scene.get_surfaces()[surface_index].absorption *
           area(scene, surface_index);
}

/// Sum of the absorption areas for all materials in the scene.
template <typename Vertex, typename Surface>
auto equivalent_absorption_area(
        const generic_scene_data<Vertex, Surface>& scene) {
    const auto num_surfaces = scene.get_surfaces().size();
    decltype(absorption_area(scene, 0)) running_total{};
    for (auto i = 0u; i != num_surfaces; ++i) {
        running_total += absorption_area(scene, i);
    }
    return running_total;
}

////////////////////////////////////////////////////////////////////////////////

std::array<std::pair<cl_uint, cl_uint>, 3> get_index_pairs(const triangle& t);

/// This probably isn't the fastest way of doing this...
template <typename It>  /// Iterator through a collection of triangles.
bool triangles_are_oriented(It begin, It end) {
    //  The scene is consistently oriented if no triangles in the scene share
    //  an edge with the same winding direction.
    util::aligned::set<std::pair<cl_uint, cl_uint>> table;

    //  For each triangle.
    for (; begin != end; ++begin) {
        //  For each pair of vertices.
        for (const auto& pair : get_index_pairs(*begin)) {
            //  See whether this pair of vertices has already been found.
            if (!table.insert(pair).second) {
                //  The pair already existed.
                return false;
            }
        }
    }
    //  No triangles share a pair of vertices.
    return true;
}

float six_times_tetrahedron_volume(const geo::triangle_vec3& t);

/// http://research.microsoft.com/en-us/um/people/chazhang/publications/icip01_ChaZhang.pdf
template <typename Vertex, typename Surface>
float estimate_room_volume(const generic_scene_data<Vertex, Surface>& scene) {
    return std::abs(std::accumulate(
                   begin(scene.get_triangles()),
                   end(scene.get_triangles()),
                   0.0f,
                   [&](auto running_total, auto tri) {
                       return running_total +
                              six_times_tetrahedron_volume(
                                      geo::get_triangle_vec3(
                                              tri, scene.get_vertices()));
                   })) /
           6;
}

////////////////////////////////////////////////////////////////////////////////

/// Sound intensity absorption coefficient calculator. (fu2015 eq. 11)
float estimate_air_intensity_absorption(float frequency, float humidity);

template <size_t... Ix>
auto estimate_air_intensity_absorption(
        const std::array<float, sizeof...(Ix)>& band_centres,
        float humidity,
        std::index_sequence<Ix...>) {
    return std::array<float, sizeof...(Ix)>{{estimate_air_intensity_absorption(
            std::get<Ix>(band_centres), humidity)...}};
}

template <size_t Bands>
auto estimate_air_intensity_absorption(
        const std::array<float, Bands>& band_centres, float humidity) {
    return estimate_air_intensity_absorption(
            band_centres, humidity, std::make_index_sequence<Bands>{});
}

////////////////////////////////////////////////////////////////////////////////

//  USE THESE ONES

template <typename Absorption, typename Coeff>
auto sabine_reverb_time(double room_volume,
                        Absorption absorption_area,
                        Coeff air_coefficient) {
    if (room_volume <= 0) {
        throw std::runtime_error{
                "sabine_reverb_time: room_volume must be greater than 0"};
    }
    if (any(absorption_area <= 0)) {
        throw std::runtime_error{
                "sabine_reverb_time: absorption_area must be greater than 0"};
    }

    const auto numerator = 0.161f * room_volume;
    const auto denominator =
            absorption_area + (4 * room_volume * air_coefficient);
    return numerator / denominator;
}

/// Sabine reverb time (use the damping constant function above too)
/// (kuttruff 5.9) (vorlander 4.33)
template <typename Vertex, typename Surface, typename Coeff>
auto sabine_reverb_time(const generic_scene_data<Vertex, Surface>& scene,
                        Coeff air_coefficient) {
    return sabine_reverb_time(estimate_room_volume(scene),
                              equivalent_absorption_area(scene),
                              air_coefficient);
}

template <typename Absorption, typename Coeff>
auto eyring_reverb_time(double room_volume,
                        Absorption absorption_area,
                        double full_area,
                        Coeff air_coefficient) {
    if (room_volume <= 0) {
        throw std::runtime_error{
                "eyring_reverb_time: room_volume must be greater than 0"};
    }
    if (any(absorption_area <= 0)) {
        throw std::runtime_error{
                "eyring_reverb_time: absorption_area must be greater than 0"};
    }

    const auto numerator = 0.161f * room_volume;
    const auto denominator =
            -full_area * log(1 - (absorption_area / full_area)) +
            (4 * room_volume * air_coefficient);
    return numerator / denominator;
}

/// Eyring reverb time (kuttruff 5.24) (vorlander 4.32)
template <typename Vertex, typename Surface, typename Coeff>
auto eyring_reverb_time(const generic_scene_data<Vertex, Surface>& scene,
                        Coeff air_coefficient) {
    return eyring_reverb_time(estimate_room_volume(scene),
                              equivalent_absorption_area(scene),
                              area(scene),
                              air_coefficient);
}
