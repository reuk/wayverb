#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/iterative_builder.h"

#include "common/aligned/vector.h"
#include "common/cl/include.h"

#include "glm/fwd.hpp"

class voxelised_scene_data;

namespace raytracer {
namespace image_source {

class finder final {
public:
    finder(size_t rays, size_t depth);

    void push(const aligned::vector<reflection>&);
    aligned::vector<impulse> get_results(const glm::vec3& source,
                                         const glm::vec3& receiver,
                                         const voxelised_scene_data& scene_data,
                                         double speed_of_sound);

    struct item final {
        cl_ulong index;
        bool visible;
    };

private:
    iterative_builder<item> reflection_path_builder;
};

}  // namespace image_source
}  // namespace raytracer
