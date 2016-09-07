#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/tree.h"

#include "common/aligned/vector.h"

class voxelised_scene_data;

namespace raytracer {
namespace image_source {

/// Every time we get a reflection bundle, add it to the store of paths.
/// If the paths fill up, add them to the tree and remove them.

class finder final {
public:
    void push(const aligned::vector<aligned::vector<path_element>>&);

    void postprocess(const glm::vec3& source,
                     const glm::vec3& receiver,
                     const voxelised_scene_data& voxelised,
                     float speed_of_sound,
                     const postprocessor& callback) const;

private:
    tree tree_;
};

template <typename Func>
auto postprocess(const finder& finder,
                 const glm::vec3& source,
                 const glm::vec3& receiver,
                 const voxelised_scene_data& voxelised,
                 float speed_of_sound,
                 const Func& callback) {
    using value_type = decltype(
            callback(std::declval<glm::vec3>(),
                     std::declval<aligned::vector<reflection_metadata>>()));
    aligned::vector<value_type> ret{};
    finder.postprocess(source,
                       receiver,
                       voxelised,
                       speed_of_sound,
                       [&callback, &ret](const auto& a, const auto& b) {
                           ret.push_back(callback(a, b));
                       });
    return ret;
}

}  // namespace image_source
}  // namespace raytracer
