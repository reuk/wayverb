#include "raytracer/image_source/postprocess_branches.h"
#include "raytracer/image_source/fast_pressure_calculator.h"

namespace raytracer {
namespace image_source {

aligned::vector<impulse<8>> postprocess_branches(
        const multitree<path_element>& tree,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        bool flip_phase) {
    callback_accumulator<fast_pressure_calculator> callback{
            receiver, voxelised.get_scene_data().get_surfaces(), flip_phase};
    find_valid_paths(
            tree,
            source,
            receiver,
            voxelised,
            [&](auto img, auto begin, auto end) { callback(img, begin, end); });
    return callback.get_output();
}

}  // namespace image_source
}  // namespace raytracer
