#include "raytracer/image_source/finder.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/tree.h"
#include "raytracer/iterative_builder.h"

#include "common/output_iterator_callback.h"

#include <experimental/optional>
#include <numeric>

namespace raytracer {
namespace image_source {

class finder::impl final {
public:
    impl(size_t rays, size_t depth)
            : reflection_path_builder_(rays, depth) {}

    void push(const aligned::vector<reflection>& x) {
        reflection_path_builder_.push(x, [](const reflection& i) {
            return i.keep_going
                           ? std::experimental::make_optional(path_element{
                                     i.triangle,
                                     static_cast<bool>(i.receiver_visible)})
                           : std::experimental::nullopt;
        });
    }

    void postprocess(const glm::vec3& source,
                     const glm::vec3& receiver,
                     const voxelised_scene_data& voxelised,
                     float speed_of_sound,
                     const postprocessor& callback) const {
        tree{reflection_path_builder_.get_data()}.find_valid_paths(
                source, receiver, voxelised, callback);
    }

private:
    iterative_builder<path_element> reflection_path_builder_;
};

//----------------------------------------------------------------------------//

finder::finder(size_t rays, size_t depth)
        : pimpl_(std::make_unique<impl>(rays, depth)) {}

finder::~finder() noexcept = default;

void finder::push(const aligned::vector<reflection>& reflections) {
    pimpl_->push(reflections);
}

void finder::postprocess(const glm::vec3& source,
                         const glm::vec3& receiver,
                         const voxelised_scene_data& voxelised,
                         float speed_of_sound,
                         const postprocessor& callback) const {
    return pimpl_->postprocess(
            source, receiver, voxelised, speed_of_sound, callback);
}

}  // namespace image_source
}  // namespace raytracer
