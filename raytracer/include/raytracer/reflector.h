#pragma once

#include "raytracer/cl_structs.h"

#include "common/aligned/vector.h"
#include "common/cl_include.h"

namespace raytracer {

class scene_buffers;

class reflector final {
public:
    reflector(const cl::Context&, const cl::Device&, size_t rays);

    reflector(reflector&&);
    reflector& operator=(reflector&&);

    ~reflector() noexcept;

    /// call init once, then run_step until you have enough reflections
    void init(const glm::vec3& source);
    aligned::vector<Reflection> run_step(scene_buffers& buffers);

private:
    cl::Context context;
    cl::Device device;
    size_t rays;

    class invocation;
    std::unique_ptr<invocation> inv;

    cl::Buffer rng_buffer;
    cl::Buffer reflection_buffer;
};

}  // namespace raytracer
