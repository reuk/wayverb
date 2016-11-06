#include "raytracer/reflector.h"

#include "core/azimuth_elevation.h"
#include "core/conversions.h"
#include "core/spatial_division/scene_buffers.h"

#include <random>

namespace wayverb {
namespace raytracer {
namespace {

util::aligned::vector<cl_float> get_direction_rng(size_t num) {
    util::aligned::vector<cl_float> ret;
    ret.reserve(2 * num);
    std::default_random_engine engine{std::random_device()()};

    for (auto i = 0ul; i != num; ++i) {
        const core::direction_rng rng(engine);
        ret.emplace_back(rng.get_z());
        ret.emplace_back(rng.get_theta());
    }

    return ret;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

util::aligned::vector<reflection> reflector::run_step(
        const core::scene_buffers& buffers) {
    //  get some new rng and copy it to device memory
    const auto rng{get_direction_rng(rays_)};
    cl::copy(queue_, std::begin(rng), std::end(rng), rng_buffer_);

    //  get the kernel and run it
    kernel_(cl::EnqueueArgs(queue_, cl::NDRange(rays_)),
            ray_buffer_,
            receiver_,
            buffers.get_voxel_index_buffer(),
            buffers.get_global_aabb(),
            buffers.get_side(),
            buffers.get_triangles_buffer(),
            buffers.get_vertices_buffer(),
            buffers.get_surfaces_buffer(),
            rng_buffer_,
            reflection_buffer_);

    return core::read_from_buffer<reflection>(queue_, reflection_buffer_);
}

util::aligned::vector<core::ray> reflector::get_rays() {
    return core::read_from_buffer<core::ray>(queue_, ray_buffer_);
}

util::aligned::vector<reflection> reflector::get_reflections() {
    return core::read_from_buffer<reflection>(queue_, reflection_buffer_);
}

util::aligned::vector<cl_float> reflector::get_rng() {
    return core::read_from_buffer<cl_float>(queue_, rng_buffer_);
}

}  // namespace raytracer
}  // namespace wayverb
