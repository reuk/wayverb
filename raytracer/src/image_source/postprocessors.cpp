#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/postprocessors.h"

#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/surfaces.h"

namespace raytracer {
namespace image_source {

intensity_calculator::intensity_calculator(
        const glm::vec3&,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float speed_of_sound,
        float)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , speed_of_sound_{speed_of_sound} {}

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
                return i * reflectance;
            })};
    return construct_impulse(
            surface_attenuation, image_source, receiver_, speed_of_sound_);
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
        float speed_of_sound,
        float acoustic_impedance)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , speed_of_sound_{speed_of_sound}
        , acoustic_impedance_{acoustic_impedance}
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
    const auto surface_attenuation{proc::accumulate(
            intersections, make_volume_type(1), [&](auto i, auto j) {
                const auto surface_index{voxelised_.get_scene_data()
                                                 .get_triangles()[j.index]
                                                 .surface};
                const auto impedance{surface_impedances_[surface_index]};
                const auto reflectance{
                        average_wall_impedance_to_pressure_reflectance(
                                impedance, j.angle)};
                return i * reflectance;
            })};

    //  siltanen2013 equation 2
    const auto distance{glm::distance(image_source, receiver_)};

    //  pressure = sqrt((strength * acoustic_impedance) / (4 * M_PI)) / distance
    //  'strength' is implicitly 1
    const float raw_pressure = std::sqrt(acoustic_impedance_ / (4 * M_PI));
    return impulse{surface_attenuation * raw_pressure,
                   to_cl_float3(image_source),
                   static_cast<cl_float>(distance / speed_of_sound_)};
}

//----------------------------------------------------------------------------//

#if 0
//  I'm calling this a 'nice-to-have' for the time-being.
dumb_slow_fft_pressure_calculator::dumb_slow_fft_pressure_calculator(
        const glm::vec3&,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float speed_of_sound)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , speed_of_sound_{speed_of_sound} {}

std::array<std::complex<float>,
           dumb_slow_fft_pressure_calculator::output_spectrum_size>
dumb_slow_fft_pressure_calculator::operator()(
        const glm::vec3& image_source,
        const aligned::vector<reflection_metadata>& intersections) const {
    //  TODO
    //  Work out how large the final spectrum needs to be.
    //      It should be large enough that when the irfft is taken, the signal
    //      has the correct reverb length.
    //  For each surface in the scene, produce a base reflection spectrum.
    throw std::runtime_error("not implemented");
}
#endif

}  // namespace image_source
}  // namespace raytracer
