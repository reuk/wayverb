#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/postprocessors.h"

#include "common/surfaces.h"

namespace raytracer {
namespace image_source {

intensity_calculator::intensity_calculator(
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float speed_of_sound)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , speed_of_sound_{speed_of_sound} {}

impulse intensity_calculator::operator()(
        const glm::vec3& image_source,
        const aligned::vector<reflection_metadata>& intersections) const {
    const auto surface_attenuation{proc::accumulate(
            intersections, make_volume_type(1), [&](auto i, auto j) {
                const auto triangle{
                        voxelised_.get_scene_data().get_triangles()[j.index]};
                const auto surface{voxelised_.get_scene_data()
                                           .get_materials()[triangle.surface]
                                           .surface};
                const auto reflectance{absorption_to_energy_reflectance(
                        surface.specular_absorption)};
                return i * reflectance;
            })};
    return construct_impulse(
            surface_attenuation, image_source, receiver_, speed_of_sound_);
}

//----------------------------------------------------------------------------//

phase_calculator_v1::phase_calculator_v1(const glm::vec3& receiver,
                                         const voxelised_scene_data& voxelised,
                                         float speed_of_sound)
        : receiver_{receiver}
        , voxelised_{voxelised}
        , speed_of_sound_{speed_of_sound} {}

std::array<std::complex<float>, 8> phase_calculator_v1::operator()(
        const glm::vec3& image_source,
        const aligned::vector<reflection_metadata>& intersections) const {
    //  TODO
    throw std::runtime_error("not implemented");
}

}  // namespace image_source
}  // namespace raytracer
