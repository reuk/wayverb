#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/postprocessors.h"

namespace raytracer {
namespace image_source {

intensity_calculator::intensity_calculator(
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        float speed_of_sound)
        : receiver_(receiver)
        , voxelised_(voxelised)
        , speed_of_sound_(speed_of_sound) {}

impulse intensity_calculator::operator()(
        const glm::vec3& image_source,
        const aligned::vector<image_source_tree::intersection>& intersections)
        const {
    const auto surface_attenuation{proc::accumulate(
            intersections, make_volume_type(1), [&](auto i, auto j) {
                const auto triangle{
                        voxelised_.get_scene_data().get_triangles()[j.index]};
                const auto surface{voxelised_.get_scene_data()
                                           .get_surfaces()[triangle.surface]};
                return i * surface.specular;
            })};
    return construct_impulse(
            surface_attenuation, image_source, receiver_, speed_of_sound_);
}

//----------------------------------------------------------------------------//

}  // namespace image_source
}  // namespace raytracer
