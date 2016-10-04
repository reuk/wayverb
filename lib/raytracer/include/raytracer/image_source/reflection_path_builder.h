#pragma once

#include "raytracer/image_source/tree.h"
#include "raytracer/iterative_builder.h"

namespace raytracer {
namespace image_source {

class reflection_path_builder final {
public:
    reflection_path_builder(size_t rays);

    void push(const aligned::vector<reflection>& x);

    const aligned::vector<aligned::vector<path_element>> get_data() const;
private:
    iterative_builder<path_element> reflection_path_builder_;
};

}  // namespace image_source
}  // namespace raytracer
