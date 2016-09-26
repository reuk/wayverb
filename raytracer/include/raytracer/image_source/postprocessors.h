#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/surfaces.h"

#include "glm/fwd.hpp"

#include <complex>

/// The image source finder is designed just to ascertain whether or not each
/// 'possible' image-source path is an *actual* image-source path.
/// Once it's worked this out, it delegates to one of these secondary functions
/// to actually compute the pressure or intensity contributed by that particular
/// path.

namespace raytracer {
namespace image_source {

struct reflection_metadata final {
    cl_uint index;    /// The index of the triangle that was intersected.
    float cos_angle;  /// The cosine of the angle against the triangle normal.
};

//----------------------------------------------------------------------------//

template <typename Vertex, typename Surface, typename It>
auto compute_intensity(const glm::vec3& receiver,
                       const voxelised_scene_data<Vertex, Surface>& voxelised,
                       bool flip_phase,
                       const glm::vec3& image_source,
                       It begin,
                       It end) {
    const auto surface_attenuation{std::accumulate(
            begin, end, make_volume_type(1), [&](auto i, auto j) {
                const auto surface_index{voxelised.get_scene_data()
                                                 .get_triangles()[j.index]
                                                 .surface};
                const auto surface{voxelised.get_scene_data()
                                           .get_surfaces()[surface_index]};
                const auto reflectance{absorption_to_energy_reflectance(
                        get_specular_absorption(surface))};
                return i * reflectance * (flip_phase ? -1 : 1);
            })};

    const auto distance{glm::distance(image_source, receiver)};
    return impulse{surface_attenuation, to_cl_float3(image_source), distance};
}

class intensity_calculator final {
public:
    intensity_calculator(
            const glm::vec3& source,
            const glm::vec3& receiver,
            const voxelised_scene_data<cl_float3, surface>& voxelised,
            float acoustic_impedance,
            bool flip_phase);

    template <typename It>
    auto operator()(const glm::vec3& image_source, It begin, It end) const {
        return compute_intensity(
                receiver_, voxelised_, flip_phase_, image_source, begin, end);
    }

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data<cl_float3, surface>& voxelised_;
    bool flip_phase_;
};

//----------------------------------------------------------------------------//

template <typename Vertex, typename Surface, typename Imp, typename It>
auto compute_fast_pressure(
        const glm::vec3& receiver,
        const voxelised_scene_data<Vertex, Surface>& voxelised,
        const aligned::vector<Imp>& surface_impedances,
        bool flip_phase,
        const glm::vec3& image_source,
        It begin,
        It end) {
    const auto surface_attenuation{std::accumulate(
            begin, end, make_volume_type(1), [&](auto i, auto j) {
                const auto surface_index{voxelised.get_scene_data()
                                                 .get_triangles()[j.index]
                                                 .surface};
                const auto impedance{surface_impedances[surface_index]};
                const auto reflectance{
                        average_wall_impedance_to_pressure_reflectance(
                                impedance, j.cos_angle)};
                return i * reflectance * (flip_phase ? -1 : 1);
            })};

    return impulse{surface_attenuation,
                   to_cl_float3(image_source),
                   glm::distance(image_source, receiver)};
}

class fast_pressure_calculator final {
public:
    fast_pressure_calculator(
            const glm::vec3& source,
            const glm::vec3& receiver,
            const voxelised_scene_data<cl_float3, surface>& voxelised,
            float acoustic_impedance,
            bool flip_phase);

    template <typename It>
    auto operator()(const glm::vec3& image_source, It begin, It end) const {
        return compute_fast_pressure(receiver_,
                                     voxelised_,
                                     surface_impedances_,
                                     flip_phase_,
                                     image_source,
                                     begin,
                                     end);
    }

private:
    const glm::vec3& receiver_;
    const voxelised_scene_data<cl_float3, surface>& voxelised_;
    bool flip_phase_;

    aligned::vector<volume_type> surface_impedances_;
};

//----------------------------------------------------------------------------//

template <typename Impl>
class depth_counter_calculator final {
public:
    template <typename... Ts>
    depth_counter_calculator(Ts&&... ts)
            : impl_{std::forward<Ts>(ts)...} {}

    struct return_type final {
        decltype(std::declval<Impl>().operator()(
                std::declval<glm::vec3>(),
                std::declval<
                        aligned::vector<reflection_metadata>::const_iterator>(),
                std::declval<aligned::vector<
                        reflection_metadata>::const_iterator>())) value;
        std::ptrdiff_t depth;
    };

    template <typename It>
    auto operator()(const glm::vec3& image_source,
                           It begin,
                           It end) const {
        return return_type{impl_(image_source, begin, end),
                           std::distance(begin, end)};
    }

private:
    Impl impl_;
};

}  // namespace image_source
}  // namespace raytracer
