#include "raytracer/construct_impulse.h"
#include "raytracer/image_source.h"
#include "raytracer/image_source_impl.h"

#include "common/aligned/set.h"
#include "common/conversions.h"
#include "common/geometric.h"
#include "common/progress_bar.h"
#include "common/voxel_collection.h"

#include <experimental/optional>
#include <numeric>

namespace raytracer {

image_source_finder::image_source_finder(size_t rays, size_t depth)
        : reflection_path_builder(rays, depth) {}

void image_source_finder::push(const aligned::vector<Reflection>& reflections) {
    reflection_path_builder.push(reflections, [](const Reflection& i) {
        return i.keep_going ? std::experimental::make_optional(item{
                                      i.triangle,
                                      static_cast<bool>(i.receiver_visible)})
                            : std::experimental::nullopt;
    });
}

aligned::vector<Impulse> image_source_finder::get_results(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const CopyableSceneData& scene_data,
        const VoxelCollection& vox) {
    auto unique_paths =
            compute_unique_paths(std::move(reflection_path_builder.get_data()));
    aligned::vector<Impulse> ret;

    const VoxelCollection::TriangleTraversalCallback callback(scene_data);
    for (const auto& i : unique_paths) {
        if (auto impulse = follow_ray_path(
                    i, source, receiver, scene_data, vox, callback)) {
            ret.push_back(*impulse);
        }
    }

    return ret;
}

}  // namespace raytracer
