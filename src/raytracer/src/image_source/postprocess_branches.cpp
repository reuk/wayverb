#include "raytracer/image_source/postprocess_branches.h"
#include "raytracer/image_source/fast_pressure_calculator.h"

namespace raytracer {
namespace image_source {

util::aligned::vector<impulse<core::simulation_bands>> postprocess_branches(
        const multitree<path_element>& tree,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        bool flip_phase) {
    auto callback =
            core::make_callback_accumulator(make_fast_pressure_calculator(
                    begin(voxelised.get_scene_data().get_surfaces()),
                    end(voxelised.get_scene_data().get_surfaces()),
                    receiver,
                    flip_phase));
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
