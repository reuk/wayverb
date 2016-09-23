#include "raytracer/image_source/postprocessors.h"

#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/pressure_intensity.h"
#include "common/surfaces.h"

namespace raytracer {
namespace image_source {

intensity_calculator::intensity_calculator(
        const glm::vec3&,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float,
        bool flip_phase)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , flip_phase_{flip_phase} {}

impulse intensity_calculator::operator()(
        const glm::vec3& image_source,
        const aligned::vector<reflection_metadata>& intersections) const {
    const auto surface_attenuation{proc::accumulate(
            intersections, make_volume_type(1), [&](auto i, auto j) {
                const auto surface_index{voxelised_.get_scene_data()
                                                 .get_triangles()[j.index]
                                                 .surface};
                const auto surface{voxelised_.get_scene_data()
                                           .get_materials()[surface_index]
                                           .surface};
                const auto reflectance{absorption_to_energy_reflectance(
                        surface.specular_absorption)};
                return i * reflectance * (flip_phase_ ? -1 : 1);
            })};

    const auto distance{glm::distance(image_source, receiver_)};
    return {surface_attenuation, to_cl_float3(image_source), distance};
}

//----------------------------------------------------------------------------//

//  A simple pressure calculator which accumulates pressure responses in
//  the time domain.
//  Uses equation 9.22 from the kuttruff book, assuming single-sample
//  reflection/convolution kernels.
fast_pressure_calculator::fast_pressure_calculator(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float acoustic_impedance,
        bool flip_phase)
        : receiver_{receiver}
        , voxelised_{voxelised}
//        , acoustic_impedance_{acoustic_impedance}
        , flip_phase_{flip_phase}
        , surface_impedances_{map_to_vector(
                  voxelised.get_scene_data().get_materials(),
                  [](auto material) {
                      const auto absorption{
                              material.surface.specular_absorption};
                      const auto reflectance{
                              absorption_to_pressure_reflectance(absorption)};
                      return pressure_reflectance_to_average_wall_impedance(
                              reflectance);
                  })} {}

impulse fast_pressure_calculator::operator()(
        const glm::vec3& image_source,
        const aligned::vector<reflection_metadata>& intersections) const {
    //  Find the product of all reflection coefficients along the ray path.
    const auto surface_attenuation{proc::accumulate(
            intersections, make_volume_type(1), [&](auto i, auto j) {
                const auto surface_index{voxelised_.get_scene_data()
                                                 .get_triangles()[j.index]
                                                 .surface};
                const auto impedance{surface_impedances_[surface_index]};
                const auto reflectance{
                        average_wall_impedance_to_pressure_reflectance(
                                impedance, j.cos_angle)};
                return i * reflectance * (flip_phase_ ? -1 : 1);
            })};

    return {surface_attenuation,
            to_cl_float3(image_source),
            glm::distance(image_source, receiver_)};
}

//----------------------------------------------------------------------------//

comparison_calculator::comparison_calculator(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float acoustic_impedance,
        bool flip_phase)
        : intensity_{source,
                     receiver,
                     voxelised,
                     acoustic_impedance,
                     flip_phase}
        , fast_pressure_{source,
                         receiver,
                         voxelised,
                         acoustic_impedance,
                         flip_phase} {}

impulse comparison_calculator::operator()(
        const glm::vec3& image_source,
        const aligned::vector<reflection_metadata>& intersections) const {
    const auto intensity{intensity_(image_source, intersections)};
    const auto fast_pressure{fast_pressure_(image_source, intersections)};

    if (intensity.position != fast_pressure.position) {
        throw std::runtime_error("mismatched position");
    }

    if (intensity.distance != fast_pressure.distance) {
        throw std::runtime_error("mismatched times");
    }

    return fast_pressure;
}

}  // namespace image_source
}  // namespace raytracer
