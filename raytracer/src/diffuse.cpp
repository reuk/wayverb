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
                               size_t rays,
                               size_t depth)
        : context(context)
        , device(device)
        , receiver(to_cl_float3(receiver))
//        , rays(rays)
        , reflection_buffer(
                  context, CL_MEM_READ_WRITE, sizeof(Reflection) * rays)
        , diffuse_path_buffer(
                  context, CL_MEM_READ_WRITE, sizeof(DiffusePathInfo) * rays)
        , impulse_buffer(context, CL_MEM_READ_WRITE, sizeof(Impulse) * rays)
        , impulse_builder(rays, depth) {}

void diffuse_finder::push(const aligned::vector<Reflection>& reflections,
                          scene_buffers& scene_buffers) {
    //  TODO
    impulse_builder.push(aligned::vector<Impulse>(reflections.size()));
}

const aligned::vector<aligned::vector<Impulse>>& diffuse_finder::get_results()
        const {
    return impulse_builder.get_data();
}

aligned::vector<aligned::vector<Impulse>>& diffuse_finder::get_results() {
    return impulse_builder.get_data();
}

}  // namespace raytracer
