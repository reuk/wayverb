#include "raytracer/reflector.h"
#include "raytracer/random_directions.h"
#include "raytracer/scene_buffers.h"

#include "common/conversions.h"
#include "common/scene_data.h"

#include <random>

namespace {

aligned::vector<cl_float> get_direction_rng(size_t num) {
    aligned::vector<cl_float> ret;
    ret.reserve(2 * num);
    std::default_random_engine engine{std::random_device()()};

    for (auto i = 0u; i != num; ++i) {
        const raytracer::direction_rng rng(engine);
        ret.push_back(rng.get_z());
        ret.push_back(rng.get_theta());
    }

    return ret;
}

aligned::vector<Ray> get_random_rays(size_t num, const glm::vec3& source) {
    aligned::vector<Ray> ret;
    ret.reserve(num);
    std::default_random_engine engine{std::random_device()()};

    auto src = to_cl_float3(source);

    for (auto i = 0u; i != num; ++i) {
        const raytracer::direction_rng rng(engine);
        ret.push_back(Ray{
                src, to_cl_float3(sphere_point(rng.get_z(), rng.get_theta()))});
    }
    return ret;
}

}  // namespace

//----------------------------------------------------------------------------//

namespace raytracer {

class reflector::invocation {
public:
    invocation(const cl::Context& context, size_t rays, const glm::vec3& source)
            : invocation(context, get_random_rays(rays, source)) {}

    cl::Buffer& get_ray_buffer() { return ray_buffer; }
    const cl::Buffer& get_ray_buffer() const { return ray_buffer; }

private:
    invocation(const cl::Context& context, aligned::vector<Ray> rays)
            : ray_buffer(context, std::begin(rays), std::end(rays), false) {}

    cl::Buffer ray_buffer;
};

//----------------------------------------------------------------------------//

reflector::reflector(const cl::Context& context,
                     const cl::Device& device,
                     size_t rays)
        : context(context)
        , device(device)
        , rays(rays)
        , rng_buffer(context, CL_MEM_READ_WRITE, rays * 2 * sizeof(cl_float))
        , reflection_buffer(
                  context, CL_MEM_READ_WRITE, rays * sizeof(Reflection)) {}

reflector::reflector(reflector&&) = default;
reflector& reflector::operator=(reflector&&) = default;
reflector::~reflector() noexcept             = default;

void reflector::init(const glm::vec3& source) {
    //  set the ray directions randomly
    inv = std::make_unique<invocation>(context, rays, source);
}

aligned::vector<Reflection> reflector::run_step(
        scene_buffers& buffers) {
    //  make sure there's a valid ray buffer somewhere
    if (!inv) {
        throw std::runtime_error(
                "must call 'init' before 'run_step' on reflector");
    }

    //  get some new rng and copy it to device memory
    auto rng = get_direction_rng(rays);
    cl::copy(buffers.get_queue(), std::begin(rng), std::end(rng), rng_buffer);

    //  get the kernel and run it
    auto kernel = raytracer_program(context, device).get_reflections_kernel();
    kernel(cl::EnqueueArgs(buffers.get_queue(), cl::NDRange(rays)),
           inv->get_ray_buffer(),
           buffers.get_voxel_index_buffer(),
           buffers.get_global_aabb(),
           buffers.get_side(),
           buffers.get_triangles_buffer(),
           buffers.get_vertices_buffer(),
           buffers.get_surfaces_buffer(),
           rng_buffer,
           reflection_buffer);

    aligned::vector<Reflection> ret(rays);
    cl::copy(buffers.get_queue(),
             reflection_buffer,
             std::begin(ret),
             std::end(ret));
    return ret;
}

}  // namespace raytracer
