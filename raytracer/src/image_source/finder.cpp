#include "raytracer/image_source/finder.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/tree.h"

#include "common/aligned/set.h"
#include "common/conversions.h"
#include "common/geo/geometric.h"
#include "common/progress_bar.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <experimental/optional>
#include <numeric>

namespace raytracer {
namespace image_source {

finder::finder(size_t rays, size_t depth)
        : reflection_path_builder(rays, depth) {}

void finder::push(const aligned::vector<reflection>& reflections) {
    reflection_path_builder.push(reflections, [](const reflection& i) {
        return i.keep_going ? std::experimental::make_optional(item{
                                      i.triangle,
                                      static_cast<bool>(i.receiver_visible)})
                            : std::experimental::nullopt;
    });
}

aligned::vector<impulse> finder::get_results(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& voxelised,
        double speed_of_sound) {
    return compute_impulses(reflection_path_builder.get_data(),
                            source,
                            receiver,
                            voxelised,
                            speed_of_sound);
}

}  // namespace image_source
}  // namespace raytracer
