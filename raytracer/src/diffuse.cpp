#include "raytracer/diffuse.h"

namespace raytracer {

//  each reflection contains
//      position
//      specular direction
//      the intersected triangle
//      whether or not to keep going
//      whether or not it is visible from the receiver
//
//  the diffuse finder needs to keep track of
//      specular energy of each ray
//      distance travelled by each ray

diffuse_finder::diffuse_finder(const cl::Context& context,
                               const cl::Device& device,
                               const glm::vec3& receiver,
                               const VolumeType& air_coefficient,
                               size_t rays,
                               size_t depth)
        : context(context)
        , device(device)
        , kernel(raytracer_program(context, device).get_diffuse_kernel())
        , receiver(to_cl_float3(receiver))
        , air_coefficient(air_coefficient)
        , rays(rays)
        , reflections_buffer(
                  context, CL_MEM_READ_WRITE, sizeof(Reflection) * rays)
        , diffuse_path_buffer(load_to_buffer(
                  context,
                  aligned::vector<DiffusePathInfo>(
                          rays,
                          DiffusePathInfo{VolumeType{{1, 1, 1, 1, 1, 1, 1, 1}},
                                          to_cl_float3(receiver),
                                          0}),
                  false))
        , impulse_buffer(context, CL_MEM_READ_WRITE, sizeof(Impulse) * rays)
        , impulse_builder(rays, depth) {}

void diffuse_finder::push(const aligned::vector<Reflection>& reflections,
                          scene_buffers& buffers) {
    //  copy the current batch of reflections to the device
    cl::copy(buffers.get_queue(),
             reflections.begin(),
             reflections.end(),
             reflections_buffer);

    //  get the kernel and run it
    kernel(cl::EnqueueArgs(buffers.get_queue(), cl::NDRange(rays)),
           reflections_buffer,
           receiver,
           air_coefficient,
           buffers.get_triangles_buffer(),
           buffers.get_vertices_buffer(),
           buffers.get_surfaces_buffer(),
           diffuse_path_buffer,
           impulse_buffer);

    //  copy impulses out
    aligned::vector<Impulse> ret(rays);
    cl::copy(buffers.get_queue(), impulse_buffer, ret.begin(), ret.end());
    impulse_builder.push(std::move(ret));
}

const aligned::vector<aligned::vector<Impulse>>& diffuse_finder::get_results()
        const {
    return impulse_builder.get_data();
}

aligned::vector<aligned::vector<Impulse>>& diffuse_finder::get_results() {
    return impulse_builder.get_data();
}

}  // namespace raytracer
