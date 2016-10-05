#include "raytracer/image_source/reflection_path_builder.h"

namespace raytracer {
namespace image_source {

reflection_path_builder::reflection_path_builder(size_t rays)
        : reflection_path_builder_(rays) {}

void reflection_path_builder::push(const aligned::vector<reflection>& x) {
    reflection_path_builder_.push(x, [](const reflection& i) {
        return i.keep_going ? std::experimental::make_optional(path_element{
                                      i.triangle,
                                      static_cast<bool>(i.receiver_visible)})
                            : std::experimental::nullopt;
    });
}

const aligned::vector<aligned::vector<path_element>>
reflection_path_builder::get_data() const {
    return reflection_path_builder_.get_data();
}

}  // namespace image_source
}  // namespace raytracer
