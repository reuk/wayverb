#pragma once

#include "program.h"

#include "raytracer/cl/structs.h"
#include "raytracer/iterative_builder.h"

#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/spatial_division/scene_buffers.h"

#include "utilities/aligned/vector.h"

namespace raytracer {
namespace diffuse {

class finder final {
public:
    finder(const compute_context& cc,
           const glm::vec3& source,
           const glm::vec3& receiver,
           size_t rays,
           size_t depth);

    void push(const aligned::vector<reflection>& reflections,
              const scene_buffers& scene_buffers);
    const aligned::vector<aligned::vector<impulse<8>>>& get_results() const;
    aligned::vector<aligned::vector<impulse<8>>>& get_results();

private:
    using kernel_t = decltype(std::declval<program>().get_kernel());

    compute_context cc;
    cl::CommandQueue queue;
    kernel_t kernel;
    cl_float3 receiver;
    size_t rays;

    cl::Buffer reflections_buffer;
    cl::Buffer diffuse_path_buffer;
    cl::Buffer impulse_buffer;

    iterative_builder<impulse<8>> impulse_builder;
};

}  // namespace diffuse
}  // namespace raytracer
