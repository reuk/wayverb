#pragma once

#include "raytracer/image_source/tree.h"
#include "raytracer/iterative_builder.h"

namespace wayverb {
namespace raytracer {
namespace image_source {

class reflection_path_builder final {
public:
    reflection_path_builder(size_t rays)
            : reflection_path_builder_{rays} {}

    template <typename It>
    void push(It b, It e) {
        reflection_path_builder_.push(b, e, [](const reflection& i) {
            return i.keep_going
                           ? std::experimental::make_optional(path_element{
                                     i.triangle,
                                     static_cast<bool>(i.receiver_visible)})
                           : std::experimental::nullopt;
        });
    }

    const auto& get_data() const { return reflection_path_builder_.get_data(); }
    auto& get_data() { return reflection_path_builder_.get_data(); }

private:
    iterative_builder<path_element> reflection_path_builder_;
};

}  // namespace image_source
}  // namespace raytracer
}  // namespace wayverb
