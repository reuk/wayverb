#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/image_source/postprocessors.h"

#include "common/aligned/vector.h"

#include "glm/fwd.hpp"

class voxelised_scene_data;

namespace raytracer {
namespace image_source {

class finder final {
public:
    finder(size_t rays, size_t depth);
    ~finder() noexcept;

    void push(const aligned::vector<reflection>&);

    void postprocess(const glm::vec3& source,
                     const glm::vec3& receiver,
                     const voxelised_scene_data& voxelised,
                     float speed_of_sound,
                     const postprocessor& callback) const;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace image_source
}  // namespace raytracer
