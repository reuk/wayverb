#include "raytracer/reflector.h"

#include "common/azimuth_elevation.h"
#include "common/conversions.h"
#include "common/spatial_division/scene_buffers.h"

#include <random>

namespace {

aligned::vector<cl_float> get_direction_rng(size_t num) {
    aligned::vector<cl_float> ret;
    ret.reserve(2 * num);
    std::default_random_engine engine{std::random_device()()};

    for (auto i = 0u; i != num; ++i) {
        const direction_rng rng(engine);
        ret.emplace_back(rng.get_z());
        ret.emplace_back(rng.get_theta());
    }

    return ret;
}

}  // namespace

namespace raytracer {

aligned::vector<geo::ray> get_random_rays(const glm::vec3& source, size_t num) {
    const auto directions{get_random_directions(num)};
    return get_rays_from_directions(
            directions.begin(), directions.end(), source);
}

//----------------------------------------------------------------------------//

reflector::reflector(const compute_context& cc,
                     const glm::vec3& receiver,
                     const aligned::vector<geo::ray>& rays)
        : cc(cc)
        , queue(cc.context, cc.device)
        , kernel(program{cc}.get_kernel())
        , receiver(to_cl_float3(receiver))
        , rays(rays.size())
        , ray_buffer(load_to_buffer(
                  cc.context,
                  map_to_vector(rays,
                                [&](const auto& i) { return convert(i); }),
                  false))
        , reflection_buffer(load_to_buffer(
                  cc.context,
                  aligned::vector<reflection>(rays.size(),
                                              reflection{cl_float3{},
                                                         cl_float3{},
                                                         ~cl_uint{0},
                                                         cl_char{true},
                                                         cl_char{}}),
                  false))
        , rng_buffer(cc.context,
                     CL_MEM_READ_WRITE,
                     rays.size() * 2 * sizeof(cl_float)) {}

aligned::vector<reflection> reflector::run_step(const scene_buffers& buffers) {
    //  get some new rng and copy it to device memory
    auto rng = get_direction_rng(rays);
    cl::copy(queue, std::begin(rng), std::end(rng), rng_buffer);

    //  get the kernel and run it
    kernel(cl::EnqueueArgs(queue, cl::NDRange(rays)),
           ray_buffer,
           receiver,
           buffers.get_voxel_index_buffer(),
           buffers.get_global_aabb(),
           buffers.get_side(),
           buffers.get_triangles_buffer(),
           buffers.get_vertices_buffer(),
           buffers.get_surfaces_buffer(),
           rng_buffer,
           reflection_buffer);

    return read_from_buffer<reflection>(queue, reflection_buffer);
}

aligned::vector<ray> reflector::get_rays() const {
    return read_from_buffer<ray>(queue, ray_buffer);
}

aligned::vector<reflection> reflector::get_reflections() const {
    return read_from_buffer<reflection>(queue, reflection_buffer);
}

aligned::vector<cl_float> reflector::get_rng() const {
    return read_from_buffer<cl_float>(queue, rng_buffer);
}

}  // namespace raytracer
