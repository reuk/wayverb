#include "raytracer/image_source/finder.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/iterative_builder.h"

#include "common/output_iterator_callback.h"

#include <experimental/optional>
#include <numeric>

namespace raytracer {
namespace image_source {

void finder::push(const aligned::vector<aligned::vector<path_element>>& paths) {
    for (const auto& path: paths) {
        tree_.push(path);
    }
}

void finder::postprocess(const glm::vec3& source,
                         const glm::vec3& receiver,
                         const voxelised_scene_data& voxelised,
                         float speed_of_sound,
                         const postprocessor& callback) const {
    tree_.find_valid_paths(source, receiver, voxelised, callback);
}

}  // namespace image_source
}  // namespace raytracer
